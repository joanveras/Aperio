#include "ui/screens/WifiMonitorMenuScreen.hpp"
#include "ui/UiStyle.hpp"
#include "Config.hpp"

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
  UiStyle::drawTitleTag(display, APERIO_VERSION);
}

void WifiMonitorMenuScreen::drawMenuItems()
{
  constexpr int16_t START_Y = 60;
  constexpr int16_t ITEM_SPACING = 32;

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
  constexpr int16_t ITEM_HEIGHT = 25;
  constexpr uint16_t ITEM_COLOR = UiColor::rgb(70, 124, 84);

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

  // 1.5x text, vertically centred in the row; dark green unless selected.
  UiStyle::drawLargeText(
    display,
    monitorMenuItems[index].label,
    20,
    y + 2,
    selected ? UiColor::TEXT : ITEM_COLOR,
    selected ? UiColor::SELECTION : UiColor::BACKGROUND
  );
}

void WifiMonitorMenuScreen::drawFooter()
{
  UiStyle::drawFooter(display);
}
