#include "TestEngine.h"

static const PotTestCase testCases[] = {
    // { TestID, PotNo, DirençDeğeri, ModbusRegDeğeri }
    { 1, 1, 1.0f,   1000 }, // Pot 1 (AD5248 Ch0) -> 1000 Ohm
    { 2, 2, 1.385f, 1385 }, // Pot 2 (AD5248 Ch1) -> 1385 Ohm
    { 3, 3, 25.0f,  25 }, // Pot 3 (MCP4632) -> 25 °C (10.0 kOhm)
    { 4, 4, 30.0f,  30 }, // Pot 4 (MCP4632) -> 30 °C (8.03 kOhm)
    { 5, 5, 35.0f,  35 }, // Pot 5 (MCP4632) -> 35 °C (6.50 kOhm)
    { 6, 6, 40.0f,  40 }, // Pot 6 (MCP4632) -> 40 °C (5.29 kOhm)
    { 7, 7, 50.0f,  50 }, // Pot 7 (MCP4632) -> 50 °C (3.58 kOhm)
    { 8, 8, 60.0f,  60 }  // Pot 8 (MCP4632) -> 60 °C (2.46 kOhm)
};

TestEngine::TestEngine() {
    _currentState   = STATE_IDLE;
    _testTimerStart = 0;
    _mb             = nullptr;
    _dgtlPot        = nullptr;
    _ad5248         = nullptr;
}

void TestEngine::resetPotToSafe(uint8_t potNo) {
    if (potNo == 1 && _ad5248) {
        _ad5248->setChanelResistance(DGTLPOT_1, 2.5f); // 2.5 kOhm
        _mb->Hreg(REG_POT_START + (potNo - 1), 2500); 
    } 
    else if (potNo == 2 && _ad5248) {
        _ad5248->setChanelResistance(DGTLPOT_2, 2.5f); // 2.5 kOhm
        _mb->Hreg(REG_POT_START + (potNo - 1), 2500); 
    } 
    else if (potNo >= 3 && potNo <= 8 && _dgtlPot) {
        DGTLPOT_Channel ch = static_cast<DGTLPOT_Channel>(potNo - 3);
        
        // Güvenli değer olarak -8 °C seçiyoruz (Tabloda ~50.3 kOhm yapar)
        int16_t safeTemp = -8; 
        float safeKohm = NTCLookup::temperatureToKohm((float)safeTemp);
        
        _dgtlPot->setChanelResistance(ch, safeKohm);
        
        uint8_t regOffset = potNo - 1;
        _mb->Hreg(REG_POT_START + regOffset, (uint16_t)safeTemp); // Modbus'a -8 yazıyoruz
        
        Serial.printf("[SAFE] Pot %d (MCP4632) güvenli değere çekildi -> %d °C (%.2f kOhm)\n", potNo, safeTemp, safeKohm);
    }
}

void TestEngine::begin(ModbusRTU* mb, DGTLPOT* dgtlPot, AD5248* ad5248) {
    _mb      = mb;
    _dgtlPot = dgtlPot;
    _ad5248  = ad5248;

    _mb->Hreg(REG_SYS_CMD, 0);
    _mb->Hreg(REG_TEST_ID, 0);

    for (uint8_t i = 1; i <= 8; i++) {
        resetPotToSafe(i);
    }
}

void TestEngine::setChannelStatus(uint8_t channelIndex, uint16_t statusCode) {
    if (_mb) _mb->Hreg(REG_STATUS_BASE + channelIndex, statusCode);
}

// 3. TABLO TABANLI TEST ÇALIŞTIRICI (DÜZELTİLDİ)
void TestEngine::runTest(uint8_t testId) {
    size_t totalTests = sizeof(testCases) / sizeof(testCases[0]);
    
    for (size_t i = 0; i < totalTests; i++) {
        if (testCases[i].testId == testId) {
            const auto& test = testCases[i];
            uint8_t regOffset = test.potIndex - 1;

            if (test.potIndex == 1) {
                _ad5248->setChanelResistance(DGTLPOT_1, test.resistance);
            } 
            else if (test.potIndex == 2) {
                _ad5248->setChanelResistance(DGTLPOT_2, test.resistance);
            } 
            else if (test.potIndex >= 3 && test.potIndex <= 8) {
                DGTLPOT_Channel ch = static_cast<DGTLPOT_Channel>(test.potIndex - 3);
    
                // Struct içindeki test.resistance alanını artık Sıcaklık (°C) olarak yorumluyoruz
                float targetKohm = NTCLookup::temperatureToKohm(test.resistance); 
    
                _dgtlPot->setChanelResistance(ch, targetKohm);
                Serial.printf("[TEST Engine] NTC Sıcaklık Testi: %.1f °C -> %.2f kOhm uygulandı\n", test.resistance, targetKohm);
            }

            _mb->Hreg(REG_POT_START + regOffset, test.regValue);
            Serial.printf("[TEST Engine] Test %d başladı (Pot %d set edildi).\n", testId, test.potIndex);
            return;
        }
    }
}

// 4. ANA DÖNGÜ (DÜZELTİLDİ)
void TestEngine::update() {
    if (!_mb || !_dgtlPot || !_ad5248) return;

    uint16_t cmd    = _mb->Hreg(REG_SYS_CMD);
    uint16_t testId = _mb->Hreg(REG_TEST_ID);

    size_t totalTests = sizeof(testCases) / sizeof(testCases[0]);

    // Test Başlatma (Sabit 8 yerine dinamik totalTests kullanıldı)
    if (cmd == 1 && _currentState == STATE_IDLE) {
        if (testId >= 1 && testId <= totalTests) {
            _currentState   = STATE_RUNNING;
            _testTimerStart = millis();
            runTest(testId);
        } else {
            _mb->Hreg(REG_SYS_CMD, 0); // Geçersiz ID gelirse sıfırla
        }
    }

    // 5 Saniye Sonra Test Biter
    if (_currentState == STATE_RUNNING) {
        if (millis() - _testTimerStart > 5000) {
            
            resetPotToSafe(testId);         
            setChannelStatus(testId - 1, 2); // Status: PASS (2)

            _mb->Hreg(REG_SYS_CMD, 0); 
            _mb->Hreg(REG_TEST_ID, 0); 
            
            PeripheralManager::getInstance().beep(200);
            Serial.printf("[TEST Engine] Test %d bitti. Pot %d güvenli değere çekildi.\n", testId, testId);
            
            _currentState = STATE_IDLE;
        }
    }
}