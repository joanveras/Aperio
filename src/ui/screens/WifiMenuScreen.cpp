#include "../../../include/ui/screens/WifiMenuScreen.hpp"

namespace
{
  constexpr MenuItem mainMenuItems[] = {
    {"Scan",       ScreenId::WIFI_SCAN},
    {"Networks",   ScreenId::WIFI_NETWORKS},
    {"Channels",   ScreenId::WIFI_CHANNELS},
    {"Monitor",    ScreenId::WIFI_MONITOR_MENU}
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
    firstVisibleItem(0),
    needsRedraw(true)
{
}

void WifiMenuScreen::onEnter()
{
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

  drawHeader();
  drawMenuItems();

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

void WifiMenuScreen::drawHeader()
{
  display->fillScreen(ILI9341_BLACK);
  display->setTextWrap(false);

  display->setTextSize(2);
  display->setTextColor(ILI9341_WHITE);
  display->setCursor(10, 10);
  display->print("WI-FI");

  display->drawFastHLine(
    8,
    36,
    display->width() - 16,
    ILI9341_WHITE
  );
}

void WifiMenuScreen::drawMenuItems()
{
  constexpr int MENU_START_Y = 52;
  constexpr int ITEM_SPACING = 30;

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
    184,
    display->width() - 16,
    ILI9341_WHITE
  );

  display->setTextSize(1);
  display->setTextColor(ILI9341_WHITE);

  display->setCursor(12, 198);
  display->print("< PREV");

  display->setCursor(154, 198);
  display->print("OK");

  display->setCursor(272, 198);
  display->print("NEXT >");

  const char* backText = "HOLD OK : BACK";

  int16_t x1, y1;
  uint16_t width, height;

  display->getTextBounds(
    backText,
    0,
    0,
    &x1,
    &y1,
    &width,
    &height
  );

  int16_t x = (display->width() - width) / 2;

  display->setCursor(x, 220);
  display->print(backText);
}

void WifiMenuScreen::drawMenuItem(
  int index,
  int y,
  bool selected
)
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
