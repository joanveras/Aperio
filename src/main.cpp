#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_ILI9341.h>

// For the Adafruit shield, these are the default.
#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 27

constexpr uint8_t BTN_LEFT   = 25;
constexpr uint8_t BTN_SELECT = 26;
constexpr uint8_t BTN_RIGHT  = 32;

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
// If using the breakout, change pins as desired
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

void printCentered(const char *text, int16_t y, uint8_t textSize, uint16_t color);
void showTitle();

void setup()
{
  Serial.begin(115200);

  Serial.println("Serial ready!");

  // ESP32 default is 40MHz, unreliable over breadboard jumper wires; slow down for signal integrity.
  tft.begin(8000000);
  tft.setRotation(1);

  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
  delay(500);

  showTitle();

  // Testing push buttons
  pinMode(25, INPUT_PULLUP);
  pinMode(26, INPUT_PULLUP);
  pinMode(32, INPUT_PULLUP);
}

void loop()
{
  if (digitalRead(BTN_LEFT) == LOW)
  {
    Serial.println("LEFT pressionado");
    delay(200);
  }

  if (digitalRead(BTN_SELECT) == LOW)
  {
    Serial.println("SELECT pressionado");
    delay(200);
  }

  if (digitalRead(BTN_RIGHT) == LOW)
  {
    Serial.println("RIGHT pressionado");
    delay(200);
  }
}

void printCentered(const char *text, int16_t y, uint8_t textSize, uint16_t color)
{
  int16_t x1, y1;
  uint16_t w, h;

  tft.setTextSize(textSize);
  tft.setTextColor(color);
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  int16_t x = (tft.width() - w) / 2;
  tft.setCursor(x, y);
  tft.print(text);
}

void showTitle()
{
  tft.fillScreen(ILI9341_BLACK);

  printCentered("Aperio", 90, 4, ILI9341_WHITE);
  printCentered("Quod Latet", 140, 2, ILI9341_WHITE);
}
