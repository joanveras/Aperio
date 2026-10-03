#include "ui/screens/WifiNetworkDetailScreen.hpp"
#include "wifi/WifiUtils.hpp"

WifiNetworkDetailScreen::WifiNetworkDetailScreen(
  Adafruit_ILI9341* displayInstance
)
  : display(displayInstance)
{
}

void WifiNetworkDetailScreen::setNetwork(
  const WifiNetwork* networkInstance
)
{
  network = networkInstance;
  needsRedraw = true;
}

void WifiNetworkDetailScreen::onEnter()
{
  needsRedraw = true;
}

void WifiNetworkDetailScreen::handleInput(
  InputEvent event
)
{
  // BACK is handled by ScreenManager.
  // SELECT will be used later to open the Monitor.
  (void)event;
}

void WifiNetworkDetailScreen::update()
{
}

void WifiNetworkDetailScreen::render()
{
  if (!needsRedraw || display == nullptr)
  {
    return;
  }

  display->fillScreen(ILI9341_BLACK);
  display->setTextWrap(false);

  drawHeader();

  if (network == nullptr)
  {
    drawCentered(
      "NO NETWORK SELECTED",
      105,
      2,
      ILI9341_WHITE
    );
  }
  else
  {
    drawDetails();
  }

  drawFooter();

  needsRedraw = false;
}

void WifiNetworkDetailScreen::drawHeader()
{
  display->setTextSize(2);
  display->setTextColor(ILI9341_WHITE);

  display->setCursor(10, 10);
  display->print("NETWORK DETAILS");

  display->drawFastHLine(
    8,
    36,
    display->width() - 16,
    ILI9341_WHITE
  );
}

void WifiNetworkDetailScreen::drawDetails()
{
  if (network == nullptr)
  {
    return;
  }

  display->setTextSize(1);
  display->setTextColor(
    ILI9341_WHITE,
    ILI9341_BLACK
  );

  // SSID
  display->setCursor(20, 52);
  display->print("SSID");

  display->setCursor(20, 66);

  if (network->ssid.isEmpty())
  {
    display->print("-- hidden --");
  }
  else
  {
    String ssid = network->ssid;

    constexpr size_t MAX_SSID_LENGTH = 42;

    if (ssid.length() > MAX_SSID_LENGTH)
    {
      ssid =
        ssid.substring(
          0,
          MAX_SSID_LENGTH - 3
        ) + "...";
    }

    display->print(ssid);
  }

  // BSSID
  display->setCursor(20, 90);
  display->print("BSSID");

  display->setCursor(20, 104);
  display->print(network->bssid);

  // RSSI
  display->setCursor(20, 130);
  display->print("RSSI");

  display->setCursor(110, 130);
  display->print(network->rssi);
  display->print(" dBm");

  // Channel
  display->setCursor(20, 148);
  display->print("Channel");

  display->setCursor(110, 148);
  display->print(network->channel);

  // Security
  display->setCursor(20, 166);
  display->print("Security");

  display->setCursor(110, 166);
  display->print(
    wifiAuthModeToString(
      network->security
    )
  );
}

void WifiNetworkDetailScreen::drawFooter()
{
  display->drawFastHLine(
    8,
    194,
    display->width() - 16,
    ILI9341_WHITE
  );

  drawCentered(
    "HOLD OK : BACK",
    216,
    1,
    ILI9341_WHITE
  );
}

void WifiNetworkDetailScreen::drawCentered(
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
