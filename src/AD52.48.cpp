#include "AD5248.h"
#include <Wire.h>
#include <stdio.h>
#include <math.h>

AD5248::AD5248(byte deviceAddr) {
    _deviceAddr = deviceAddr;
}

// Low-level I2C transaction with standard error logging
void AD5248::setStep(byte wiperChannel, byte stepValue) {
    if (wiperChannel != 0 && wiperChannel != 1) {
        fprintf(stderr, "[ERROR] [%s:%d in %s()] Invalid AD5248 wiper channel: %d\n", 
                __FILE__, __LINE__, __func__, (int)wiperChannel);
        return;
    }

    byte cmdByte = (wiperChannel == 0) ? CMD_WIPER0 : CMD_WIPER1;

    Wire.beginTransmission(_deviceAddr);
    Wire.write(cmdByte);
    Wire.write(stepValue);
    byte error = Wire.endTransmission();

    if (error != 0) {
        Serial.printf("[I2C ERROR] AD5248 Address: 0x%02X | Error Code: %d\n", _deviceAddr, error);
    }
}

// Maps float kOhm values to 8-bit steps (0-255) with strict boundary warnings
void AD5248::setResistance(byte wiperChannel, float kohm) {
    // Check for lower boundary error
    if (kohm < 0.0f) {
        fprintf(stderr, "[ERROR] [%s:%d in %s()] Target resistance too low: %.3f kOhm. Clamping to 0.0 kOhm.\n", 
                __FILE__, __LINE__, __func__, kohm);
        kohm = 0.0f;
    }
    // Check for upper boundary error
    else if (kohm > TOTAL_RESISTANCE) {
        fprintf(stderr, "[ERROR] [%s:%d in %s()] Target resistance too high: %.3f kOhm. Clamping to %.1f kOhm.\n", 
                __FILE__, __LINE__, __func__, kohm, TOTAL_RESISTANCE);
        kohm = TOTAL_RESISTANCE;
    }

    float calculatedStep = (kohm / TOTAL_RESISTANCE) * MAX_STEPS;
    byte targetStep = (byte)round(calculatedStep);

    setStep(wiperChannel, targetStep);
}

// Simulates a PT1000 sensor by calculating standard polynomial resistance curves
void AD5248::setChanelTemperaturePT1000(byte wiperChannel, float celcius) {
    float r0 = 1000.0f; // 1000 Ohms at 0 degrees Celsius
    float A = 3.9083e-3f;
    float B = -5.775e-7f;
    float rOhm = r0;

    if (celcius >= 0.0f) {
        rOhm = r0 * (1.0f + (A * celcius) + (B * celcius * celcius));
    } else {
        float C = -4.183e-12f;
        rOhm = r0 * (1.0f + (A * celcius) + (B * celcius * celcius) + (C * (celcius - 100.0f) * celcius * celcius * celcius));
    }
    
    // Convert Ohm to kOhm and send to resistance handler
    float kohm = rOhm / 1000.0f;
    this->setResistance(wiperChannel, kohm);
}

// Resets both channels to zero position safely
void AD5248::clearAll() {
    setStep(0, 0);
    setStep(1, 0);
}