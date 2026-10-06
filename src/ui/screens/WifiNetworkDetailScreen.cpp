#include "ui/screens/WifiNetworkDetailScreen.hpp"
#include "wifi/WifiUtils.hpp"
#include "ui/UiStyle.hpp"

WifiNetworkDetailScreen::WifiNetworkDetailScreen(
  Adafruit_ILI9341* displayInstance
)
  : display(displayInstance)
{
}

void WifiNetworkDetailScreen::setNetwork(
  const WifiNetwork& networkInstance
)
{
  network = networkInstance;
  hasNetwork = true;

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

  display->fillScreen(UiColor::BACKGROUND);
  display->setTextWrap(false);

  drawHeader();

  if (!hasNetwork)
  {
    drawCentered(
      "NO NETWORK SELECTED",
      105,
      2,
      UiColor::TEXT_MUTED
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
  UiStyle::drawTitle(display, "NETWORK DETAILS");
}

void WifiNetworkDetailScreen::drawDetails()
{
  if (!hasNetwork)
  {
    return;
  }

  display->setTextSize(1);

  // SSID
  display->setTextColor(UiColor::TEXT_MUTED, UiColor::BACKGROUND);
  display->setCursor(20, 46);
  display->print("SSID");

  display->setTextColor(UiColor::TEXT, UiColor::BACKGROUND);
  display->setCursor(20, 60);

  if (network.ssid.isEmpty())
  {
    display->print("-- hidden --");
  }
  else
  {
    String ssid = network.ssid;

    constexpr size_t MAX_SSID_LENGTH = 42;

    if (ssid.length() > MAX_SSID_LENGTH)
    {
      ssid = ssid.substring(0, MAX_SSID_LENGTH - 3) + "...";
    }

    display->print(ssid);
  }

  // BSSID
  display->setTextColor(UiColor::TEXT_MUTED, UiColor::BACKGROUND);
  display->setCursor(20, 82);
  display->print("BSSID");

  display->setTextColor(UiColor::TEXT, UiColor::BACKGROUND);
  display->setCursor(20, 96);
  display->print(network.bssid);

  // RSSI
  display->setTextColor(UiColor::TEXT_MUTED, UiColor::BACKGROUND);
  display->setCursor(20, 120);
  display->print("RSSI");

  display->setTextColor(UiColor::TEXT, UiColor::BACKGROUND);
  display->setCursor(110, 120);
  display->print(network.rssi);
  display->print(" dBm");

  // Channel
  display->setTextColor(UiColor::TEXT_MUTED, UiColor::BACKGROUND);
  display->setCursor(20, 138);
  display->print("Channel");

  display->setTextColor(UiColor::TEXT, UiColor::BACKGROUND);
  display->setCursor(110, 138);
  display->print(network.channel);

  // Security
  display->setTextColor(UiColor::TEXT_MUTED, UiColor::BACKGROUND);
  display->setCursor(20, 156);
  display->print("Security");

  display->setTextColor(UiColor::TEXT, UiColor::BACKGROUND);
  display->setCursor(110, 156);
  display->print(
    wifiAuthModeToString(
      network.security
    )
  );

  // PMF
  display->setTextColor(UiColor::TEXT_MUTED, UiColor::BACKGROUND);
  display->setCursor(20, 174);
  display->print("PMF");

  display->setTextColor(UiColor::TEXT, UiColor::BACKGROUND);
  display->setCursor(110, 174);
  display->print(
    wifiPmfModeToString(
      network.pmf
    )
  );
}

void WifiNetworkDetailScreen::drawFooter()
{
  UiStyle::drawFooter(display, nullptr, nullptr, nullptr);
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
