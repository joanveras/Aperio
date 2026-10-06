#pragma once

#include <Adafruit_ILI9341.h>
#include <functional>

#include "../Screen.hpp"
#include "../MenuItem.hpp"
#include "../../wifi/WifiScanner.hpp"

class WifiNetworkListScreen : public Screen
{
public:
  WifiNetworkListScreen(
    Adafruit_ILI9341* displayInstance,
    WifiScanner* scannerInstance,
    std::function<void(ScreenId)> navigationCallback
  );

  void setTitle(const char* title);
  void resetSelection();

  void setSelectDestination(ScreenId destination);

  const WifiNetwork* getSelectedNetwork() const;

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void update() override;
  void render() override;

private:
  static constexpr size_t VISIBLE_ITEM_COUNT = 6;

  Adafruit_ILI9341* display;
  WifiScanner* scanner;

  std::function<void(ScreenId)> navigationCallback;

  const char* title = "NETWORKS";

  ScreenId selectDestination = ScreenId::WIFI_NETWORK_DETAILS;

  size_t selectedIndex = 0;
  size_t firstVisibleItem = 0;

  bool needsRedraw = true;

  void moveSelection(int direction);
  void openSelectedNetwork();

  void drawHeader();
  void drawNetworks();

  void drawNetworkItem(
    size_t index,
    int y,
    bool selected
  );

  void drawFooter();

  void drawCentered(
    const char* text,
    int16_t y,
    uint8_t textSize,
    uint16_t color
  );
};
