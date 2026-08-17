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

    // Sistem ilk açıldığında tüm potları güvenli 50 kOhm konuma getir
    if (_dgtlPot) {
        _dgtlPot->setChanelResistance(DGTLPOT_3, 50.0f);
        _dgtlPot->setChanelResistance(DGTLPOT_4, 50.0f);
        _dgtlPot->setChanelResistance(DGTLPOT_5, 50.0f);
        _dgtlPot->setChanelResistance(DGTLPOT_6, 50.0f);
        _dgtlPot->setChanelResistance(DGTLPOT_7, 50.0f);
        _dgtlPot->setChanelResistance(DGTLPOT_8, 50.0f);
        
        // Modbus pot register'larının tamamına başlangıç değeri olarak 50 (kOhm) yaz
        for (int i = 2; i <= 9; i++) {
            _mb->Hreg(i, 50);
        }
    }
}

// Belirli bir kanalın durumunu register'a yazar (Python burayı okur)
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
        Serial.printf("\n[HIL ENGINE] Test Senaryosu %d Başlatıldı...\n", testId);

        switch (testId) {
            case 1: runTestChannel1(); break; // Boş test
            case 2: runTestChannel2(); break; // Boş test
            case 3: runTestChannel3(); break; // Pot 3 Testi
            case 4: runTestChannel4(); break; // Pot 4 Testi
            case 5: runTestChannel5(); break; // Pot 5 Testi
            case 6: runTestChannel6(); break; // Pot 6 Testi
            case 7: runTestChannel7(); break; // Pot 7 Testi
            case 8: runTestChannel8(); break; // Pot 8 Testi
        }
    }

    // Test çalışıyorsa (RUNNING durumu)
    if (_currentState == STATE_RUNNING) {
        if (millis() - _testTimerStart > 5000) { // 5 saniye boyunca 10k direnç düşüşünü koru
            _currentState = STATE_PASS;
            
            if (testId >= 1 && testId <= 8) {
                setChannelStatus(testId - 1, 2); // İlgili kanalı PASS (2) yap
                
                // 5 saniye dolunca ilgili potu tekrar 50 kOhm'a çek ve register değerini 50 yap
                switch (testId) {
                    case 1: /* Boş */ _mb->Hreg(2, 50); break;
                    case 2: /* Boş */ _mb->Hreg(3, 50); break;
                    case 3: _dgtlPot->setChanelResistance(DGTLPOT_3, 50.0f); _mb->Hreg(4, 50); break;
                    case 4: _dgtlPot->setChanelResistance(DGTLPOT_4, 50.0f); _mb->Hreg(5, 50); break;
                    case 5: _dgtlPot->setChanelResistance(DGTLPOT_5, 50.0f); _mb->Hreg(6, 50); break;
                    case 6: _dgtlPot->setChanelResistance(DGTLPOT_6, 50.0f); _mb->Hreg(7, 50); break;
                    case 7: _dgtlPot->setChanelResistance(DGTLPOT_7, 50.0f); _mb->Hreg(8, 50); break;
                    case 8: _dgtlPot->setChanelResistance(DGTLPOT_8, 50.0f); _mb->Hreg(9, 50); break;
                }
            }

            // Test Bitti: Sistemi tamamen sıfırla ki tekrar test seçilebilsin
            _mb->Hreg(REG_SYS_CMD, 0); 
            _mb->Hreg(REG_TEST_ID, 0); 
            
            PeripheralManager::getInstance().beep(200);
            Serial.println("[HIL ENGINE] TEST PASSED & RESTORED TO 50k!");
            
            _currentState = STATE_IDLE; // Tekrar komut beklemeye dön
        }
    }
}

// TEST TANIMLARI (Test 1 ve 2 boş, Test 3'ten itibaren Pot 3 -> Pot 8 sıralı)
void TestEngine::runTestChannel1() {
    _mb->Hreg(2, 50);
    Serial.println("[DIAGNOSTIC] Test 1: Boş Test (Aktif pot yok).");
}

void TestEngine::runTestChannel2() {
    _mb->Hreg(3, 50);
    Serial.println("[DIAGNOSTIC] Test 2: Boş Test (Aktif pot yok).");
}

void TestEngine::runTestChannel3() {
    _dgtlPot->setChanelResistance(DGTLPOT_3, 10.0f); 
    _mb->Hreg(4, 10);
    Serial.println("[DIAGNOSTIC] Test 3: Pot 3 -> 10 kOhm'a düşürüldü (5sn).");
}

void TestEngine::runTestChannel4() {
    _dgtlPot->setChanelResistance(DGTLPOT_4, 10.0f); 
    _mb->Hreg(5, 10);
    Serial.println("[DIAGNOSTIC] Test 4: Pot 4 -> 10 kOhm'a düşürüldü (5sn).");
}

void TestEngine::runTestChannel5() {
    _dgtlPot->setChanelResistance(DGTLPOT_5, 10.0f); 
    _mb->Hreg(6, 10);
    Serial.println("[DIAGNOSTIC] Test 5: Pot 5 -> 10 kOhm'a düşürüldü (5sn).");
}

void TestEngine::runTestChannel6() {
    _dgtlPot->setChanelResistance(DGTLPOT_6, 10.0f); 
    _mb->Hreg(7, 10);
    Serial.println("[DIAGNOSTIC] Test 6: Pot 6 -> 10 kOhm'a düşürüldü (5sn).");
}

void TestEngine::runTestChannel7() {
    _dgtlPot->setChanelResistance(DGTLPOT_7, 10.0f); 
    _mb->Hreg(8, 10);
    Serial.println("[DIAGNOSTIC] Test 7: Pot 7 -> 10 kOhm'a düşürüldü (5sn).");
}

void TestEngine::runTestChannel8() {
    _dgtlPot->setChanelResistance(DGTLPOT_8, 10.0f); 
    _mb->Hreg(9, 10);
    Serial.println("[DIAGNOSTIC] Test 8: Pot 8 -> 10 kOhm'a düşürüldü (5sn).");
}