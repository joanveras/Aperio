#include "ui/screens/WifiChannelScreen.hpp"
#include "ui/UiStyle.hpp"

WifiChannelScreen::WifiChannelScreen(
  Adafruit_ILI9341* displayInstance,
  WifiScanner* scannerInstance,
  std::function<void(ScreenId)> navigationCallback
)
  : display(displayInstance),
    scanner(scannerInstance),
    navigationCallback(navigationCallback)
{
}

uint8_t WifiChannelScreen::getSelectedChannel() const
{
  return selectedChannel;
}

void WifiChannelScreen::onEnter()
{
  buildChannelStats();

  needsRedraw = true;
}

void WifiChannelScreen::handleInput(
  InputEvent event
)
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
      openSelectedChannel();
      break;

    default:
      break;
  }
}

void WifiChannelScreen::update()
{
}

void WifiChannelScreen::render()
{
  if (
    !needsRedraw || display == nullptr || scanner == nullptr)
  {
    return;
  }

  drawHeader();
  drawChannels();
  drawFooter();

  needsRedraw = false;
}

void WifiChannelScreen::buildChannelStats()
{
  for (
    uint8_t channel = MIN_CHANNEL; channel <= MAX_CHANNEL; channel++
  )
  {
    WifiChannelStats& stats = channelStats[channel - MIN_CHANNEL];

    stats.channel = channel;
    stats.networkCount = 0;
    stats.strongestRssi = -127;
  }

  if (scanner == nullptr)
  {
    return;
  }

  size_t networkCount = scanner->getNetworkCount();

  for (size_t i = 0; i < networkCount; i++)
  {
    const WifiNetwork& network = scanner->getNetwork(i);

    if (
      network.channel < MIN_CHANNEL || network.channel > MAX_CHANNEL
    )
    {
      continue;
    }

    WifiChannelStats& stats =
      channelStats[network.channel - MIN_CHANNEL];

    stats.networkCount++;

    if (network.rssi > stats.strongestRssi)
    {
      stats.strongestRssi = network.rssi;
    }
  }
}

void WifiChannelScreen::moveSelection(
  int direction
)
{
  if (direction < 0)
  {
    if (selectedChannel == MIN_CHANNEL)
    {
      selectedChannel = MAX_CHANNEL;
    }
    else
    {
      selectedChannel--;
    }
  }
  else
  {
    if (selectedChannel == MAX_CHANNEL)
    {
      selectedChannel = MIN_CHANNEL;
    }
    else
    {
      selectedChannel++;
    }
  }

  if (selectedChannel < firstVisibleChannel)
  {
    firstVisibleChannel = selectedChannel;
  }
  else if (
    selectedChannel >= firstVisibleChannel + VISIBLE_ITEM_COUNT
  )
  {
    firstVisibleChannel =
      selectedChannel - VISIBLE_ITEM_COUNT + 1;
  }

  /*
    Handle wrap-around.
  */
  if (selectedChannel == MIN_CHANNEL)
  {
    firstVisibleChannel = MIN_CHANNEL;
  }
  else if (
    selectedChannel == MAX_CHANNEL
  )
  {
    firstVisibleChannel = MAX_CHANNEL - VISIBLE_ITEM_COUNT + 1;
  }

  needsRedraw = true;
}

void WifiChannelScreen::openSelectedChannel()
{
  if (navigationCallback)
  {
    navigationCallback(
      ScreenId::WIFI_MONITOR
    );
  }
}

void WifiChannelScreen::drawHeader()
{
  display->fillScreen(
    UiColor::BACKGROUND
  );

  UiStyle::drawTitle(display, "CHANNELS");

  display->setTextSize(1);
  display->setTextColor(
    UiColor::TEXT_MUTED,
    UiColor::BACKGROUND
  );

  display->setCursor(
    display->width() - 42,
    14
  );

  display->print("2.4G");

}

void WifiChannelScreen::drawChannels()
{
  constexpr int16_t START_Y = 50;
  constexpr int16_t ITEM_SPACING = 25;

  uint8_t lastVisibleChannel =
    firstVisibleChannel + VISIBLE_ITEM_COUNT - 1;

  if (
    lastVisibleChannel > MAX_CHANNEL
  )
  {
    lastVisibleChannel = MAX_CHANNEL;
  }

  int visibleIndex = 0;

  for (
    uint8_t channel = firstVisibleChannel;
    channel <= lastVisibleChannel;
    channel++
  )
  {
    int16_t y = START_Y + (visibleIndex * ITEM_SPACING);

    drawChannelItem(
      channel,
      y,
      channel == selectedChannel
    );

    visibleIndex++;
  }
}

void WifiChannelScreen::drawChannelItem(
  uint8_t channel,
  int16_t y,
  bool selected
)
{
  constexpr int16_t ITEM_X = 12;
  constexpr int16_t ITEM_WIDTH = 296;
  constexpr int16_t ITEM_HEIGHT = 22;

  WifiChannelStats& stats = channelStats[channel - MIN_CHANNEL];

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

  char channelText[12];

  snprintf(
    channelText,
    sizeof(channelText),
    "CH %u",
    static_cast<unsigned>(
      channel
    )
  );

  display->setCursor(
    20,
    y
  );

  display->print(
    channelText
  );

  char networkText[16];

  snprintf(
    networkText,
    sizeof(networkText),
    "%u AP",
    static_cast<unsigned>(
      stats.networkCount
    )
  );

  if (!selected)
  {
    display->setTextColor(
      stats.networkCount == 0 ? UiColor::TEXT_DIM : UiColor::TEXT_MUTED,
      UiColor::BACKGROUND
    );
  }

  display->setCursor(
    110,
    y
  );

  display->print(
    networkText
  );

  display->setCursor(
    210,
    y
  );

  if (stats.networkCount == 0)
  {
    display->print("--");
  }
  else
  {
    display->print(
      stats.strongestRssi
    );

    display->print(
      " dBm"
    );
  }
}

void WifiChannelScreen::drawFooter()
{
  UiStyle::drawFooter(display);
}
