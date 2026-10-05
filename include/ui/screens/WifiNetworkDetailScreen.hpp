#pragma once

#include <Adafruit_ILI9341.h>

#include "../Screen.hpp"
#include "../../wifi/WifiNetwork.hpp"

class WifiNetworkDetailScreen : public Screen
{
public:
  explicit WifiNetworkDetailScreen(
    Adafruit_ILI9341* displayInstance
  );

  void setNetwork(const WifiNetwork& networkInstance);

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void update() override;
  void render() override;

private:
  Adafruit_ILI9341* display;
  WifiNetwork network;
  bool hasNetwork = false;

  bool needsRedraw = true;

  void drawHeader();
  void drawDetails();
  void drawFooter();

  void drawCentered(
    const char* text,
    int16_t y,
    uint8_t textSize,
    uint16_t color
  );
};
