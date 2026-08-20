#ifndef TEST_ENGINE_H
#define TEST_ENGINE_H

#include <Arduino.h>
#include <ModbusRTU.h>
#include "MCP4632.h"
#include "AD5248.h"
#include "PeripheralManager.h"

// Modbus Register Haritası
#define REG_SYS_CMD      0   // 1: Test Başlat, 0: Normal Mod
#define REG_TEST_ID      1   // Test ID (1 - 8)
#define REG_POT_START    2   // Reg 2..9 -> Pot 1..8 Değerleri
#define REG_TEST_STATUS  8   
#define REG_STATUS_BASE  10  

// Güvenli Varsayılan Dirençler
constexpr float    SAFE_AD5248_KOHM  = 2.5f;  // Pot 1-2 için (2.5 kOhm)
constexpr uint16_t SAFE_AD5248_OHM   = 2500;

constexpr float    SAFE_MCP4632_KOHM = 50.0f; // Pot 3-8 için (50 kOhm)
constexpr uint16_t SAFE_MCP4632_OHM  = 50;

enum TestState { STATE_IDLE, STATE_RUNNING, STATE_PASS, STATE_FAIL };

// Test Senaryosu Tablo Yapısı
struct PotTestCase {
    uint8_t testId;       // Test ID (1, 2, 3...)
    uint8_t potIndex;     // Pot No (1 .. 8)
    float resistance;     // Direnç Değeri (AD5248 için Ohm/1000 kOhm, MCP4632 için kOhm)
    uint16_t regValue;    // Modbus'a yazılacak değer
};

class TestEngine {
private:
    ModbusRTU* _mb;
    DGTLPOT*   _dgtlPot;
    AD5248*    _ad5248;
    
    TestState     _currentState;
    unsigned long _testTimerStart;

    void resetPotToSafe(uint8_t potNo); // Seçilen potu güvenli değere döndürür
    void runTest(uint8_t testId);      // İlgili testi başlatır

public:
    TestEngine();
    void begin(ModbusRTU* mb, DGTLPOT* dgtlPot, AD5248* ad5248);
    void update();
    void setChannelStatus(uint8_t channelIndex, uint16_t statusCode);
};

#endif