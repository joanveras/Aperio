#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include "Screen.hpp"
#include "MenuItem.hpp"

class MainMenuScreen : public Screen
{
public:
  explicit MainMenuScreen(Arduino_GFX* displayInstance);

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void update() override;
  void render() override;

private:
  Arduino_GFX* display;

  int selectedIndex;
  int firstVisibleItem;
  bool needsRedraw;

  void moveSelection(int direction);
  void openSelectedItem();

  void drawHeader();
  void drawMenuItems();
  void drawMenuItem(int index, int y, bool selected);
};