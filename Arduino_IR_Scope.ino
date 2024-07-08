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
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(getProtocolString(IrReceiver.decodedIRData.protocol));
    display.print(" ");
    display.print(IrReceiver.decodedIRData.address, HEX);
    display.print(" ");
    display.print(IrReceiver.decodedIRData.command, HEX);
    display.display();

    displayWaveform(IrReceiver.decodedIRData.rawDataPtr->rawbuf, IrReceiver.decodedIRData.rawlen);

    Serial.print("Protocol: ");
    Serial.print(getProtocolString(IrReceiver.decodedIRData.protocol));
    Serial.print(" Address: 0x");
    Serial.print(IrReceiver.decodedIRData.address, HEX);
    Serial.print(" Command: 0x");
    Serial.print(IrReceiver.decodedIRData.command, HEX);
    Serial.println();

    IrReceiver.printIRResultRawFormatted(&Serial, true);
    IrReceiver.printIRResultShort(&Serial);
    IrReceiver.printIRSendUsage(&Serial);
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    IrReceiver.resume();
  }
  delay(10);
}

void displayWaveform(IRRawbufType* rawbuf, int rawlen) {
  int yOffset = 5; // offset to avoid the text top
  display.drawLine(0, yOffset, 0, SCREEN_HEIGHT - 1, SSD1306_WHITE); 
  display.drawLine(SCREEN_WIDTH - 1, yOffset, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);
  display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);

  int x = 0;
  int rowHeight = (SCREEN_HEIGHT - yOffset) / 4; // split into 4 rows
  int currentRow = 0;
  int yTop = yOffset + currentRow * rowHeight + 4;
  int yBottom = yOffset + currentRow * rowHeight + rowHeight - 4;

  float scaleFactor = 0.41; // adjust for scale

  for (int i = 0; i < rawlen; i++) {
    int length = rawbuf[i] * scaleFactor;

    if (i % 2 == 0) {
      // On
      display.drawLine(x, yTop, x + length, yTop, SSD1306_WHITE);
      if (i < rawlen - 1) {
        display.drawLine(x + length, yTop, x + length, yBottom, SSD1306_WHITE);
      }
    } else {
      // Off
      display.drawLine(x, yBottom, x + length, yBottom, SSD1306_WHITE);
      if (i < rawlen - 1) {
        display.drawLine(x + length, yBottom, x + length, yTop, SSD1306_WHITE);
      }
    }

    x += length;
    if (x >= SCREEN_WIDTH) {
      x = 0;
      currentRow++;
      if (currentRow >= 4) {
        break; // stop drawing if we run out of vertical space
      }
      yTop = yOffset + currentRow * rowHeight + 4;
      yBottom = yOffset + currentRow * rowHeight + rowHeight - 6;
    }
  }

  display.display();
}
