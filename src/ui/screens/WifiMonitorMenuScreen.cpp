#include "ui/screens/WifiMonitorMenuScreen.hpp"
#include "ui/UiStyle.hpp"

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
    UiColor::BACKGROUND
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
  UiStyle::drawTitle(display, "MONITOR");
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
  constexpr int16_t ITEM_X = 12;
  constexpr int16_t ITEM_WIDTH = 296;
  constexpr int16_t ITEM_HEIGHT = 27;

  if (index >= MENU_ITEM_COUNT)
  {
    return;
  }

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
    20,
    y,
    selected ? UiColor::TEXT : UiColor::TEXT_MUTED,
    monitorMenuItems[index].label
  );
}

void WifiMonitorMenuScreen::drawFooter()
{
  UiStyle::drawDivider(display, 184);

  display->setTextSize(1);

  display->setTextColor(
    UiColor::TEXT_MUTED,
    UiColor::BACKGROUND
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
    UiColor::TEXT_DIM
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
    UiColor::BACKGROUND
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
