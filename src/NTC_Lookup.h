#ifndef NTC_LOOKUP_H
#define NTC_LOOKUP_H

#include <Arduino.h>

struct NTCPair {
    float tempC;
    float kohm;
};

class NTCLookup {
public:
    static float tempToKohm(float targetTempC);

private:
    static const size_t TABLE_SIZE = 151;
    static const NTCPair ntcTable[TABLE_SIZE];
};

#endif // NTC_LOOKUP_H