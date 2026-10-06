#include "ui/screens/WifiManagementEventsScreen.hpp"

#include "wifi/WifiManagementUtils.hpp"
#include "ui/UiStyle.hpp"

WifiManagementEventsScreen::WifiManagementEventsScreen(
  Adafruit_ILI9341* displayInstance,
  WifiMonitor* monitorInstance,
  std::function<void(ScreenId)> navigationCallback
)
  : display(displayInstance),
    monitor(monitorInstance),
    navigationCallback(navigationCallback)
{
}

void WifiManagementEventsScreen::onEnter()
{
  if (monitor == nullptr)
  {
    return;
  }

  size_t eventCount = monitor->getManagementEventCount();

  if (eventCount == 0 || selectedIndex >= eventCount)
  {
    selectedIndex = 0;
    firstVisibleItem = 0;
  }

  needsRedraw = true;
}

void WifiManagementEventsScreen::handleInput(InputEvent event)
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
      openSelectedEvent();
      break;

    default:
      break;
  }
}

void WifiManagementEventsScreen::update()
{
}

void WifiManagementEventsScreen::render()
{
  if (!needsRedraw || display == nullptr || monitor == nullptr)
  {
    return;
  }

  display->fillScreen(
    UiColor::BACKGROUND
  );

  display->setTextWrap(false);

  drawHeader();
  drawEvents();
  drawFooter();

  needsRedraw = false;
}

bool WifiManagementEventsScreen::getSelectedEvent(
  WifiManagementEvent& event
) const
{
  if (monitor == nullptr)
  {
    return false;
  }

  return monitor->getManagementEvent(selectedIndex, event);
}

void WifiManagementEventsScreen::moveSelection(int direction)
{
  if (monitor == nullptr)
  {
    return;
  }

  size_t eventCount = monitor->getManagementEventCount();

  if (eventCount == 0)
  {
    return;
  }

  if (direction < 0)
  {
    if (selectedIndex == 0)
    {
      selectedIndex = eventCount - 1;
    }
    else
    {
      selectedIndex--;
    }
  }
  else
  {
    selectedIndex++;

    if (selectedIndex >= eventCount)
    {
      selectedIndex = 0;
    }
  }

  if (selectedIndex < firstVisibleItem)
  {
    firstVisibleItem = selectedIndex;
  }
  else if (selectedIndex >= firstVisibleItem + VISIBLE_ITEM_COUNT)
  {
    firstVisibleItem =
      selectedIndex - VISIBLE_ITEM_COUNT + 1;
  }

  /*
   * Handle wrap-around.
  */
  if (selectedIndex == 0)
  {
    firstVisibleItem = 0;
  }
  else if (selectedIndex == eventCount - 1 &&
    eventCount > VISIBLE_ITEM_COUNT
  )
  {
    firstVisibleItem = eventCount - VISIBLE_ITEM_COUNT;
  }

  needsRedraw = true;
}

void WifiManagementEventsScreen::openSelectedEvent()
{
  if (monitor == nullptr)
  {
    return;
  }

  if (
    monitor->getManagementEventCount() == 0
  )
  {
    return;
  }

  if (navigationCallback)
  {
    navigationCallback(
      ScreenId::WIFI_MANAGEMENT_EVENT
    );
  }
}

void WifiManagementEventsScreen::drawHeader()
{
  UiStyle::drawTitle(display, "MANAGEMENT EVENTS");

  size_t eventCount = monitor->getManagementEventCount();

  char counter[16];

  if (eventCount == 0)
  {
    snprintf(counter, sizeof(counter), "0/0");
  }
  else
  {
    snprintf(
      counter,
      sizeof(counter),
      "%u/%u",
      static_cast<unsigned>(selectedIndex + 1),
      static_cast<unsigned>(eventCount)
    );
  }

  display->setTextSize(1);

  display->setTextColor(
    UiColor::TEXT_MUTED,
    UiColor::BACKGROUND
  );

  int16_t x1;
  int16_t y1;

  uint16_t width;
  uint16_t height;

  display->getTextBounds(
    counter,
    0,
    0,
    &x1,
    &y1,
    &width,
    &height
  );

  display->setCursor(
    display->width() -
    width -
    10,
    14
  );

  display->print(counter);

}

void WifiManagementEventsScreen::drawEvents()
{
  size_t eventCount = monitor->getManagementEventCount();

  if (eventCount == 0)
  {
    drawCentered(
      "NO EVENTS CAPTURED",
      105,
      2,
      UiColor::TEXT_MUTED
    );

    return;
  }

  constexpr int16_t START_Y = 52;
  constexpr int16_t ITEM_SPACING = 26;

  size_t lastVisibleItem = firstVisibleItem + VISIBLE_ITEM_COUNT;

  if (lastVisibleItem > eventCount)
  {
    lastVisibleItem = eventCount;
  }

  int visibleIndex = 0;

  for (size_t i = firstVisibleItem; i < lastVisibleItem; i++)
  {
    int16_t y = START_Y + (visibleIndex * ITEM_SPACING);

    drawEventItem(
      i,
      y,
      i == selectedIndex
    );

    visibleIndex++;
  }
}

void WifiManagementEventsScreen::drawEventItem(
  size_t index,
  int16_t y,
  bool selected
)
{
  constexpr int16_t ITEM_X = 12;
  constexpr int16_t ITEM_WIDTH = 296;
  constexpr int16_t ITEM_HEIGHT = 22;

  WifiManagementEvent event;

  if (!monitor->getManagementEvent(index, event)
  )
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

  display->setTextSize(1);

  /*
   * Event type
  */
  display->setCursor(
    20,
    y
  );

  display->print(
    wifiManagementEventTypeToString(
      event.type
    )
  );

  /*
   * Reason Code
  */
  display->setCursor(
    82,
    y
  );

  display->print("R");
  display->print(
    event.reasonCode
  );

  /*
   * Direction
  */
  WifiManagementDirection direction =
    getManagementEventDirection(
      event
    );

  display->setCursor(
    118,
    y
  );

  display->print(
    wifiManagementDirectionToString(
      direction
    )
  );

  /*
   * RSSI
  */
  display->setCursor(
    244,
    y
  );

  display->print(
    event.rssi
  );

  display->print(
    "dBm"
  );
}

void WifiManagementEventsScreen::drawFooter()
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
    272,
    198
  );

  display->print(
    "NEXT >"
  );

  UiStyle::drawFooterAction(display, "OK", 198);

  drawCentered(
    "HOLD OK : BACK",
    220,
    1,
    UiColor::TEXT_DIM
  );
}

void WifiManagementEventsScreen::drawCentered(
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

  display->print(text);
}
