#include "../../../include/ui/screens/MainMenuScreen.hpp"
#include "ui/CarouselMenu.hpp"

namespace
{
  constexpr MenuItem mainMenuItems[] = {
    {"Wi-Fi",     ScreenId::WIFI_MENU},
    {"Bluetooth", ScreenId::BLUETOOTH_MENU},
    {"System",    ScreenId::SYSTEM_INFO},
    {"About",     ScreenId::ABOUT}
  };

  constexpr int MENU_ITEM_COUNT = sizeof(mainMenuItems) / sizeof(mainMenuItems[0]);
}

MainMenuScreen::MainMenuScreen(
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

void MainMenuScreen::onEnter()
{
  needsFrame = true;
  needsRedraw = true;
}

void MainMenuScreen::handleInput(InputEvent event)
{
  if (event == InputEvent::PREVIOUS)
    moveSelection(-1);
  else if (event == InputEvent::NEXT)
    moveSelection(1);
  else if (event == InputEvent::SELECT)
    openSelectedItem();
}

void MainMenuScreen::update()
{
}

void MainMenuScreen::render()
{
  if (!needsRedraw)
  {
    return;
  }

  if (needsFrame)
  {
    CarouselMenu::drawFrame(display, "APERIO", "-QUOD LATET-");
    needsFrame = false;
  }

  CarouselMenu::drawItems(display, mainMenuItems, MENU_ITEM_COUNT, selectedIndex);

  needsRedraw = false;
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

void MainMenuScreen::openSelectedItem()
{
  ScreenId destination = mainMenuItems[selectedIndex].destination;

  if (navigationCallback)
  {
    navigationCallback(destination);
  }
}
