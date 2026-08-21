#ifndef PT1000_LOOKUP_H
#define PT1000_LOOKUP_H

#include <Arduino.h>

struct PT1000Point {
    int16_t tempC;   // Sıcaklık (°C)
    float   ohm;     // Direnç (Ohm)
};

class PT1000Lookup {
private:
    static const PT1000Point table[];
    static const int tableSize;

public:
    // Sıcaklık (°C) -> Direnç (Ohm) dönüşümü
    static float temperatureToOhm(float celsius);

    // Direnç (Ohm) -> Sıcaklık (°C) dönüşümü
    static float ohmToTemperature(float ohm);
};

#endif // PT1000_LOOKUP_H