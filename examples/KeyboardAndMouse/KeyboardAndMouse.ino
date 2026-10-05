// Types a line, nudges the volume and moves the mouse every 3 seconds while a host is connected.
#include <BleKeyboard.h>
#include <BleMouse.h>

BleKeyboard keyboard("OMS HID Demo");
BleMouse mouse("OMS HID Demo");

void setup() {
  Serial.begin(115200);
  keyboard.begin();
  mouse.begin();
  Serial.println("Pair with \"OMS HID Demo\" from your computer or phone.");
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
