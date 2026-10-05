// OMS BLE HID — Bluetooth LE keyboard for ESP32.
// Copyright (c) 2026 Open Maker Studio. MIT License (see LICENSE).
//
// Key codes follow the Arduino Keyboard library convention: printable ASCII characters are typed as
// themselves (US layout), modifiers are 0x80-0x87, and other special keys are 0x88 + their HID usage.

#pragma once

#include "OmsBleHid.h"

// Modifiers
#define KEY_LEFT_CTRL    0x80
#define KEY_LEFT_SHIFT   0x81
#define KEY_LEFT_ALT     0x82
#define KEY_LEFT_GUI     0x83
#define KEY_RIGHT_CTRL   0x84
#define KEY_RIGHT_SHIFT  0x85
#define KEY_RIGHT_ALT    0x86
#define KEY_RIGHT_GUI    0x87

// Special keys (0x88 + HID usage)
#define KEY_RETURN       0xB0
#define KEY_ESC          0xB1
#define KEY_BACKSPACE    0xB2
#define KEY_TAB          0xB3
#define KEY_CAPS_LOCK    0xC1
#define KEY_F1           0xC2
#define KEY_F2           0xC3
#define KEY_F3           0xC4
#define KEY_F4           0xC5
#define KEY_F5           0xC6
#define KEY_F6           0xC7
#define KEY_F7           0xC8
#define KEY_F8           0xC9
#define KEY_F9           0xCA
#define KEY_F10          0xCB
#define KEY_F11          0xCC
#define KEY_F12          0xCD
#define KEY_PRTSC        0xCE
#define KEY_INSERT       0xD1
#define KEY_HOME         0xD2
#define KEY_PAGE_UP      0xD3
#define KEY_DELETE       0xD4
#define KEY_END          0xD5
#define KEY_PAGE_DOWN    0xD6
#define KEY_RIGHT_ARROW  0xD7
#define KEY_LEFT_ARROW   0xD8
#define KEY_DOWN_ARROW   0xD9
#define KEY_UP_ARROW     0xDA

// Media keys (bits of the consumer-control report — see OmsBleHid.cpp)
const MediaKey KEY_MEDIA_MUTE           = { 0x0001 };
const MediaKey KEY_MEDIA_VOLUME_UP      = { 0x0002 };
const MediaKey KEY_MEDIA_VOLUME_DOWN    = { 0x0004 };
const MediaKey KEY_MEDIA_PLAY_PAUSE     = { 0x0008 };
const MediaKey KEY_MEDIA_NEXT_TRACK     = { 0x0010 };
const MediaKey KEY_MEDIA_PREVIOUS_TRACK = { 0x0020 };

class BleKeyboard : public Print {
public:
    BleKeyboard(const char* deviceName = "OMS Keyboard",
                const char* manufacturer = "Open Maker Studio",
                uint8_t batteryLevel = 100);

    void begin();
    bool isConnected();
    void setBatteryLevel(uint8_t level);

    size_t press(uint8_t key);
    size_t release(uint8_t key);
    void releaseAll();

    size_t press(MediaKey key);
    size_t release(MediaKey key);

    // Print: press + release each character, so print("Hello") types it.
    size_t write(uint8_t c) override;
    size_t write(const uint8_t* buffer, size_t size) override;
    size_t write(MediaKey key);
    using Print::write;

private:
    String _name;
    String _manufacturer;
    uint8_t _battery;
    uint8_t _modifiers = 0;
    uint8_t _keys[6] = { 0, 0, 0, 0, 0, 0 };
    uint16_t _media = 0;

    void sendKeys();
};
