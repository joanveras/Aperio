#pragma once

#include <Adafruit_ILI9341.h>
#include <functional>

#include "../Screen.hpp"
#include "../MenuItem.hpp"
#include "../../wifi/WifiMonitor.hpp"
#include "../../wifi/WifiManagementEvent.hpp"

class WifiManagementEventsScreen : public Screen
{
public:
  WifiManagementEventsScreen(
    Adafruit_ILI9341* displayInstance,
    WifiMonitor* monitorInstance,
    std::function<void(ScreenId)> navigationCallback
  );

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void update() override;
  void render() override;

  bool getSelectedEvent(
    WifiManagementEvent& event
  ) const;

private:
  static constexpr size_t VISIBLE_ITEM_COUNT = 6;

  Adafruit_ILI9341* display;
  WifiMonitor* monitor;

  std::function<void(ScreenId)> navigationCallback;

  size_t selectedIndex = 0;
  size_t firstVisibleItem = 0;

  bool needsRedraw = true;

  void moveSelection(int direction);
  void openSelectedEvent();

  void drawHeader();
  void drawEvents();

  void drawEventItem(
    size_t index,
    int16_t y,
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
