#ifndef NTC_LOOKUP_H
#define NTC_LOOKUP_H

#include <Arduino.h>

// Tek bir tablo noktası: Sıcaklık (°C) <-> Direnç (kOhm)
struct NTCPoint {
    int16_t tempC;   // Tam sayı derece
    float   kohm;    // O sıcaklıktaki NTC direnci (kOhm)
};

class NTCLookup {
private:
    static const NTCPoint table[];
    static const int tableSize;

public:
    // Sıcaklık (°C) -> Direnç (kOhm)
    static float temperatureToKohm(float celsius);

    // Direnç (kOhm) -> Sıcaklık (°C)
    static float kohmToTemperature(float kohm);
};

#endif // NTC_LOOKUP_H