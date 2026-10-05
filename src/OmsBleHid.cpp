// OMS BLE HID — shared device. Copyright (c) 2026 Open Maker Studio. MIT License (see LICENSE).

#include "OmsBleHid.h"

#include <string.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include <BLE2902.h>
#include <BLESecurity.h>
#include <HIDTypes.h>

namespace {

constexpr uint8_t REPORT_ID_KEYBOARD = 1;
constexpr uint8_t REPORT_ID_MEDIA    = 2;
constexpr uint8_t REPORT_ID_MOUSE    = 3;

// HID report descriptor, written from the USB HID 1.11 spec and HID Usage Tables:
//   report 1 — keyboard: modifier bits, reserved byte, 6 key slots (+ LED output report)
//   report 2 — consumer control: 6 media keys as bits, padded to 16 bits
//   report 3 — mouse: 5 buttons, X, Y, wheel (relative, -127..127)
const uint8_t REPORT_MAP[] = {
    // Keyboard
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, REPORT_ID_KEYBOARD,
    0x05, 0x07,        //   Usage Page (Keyboard/Keypad)
    0x19, 0xE0,        //   Usage Minimum (Left Control)
    0x29, 0xE7,        //   Usage Maximum (Right GUI)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)  — modifier bits
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Constant)                  — reserved byte
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x01,        //   Report Size (1)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (Num Lock)
    0x29, 0x05,        //   Usage Maximum (Kana)
    0x91, 0x02,        //   Output (Data, Variable, Absolute)  — LED states from the host
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x03,        //   Report Size (3)
    0x91, 0x01,        //   Output (Constant)                  — LED padding
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x73,        //   Logical Maximum (115)
    0x05, 0x07,        //   Usage Page (Keyboard/Keypad)
    0x19, 0x00,        //   Usage Minimum (0)
    0x29, 0x73,        //   Usage Maximum (115)
    0x81, 0x00,        //   Input (Data, Array)                — 6 key slots
    0xC0,              // End Collection

    // Media keys
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, REPORT_ID_MEDIA,
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x06,        //   Report Count (6)
    0x09, 0xE2,        //   Usage (Mute)             bit 0
    0x09, 0xE9,        //   Usage (Volume Increment) bit 1
    0x09, 0xEA,        //   Usage (Volume Decrement) bit 2
    0x09, 0xCD,        //   Usage (Play/Pause)       bit 3
    0x09, 0xB5,        //   Usage (Scan Next Track)  bit 4
    0x09, 0xB6,        //   Usage (Scan Prev Track)  bit 5
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    0x95, 0x0A,        //   Report Count (10)
    0x81, 0x01,        //   Input (Constant)         — pad to 16 bits
    0xC0,              // End Collection

    // Mouse
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x85, REPORT_ID_MOUSE,
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (1)
    0x29, 0x05,        //     Usage Maximum (5)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x05,        //     Report Count (5)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data, Variable, Absolute) — buttons
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x03,        //     Report Size (3)
    0x81, 0x01,        //     Input (Constant)                 — padding
    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x81, 0x06,        //     Input (Data, Variable, Relative)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};

bool started = false;
volatile bool connected = false;
BLEHIDDevice* hid = nullptr;
BLECharacteristic* keyboardInput = nullptr;
BLECharacteristic* mediaInput = nullptr;
BLECharacteristic* mouseInput = nullptr;

// Turns the input report's notifications on/off from our side, so reports flow as soon as a host
// connects. On NimBLE-based cores (ESP32 core 3.x on S3/C3...) BLE2902 is marked deprecated but still
// works; the warning is silenced here so it doesn't show in every user's compile log.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
void enableNotifications(BLECharacteristic* input, bool on) {
    if (!input) return;
    auto* cccd = static_cast<BLE2902*>(input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902)));
    if (cccd) cccd->setNotifications(on);
}
#pragma GCC diagnostic pop

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer*) override {
        connected = true;
        enableNotifications(keyboardInput, true);
        enableNotifications(mediaInput, true);
        enableNotifications(mouseInput, true);
    }
    void onDisconnect(BLEServer*) override {
        connected = false;
        enableNotifications(keyboardInput, false);
        enableNotifications(mediaInput, false);
        enableNotifications(mouseInput, false);
        // Advertise again so the host (or another one) can reconnect.
        BLEDevice::startAdvertising();
    }
};

void notify(BLECharacteristic* input, uint8_t* data, size_t len) {
    if (!connected || !input) return;
    input->setValue(data, len);
    input->notify();
    delay(7);  // give the host time to read each report (key down/up pairs get lost otherwise)
}

}  // namespace

namespace OmsBleHid {

void begin(const char* deviceName, const char* manufacturer, uint8_t batteryLevel) {
    if (started) return;
    started = true;

    BLEDevice::init(deviceName);
    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks());

    hid = new BLEHIDDevice(server);
    keyboardInput = hid->inputReport(REPORT_ID_KEYBOARD);
    hid->outputReport(REPORT_ID_KEYBOARD);
    mediaInput = hid->inputReport(REPORT_ID_MEDIA);
    mouseInput = hid->inputReport(REPORT_ID_MOUSE);

    hid->manufacturer()->setValue((uint8_t*)manufacturer, strlen(manufacturer));
    hid->pnp(0x02, 0x303A, 0x4F4D, 0x0100);  // USB-IF vendor source, Espressif VID, "OM", v1.0
    hid->hidInfo(0x00, 0x01);
    hid->reportMap((uint8_t*)REPORT_MAP, sizeof(REPORT_MAP));
    hid->startServices();
    hid->setBatteryLevel(batteryLevel);

    BLESecurity* security = new BLESecurity();
    security->setAuthenticationMode(ESP_LE_AUTH_BOND);

    BLEAdvertising* advertising = server->getAdvertising();
    advertising->setAppearance(HID_KEYBOARD);
    advertising->addServiceUUID(hid->hidService()->getUUID());
    advertising->start();
}

bool isConnected() { return connected; }

void setBatteryLevel(uint8_t level) {
    if (hid) hid->setBatteryLevel(level);
}

void sendKeyboard(uint8_t modifiers, const uint8_t keys[6]) {
    uint8_t report[8] = { modifiers, 0, keys[0], keys[1], keys[2], keys[3], keys[4], keys[5] };
    notify(keyboardInput, report, sizeof(report));
}

void sendMedia(uint16_t bits) {
    uint8_t report[2] = { (uint8_t)(bits & 0xFF), (uint8_t)(bits >> 8) };
    notify(mediaInput, report, sizeof(report));
}

void sendMouse(uint8_t buttons, int8_t x, int8_t y, int8_t wheel) {
    uint8_t report[4] = { buttons, (uint8_t)x, (uint8_t)y, (uint8_t)wheel };
    notify(mouseInput, report, sizeof(report));
}

}  // namespace OmsBleHid
