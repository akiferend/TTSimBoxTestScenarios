#include <Arduino.h>
#include <ModbusRTU.h>
#include <Wire.h>        
#include <Preferences.h>
#include "MCP4632.h"    
#include "AD5248.h"     
#include "PeripheralManager.h"
#include "TestEngine.h" 
#include "NTC_Lookup.h"
#include "PT1000_Lookup.h"

#define RXD2   16  
#define TXD2   17  
#define DE_PIN 4   

// Donanım Sürücü ve Yardımcı Nesneler
ModbusRTU    mb;
DGTLPOT      dgtlPot;   // MCP4632 (Pot 3 - Pot 8)
AD5248       ad5248;    // AD5248  (Pot 1 - Pot 2)
Preferences  prefs;
TestEngine   testEngine;

// Pot Değişim Takibi (Reg 2 - Reg 9 arası / Pot 1 - Pot 8)
uint16_t eskiPotDegerleri[10] = {0}; 

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n==========================================");
  Serial.println(" Taytech Simulator HIL Kartı Başlatılıyor ");
  Serial.println("==========================================");

  // 1. Periferik ve I2C Başlatma
  PeripheralManager::getInstance().begin();
  PeripheralManager::getInstance().beep(100);

  Wire.begin();
  Wire.setClock(100000);

  // Dijital Pot Temizliği
  dgtlPot.clearAll();

  // 2. RS485 & Modbus RTU Başlatma
  pinMode(DE_PIN, OUTPUT);
  digitalWrite(DE_PIN, LOW);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  mb.begin(&Serial2, DE_PIN);
  mb.slave(1);

  // Holding Register'ları Tanımla (Reg 0 - 18)
  for (int i = 0; i <= 50; i++) {
    mb.addHreg(i, 0);
  }

  // 3. Test Motorunu Başlat (Tüm Potlar Güvenli Varsayılan Değerlere Çekilir)
  testEngine.begin(&mb, &dgtlPot, &ad5248);

  // 4. NVS'den Son Kayıtlı Direnç Değerlerini Yükle
  prefs.begin("pot_data", true);
  Serial.println("[NVS] Kayıtlı Direnç Değerleri Yükleniyor...");

  for (int reg = REG_POT_START; reg < REG_POT_START + 8; reg++) {
    char key[8];
    snprintf(key, sizeof(key), "reg_%d", reg);

    uint8_t potIndex = reg - REG_POT_START + 1; // Pot 1 .. Pot 8

    if (potIndex == 1 || potIndex == 2) {
      // Pot 1 ve 2 (AD5248 - Ohm Cinsinden)
      uint16_t kayitliDeger = prefs.getUShort(key, SAFE_AD5248_OHM);
      eskiPotDegerleri[reg] = kayitliDeger;
      mb.Hreg(reg, kayitliDeger);
      
      // DÜZELTME: uint8_t -> AD5248_Channel dönüşümü yapıldı
      AD5248_Channel ch = static_cast<AD5248_Channel>(potIndex - 1);
      ad5248.setChanelResistance(ch, static_cast<float>(kayitliDeger) / 1000.0f);
      Serial.printf(" -> Pot %d (Reg %d): %d Ohm (AD5248)\n", potIndex, reg, kayitliDeger);
    } 
    else {
      // Modbus'tan uint16_t okuyoruz ama sıcaklık eksi (-25 gibi) olabileceği için int16_t'ye cast ediyoruz
      int16_t kayitliSicaklik = (int16_t)prefs.getUShort(key, 25); // Varsayılan güvenli sıcaklık: 25 °C
      eskiPotDegerleri[reg] = (uint16_t)kayitliSicaklik; 
      mb.Hreg(reg, (uint16_t)kayitliSicaklik);

      // Sıcaklığı kOhm'a çevirip dijital pota uyguluyoruz
      float targetKohm = NTCLookup::temperatureToKohm((float)kayitliSicaklik);

      DGTLPOT_Channel ch = static_cast<DGTLPOT_Channel>(potIndex - 3);
      dgtlPot.setChanelResistance(ch, targetKohm);
      Serial.printf(" -> Pot %d (Reg %d): %d °C (%.2f kOhm) (MCP4632)\n", potIndex, reg, kayitliSicaklik, targetKohm);
    }
  }
  prefs.end();

  Serial.println("[SİSTEM] Modbus RTU, Pot Sürücüleri ve Test Engine Hazır!\n");
}

void loop() {
  mb.task(); // Modbus Haberleşme Görevini Koştur

  // 1. MANUEL MOD KONTROLÜ (Otomatik Test Çalışmıyorsa - REG_SYS_CMD == 0)
  if (mb.Hreg(REG_SYS_CMD) == 0) {
    for (int reg = REG_POT_START; reg < REG_POT_START + 8; reg++) {
      uint16_t anlikDeger = mb.Hreg(reg);

      // Register Değeri Değişti mi?
      if (anlikDeger != eskiPotDegerleri[reg]) {
        uint8_t potIndex = reg - REG_POT_START + 1; // Pot 1 .. 8

        if (potIndex == 1 || potIndex == 2) {
            // Pot 1 ve Pot 2 (AD5248 - Sıcaklık Kontrolü)
            int16_t anlikSicaklik = static_cast<int16_t>(anlikDeger);
            float targetOhm = PT1000Lookup::temperatureToOhm((float)anlikSicaklik);

            AD5248_Channel ch = static_cast<AD5248_Channel>(potIndex - 1);
            ad5248.setChanelResistance(ch, targetOhm / 1000.0f);
            Serial.printf("[MANUEL] Pot %d (AD5248 Ch%d) -> %d °C (Hedef Direnç: %.2f Ohm)\n", potIndex, static_cast<int>(ch), anlikSicaklik, targetOhm);
        } 
        else {
          // Pot 3 .. Pot 8 (MCP4632 - Sıcaklık Kontrolü)
          int16_t anlikSicaklik = static_cast<int16_t>(anlikDeger);
          float targetKohm = NTCLookup::temperatureToKohm((float)anlikSicaklik);

          DGTLPOT_Channel ch = static_cast<DGTLPOT_Channel>(potIndex - 3);
          dgtlPot.setChanelResistance(ch, targetKohm);
          Serial.printf("[MANUEL] Pot %d (MCP4632) -> %d °C (Hedef Direnç: %.2f kOhm)\n", potIndex, anlikSicaklik, targetKohm);
        }

        eskiPotDegerleri[reg] = anlikDeger;
        PeripheralManager::getInstance().beep(30);
      }
    }
  }

  // 2. OTOMATİK TEST MOTORUNU KOŞTUR
  testEngine.update();

  // 3. PERİFERİKLERİ VE FADE ENGINE'İ GÜNCELLE
  PeripheralManager::getInstance().update();
  dgtlPot.updateFades();

  delay(1);
}