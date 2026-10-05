// OMS BLE HID — Bluetooth LE mouse for ESP32.
// Copyright (c) 2026 Open Maker Studio. MIT License (see LICENSE).

#pragma once

#include "OmsBleHid.h"

class BleMouse {
public:
    BleMouse(const char* deviceName = "OMS Mouse",
             const char* manufacturer = "Open Maker Studio",
             uint8_t batteryLevel = 100);

    void begin();
    bool isConnected();
    void setBatteryLevel(uint8_t level);

    // Relative movement (-127..127 per call); wheel > 0 scrolls up.
    void move(int8_t x, int8_t y, int8_t wheel = 0);
    void press(uint8_t buttons = MOUSE_LEFT);
    void release(uint8_t buttons = MOUSE_LEFT);
    void click(uint8_t buttons = MOUSE_LEFT);
    bool isPressed(uint8_t buttons = MOUSE_LEFT);

private:
    String _name;
    String _manufacturer;
    uint8_t _battery;
    uint8_t _buttons = 0;
};
