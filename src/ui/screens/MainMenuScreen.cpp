#include "../../../include/ui/screens/MainMenuScreen.hpp"

namespace
{
  constexpr MenuItem mainMenuItems[] = {
    {"Wi-Fi",     ScreenId::WIFI_MENU},
    {"Bluetooth", ScreenId::BLUETOOTH_MENU},
    {"System",   ScreenId::SYSTEM_INFO},
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
  display->fillScreen(ILI9341_BLACK);

  display->setTextWrap(false);

  display->setTextSize(2);
  display->setTextColor(ILI9341_WHITE);
  display->setCursor(10, 10);
  display->print("APERIO");

  display->setTextSize(1);
  display->setCursor(286, 14);
  display->print("v0.1");

  display->drawFastHLine(
    8,
    36,
    display->width() - 16,
    ILI9341_WHITE
  );
}

void MainMenuScreen::drawMenuItems()
{
  constexpr int MENU_START_Y = 58;
  constexpr int ITEM_SPACING = 32;

  for (int i = 0; i < MENU_ITEM_COUNT; i++)
  {
    int y = MENU_START_Y + (i * ITEM_SPACING);

    drawMenuItem(
      i,
      y,
      i == selectedIndex
    );
  }

  display->drawFastHLine(
    8,
    199,
    display->width() - 16,
    ILI9341_WHITE
  );

  // Rodapé
  display->setTextSize(1);
  display->setTextColor(ILI9341_WHITE);

  display->setCursor(12, 216);
  display->print("< PREV");

  display->setCursor(154, 216);
  display->print("OK");

  display->setCursor(272, 216);
  display->print("NEXT >");
}

void MainMenuScreen::drawMenuItem(int index, int y, bool selected)
{
  constexpr int ITEM_X = 12;
  constexpr int ITEM_WIDTH = 296;
  constexpr int ITEM_HEIGHT = 27;
  constexpr int TEXT_X = 20;

  if (selected)
  {
    display->fillRect(
      ITEM_X,
      y - 5,
      ITEM_WIDTH,
      ITEM_HEIGHT,
      ILI9341_WHITE
    );

    display->setTextColor(
      ILI9341_BLACK,
      ILI9341_WHITE
    );
  }
  else
  {
    display->setTextColor(
      ILI9341_WHITE,
      ILI9341_BLACK
    );
  }

  display->setTextSize(2);
  display->setCursor(TEXT_X, y);

  display->print(mainMenuItems[index].label);
}
