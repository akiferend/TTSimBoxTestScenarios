#ifndef AD5248_H
#define AD5248_H

#include <Arduino.h>

// If these are not defined in your main or build flags, defaults are used.
#ifndef AD5248_ADDR_DEFAULT
#define AD5248_ADDR_DEFAULT 0x2C
#endif

class AD5248 {
public:
    // Initializes the device with the dynamically configured default address
    AD5248(byte deviceAddr = AD5248_ADDR_DEFAULT);

    // Writes raw step values (0-255) directly to the specified channel (0 or 1)
    void setStep(byte wiperChannel, byte stepValue);

    // Calculates and writes dynamic steps using float kOhm values (0.0 - 2.5 kOhm)
    void setResistance(byte wiperChannel, float kohm);

    // Converts Celsius to resistance for PT1000 RTD simulation and applies it
    void setChanelTemperaturePT1000(byte wiperChannel, float celcius);

    // Resets both channels to 0 steps safely
    void clearAll();

private:
    byte _deviceAddr;
    
    // AD5248 Register/Command Bytes
    static const byte CMD_WIPER0 = 0x00; // Selects RDAC1
    static const byte CMD_WIPER1 = 0x80; // Selects RDAC2
    
    // Hardware constraints
    static const int MAX_STEPS = 255;
    static constexpr float TOTAL_RESISTANCE = 2.5f; // 2.5 kOhm nominal resistance
};

#endif