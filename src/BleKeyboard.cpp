// OMS BLE HID — keyboard. Copyright (c) 2026 Open Maker Studio. MIT License (see LICENSE).

#include "BleKeyboard.h"

namespace {

constexpr uint8_t SHIFT_BIT = 0x02;  // left-shift bit in the modifier byte

// HID usage for a US-layout character; sets `shift` when the character needs Shift.
// Returns 0 for characters a US keyboard can't type.
uint8_t usageForChar(uint8_t c, bool& shift) {
    shift = false;
    if (c >= 'a' && c <= 'z') return 0x04 + (c - 'a');
    if (c >= 'A' && c <= 'Z') { shift = true; return 0x04 + (c - 'A'); }
    if (c >= '1' && c <= '9') return 0x1E + (c - '1');
    if (c == '0') return 0x27;
    switch (c) {
        case '\n': return 0x28;  // Enter
        case '\b': return 0x2A;  // Backspace
        case '\t': return 0x2B;  // Tab
        case ' ':  return 0x2C;
        case '-':  return 0x2D;
        case '=':  return 0x2E;
        case '[':  return 0x2F;
        case ']':  return 0x30;
        case '\\': return 0x31;
        case ';':  return 0x33;
        case '\'': return 0x34;
        case '`':  return 0x35;
        case ',':  return 0x36;
        case '.':  return 0x37;
        case '/':  return 0x38;
    }
    shift = true;
    switch (c) {
        case '!': return 0x1E;
        case '@': return 0x1F;
        case '#': return 0x20;
        case '$': return 0x21;
        case '%': return 0x22;
        case '^': return 0x23;
        case '&': return 0x24;
        case '*': return 0x25;
        case '(': return 0x26;
        case ')': return 0x27;
        case '_': return 0x2D;
        case '+': return 0x2E;
        case '{': return 0x2F;
        case '}': return 0x30;
        case '|': return 0x31;
        case ':': return 0x33;
        case '"': return 0x34;
        case '~': return 0x35;
        case '<': return 0x36;
        case '>': return 0x37;
        case '?': return 0x38;
    }
    shift = false;
    return 0;
}

}  // namespace

BleKeyboard::BleKeyboard(const char* deviceName, const char* manufacturer, uint8_t batteryLevel)
    : _name(deviceName), _manufacturer(manufacturer), _battery(batteryLevel) {}

void BleKeyboard::begin() { OmsBleHid::begin(_name.c_str(), _manufacturer.c_str(), _battery); }

bool BleKeyboard::isConnected() { return OmsBleHid::isConnected(); }

void BleKeyboard::setBatteryLevel(uint8_t level) {
    _battery = level;
    OmsBleHid::setBatteryLevel(level);
}

void BleKeyboard::sendKeys() { OmsBleHid::sendKeyboard(_modifiers, _keys); }

size_t BleKeyboard::press(uint8_t key) {
    uint8_t usage;
    if (key >= 0x88) {
        usage = key - 0x88;
    } else if (key >= 0x80) {
        _modifiers |= (uint8_t)(1 << (key - 0x80));
        sendKeys();
        return 1;
    } else {
        bool shift;
        usage = usageForChar(key, shift);
        if (!usage) return 0;
        if (shift) _modifiers |= SHIFT_BIT;
    }
    for (uint8_t k : _keys) if (k == usage) { sendKeys(); return 1; }
    for (uint8_t& k : _keys) {
        if (k == 0) { k = usage; sendKeys(); return 1; }
    }
    return 0;  // all 6 slots in use
}

size_t BleKeyboard::release(uint8_t key) {
    uint8_t usage;
    if (key >= 0x88) {
        usage = key - 0x88;
    } else if (key >= 0x80) {
        _modifiers &= (uint8_t)~(1 << (key - 0x80));
        sendKeys();
        return 1;
    } else {
        bool shift;
        usage = usageForChar(key, shift);
        if (!usage) return 0;
        if (shift) _modifiers &= (uint8_t)~SHIFT_BIT;
    }
    for (uint8_t& k : _keys) if (k == usage) k = 0;
    sendKeys();
    return 1;
}

void BleKeyboard::releaseAll() {
    _modifiers = 0;
    for (uint8_t& k : _keys) k = 0;
    _media = 0;
    sendKeys();
    OmsBleHid::sendMedia(0);
}

size_t BleKeyboard::press(MediaKey key) {
    _media |= key.bit;
    OmsBleHid::sendMedia(_media);
    return 1;
}

size_t BleKeyboard::release(MediaKey key) {
    _media &= (uint16_t)~key.bit;
    OmsBleHid::sendMedia(_media);
    return 1;
}

size_t BleKeyboard::write(uint8_t c) {
    size_t n = press(c);
    release(c);
    return n;
}

size_t BleKeyboard::write(const uint8_t* buffer, size_t size) {
    size_t n = 0;
    while (size--) {
        if (*buffer != '\r' && write(*buffer)) n++;
        buffer++;
    }
    return n;
}

size_t BleKeyboard::write(MediaKey key) {
    press(key);
    release(key);
    return 1;
}
