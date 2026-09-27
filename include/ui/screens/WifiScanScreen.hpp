#pragma once

#include <Adafruit_ILI9341.h>
#include <functional>

#include "../Screen.hpp"
#include "../MenuItem.hpp"
#include "../../wifi/WifiScanner.hpp"

class WifiScanScreen : public Screen
{
public:
  WifiScanScreen(
    Adafruit_ILI9341* displayInstance,
    WifiScanner* scannerInstance,
    std::function<void(ScreenId)> navigationCallback
  );

  void startScan();

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void update() override;
  void render() override;

  const WifiNetwork* getSelectedNetwork() const;

private:
  static constexpr size_t VISIBLE_ITEM_COUNT = 5;

  Adafruit_ILI9341* display;
  WifiScanner* scanner;

  std::function<void(ScreenId)> navigationCallback;

  size_t selectedIndex;
  size_t firstVisibleItem;

  bool needsRedraw;

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
    int y,
    uint8_t textSize,
    uint16_t color
  );
};
