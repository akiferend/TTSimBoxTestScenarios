#ifndef TEST_ENGINE_H
#define TEST_ENGINE_H

#include <Arduino.h>
#include <ModbusRTU.h>
#include "MCP4632.h"
#include "PeripheralManager.h"

// Modbus Register Haritası
#define REG_SYS_CMD       0   // 1: Test Başlat, 0: Normal Mod
#define REG_TEST_ID        1   // Koşturulacak Test ID
#define REG_POT_START     2   // Reg 2 - Reg 7 (Pot 1 - Pot 6)
#define REG_TEST_STATUS  8  // 0: IDLE, 1: RUNNING, 2: PASS, 3: FAIL
#define REG_STATUS_BASE   10  // Kanal Status Taban Adresi (Reg 10-17)

enum TestState { STATE_IDLE, STATE_RUNNING, STATE_PASS, STATE_FAIL };

class TestEngine {
private:
    ModbusRTU* _mb;
    DGTLPOT* _dgtlPot;
    
    TestState _currentState;
    unsigned long _testTimerStart;

    void runScenario1();
    void runScenario2();
    void runScenario3();
    void runScenario4();
    void runScenario5();
    void runScenario6();

public:
    TestEngine();
    void begin(ModbusRTU* mb, DGTLPOT* dgtlPot);
    void update();
    
    // YENİ EKLENEN METOT DEKLARASYONU:
    void setChannelStatus(uint8_t channelIndex, uint16_t statusCode);
};

#endif