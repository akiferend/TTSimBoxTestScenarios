#ifndef PERIPHERAL_MANAGER_H
#define PERIPHERAL_MANAGER_H

#include <Arduino.h>

class PeripheralManager {
public:
    static PeripheralManager& getInstance() {
        static PeripheralManager instance;
        return instance;
    }

    PeripheralManager(const PeripheralManager&) = delete;
    void operator=(const PeripheralManager&) = delete;

    void begin();
    void update();

    // ==========================================
    // BUTTON FUNCTIONS (IO12)
    // ==========================================
    bool getPressed();
    void attachPressEvent(void (*callback)());
    void attachDoublePressEvent(void (*callback)());
    void attachLongPressEvent(void (*callback)());

    // ==========================================
    // LED FUNCTIONS (IO23)
    // ==========================================
    void ledOn();
    void ledOff();
    void setLedBlink(unsigned long onTimeMs, unsigned long offTimeMs);

    // ==========================================
    // BUZZER FUNCTION (IO2)
    // ==========================================
    void beep(unsigned long durationMs = 100);

private:
    PeripheralManager();

    static const int PIN_BUTTON = 12;
    static const int PIN_LED = 23;
    static const int PIN_BUZZER = 2;

    static const int BUZZER_PWM_CHANNEL = 0;
    static const int BUZZER_PWM_RES = 8;
    static const unsigned int BUZZER_FREQ = 2000; // Sabit bip frekansı

    // Button Variables
    bool _lastButtonState;
    bool _debouncedButtonState;
    unsigned long _lastDebounceTime;
    unsigned long _buttonPressedTime;
    unsigned long _buttonReleasedTime;
    int _clickCount;
    bool _longPressFired;

    static const unsigned long DEBOUNCE_DELAY_MS = 50;
    static const unsigned long DOUBLE_CLICK_WINDOW_MS = 300;
    static const unsigned long LONG_PRESS_DURATION_MS = 1000;

    void (*_pressCallback)();
    void (*_doublePressCallback)();
    void (*_longPressCallback)();

    // LED Blink Variables
    bool _isBlinking;
    bool _ledCurrentState;
    unsigned long _ledOnTimeMs;
    unsigned long _ledOffTimeMs;
    unsigned long _lastLedToggleTime;
};

#endif