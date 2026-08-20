#include "TestEngine.h"

TestEngine::TestEngine() {
    _currentState   = STATE_IDLE;
    _testTimerStart = 0;
    _mb             = nullptr;
    _dgtlPot        = nullptr;
    _ad5248         = nullptr;
}

// 1. TEK FONKSİYON: İstenen Pot'u Güvenli Değerine Çeker
void TestEngine::resetPotToSafe(uint8_t potNo) {
    if (potNo == 1 && _ad5248) {
        _ad5248->setResistance(0, SAFE_AD5248_KOHM);
        _mb->Hreg(REG_POT_START, SAFE_AD5248_OHM);
    } 
    else if (potNo == 2 && _ad5248) {
        _ad5248->setResistance(1, SAFE_AD5248_KOHM);
        _mb->Hreg(REG_POT_START + 1, SAFE_AD5248_OHM);
    } 
    else if (potNo >= 3 && potNo <= 8 && _dgtlPot) {
        DGTLPOT_Channel ch = static_cast<DGTLPOT_Channel>(potNo - 3);
        _dgtlPot->setChanelResistance(ch, SAFE_MCP4632_KOHM);
        _mb->Hreg(REG_POT_START + (potNo - 1), SAFE_MCP4632_OHM);
    }
}

// 2. BAŞLANGIÇ: Tüm Potları Güvenli Değere Getir
void TestEngine::begin(ModbusRTU* mb, DGTLPOT* dgtlPot, AD5248* ad5248) {
    _mb      = mb;
    _dgtlPot = dgtlPot;
    _ad5248  = ad5248;

    _mb->Hreg(REG_SYS_CMD, 0);
    _mb->Hreg(REG_TEST_ID, 0);

    // 8 Potun hepsini tek döngüde güvenli konuma al
    for (uint8_t i = 1; i <= 8; i++) {
        resetPotToSafe(i);
    }
}

void TestEngine::setChannelStatus(uint8_t channelIndex, uint16_t statusCode) {
    if (_mb) _mb->Hreg(REG_STATUS_BASE + channelIndex, statusCode);
}

// 3. TESTLERİ ÇALIŞTIRAN METOT
void TestEngine::runTest(uint8_t testId) {
    switch (testId) {
        case 1: // Pot 1 (AD5248 Ch0) -> 1000 Ohm
            _ad5248->setResistance(0, 1.0f);
            _mb->Hreg(REG_POT_START, 1000);
            break;
            
        case 2: // Pot 2 (AD5248 Ch1) -> 1385 Ohm
            _ad5248->setResistance(1, 1.385f);
            _mb->Hreg(REG_POT_START + 1, 1385);
            break;
            
        case 3: _dgtlPot->setChanelResistance(DGTLPOT_3, 10.0f); _mb->Hreg(REG_POT_START + 2, 10); break;
        case 4: _dgtlPot->setChanelResistance(DGTLPOT_4, 15.0f); _mb->Hreg(REG_POT_START + 3, 15); break;
        case 5: _dgtlPot->setChanelResistance(DGTLPOT_5, 20.0f); _mb->Hreg(REG_POT_START + 4, 20); break;
        case 6: _dgtlPot->setChanelResistance(DGTLPOT_6, 25.0f); _mb->Hreg(REG_POT_START + 5, 25); break;
        case 7: _dgtlPot->setChanelResistance(DGTLPOT_7, 30.0f); _mb->Hreg(REG_POT_START + 6, 30); break;
        case 8: _dgtlPot->setChanelResistance(DGTLPOT_8, 40.0f); _mb->Hreg(REG_POT_START + 7, 40); break;
    }
    Serial.printf("[TEST Engine] Test %d başladı (Pot %d set edildi).\n", testId, testId);
}

// 4. ANA DÖNGÜ: Testi başlatır, 5sn sonra test yapılan potu güvenli değere çevirir
void TestEngine::update() {
    if (!_mb || !_dgtlPot || !_ad5248) return;

    uint16_t cmd    = _mb->Hreg(REG_SYS_CMD);
    uint16_t testId = _mb->Hreg(REG_TEST_ID);

    // Test Başlatma
    if (cmd == 1 && _currentState == STATE_IDLE) {
        if (testId >= 1 && testId <= 8) {
            _currentState   = STATE_RUNNING;
            _testTimerStart = millis();
            runTest(testId);
        } else {
            _mb->Hreg(REG_SYS_CMD, 0); // Geçersiz ID gelirse sıfırla
        }
    }

    // 5 Saniye Sonra Test Biter -> Sadece Çalışan Pot Güvenli Değere Dönüş Yapar
    if (_currentState == STATE_RUNNING) {
        if (millis() - _testTimerStart > 5000) {
            
            resetPotToSafe(testId);         // Testi biten potu güvenli değere döndür
            setChannelStatus(testId - 1, 2); // Status: PASS (2)

            _mb->Hreg(REG_SYS_CMD, 0); 
            _mb->Hreg(REG_TEST_ID, 0); 
            
            PeripheralManager::getInstance().beep(200);
            Serial.printf("[TEST Engine] Test %d bitti. Pot %d güvenli değere çekildi.\n", testId, testId);
            
            _currentState = STATE_IDLE;
        }
    }
}