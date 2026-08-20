#include "AD5248.h"
#include <Wire.h>
#include <stdio.h>

// Static üyenin tanımı (Linker hatasını çözen kısım)
constexpr CalibPoint AD5248::calibTable[6];

AD5248::AD5248(byte deviceAddr) {
    _deviceAddr = deviceAddr;
}

// Parçalı Lineer İnterpolasyon ile Donanım Sapmasını Sıfırlama
float AD5248::getCalibratedOhm(float targetOhm) {
    const int CALIB_COUNT = 6;

    // 1. En küçük değerin altındaysa
    if (targetOhm <= calibTable[0].targetOhm) {
        float offset = calibTable[0].measuredOhm - calibTable[0].targetOhm;
        return max(0.0f, targetOhm - offset);
    }

    // 2. En büyük değerin üstündeyse
    if (targetOhm >= calibTable[CALIB_COUNT - 1].targetOhm) {
        float offset = calibTable[CALIB_COUNT - 1].measuredOhm - calibTable[CALIB_COUNT - 1].targetOhm;
        return targetOhm - offset;
    }

    // 3. Tablodaki iki nokta arasında ise interpolasyon uygula
    for (int i = 0; i < CALIB_COUNT - 1; i++) {
        if (targetOhm >= calibTable[i].targetOhm && targetOhm <= calibTable[i + 1].targetOhm) {
            float t1 = calibTable[i].targetOhm;
            float m1 = calibTable[i].measuredOhm;
            float t2 = calibTable[i + 1].targetOhm;
            float m2 = calibTable[i + 1].measuredOhm;

            float measuredNeeded = m1 + (targetOhm - t1) * (m2 - m1) / (t2 - t1);
            float currentOffset = measuredNeeded - targetOhm;
            
            return max(0.0f, targetOhm - currentOffset);
        }
    }
    return targetOhm;
}

void AD5248::setStep(byte wiperChannel, byte stepValue) {
    if (stepValue > MAX_STEPS) {
        stepValue = MAX_STEPS;
    }

    if (wiperChannel != 0 && wiperChannel != 1) {
        return;
    }

    byte instruction = (wiperChannel == 0) ? CMD_RDAC1 : CMD_RDAC2;

    Wire.beginTransmission(_deviceAddr);
    Wire.write(instruction);
    Wire.write(stepValue);
    Wire.endTransmission();
}

void AD5248::setResistance(byte wiperChannel, float kohm) {
    if (kohm < 0.0f) kohm = 0.0f;
    if (kohm > TOTAL_RESISTANCE) kohm = TOTAL_RESISTANCE;

    // kOhm gelen değeri Ohm'a çevirip interpolasyondan geçiriyoruz
    float targetOhm = kohm * 1000.0f;
    float calibratedOhm = getCalibratedOhm(targetOhm);
    
    // Kalibre edilmiş Ohm değerini kOhm cinsinden step'e dönüştürüyoruz
    float calibratedKohm = calibratedOhm / 1000.0f;
    float calculatedStep = (calibratedKohm / TOTAL_RESISTANCE) * static_cast<float>(MAX_STEPS);
    byte targetStep = static_cast<byte>(round(calculatedStep));

    setStep(wiperChannel, targetStep);
}

void AD5248::clearAll() {
    setStep(0, 0);
    setStep(1, 0);
}

void AD5248::setMaxResistanceAll() {
    setResistance(0, TOTAL_RESISTANCE); // Kanal 0 -> 2.5 kOhm
    setResistance(1, TOTAL_RESISTANCE); // Kanal 1 -> 2.5 kOhm
}