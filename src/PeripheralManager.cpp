#include "PeripheralManager.h"
#include <stdio.h>

PeripheralManager::PeripheralManager() {
    _pressCallback = nullptr;
    _doublePressCallback = nullptr;
    _longPressCallback = nullptr;

    _lastButtonState = HIGH;
    _debouncedButtonState = HIGH;
    _lastDebounceTime = 0;
    _buttonPressedTime = 0;
    _buttonReleasedTime = 0;
    _clickCount = 0;
    _longPressFired = false;

    _isBlinking = false;
    _ledCurrentState = LOW;
    _ledOnTimeMs = 0;
    _ledOffTimeMs = 0;
    _lastLedToggleTime = 0;
}

void PeripheralManager::begin() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    // ESP32 PWM Kurulumu
    ledcSetup(BUZZER_PWM_CHANNEL, BUZZER_FREQ, BUZZER_PWM_RES);
    ledcAttachPin(PIN_BUZZER, BUZZER_PWM_CHANNEL);
    ledcWrite(BUZZER_PWM_CHANNEL, 0);
}

bool PeripheralManager::getPressed() {
    return (digitalRead(PIN_BUTTON) == LOW);
}

void PeripheralManager::attachPressEvent(void (*callback)()) { _pressCallback = callback; }
void PeripheralManager::attachDoublePressEvent(void (*callback)()) { _doublePressCallback = callback; }
void PeripheralManager::attachLongPressEvent(void (*callback)()) { _longPressCallback = callback; }

void PeripheralManager::ledOn() {
    _isBlinking = false;
    _ledCurrentState = HIGH;
    digitalWrite(PIN_LED, HIGH);
}

void PeripheralManager::ledOff() {
    _isBlinking = false;
    _ledCurrentState = LOW;
    digitalWrite(PIN_LED, LOW);
}

void PeripheralManager::setLedBlink(unsigned long onTimeMs, unsigned long offTimeMs) {
    if (onTimeMs == 0 || offTimeMs == 0) return;
    _ledOnTimeMs = onTimeMs;
    _ledOffTimeMs = offTimeMs;
    _isBlinking = true;
    _lastLedToggleTime = millis();
    _ledCurrentState = HIGH;
    digitalWrite(PIN_LED, HIGH);
}

// SADECE BİPLETEN TEMİZ FONKSİYON
void PeripheralManager::beep(unsigned long durationMs) {
    ledcWriteTone(BUZZER_PWM_CHANNEL, BUZZER_FREQ);
    ledcWrite(BUZZER_PWM_CHANNEL, 128); // Sesi Aç
    delay(durationMs);
    ledcWrite(BUZZER_PWM_CHANNEL, 0);   // Sesi Kapat
}

void PeripheralManager::update() {
    unsigned long currentMillis = millis();

    // Button Engine
    int reading = digitalRead(PIN_BUTTON);
    if (reading != _lastButtonState) {
        _lastDebounceTime = currentMillis;
    }

    if ((currentMillis - _lastDebounceTime) > DEBOUNCE_DELAY_MS) {
        if (reading != _debouncedButtonState) {
            _debouncedButtonState = reading;

            if (_debouncedButtonState == LOW) {
                _buttonPressedTime = currentMillis;
                _longPressFired = false;
            } else {
                unsigned long pressDuration = currentMillis - _buttonPressedTime;
                if (!_longPressFired && pressDuration < LONG_PRESS_DURATION_MS) {
                    _clickCount++;
                    _buttonReleasedTime = currentMillis;
                }
            }
        }
    }
    _lastButtonState = reading;

    if (_debouncedButtonState == LOW && !_longPressFired) {
        if ((currentMillis - _buttonPressedTime) >= LONG_PRESS_DURATION_MS) {
            if (_longPressCallback != nullptr) _longPressCallback();
            _longPressFired = true;
            _clickCount = 0;
        }
    }

    if (_clickCount > 0 && (_debouncedButtonState == HIGH)) {
        if ((currentMillis - _buttonReleasedTime) >= DOUBLE_CLICK_WINDOW_MS) {
            if (_clickCount == 1) {
                if (_pressCallback != nullptr) _pressCallback();
            } else if (_clickCount >= 2) {
                if (_doublePressCallback != nullptr) _doublePressCallback();
            }
            _clickCount = 0;
        }
    }

    // LED Engine
    if (_isBlinking) {
        unsigned long threshold = _ledCurrentState ? _ledOnTimeMs : _ledOffTimeMs;
        if (currentMillis - _lastLedToggleTime >= threshold) {
            _lastLedToggleTime = currentMillis;
            _ledCurrentState = !_ledCurrentState;
            digitalWrite(PIN_LED, _ledCurrentState ? HIGH : LOW);
        }
    }
}