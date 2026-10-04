#pragma once

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <functional>

#include "ui/Screen.hpp"
#include "ui/MenuItem.hpp"

using NavigationCallback = std::function<void(ScreenId)>;

class WifiMenuScreen : public Screen
{
public:
  explicit WifiMenuScreen(
    Adafruit_ILI9341* displayInstance,
    NavigationCallback navigationCallback
  );

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void update() override;
  void render() override;

private:
  static constexpr int VISIBLE_ITEM_COUNT = 4;

  Adafruit_ILI9341* display;
  NavigationCallback navigationCallback;

  int selectedIndex;
  int firstVisibleItem;
  bool needsRedraw;

  void moveSelection(int direction);
  void openSelectedItem();

  void drawHeader();
  void drawMenuItems();
  void drawMenuItem(int index, int y, bool selected);
};
