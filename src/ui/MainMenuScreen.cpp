#include "ui/MainMenuScreen.hpp"

namespace
{
  constexpr MenuItem mainMenuItems[] = {
    {"Wi-Fi",     ScreenId::WIFI_MENU},
    {"Bluetooth", ScreenId::BLUETOOTH_MENU},
    {"Sistema",   ScreenId::SYSTEM_INFO},
    {"Sobre",     ScreenId::ABOUT}
  };

  constexpr int MENU_ITEM_COUNT =
    sizeof(mainMenuItems) / sizeof(mainMenuItems[0]);
}

MainMenuScreen::MainMenuScreen(Arduino_GFX* displayInstance)
  : display(displayInstance),
    selectedIndex(0),
    firstVisibleItem(0),
    needsRedraw(true)
{
}

void MainMenuScreen::moveSelection(int direction)
{
  selectedIndex += direction;

  if (selectedIndex < 0)
  {
    selectedIndex = MENU_ITEM_COUNT - 1;
  }

  if (selectedIndex >= MENU_ITEM_COUNT)
  {
    selectedIndex = 0;
  }

  needsRedraw = true;
}

void MainMenuScreen::render()
{
  if (!needsRedraw)
  {
    return;
  }

  drawHeader();
  drawMenuItems();

  needsRedraw = false;
}