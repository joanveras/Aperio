#include "../../../include/ui/screens/WifiMenuScreen.hpp"
#include "ui/UiStyle.hpp"

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

  if (selectedIndex < firstVisibleItem)
  {
    firstVisibleItem = selectedIndex;
  }
  else if (
    selectedIndex >= firstVisibleItem + VISIBLE_ITEM_COUNT
  )
  {
    firstVisibleItem = selectedIndex - VISIBLE_ITEM_COUNT + 1;
  }

  if (selectedIndex == 0)
  {
    firstVisibleItem = 0;
  }

  if (selectedIndex == MENU_ITEM_COUNT - 1)
  {
    firstVisibleItem = MENU_ITEM_COUNT - VISIBLE_ITEM_COUNT;

    if (firstVisibleItem < 0)
    {
      firstVisibleItem = 0;
    }
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
  display->fillScreen(UiColor::BACKGROUND);

  UiStyle::drawTitle(display, "WI-FI");
}

void WifiMenuScreen::drawMenuItems()
{
  constexpr int MENU_START_Y = 52;
  constexpr int ITEM_SPACING = 30;

  int lastVisibleItem = firstVisibleItem + VISIBLE_ITEM_COUNT;

  if (lastVisibleItem > MENU_ITEM_COUNT)
  {
    lastVisibleItem = MENU_ITEM_COUNT;
  }

  int visibleIndex = 0;

  for (int i = firstVisibleItem; i < lastVisibleItem; i++)
  {
    int y = MENU_START_Y + (visibleIndex * ITEM_SPACING);

    drawMenuItem(
      i,
      y,
      i == selectedIndex
    );

    visibleIndex++;
  }

  UiStyle::drawDivider(display, 184);

  display->setTextSize(1);
  display->setTextColor(
    UiColor::TEXT_MUTED,
    UiColor::BACKGROUND
  );

  display->setCursor(12, 198);
  display->print("< PREV");

  display->setCursor(154, 198);
  display->print("OK");

  display->setCursor(272, 198);
  display->print("NEXT >");

  const char* backText = "HOLD OK : BACK";

  int16_t x1;
  int16_t y1;

  uint16_t width;
  uint16_t height;

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

  display->setTextColor(
    UiColor::TEXT_DIM,
    UiColor::BACKGROUND
  );

  display->setCursor(
    x,
    220
  );

  display->print(
    backText
  );
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
    UiStyle::drawSelection(
      display,
      ITEM_X,
      y - 5,
      ITEM_WIDTH,
      ITEM_HEIGHT
    );
  }

  UiStyle::text(
    display,
    UiFont::ITEM,
    TEXT_X,
    y,
    selected ? UiColor::TEXT : UiColor::TEXT_MUTED,
    mainMenuItems[index].label
  );
}
