#include "MCP4632.h"
#include <Wire.h>
#include <stdio.h> 

// =================================================================
// MCP4632 BASE DRIVER IMPLEMENTATION
// =================================================================

MCP4632::MCP4632(byte deviceAddr) {
    _deviceAddr = deviceAddr;
}

void MCP4632::setStep(byte wiperChannel, byte stepValue) {

    if (stepValue > MAX_STEPS) {
        fprintf(stderr, "[ERROR] [%s:%d in %s()] Invalid step value: %d! Max allowed is %d. Clipping and stabilizing value to %d.\n", 
                __FILE__, __LINE__, __func__, stepValue, MAX_STEPS, MAX_STEPS);
        stepValue = MAX_STEPS;
    }

    // Wiper channel mapping (Only dual channels 0 and 1 physically exist)
    if (wiperChannel != 0 && wiperChannel != 1) {
        fprintf(stderr, "[ERROR] [%s:%d in %s()] Invalid wiper channel: %d! Only 0 or 1 are supported.\n", 
                __FILE__, __LINE__, __func__, wiperChannel);
        return; 
    }

    byte wiperCmd = (wiperChannel == 0) ? CMD_WIPER0 : CMD_WIPER1;

    Wire.beginTransmission(_deviceAddr);
    Wire.write(wiperCmd);
    Wire.write(stepValue);
    byte error = Wire.endTransmission();

    // Check I2C acknowledge status to verify physical hardware connectivity
    if (error != 0) {
        fprintf(stderr, "[ERROR] [%s:%d in %s()] I2C Write Failed! Chip Address: 0x%02X, I2C Error Code: %d\n", 
                __FILE__, __LINE__, __func__, _deviceAddr, error);
    }
}

void MCP4632::setPreset(byte wiperChannel, ResistanceStep step) {
    byte stepVal = (byte)step;
    setStep(wiperChannel, stepVal);
}

static const CalibrationPoint calTable[] = DGTLPOT_CALIBRATION_TABLE;
static const int calTableSize = sizeof(calTable) / sizeof(CalibrationPoint);

// Linear Interpolation (Ara Değerleme) Yardımcı Fonksiyonu
static float interpolateKohm(float inputKohm) {
    // 1. Alt sınır kontrolü
    if (inputKohm <= calTable[0].targetKohm) {
        return calTable[0].actualKohm;
    }
    // 2. Üst sınır kontrolü
    if (inputKohm >= calTable[calTableSize - 1].targetKohm) {
        return calTable[calTableSize - 1].actualKohm;
    }

    // 3. Değerin hangi iki tablo noktası arasında olduğunu bulalım
    for (int i = 0; i < calTableSize - 1; i++) {
        if (inputKohm >= calTable[i].targetKohm && inputKohm <= calTable[i + 1].targetKohm) {
            float x0 = calTable[i].targetKohm;
            float y0 = calTable[i].actualKohm;
            float x1 = calTable[i + 1].targetKohm;
            float y1 = calTable[i + 1].actualKohm;

            // Interpolation Formülü: y = y0 + (x - x0) * (y1 - y0) / (x1 - x0)
            return y0 + (inputKohm - x0) * (y1 - y0) / (x1 - x0);
        }
    }

    return inputKohm; // Güvenlik durumunda varsayılan dön
}

void MCP4632::setResistance(byte wiperChannel, float kohm) {
    if (kohm < 0.0f) {
        fprintf(stderr, "[ERROR] [%s:%d in %s()] Target resistance %f kOhm is below 0.0! Clipping to 0.0 kOhm.\n", 
                __FILE__, __LINE__, __func__, kohm);
        kohm = 0.0f;
    }

    // 1. Interpolasyon Tablosundan Haritalandırılmış Gerçek kOhm Değerini Al
    float mappedKohm = interpolateKohm(kohm);

    // 2. Toplam Direnç Tavanına Sınırla (Clipping)
    if (mappedKohm > TOTAL_RESISTANCE) {
        mappedKohm = TOTAL_RESISTANCE;
    }

    // 3. Adım Hesabını Mapped Değer Üzerinden 128 Adıma Dönüştür
    float calculatedStep = (mappedKohm / TOTAL_RESISTANCE) * MAX_STEPS;
    byte targetStep = (byte)round(calculatedStep);

    setStep(wiperChannel, targetStep);
}

// YENİ/DEĞİŞEN KISIM: Sabit adresler yerine doğrudan header'daki makroları içeri üflüyoruz.
// Eğer kullanıcı main üstünde yeni adres tanımladıysa otomatik olarak onlar buraya basılacak.
DGTLPOT::DGTLPOT() 
    : _potA(DGTLPOT_ADDR_A), _potB(DGTLPOT_ADDR_B), _potC(DGTLPOT_ADDR_C) {
    // Kurulum anında tüm kanalların fade struct'larını sıfırla
    for (int i = 0; i < MAX_POT_CHANNELS; i++) {
        _channelFades[i].active = false;
    }
}

// Routes named schematic outputs directly to their physical chip and wiper banyos (Wiper çaprazlığı düzeltildi)
void DGTLPOT::setChanelPreset(DGTLPOT_Channel channel, ResistanceStep preset) {
    switch (channel) {
        case DGTLPOT_3: _potA.setPreset(1, preset); break; // Eskiden 0 idi -> Şimdi 1 (Wiper 1)
        case DGTLPOT_4: _potA.setPreset(0, preset); break; // Eskiden 1 idi -> Şimdi 0 (Wiper 0)
        
        case DGTLPOT_5: _potB.setPreset(1, preset); break; // Eskiden 0 idi -> Şimdi 1 (Wiper 1)
        case DGTLPOT_6: _potB.setPreset(0, preset); break; // Eskiden 1 idi -> Şimdi 0 (Wiper 0)
        
        case DGTLPOT_7: _potC.setPreset(1, preset); break; // Eskiden 0 idi -> Şimdi 1 (Wiper 1)
        case DGTLPOT_8: _potC.setPreset(0, preset); break; // Eskiden 1 idi -> Şimdi 0 (Wiper 0)
        default:
            fprintf(stderr, "[ERROR] [%s:%d in %s()] Unknown DGTLPOT Channel enum: %d\n", 
                    __FILE__, __LINE__, __func__, (int)channel);
            break;
    }
}

// Calculates dynamic steps for named schematic outputs using float kOhm values (Wiper çaprazlığı düzeltildi)
void DGTLPOT::setChanelResistance(DGTLPOT_Channel channel, float kohm) {
    switch (channel) {
        case DGTLPOT_3: _potA.setResistance(1, kohm); break; // Eskiden 0 idi -> Şimdi 1
        case DGTLPOT_4: _potA.setResistance(0, kohm); break; // Eskiden 1 idi -> Şimdi 0
        
        case DGTLPOT_5: _potB.setResistance(1, kohm); break; // Eskiden 0 idi -> Şimdi 1
        case DGTLPOT_6: _potB.setResistance(0, kohm); break; // Eskiden 1 idi -> Şimdi 0
        
        case DGTLPOT_7: _potC.setResistance(1, kohm); break; // Eskiden 0 idi -> Şimdi 1
        case DGTLPOT_8: _potC.setResistance(0, kohm); break; // Eskiden 1 idi -> Şimdi 0
        default:
            fprintf(stderr, "[ERROR] [%s:%d in %s()] Unknown DGTLPOT Channel enum: %d\n", 
                    __FILE__, __LINE__, __func__, (int)channel);
            break;
    }
}

// Bir kanal için fade tetikleyen yeni üye fonksiyon
void DGTLPOT::startChanelFade(DGTLPOT_Channel channel, float startKohm, float endKohm, float stepSizeKohm, unsigned long delayMs) {
    int idx = (int)channel;
    if (idx < 0 || idx >= MAX_POT_CHANNELS) return;

    _channelFades[idx].active = true;
    _channelFades[idx].currentKohm = startKohm;
    _channelFades[idx].endKohm = endKohm;
    _channelFades[idx].stepSizeKohm = stepSizeKohm;
    _channelFades[idx].delayMs = delayMs;
    _channelFades[idx].direction = (endKohm > startKohm) ? 1.0f : -1.0f;
    _channelFades[idx].lastUpdateTime = millis();

    // İlk adım direncini hemen gönderelim
    this->setChanelResistance(channel, startKohm);
}

// loop() fonksiyonunda çağrılıp arka planda paralel geçişleri yönetecek motor mekanizma
void DGTLPOT::updateFades() {
    unsigned long currentMillis = millis();

    for (int i = 0; i < MAX_POT_CHANNELS; i++) {
        if (!_channelFades[i].active) continue;

        if (currentMillis - _channelFades[i].lastUpdateTime >= _channelFades[i].delayMs) {
            _channelFades[i].lastUpdateTime = currentMillis;

            // Yön bilgisine göre hedefe ulaşıldı mı kontrolü
            bool reached = (_channelFades[i].direction > 0 && _channelFades[i].currentKohm >= _channelFades[i].endKohm) ||
                           (_channelFades[i].direction < 0 && _channelFades[i].currentKohm <= _channelFades[i].endKohm);

            // Cast işlemiyle döngü indisini tekrar Enum tipine güvenli biçimde çeviriyoruz
            DGTLPOT_Channel currentChan = (DGTLPOT_Channel)i;

            if (reached) {
                this->setChanelResistance(currentChan, _channelFades[i].endKohm);
                _channelFades[i].active = false; // İşlem başarıyla bitti, bayrağı indir
            } else {
                this->setChanelResistance(currentChan, _channelFades[i].currentKohm);
                _channelFades[i].currentKohm += (_channelFades[i].stepSizeKohm * _channelFades[i].direction);
            }
        }
    }
}

void DGTLPOT::setChanelTemperatureNTC10K(DGTLPOT_Channel channel, float celcius) {
    float r0 = 10000.0f;      // 25 degrees nominal resistance (10k Ohm NTC)
    float t0Kelvin = 298.15f; // 25°C in Kelvin
    float beta = 3950.0f;     // Standard B3950 coefficient
    
    // Convert Celsius to Kelvin
    float tKelvin = celcius + 273.15f;

    // NTC Beta Equation: R = R0 * e^(Beta * (1/T - 1/T0))
    float exponent = beta * ((1.0f / tKelvin) - (1.0f / t0Kelvin));
    float rOhm = r0 * exp(exponent);

    // Convert calculated Ohm to kOhm for MCP4632 (Clamps automatically inside setChanelResistance)
    float kohm = rOhm / 1000.0f; 

    // Route directly to the channel resistance handler
    this->setChanelResistance(channel, kohm);
}

void DGTLPOT::runHardwareTest(float ch3_kohm, float ch4_kohm, float ch5_kohm, float ch6_kohm, float ch7_kohm, float ch8_kohm) {
    Serial.println(F("\n====== DGTLPOT HARDWARE TEST ======"));
    
    // 1. I2C Cihaz Varlık Kontrolü
    byte addresses[3] = {DGTLPOT_ADDR_A, DGTLPOT_ADDR_B, DGTLPOT_ADDR_C};
    char chipNames[3] = {'A', 'B', 'C'};
    
    for (int i = 0; i < 3; i++) {
        Wire.beginTransmission(addresses[i]);
        byte error = Wire.endTransmission();
        
        Serial.print(F("POT_"));
        Serial.print(chipNames[i]);
        Serial.print(F(" (0x"));
        Serial.print(addresses[i], HEX);
        Serial.print(F("): "));
        
        if (error == 0) {
            Serial.println(F("[OK] Connected."));
        } else {
            Serial.print(F("[ERROR] Missing! Code: "));
            Serial.println(error);
        }
    }
    
    Serial.println(F("-----------------------------------"));
    Serial.println(F("Applying static values to channels..."));
    
    // 2. Parametreden gelen sabit değerleri kanallara yazma
    this->setChanelResistance(DGTLPOT_3, ch3_kohm);
    this->setChanelResistance(DGTLPOT_4, ch4_kohm);
    this->setChanelResistance(DGTLPOT_5, ch5_kohm);
    this->setChanelResistance(DGTLPOT_6, ch6_kohm);
    this->setChanelResistance(DGTLPOT_7, ch7_kohm);
    this->setChanelResistance(DGTLPOT_8, ch8_kohm);
    
    // Log çıktıları
    Serial.print(F("Ch3: ")); Serial.print(ch3_kohm); Serial.println(F(" kOhm"));
    Serial.print(F("Ch4: ")); Serial.print(ch4_kohm); Serial.println(F(" kOhm"));
    Serial.print(F("Ch5: ")); Serial.print(ch5_kohm); Serial.println(F(" kOhm"));
    Serial.print(F("Ch6: ")); Serial.print(ch6_kohm); Serial.println(F(" kOhm"));
    Serial.print(F("Ch7: ")); Serial.print(ch7_kohm); Serial.println(F(" kOhm"));
    Serial.print(F("Ch8: ")); Serial.print(ch8_kohm); Serial.println(F(" kOhm"));
    
    Serial.println(F("====== TEST READY FOR MEASUREMENT ======\n"));
}

void DGTLPOT::startChanelFadeByDuration(DGTLPOT_Channel channel, float startKohm, float endKohm, float stepSizeKohm, unsigned long totalDurationMs) {
    if (stepSizeKohm <= 0.0f) return;

    // 1. Toplam kaç adım atılacağını hesapla
    float totalDistance = fabs(endKohm - startKohm);
    float totalSteps = totalDistance / stepSizeKohm;

    if (totalSteps <= 0.0f) totalSteps = 1.0f;

    // 2. Adım başına düşen ms süresini bul
    unsigned long calculatedDelayMs = static_cast<unsigned long>(totalDurationMs / totalSteps);

    // Minimum 1ms sınırı koyalım ki sıfıra bölünme/kilitlenme olmasın
    if (calculatedDelayMs == 0) calculatedDelayMs = 1;

    // 3. Mevcut orijinal fade motorunu çalıştır
    startChanelFade(channel, startKohm, endKohm, stepSizeKohm, calculatedDelayMs);
}

void DGTLPOT::clearAll() {
    // Aktif devam eden tüm fade işlemlerini de sıfırla
    for(int i = 0; i < MAX_POT_CHANNELS; i++) {
        _channelFades[i].active = false;
    }
    
    setChanelPreset(DGTLPOT_3, R_0_KOHM);
    setChanelPreset(DGTLPOT_4, R_0_KOHM);
    setChanelPreset(DGTLPOT_5, R_0_KOHM);
    setChanelPreset(DGTLPOT_6, R_0_KOHM);
    setChanelPreset(DGTLPOT_7, R_0_KOHM);
    setChanelPreset(DGTLPOT_8, R_0_KOHM);
}