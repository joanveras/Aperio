#include "../../../include/ui/screens/WifiMenuScreen.hpp"
#include "ui/CarouselMenu.hpp"

namespace
{
  constexpr MenuItem mainMenuItems[] = {
    {"Scan",       ScreenId::WIFI_SCAN},
    {"Networks",   ScreenId::WIFI_NETWORKS},
    {"Channels",   ScreenId::WIFI_CHANNELS},
    {"Monitor",    ScreenId::WIFI_MONITOR_MENU},
    {"TX Lab",     ScreenId::WIFI_TX_SELECT}
  };

  constexpr int MENU_ITEM_COUNT = sizeof(mainMenuItems) / sizeof(mainMenuItems[0]);
}

WifiMenuScreen::WifiMenuScreen(
  Adafruit_ILI9341* displayInstance,
  NavigationCallback navigationCallback
)
  : display(displayInstance),
    navigationCallback(navigationCallback),
    selectedIndex(0),
    needsRedraw(true),
    needsFrame(true)
{
}

void WifiMenuScreen::onEnter()
{
  needsFrame = true;
  needsRedraw = true;
}

void WifiMenuScreen::handleInput(InputEvent event)
{
  if (event == InputEvent::PREVIOUS)
    moveSelection(-1);
  else if (event == InputEvent::NEXT)
    moveSelection(1);
  else if (event == InputEvent::SELECT)
    openSelectedItem();
}

void WifiMenuScreen::update()
{
}

void WifiMenuScreen::render()
{
  if (!needsRedraw)
  {
    return;
  }

  if (needsFrame)
  {
    CarouselMenu::drawFrame(display, "WI-FI", nullptr, true);
    needsFrame = false;
  }

  CarouselMenu::drawItems(display, mainMenuItems, MENU_ITEM_COUNT, selectedIndex);

  needsRedraw = false;
}

void WifiMenuScreen::moveSelection(int direction)
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

void WifiMenuScreen::openSelectedItem()
{
  ScreenId destination = mainMenuItems[selectedIndex].destination;

  if (navigationCallback)
  {
    navigationCallback(destination);
  }
}
