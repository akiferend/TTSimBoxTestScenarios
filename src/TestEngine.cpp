#include "TestEngine.h"

TestEngine::TestEngine() {
    _currentState = STATE_IDLE;
    _testTimerStart = 0;
    _mb = nullptr;
    _dgtlPot = nullptr;
}

void TestEngine::begin(ModbusRTU* mb, DGTLPOT* dgtlPot) {
    _mb = mb;
    _dgtlPot = dgtlPot;

    // Register'ları Başlangıçta Temizle
    _mb->Hreg(REG_SYS_CMD, 0);
    _mb->Hreg(REG_TEST_ID, 0);
    _mb->Hreg(REG_TEST_STATUS, 0);

    // Sistem ilk açıldığında SADECE DGTLPOT_3'ü güvenli 50 kOhm konuma getir
    if (_dgtlPot) {
        _dgtlPot->setChanelResistance(DGTLPOT_3, 50.0f);
        
        // Register 2'ye başlangıç değeri olarak 50 (kOhm) yaz
        _mb->Hreg(REG_POT_START, 50);
    }
}

void TestEngine::setChannelStatus(uint8_t channelIndex, uint16_t statusCode) {
    if (!_mb) return; 
    _mb->Hreg(REG_STATUS_BASE + channelIndex, statusCode);
}

void TestEngine::update() {
    if (!_mb || !_dgtlPot) return;

    uint16_t cmd = _mb->Hreg(REG_SYS_CMD);
    uint16_t testId = _mb->Hreg(REG_TEST_ID);

    // Python'dan cmd=1 gelmişse ve sistem boşta ise (IDLE) testi başlat
    if (cmd == 1 && _currentState == STATE_IDLE) {
        _currentState = STATE_RUNNING;
        _testTimerStart = millis();
        Serial.printf("\n[HIL ENGINE] DGTLPOT_3 Test Senaryosu %d Başlatıldı...\n", testId);

        switch (testId) {
            case 1: runTestChannel1(); break;
            case 2: runTestChannel2(); break;
            case 3: runTestChannel3(); break;
            case 4: runTestChannel4(); break;
            case 5: runTestChannel5(); break;
            case 6: runTestChannel6(); break;
            case 7: runTestChannel7(); break;
            case 8: runTestChannel8(); break;
        }
    }

    // Test çalışıyorsa
    if (_currentState == STATE_RUNNING) {
        if (millis() - _testTimerStart > 5000) { // 5 saniye bekle
            _currentState = STATE_PASS;
            
            if (testId >= 1 && testId <= 8) {
                setChannelStatus(testId - 1, 2); // İlgili testi PASS (2) yap
                
                // Test bitiminde DGTLPOT_3'ü tekrar 50 kOhm varsayılan değere getir
                _dgtlPot->setChanelResistance(DGTLPOT_3, 50.0f);
                _mb->Hreg(REG_POT_START, 50);
            }

            _mb->Hreg(REG_SYS_CMD, 0); 
            _mb->Hreg(REG_TEST_ID, 0); 
            
            PeripheralManager::getInstance().beep(200);
            Serial.println("[HIL ENGINE] TEST PASSED & RESTORED TO 50k!");
            
            _currentState = STATE_IDLE;
        }
    }
}

// BÜTÜN TESTLER SADECE DGTLPOT_3 İÇİN UYARLANDI
void TestEngine::runTestChannel1() {
    _dgtlPot->setChanelResistance(DGTLPOT_3, 0.45f); // 25°C
    Serial.println("[NTC TEST] 0.45 kOhm set edildi (25°C)");
}

void TestEngine::runTestChannel2() {
    _dgtlPot->setChanelResistance(DGTLPOT_3, 5.86f);  // 36.5°C
    Serial.println("[NTC TEST] 5.86 kOhm set edildi (36.5°C)");
}

void TestEngine::runTestChannel3() {
    _dgtlPot->setChanelResistance(DGTLPOT_3, 4.79f);  // 42.5°C
    Serial.println("[NTC TEST] 4.79 kOhm set edildi (42.5°C)");
}

void TestEngine::runTestChannel4() {
    _dgtlPot->setChanelResistance(DGTLPOT_3, 1.83f);  // 68.5°C
    Serial.println("[NTC TEST] 1.83 kOhm set edildi (68.5°C)");
}

void TestEngine::runTestChannel5() {
    _dgtlPot->setChanelResistance(DGTLPOT_3, 30.0f); 
    _mb->Hreg(REG_POT_START, 30);
    Serial.println("[DIAGNOSTIC] Test 5: DGTLPOT_3 -> 30 kOhm (2.972V)");
}

void TestEngine::runTestChannel6() {
    _dgtlPot->setChanelResistance(DGTLPOT_3, 40.0f); 
    _mb->Hreg(REG_POT_START, 40);
    Serial.println("[DIAGNOSTIC] Test 6: DGTLPOT_3 -> 40 kOhm (3.048V)");
}

void TestEngine::runTestChannel7() {
    _dgtlPot->setChanelResistance(DGTLPOT_3, 50.0f); 
    _mb->Hreg(REG_POT_START, 50);
    Serial.println("[DIAGNOSTIC] Test 7: DGTLPOT_3 -> 50 kOhm (3.095V)");
}

void TestEngine::runTestChannel8() {
    _dgtlPot->setChanelResistance(DGTLPOT_3, 50.0f); 
    _mb->Hreg(REG_POT_START, 50);
    Serial.println("[DIAGNOSTIC] Test 8: DGTLPOT_3 -> Reset / 50 kOhm (3.095V)");
}