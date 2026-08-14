#include <Arduino.h>
#include <ModbusRTU.h>
#include <Wire.h>        
#include <Preferences.h>
#include "MCP4632.h"    
#include "PeripheralManager.h"
#include "TestEngine.h" // Yeni eklediğimiz TestEngine sınıfı

#define RXD2 16  
#define TXD2 17  
#define DE_PIN 4  

ModbusRTU mb;
DGTLPOT dgtlPot;
Preferences prefs;
TestEngine testEngine; // Test Motoru Nesnesi

uint16_t eskiPotDegerleri[8] = {999, 999, 999, 999, 999, 999, 999, 999};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Taytech Simulator HIL Kartı Başlatılıyor...");

  // 1. Periferikleri Başlat
  PeripheralManager::getInstance().begin();
  PeripheralManager::getInstance().beep(100);

  Wire.begin();
  Wire.setClock(100000);

  pinMode(DE_PIN, OUTPUT);
  digitalWrite(DE_PIN, LOW);

  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  mb.begin(&Serial2, DE_PIN);
  mb.slave(1);

  // Holding Register'ları tanımla (0 - 9 arası)
  for (int i = 0; i <= 18; i++) {
    mb.addHreg(i, 0);
  }

  dgtlPot.clearAll();

  // NVS'den Son Değerleri Yükle (Read-Only Modunda Güvenli Yükleme)
  prefs.begin("pot_data", true);
  Serial.println("\n[NVS] Son Kayıtlı Dirençler Yükleniyor...");

  for (int i = 2; i <= 7; i++) {
    char key[8];
    snprintf(key, sizeof(key), "reg_%d", i);

    uint16_t kayitliDeger = prefs.getUShort(key, 10); // Varsayılan 10 kOhm

    eskiPotDegerleri[i] = kayitliDeger;
    mb.Hreg(i, kayitliDeger);

    DGTLPOT_Channel hedefKanal = static_cast<DGTLPOT_Channel>(i - 2);
    dgtlPot.setChanelResistance(hedefKanal, static_cast<float>(kayitliDeger));

    Serial.printf(" -> P%d (Reg %d): %d kOhm\n", i - 1, i, kayitliDeger);
  }
  prefs.end();

  // Test Motorunu Başlat
  testEngine.begin(&mb, &dgtlPot);

  Serial.println("Modbus, Dijital Potlar ve TestEngine Aktif! Sistem Hazır.");
}

void loop() {
  mb.task();

  // 1. MANUEL MOD KONTROLÜ (Sadece Otomatik Test Çalışmıyorsa - REG_SYS_CMD == 0)
  if (mb.Hreg(REG_SYS_CMD) == 0) {
    for (int i = 2; i <= 7; i++) {
      uint16_t anlikDeger = mb.Hreg(i);

      if (anlikDeger != eskiPotDegerleri[i]) {
        DGTLPOT_Channel hedefKanal = static_cast<DGTLPOT_Channel>(i - 2);
        float kohmDegeri = static_cast<float>(anlikDeger);

        // Dijital potu sür
        dgtlPot.setChanelResistance(hedefKanal, kohmDegeri);

        eskiPotDegerleri[i] = anlikDeger;
        PeripheralManager::getInstance().beep(30);
        Serial.printf("[MANUAL] Pot %d -> %d kOhm\n", i - 1, anlikDeger);
      }
    }
  }

  // 2. OTOMATİK TEST MOTORUNU KOŞTUR
  testEngine.update();

  // 3. PERİFERİK VE FADE MOTORLARINI GÜNCELLE
  PeripheralManager::getInstance().update();
  dgtlPot.updateFades();

  delay(1);
}