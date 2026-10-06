#pragma once

#include <Adafruit_ILI9341.h>
#include <functional>

#include "../Screen.hpp"
#include "../MenuItem.hpp"

class WifiMonitorMenuScreen : public Screen
{
public:
  WifiMonitorMenuScreen(
    Adafruit_ILI9341* displayInstance,
    std::function<void(ScreenId)> navigationCallback
  );

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void update() override;
  void render() override;

private:
  static constexpr size_t MENU_ITEM_COUNT = 3;

  Adafruit_ILI9341* display;

  std::function<void(ScreenId)> navigationCallback;

  size_t selectedIndex = 0;

  bool needsRedraw = true;

  void moveSelection(int direction);
  void selectCurrentItem();

  void drawHeader();
  void drawMenuItems();

  void drawMenuItem(
    size_t index,
    int16_t y,
    bool selected
  );

  void drawFooter();
};
