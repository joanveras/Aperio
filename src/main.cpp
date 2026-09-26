#include <Arduino.h>
#include <Adafruit_ILI9341.h>

#include "AperioApp.hpp"
#include "Config.hpp"

Adafruit_ILI9341 tft(
  TFT_CS,
  TFT_DC,
  TFT_RST
);

AperioApp app(
  &tft,
  BTN_LEFT,
  BTN_SELECT,
  BTN_RIGHT
);

void setup()
{
  Serial.begin(115200);

  app.begin();
}

void loop()
{
  app.update();
}
