#include "ui/screens/WifiMonitorMenuScreen.hpp"
#include "ui/CarouselMenu.hpp"

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
  needsFrame = true;
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
  if (!needsRedraw)
  {
    return;
  }

  if (needsFrame)
  {
    CarouselMenu::drawFrame(display, "MONITOR", nullptr);
    needsFrame = false;
  }

  CarouselMenu::drawItems(display, monitorMenuItems, MENU_ITEM_COUNT, selectedIndex);

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
