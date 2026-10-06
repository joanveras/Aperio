#include "../../../include/ui/screens/MainMenuScreen.hpp"
#include "ui/UiStyle.hpp"
#include "Config.hpp"

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
    firstVisibleItem(0),
    needsRedraw(true)
{
}

void MainMenuScreen::onEnter()
{
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

  drawHeader();
  drawMenuItems();

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

void MainMenuScreen::drawHeader()
{
  display->fillScreen(UiColor::BACKGROUND);

  UiStyle::drawTitle(display, "APERIO");
  UiStyle::drawTitleTag(display, APERIO_VERSION);
}

void MainMenuScreen::drawMenuItems()
{
  constexpr int MENU_START_Y = 58;
  constexpr int ITEM_SPACING = 27;

  for (int i = 0; i < MENU_ITEM_COUNT; i++)
  {
    int y = MENU_START_Y + (i * ITEM_SPACING);

    drawMenuItem(
      i,
      y,
      i == selectedIndex
    );
  }

  UiStyle::drawMenuFooter(display);
}

void MainMenuScreen::drawMenuItem(int index, int y, bool selected)
{
  constexpr int ITEM_X = 12;
  constexpr int ITEM_WIDTH = 296;
  constexpr int ITEM_HEIGHT = 25;
  constexpr int TEXT_X = 20;

  if (selected)
  {
    UiStyle::drawSelection(
      display,
      ITEM_X,
      y - 5,
      ITEM_WIDTH,
      ITEM_HEIGHT
    );

    display->setTextColor(
      UiColor::TEXT,
      UiColor::SELECTION
    );
  }
  else
  {
    display->setTextColor(
      UiColor::TEXT_MUTED,
      UiColor::BACKGROUND
    );
  }

  display->setTextSize(2);
  display->setCursor(TEXT_X, y);

  display->print(mainMenuItems[index].label);
}
