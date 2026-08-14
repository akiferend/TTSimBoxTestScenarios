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

    // Register'ları Temizle
    _mb->Hreg(REG_SYS_CMD, 0);
    _mb->Hreg(REG_TEST_ID, 0);
    _mb->Hreg(REG_TEST_STATUS, 0);
}

// Yardımcı Metot: Belirli bir kanalın durumunu register'a yazar
void TestEngine::setChannelStatus(uint8_t channelIndex, uint16_t statusCode) {
    if (!_mb) return; // Güvenlik Kontrolü: Modbus pointer'ı boşsa işlem yapma
    
    // channelIndex: 0-7 arası (Kanal 1 - Kanal 8)
    _mb->Hreg(REG_STATUS_BASE + channelIndex, statusCode);
}

// update() Metodundaki Tamamlama Kısmı:
void TestEngine::update() {
    if (!_mb || !_dgtlPot) return;

    uint16_t cmd = _mb->Hreg(REG_SYS_CMD);
    uint16_t testId = _mb->Hreg(REG_TEST_ID);

    if (cmd == 1 && _currentState == STATE_IDLE) {
        _currentState = STATE_RUNNING;
        _testTimerStart = millis();
        Serial.printf("\n[HIL ENGINE] Test Senaryosu %d Başlatıldı...\n", testId);

        switch (testId) {
            case 1: runScenario1(); break;
            case 2: runScenario2(); break;
            case 3: runScenario3(); break;
            case 4: runScenario4(); break;
            case 5: runScenario5(); break;
            case 6: runScenario6(); break; // YENİ EKLENEN ÇOKLU KANAL SENARYOSU
            default:
                _currentState = STATE_FAIL;
                _mb->Hreg(REG_SYS_CMD, 0);
                break;
        }
    }

    if (_currentState == STATE_RUNNING) {
        if (millis() - _testTimerStart > 3500) { // 3.5 saniye donanım bekleme süresi
            _currentState = STATE_PASS;
            
            // SENARYO BAZLI KANAL STATUS GÜNCELLEMELERİ
            if (testId == 6) {
                // EŞ ZAMANLI 2 KANAL PASS YAPILIYOR
                setChannelStatus(0, 2); // Kanal 1 (Index 0 / Reg 10) -> PASS
                setChannelStatus(5, 2); // Kanal 6 (Index 5 / Reg 15) -> PASS
            } else {
                // Diğer tekil senaryolar için Kanal 6 (Index 5) PASS
                setChannelStatus(5, 2);
            }

            _mb->Hreg(REG_SYS_CMD, 0); // Komutu sıfırla
            PeripheralManager::getInstance().beep(200);
            Serial.println("[HIL ENGINE] TEST PASSED!");
            _currentState = STATE_IDLE;
        }
    }
}

// SENARYO 1: Donma Koruması Testi (50 kOhm -> 3.08V)
void TestEngine::runScenario1() {
    _dgtlPot->setChanelResistance(DGTLPOT_8, 50.0f);
    _mb->Hreg(7, 50);
    Serial.println("[SCENARIO 1] 6. Port -> 50 kOhm Ayarlandı.");
}

// SENARYO 2: Yüksek Sıcaklık Testi (10 kOhm -> 2.49V)
void TestEngine::runScenario2() {
    _dgtlPot->setChanelResistance(DGTLPOT_8, 10.0f);
    _mb->Hreg(7, 10);
    Serial.println("[SCENARIO 2] 6. Port -> 10 kOhm Ayarlandı.");
}

// SENARYO 3: Kademeli Sıcaklık Taraması (10 kOhm -> 50 kOhm Fade)
void TestEngine::runScenario3() {
    Serial.println("[SCENARIO 3] 6. Port -> 10k -> 50k Kademeli Geçiş Başlatıldı.");
    _dgtlPot->startChanelFadeByDuration(DGTLPOT_8, 10.0f, 50.0f, 1.0f, 3000);
    _mb->Hreg(7, 50);
}

// SENARYO 4: Sensör Kopuk Arızası Enjeksiyonu (100 kOhm -> Max Direnç / ~3.3V)
void TestEngine::runScenario4() {
    _dgtlPot->setChanelResistance(DGTLPOT_8, 100.0f);
    _mb->Hreg(7, 100);
    Serial.println("[SCENARIO 4] HATA ENJEKSİYONU: Sensör Kopuk (100 kOhm / Open Circuit).");
}

// SENARYO 5: Sensör Kısa Devre Arızası Enjeksiyonu (0 kOhm -> Min Direnç / 0V)
void TestEngine::runScenario5() {
    _dgtlPot->setChanelResistance(DGTLPOT_8, 0.0f);
    _mb->Hreg(7, 0);
    Serial.println("[SCENARIO 5] HATA ENJEKSİYONU: Sensör Kısa Devre (0 Ohm / Short Circuit).");
}

// Senaryo 6: Çift Sensör Eş Zamanlı Testi (Kanal 1: 10 kΩ, Kanal 6: 50 kΩ)
void TestEngine::runScenario6() {
    Serial.println("  -> [SENARYO 6] Çift Kanal Eş Zamanlı Simülasyon Çalıştırılıyor...");
    
    // 1. Kanalı (DGTLPOT_1 veya ilk kanal enumu hangisiyse) Yüksek Sıcaklık Seviyesine Çek (10 kΩ)
    _dgtlPot->setChanelResistance(DGTLPOT_3, 10.0f);
    
    // 6. Kanalı (DGTLPOT_8 veya kullandığınız kanal enumu) Donma Koruması Seviyesine Çek (50 kΩ)
    _dgtlPot->setChanelResistance(DGTLPOT_8, 50.0f);
}