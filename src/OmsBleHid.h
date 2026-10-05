// OMS BLE HID — one Bluetooth LE HID device (keyboard + media keys + mouse) for ESP32.
// Copyright (c) 2026 Open Maker Studio. MIT License (see LICENSE).
//
// BleKeyboard and BleMouse are thin front-ends over this shared device, so a sketch can use both
// at once: the host sees a single "keyboard + mouse" peripheral, not two competing BLE servers.

#pragma once

#include <Arduino.h>

#if !defined(ARDUINO_ARCH_ESP32)
#error "OMS BLE HID needs an ESP32 board with Bluetooth LE (ESP32, S3, C3, C6...)."
#endif

// Mouse buttons (bitmask, Arduino Mouse library convention).
#ifndef MOUSE_LEFT
#define MOUSE_LEFT    0x01
#define MOUSE_RIGHT   0x02
#define MOUSE_MIDDLE  0x04
#define MOUSE_BACK    0x08
#define MOUSE_FORWARD 0x10
#endif

// A media / consumer-control key. A distinct type, so BleKeyboard::write(KEY_MEDIA_MUTE) picks the
// media overload instead of being sent as an ordinary character.
struct MediaKey {
    uint16_t bit;
};

namespace OmsBleHid {

// Starts the shared HID device and advertising. Safe to call more than once; only the first call's
// name / manufacturer / battery level are used.
void begin(const char* deviceName, const char* manufacturer, uint8_t batteryLevel);

// True while a host is connected.
bool isConnected();

void setBatteryLevel(uint8_t level);

// Raw report senders (do nothing while disconnected).
void sendKeyboard(uint8_t modifiers, const uint8_t keys[6]);
void sendMedia(uint16_t bits);
void sendMouse(uint8_t buttons, int8_t x, int8_t y, int8_t wheel);

}  // namespace OmsBleHid
