// OMS BLE HID — mouse. Copyright (c) 2026 Open Maker Studio. MIT License (see LICENSE).

#include "BleMouse.h"

BleMouse::BleMouse(const char* deviceName, const char* manufacturer, uint8_t batteryLevel)
    : _name(deviceName), _manufacturer(manufacturer), _battery(batteryLevel) {}

void BleMouse::begin() { OmsBleHid::begin(_name.c_str(), _manufacturer.c_str(), _battery); }

bool BleMouse::isConnected() { return OmsBleHid::isConnected(); }

void BleMouse::setBatteryLevel(uint8_t level) {
    _battery = level;
    OmsBleHid::setBatteryLevel(level);
}

void BleMouse::move(int8_t x, int8_t y, int8_t wheel) { OmsBleHid::sendMouse(_buttons, x, y, wheel); }

void BleMouse::press(uint8_t buttons) {
    _buttons |= buttons;
    move(0, 0, 0);
}

void BleMouse::release(uint8_t buttons) {
    _buttons &= (uint8_t)~buttons;
    move(0, 0, 0);
}

void BleMouse::click(uint8_t buttons) {
    press(buttons);
    release(buttons);
}

bool BleMouse::isPressed(uint8_t buttons) { return (_buttons & buttons) != 0; }
