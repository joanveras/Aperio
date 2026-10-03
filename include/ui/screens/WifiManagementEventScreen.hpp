#pragma once

#include <Adafruit_ILI9341.h>

#include "../Screen.hpp"
#include "../../wifi/WifiManagementEvent.hpp"

class WifiManagementEventScreen : public Screen
{
public:
  explicit WifiManagementEventScreen(
    Adafruit_ILI9341* displayInstance
  );

  void setEvent(
    const WifiManagementEvent& eventInstance
  );

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void update() override;
  void render() override;

private:
  Adafruit_ILI9341* display;

  WifiManagementEvent event;

  bool needsRedraw = true;

  void drawHeader();
  void drawEvent();
  void drawFooter();

  void drawMac(
    const char* label,
    const uint8_t* mac,
    int16_t y
  );

  void drawCentered(
    const char* text,
    int16_t y,
    uint8_t textSize,
    uint16_t color
  );
};
