# OMS BLE HID

Bluetooth LE **keyboard, media keys and mouse** for ESP32 boards, as **one** HID device.

`BleKeyboard` and `BleMouse` share a single BLE HID peripheral, so one sketch can type, send media
keys and move the mouse at the same time. The host pairs with one device.

Works with the ESP32 Arduino core **2.x and 3.x** (ESP32, ESP32-S3, ESP32-C3, ESP32-C6 — any chip with
Bluetooth LE). MIT licensed. Used by the [Open Maker Studio](https://openmakerstudio.com) BLE HID blocks.

## Example

```cpp
#include <BleKeyboard.h>
#include <BleMouse.h>

BleKeyboard keyboard("My ESP32");
BleMouse mouse("My ESP32");

void setup() {
  keyboard.begin();
  mouse.begin();
}

void loop() {
  if (keyboard.isConnected()) {
    keyboard.print("Hello from ESP32\n");
    keyboard.write(KEY_MEDIA_VOLUME_UP);
    mouse.move(20, 0);
    mouse.click(MOUSE_LEFT);
  }
  delay(3000);
}
```

## API

**BleKeyboard** (extends `Print`): `begin()`, `isConnected()`, `print()` / `write(c)`, `press(key)`,
`release(key)`, `releaseAll()`, `write(KEY_MEDIA_...)`, `setBatteryLevel(level)`.
Key codes follow the Arduino Keyboard library: ASCII characters (US layout), `KEY_LEFT_CTRL`,
`KEY_LEFT_SHIFT`, `KEY_LEFT_ALT`, `KEY_LEFT_GUI`, `KEY_RETURN`, `KEY_ESC`, `KEY_BACKSPACE`, `KEY_TAB`,
`KEY_F1`–`KEY_F12`, arrows (`KEY_UP_ARROW`...), `KEY_HOME`, `KEY_END`, `KEY_DELETE`...
Media: `KEY_MEDIA_MUTE`, `KEY_MEDIA_VOLUME_UP`, `KEY_MEDIA_VOLUME_DOWN`, `KEY_MEDIA_PLAY_PAUSE`,
`KEY_MEDIA_NEXT_TRACK`, `KEY_MEDIA_PREVIOUS_TRACK`.

**BleMouse**: `begin()`, `isConnected()`, `move(x, y, wheel)`, `press(b)`, `release(b)`, `click(b)`,
`isPressed(b)` with `MOUSE_LEFT`, `MOUSE_RIGHT`, `MOUSE_MIDDLE`, `MOUSE_BACK`, `MOUSE_FORWARD`.

The device name and manufacturer come from whichever object calls `begin()` first.
