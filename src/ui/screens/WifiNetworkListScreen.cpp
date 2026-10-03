#include "ui/screens/WifiNetworkListScreen.hpp"

WifiNetworkListScreen::WifiNetworkListScreen(
  Adafruit_ILI9341* displayInstance,
  WifiScanner* scannerInstance,
  std::function<void(ScreenId)> navigationCallback
)
  : display(displayInstance),
    scanner(scannerInstance),
    navigationCallback(navigationCallback)
{
}

void WifiNetworkListScreen::setTitle(const char* title)
{
  if (title == nullptr)
  {
    return;
  }

  this->title = title;
  needsRedraw = true;
}

void WifiNetworkListScreen::resetSelection()
{
  selectedIndex = 0;
  firstVisibleItem = 0;

  needsRedraw = true;
}

void WifiNetworkListScreen::onEnter()
{
  if (scanner == nullptr)
  {
    return;
  }

  size_t networkCount =scanner->getNetworkCount();

  if (networkCount == 0 || selectedIndex >= networkCount)
  {
    selectedIndex = 0;
    firstVisibleItem = 0;
  }

  needsRedraw = true;
}

void WifiNetworkListScreen::handleInput(InputEvent event)
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
      openSelectedNetwork();
      break;

    default:
      break;
  }
}

void WifiNetworkListScreen::update()
{
}

void WifiNetworkListScreen::render()
{
  if (!needsRedraw || display == nullptr || scanner == nullptr)
  {
    return;
  }

  drawHeader();
  drawNetworks();
  drawFooter();

  needsRedraw = false;
}

void WifiNetworkListScreen::moveSelection(int direction)
{
  if (scanner == nullptr)
  {
    return;
  }

  size_t networkCount = scanner->getNetworkCount();

  if (networkCount == 0)
  {
    return;
  }

  if (direction < 0)
  {
    if (selectedIndex == 0)
    {
      selectedIndex = networkCount - 1;
    }
    else
    {
      selectedIndex--;
    }
  }
  else
  {
    selectedIndex++;

    if (selectedIndex >= networkCount)
    {
      selectedIndex = 0;
    }
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

  needsRedraw = true;
}

void WifiNetworkListScreen::openSelectedNetwork()
{
  if (scanner == nullptr || scanner->getNetworkCount() == 0)
  {
    return;
  }

  if (navigationCallback)
  {
    navigationCallback(
      ScreenId::WIFI_NETWORK_DETAILS
    );
  }
}

const WifiNetwork* WifiNetworkListScreen::getSelectedNetwork() const
{
  if (scanner == nullptr)
  {
    return nullptr;
  }

  size_t networkCount = scanner->getNetworkCount();

  if (networkCount == 0 || selectedIndex >= networkCount)
  {
    return nullptr;
  }

  return &scanner->getNetwork(selectedIndex);
}

void WifiNetworkListScreen::drawHeader()
{
  display->fillScreen(
    ILI9341_BLACK
  );

  display->setTextWrap(false);

  display->setTextSize(2);
  display->setTextColor(
    ILI9341_WHITE
  );

  display->setCursor(10, 10);
  display->print(title);

  size_t networkCount = scanner->getNetworkCount();

  char counter[24];

  if (networkCount == 0)
  {
    snprintf(counter, sizeof(counter), "0/0 AP");
  }
  else
  {
    snprintf(
      counter,
      sizeof(counter),
      "%u/%u AP",
      static_cast<unsigned>(
        selectedIndex + 1
      ),
      static_cast<unsigned>(
        networkCount
      )
    );
  }

  display->setTextSize(1);

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
    display->width() - width - 10,
    14
  );

  display->print(counter);

  display->drawFastHLine(
    8,
    36,
    display->width() - 16,
    ILI9341_WHITE
  );
}

void WifiNetworkListScreen::drawNetworks()
{
  size_t networkCount = scanner->getNetworkCount();

  if (networkCount == 0)
  {
    drawCentered(
      "NO NETWORKS AVAILABLE",
      105,
      2,
      ILI9341_WHITE
    );

    return;
  }

  constexpr int START_Y = 52;
  constexpr int ITEM_SPACING = 26;

  size_t lastVisible = firstVisibleItem + VISIBLE_ITEM_COUNT;

  if (lastVisible > networkCount)
  {
    lastVisible = networkCount;
  }

  int visibleIndex = 0;

  for (size_t i = firstVisibleItem; i < lastVisible; i++)
  {
    int y = START_Y + (visibleIndex * ITEM_SPACING);

    drawNetworkItem(i, y, i == selectedIndex);

    visibleIndex++;
  }
}

void WifiNetworkListScreen::drawNetworkItem(
  size_t index,
  int y,
  bool selected
)
{
  constexpr int ITEM_X = 12;
  constexpr int ITEM_WIDTH = 296;
  constexpr int ITEM_HEIGHT = 22;

  const WifiNetwork& network = scanner->getNetwork(index);

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

  display->setTextSize(1);

  String ssid = network.ssid.isEmpty() ?
    "-- hidden --" : network.ssid;

  constexpr size_t MAX_SSID_LENGTH = 21;

  if (ssid.length() > MAX_SSID_LENGTH)
  {
    ssid = ssid.substring(0, MAX_SSID_LENGTH - 3) + "...";
  }

  display->setCursor(20, y);
  display->print(ssid);

  char info[32];

  snprintf(
    info,
    sizeof(info),
    "%ld dBm  CH %u",
    static_cast<long>(
      network.rssi
    ),
    static_cast<unsigned>(
      network.channel
    )
  );

  display->setCursor(190, y);
  display->print(info);
}

void WifiNetworkListScreen::drawFooter()
{
  display->drawFastHLine(
    8,
    184,
    display->width() - 16,
    ILI9341_WHITE
  );

  display->setTextSize(1);
  display->setTextColor(
    ILI9341_WHITE
  );

  display->setCursor(12, 198);
  display->print("< PREV");

  display->setCursor(154, 198);
  display->print("OK");

  display->setCursor(272, 198);
  display->print("NEXT >");

  drawCentered(
    "HOLD OK : BACK",
    220,
    1,
    ILI9341_WHITE
  );
}

void WifiNetworkListScreen::drawCentered(
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

  display->setTextSize(textSize);
  display->setTextColor(color);

  display->getTextBounds(
    text,
    0,
    0,
    &x1,
    &y1,
    &width,
    &height
  );

  int16_t x =
    (display->width() - width) / 2;

  display->setCursor(x, y);
  display->print(text);
}
