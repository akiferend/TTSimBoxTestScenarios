#ifndef TEST_ENGINE_H
#define TEST_ENGINE_H

#include <Arduino.h>
#include <ModbusRTU.h>
#include "MCP4632.h"
#include "PeripheralManager.h"

// Modbus Register Haritası (Net Eşleşmeler)
#define REG_SYS_CMD      0   // 1: Test Başlat, 0: Normal Mod
#define REG_TEST_ID      1   // Koşturulacak Test ID (1 - 8)
#define REG_POT_START    2   // Reg 2 - Reg 9 (Pot 1 - Pot 8 Değerleri)
#define REG_TEST_STATUS  8   // Genel Test Durum Kaydı
#define REG_STATUS_BASE  10  // Kanal Status Taban Adresi (Reg 10-17)

enum TestState { STATE_IDLE, STATE_RUNNING, STATE_PASS, STATE_FAIL };

class TestEngine {
private:
    ModbusRTU* _mb;
    DGTLPOT* _dgtlPot;
    
    TestState _currentState;
    unsigned long _testTimerStart;

    void runTestChannel1();
    void runTestChannel2();
    void runTestChannel3();
    void runTestChannel4();
    void runTestChannel5();
    void runTestChannel6();
    void runTestChannel7();
    void runTestChannel8();

public:
    TestEngine();
    void begin(ModbusRTU* mb, DGTLPOT* dgtlPot);
    void update();
    
    void setChannelStatus(uint8_t channelIndex, uint16_t statusCode);
};

#endif