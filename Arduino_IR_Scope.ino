/*
  Arduino_IR_Scope
  by: hed0rah
  https://github.com/jrelo/Arduino_IR_Scope

  Connections:
  .91 inch OLED (32x128):
  GND -> GND
  VCC -> 5v
  SCK -> A5
  SDA -> A4
  
  IR Receiver:
  GND -> GND
  VCC -> 5v
  OUT -> 11
*/

#include <IRremote.hpp>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define IR_RECEIVE_PIN 11
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 failed"));
    for (;;);
  }
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();
}

void loop() {
  if (IrReceiver.decode()) {
    displayWaveform(IrReceiver.decodedIRData.rawDataPtr->rawbuf, IrReceiver.decodedIRData.rawDataPtr->rawlen);
    display.setCursor(4, 0);
    display.print(getProtocolString(IrReceiver.decodedIRData.protocol));
    display.print(" ");
    display.print(IrReceiver.decodedIRData.address, HEX);
    display.print(" ");
    display.print(IrReceiver.decodedIRData.command, HEX);
    display.print(" ");
    display.print(IrReceiver.decodedIRData.decodedRawData, HEX);
    display.display();
    IrReceiver.printIRResultRawFormatted(&Serial, true);
    IrReceiver.printIRResultShort(&Serial);
    IrReceiver.printIRSendUsage(&Serial);
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    IrReceiver.resume(); // Enable receiving of the next value
  }
  delay(10);
}

void displayWaveform(unsigned char* rawbuf, int rawlen) {
  display.clearDisplay();
  display.drawLine(0, 0, 0, SCREEN_HEIGHT - 1, SSD1306_WHITE); 
  display.drawLine(SCREEN_WIDTH - 1, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);
  display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);

  int totalDuration = 0;
  for (int i = 0; i < rawlen; i++) {
    totalDuration += rawbuf[i];
  }

  float scaleFactor = (float)totalDuration / (SCREEN_WIDTH * 4); // scale for pulse duration
  int rowHeight = (SCREEN_HEIGHT - 8) / 4;  // height of each row
  int midY = rowHeight / 2; // midpoint
  int x = 1;
  int yOffset = 1;
  unsigned long accumulatedTime = 0;

  for (int i = 0; i < rawlen && yOffset < SCREEN_HEIGHT; i++) {
    accumulatedTime += rawbuf[i];
    int newX = (int)(accumulatedTime / scaleFactor) % SCREEN_WIDTH;
    if (newX >= SCREEN_WIDTH) {
      newX = SCREEN_WIDTH - 1;
    }
    if (newX < x) { // move to next row if at end
      yOffset += rowHeight;
      x = 1;
    }
    if (i % 2 == 0) {
      // high pulse
      display.drawLine(x, yOffset + midY, x, yOffset, SSD1306_WHITE);
    } else {
      // low pulse, shorter line
      display.drawLine(x, yOffset + midY, x, yOffset + rowHeight / 4 + 3, SSD1306_WHITE);
    }
    x = newX + 1;
  }
  display.display();
}
