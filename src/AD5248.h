#ifndef AD5248_H
#define AD5248_H

#include <Arduino.h>

#ifndef AD5248_I2C_ADDR
#define AD5248_I2C_ADDR 0x2C
#endif

// Kalibrasyon noktası yapısı
struct CalibPoint {
    float targetOhm;   // Hedeflenen (istenilen) Ohm değeri
    float measuredOhm; // Multimetrede okunan ham Ohm değeri
};

class AD5248 {
private:
    byte _deviceAddr;
    const float TOTAL_RESISTANCE = 2.5f; // 2.5 kOhm (PT1000 Simülasyon Modülü)
    const int MAX_STEPS = 255;           // 8-bit (0-255 step)

    // Datasheet Instruction Register komutları
    const byte CMD_RDAC1 = 0x00; // Channel 0
    const byte CMD_RDAC2 = 0x80; // Channel 1 (A7 Bit high)

    // Multimetreden aldığın 6 noktalı kalibrasyon tablosu
    static constexpr CalibPoint calibTable[6] = {
        {200.0f,  275.0f},
        {500.0f,  580.0f},
        {850.0f,  938.0f},
        {1200.0f, 1279.0f},
        {1700.0f, 1781.0f},
        {2500.0f, 2586.0f}
    };

    // Dahili parçalı lineer interpolasyon fonksiyonu
    float getCalibratedOhm(float targetOhm);

public:
    void setMaxResistanceAll(); // Her iki kanalı da 2.5 kOhm (max) yapar
    AD5248(byte deviceAddr = AD5248_I2C_ADDR);

    void setStep(byte wiperChannel, byte stepValue);
    void setResistance(byte wiperChannel, float kohm);
    void clearAll();
};

#endif // AD5248_H