#include "ui/screens/WifiMonitorMenuScreen.hpp"

namespace
{
  constexpr MenuItem monitorMenuItems[] = {
    {
      "Passive Monitor",
      ScreenId::WIFI_MONITOR
    },
    {
      "Management Events",
      ScreenId::WIFI_MANAGEMENT_EVENTS
    },
    {
      "Active Survey",
      ScreenId::WIFI_ACTIVE_SURVEY
    }
  };
}

WifiMonitorMenuScreen::WifiMonitorMenuScreen(
  Adafruit_ILI9341* displayInstance,
  std::function<void(ScreenId)> navigationCallback
)
  : display(displayInstance),
    navigationCallback(navigationCallback)
{
}

void WifiMonitorMenuScreen::onEnter()
{
  needsRedraw = true;
}

void WifiMonitorMenuScreen::handleInput(InputEvent event)
{
  switch (event)
  {
    case InputEvent::PREVIOUS:
      moveSelection(-1);
      break;

    case InputEvent::NEXT:
      moveSelection(1);
      break;

    case InputEvent::SELECT:
      selectCurrentItem();
      break;

    default:
      break;
  }
}

void WifiMonitorMenuScreen::update()
{
}

void WifiMonitorMenuScreen::render()
{
  if (!needsRedraw || display == nullptr)
  {
    return;
  }

  display->fillScreen(
    ILI9341_BLACK
  );

  display->setTextWrap(false);

  drawHeader();
  drawMenuItems();
  drawFooter();

  needsRedraw = false;
}

void WifiMonitorMenuScreen::moveSelection(int direction)
{
  if (direction < 0)
  {
    if (selectedIndex == 0)
    {
      selectedIndex =
        MENU_ITEM_COUNT - 1;
    }
    else
    {
      selectedIndex--;
    }
  }
  else
  {
    selectedIndex++;

    if (selectedIndex >= MENU_ITEM_COUNT)
    {
      selectedIndex = 0;
    }
  }

  needsRedraw = true;
}

void WifiMonitorMenuScreen::selectCurrentItem()
{
  if (selectedIndex >= MENU_ITEM_COUNT)
  {
    return;
  }

  if (navigationCallback)
  {
    navigationCallback(monitorMenuItems[selectedIndex].destination);
  }
}

void WifiMonitorMenuScreen::drawHeader()
{
  display->setTextSize(2);

  display->setTextColor(
    ILI9341_WHITE,
    ILI9341_BLACK
  );

  display->setCursor(
    10,
    10
  );

  display->print(
    "MONITOR"
  );

  display->drawFastHLine(
    8,
    36,
    display->width() - 16,
    ILI9341_WHITE
  );
}

void WifiMonitorMenuScreen::drawMenuItems()
{
  constexpr int16_t START_Y = 62;
  constexpr int16_t ITEM_SPACING = 34;

  for (size_t i = 0; i < MENU_ITEM_COUNT; i++)
  {
    int16_t y = START_Y + (static_cast<int16_t>(i) *ITEM_SPACING);

    drawMenuItem(
      i,
      y,
      i == selectedIndex
    );
  }
}

void WifiMonitorMenuScreen::drawMenuItem(
  size_t index,
  int16_t y,
  bool selected
)
{
  constexpr int16_t ITEM_X = 20;
  constexpr int16_t ITEM_WIDTH = 280;
  constexpr int16_t ITEM_HEIGHT = 26;

  if (index >= MENU_ITEM_COUNT)
  {
    return;
  }

  if (selected)
  {
    display->fillRect(
      ITEM_X,
      y - 6,
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

  display->setTextSize(1);

  display->setCursor(
    32,
    y
  );

  display->print(
    monitorMenuItems[index].label
  );
}

void WifiMonitorMenuScreen::drawFooter()
{
  display->drawFastHLine(
    8,
    184,
    display->width() - 16,
    ILI9341_WHITE
  );

  display->setTextSize(1);

  display->setTextColor(
    ILI9341_WHITE,
    ILI9341_BLACK
  );

  display->setCursor(
    12,
    198
  );

  display->print(
    "< PREV"
  );

  display->setCursor(
    154,
    198
  );

  display->print(
    "OK"
  );

  display->setCursor(
    272,
    198
  );

  display->print(
    "NEXT >"
  );

  drawCentered(
    "HOLD OK : BACK",
    220,
    1,
    ILI9341_WHITE
  );
}

void WifiMonitorMenuScreen::drawCentered(
  const char* text,
  int16_t y,
  uint8_t textSize,
  uint16_t color
)
{
  int16_t x1;
  int16_t y1;

  uint16_t width;
  uint16_t height;

  display->setTextSize(
    textSize
  );

  display->setTextColor(
    color,
    ILI9341_BLACK
  );

  display->getTextBounds(
    text,
    0,
    0,
    &x1,
    &y1,
    &width,
    &height
  );

  int16_t x = (display->width() - width) / 2;

  display->setCursor(
    x,
    y
  );

  display->print(
    text
  );
}
