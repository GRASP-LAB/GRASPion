/*
  HelloGRASPion.ino

  Quick demonstration of the GRASPion Body:
  - USB serial console
  - unique 5-character robot ID
  - ambient light sensor
  - MLX90393 magnetometer
  - board power information from the AVR32DD20
  - infrared communication
  - Head UART
  - NeoPixels
  - vibration motors
*/

#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_MLX90393.h>

Adafruit_NeoPixel leds(NUM_NEOPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
Adafruit_MLX90393 mag;

char id[6];
bool blink = false;

void setup() {
  Serial.begin(115200);
  SerialHead.begin(115200);
  SerialIR.begin(2400);

  Wire.begin();
  leds.begin();
  mag.begin_I2C(MLX90393_I2C_ADDRESS);

  pinMode(BUZ_LEFT, OUTPUT);
  pinMode(BUZ_RIGHT, OUTPUT);

  getGRASPionID(id);   // 5-char URL-safe Base64 ID from the SAMD21 UID
}

void loop() {
  // Sensors
  float x, y, z;
  mag.readData(&x, &y, &z);

  uint16_t light = analogRead(LIGHT_SENSOR);

  // Board power data from AVR32DD20
  Wire.requestFrom(0x36, 5);

  uint16_t vbus = (Wire.read() << 8) | Wire.read();
  uint16_t vbat = (Wire.read() << 8) | Wire.read();
  bool usbPowered = vbus >= 4500;

  Wire.read();

  // USB console
  Serial.print("GRASPion #");
  Serial.print(id);

  Serial.print(" | Light=");
  Serial.print(light);

  Serial.print(" | B[uT]=(");
  Serial.print(x, 1);
  Serial.print(", ");
  Serial.print(y, 1);
  Serial.print(", ");
  Serial.print(z, 1);
  Serial.print(")");

  Serial.print(" | VBUS=");
  Serial.print(vbus);
  Serial.print(" mV");

  Serial.print(" | VBAT=");
  Serial.print(vbat);
  Serial.print(" mV");

  Serial.print(" | USB=");
  Serial.println(usbPowered ? "yes" : "no");

  // Send robot ID through IR and Head UART
  SerialIR.println(id);
  SerialHead.println(id);

  // NeoPixels
  uint8_t brightness = constrain(
    map(light, 5, 200, 5, 255),
    5, 255
  );

  leds.setBrightness(brightness);

  leds.setPixelColor(
    NEOPIXEL_BOTTOM,
    leds.Color(0, 80, 255)
  );

  blink = !blink;

  leds.setPixelColor(
    NEOPIXEL_TOP,
    blink ? leds.Color(50, 5, 20) : 0
  );

  leds.show();

  // Clear self-received IR data
  while (SerialIR.available())
    SerialIR.read();

  // Small motor demo
  digitalWrite(BUZ_ENABLE, HIGH);

  analogWrite(BUZ_LEFT, 200);
  delay(120);
  analogWrite(BUZ_LEFT, 127);

  analogWrite(BUZ_RIGHT, 200);
  delay(120);
  analogWrite(BUZ_RIGHT, 127);

  digitalWrite(BUZ_ENABLE, LOW);

  delay(500);
}
