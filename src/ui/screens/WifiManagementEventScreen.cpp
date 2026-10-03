#include "ui/screens/WifiManagementEventScreen.hpp"

#include "wifi/WifiManagementUtils.hpp"

WifiManagementEventScreen::WifiManagementEventScreen(
  Adafruit_ILI9341* displayInstance
)
  : display(displayInstance)
{
}

void WifiManagementEventScreen::setEvent(
  const WifiManagementEvent& eventInstance
)
{
  event = eventInstance;
  needsRedraw = true;
}

void WifiManagementEventScreen::onEnter()
{
  needsRedraw = true;
}

void WifiManagementEventScreen::handleInput(
  InputEvent eventInput
)
{
  // BACK is handled globally by ScreenManager.
  (void)eventInput;
}

void WifiManagementEventScreen::update()
{
}

void WifiManagementEventScreen::render()
{
  if (!needsRedraw || display == nullptr)
  {
    return;
  }

  display->fillScreen(ILI9341_BLACK);
  display->setTextWrap(false);

  drawHeader();

  if (event.valid)
  {
    drawEvent();
  }
  else
  {
    drawCentered(
      "NO EVENT AVAILABLE",
      105,
      2,
      ILI9341_WHITE
    );
  }

  drawFooter();

  needsRedraw = false;
}

void WifiManagementEventScreen::drawHeader()
{
  display->setTextSize(2);
  display->setTextColor(
    ILI9341_WHITE,
    ILI9341_BLACK
  );

  display->setCursor(10, 10);
  display->print("MANAGEMENT EVENT");

  display->drawFastHLine(
    8,
    36,
    display->width() - 16,
    ILI9341_WHITE
  );
}

void WifiManagementEventScreen::drawEvent()
{
  constexpr int16_t LABEL_X = 28;
  constexpr int16_t VALUE_X = 108;

  WifiManagementDirection direction =
    getManagementEventDirection(event);

  display->setTextSize(1);
  display->setTextColor(
    ILI9341_WHITE,
    ILI9341_BLACK
  );

  // Type
  display->setCursor(LABEL_X, 46);
  display->print("Type");

  display->setCursor(VALUE_X, 46);
  display->print(
    wifiManagementEventTypeToString(
      event.type
    )
  );

  // Direction
  display->setCursor(LABEL_X, 60);
  display->print("Direction");

  display->setCursor(VALUE_X, 60);
  display->print(
    wifiManagementDirectionToString(
      direction
    )
  );

  // Reason
  display->setCursor(LABEL_X, 76);
  display->print("Reason");

  display->setCursor(VALUE_X, 76);
  display->print(
    event.reasonCode
  );

  display->setCursor(132, 76);
  display->print(
    wifiReasonCodeToString(
      event.reasonCode
    )
  );

  // RSSI
  display->setCursor(LABEL_X, 96);
  display->print("RSSI");

  display->setCursor(VALUE_X, 96);
  display->print(
    event.rssi
  );
  display->print(" dBm");

  // Network channel
  display->setCursor(LABEL_X, 110);
  display->print("Network CH");

  display->setCursor(VALUE_X, 110);

  if (event.networkChannelKnown)
  {
    display->print(
      event.networkChannel
    );
  }
  else
  {
    display->print("--");
  }

  // Received channel
  display->setCursor(LABEL_X, 124);
  display->print("Received CH");

  display->setCursor(VALUE_X, 124);
  display->print(
    event.receivedChannel
  );

  // MAC addresses
  drawMac(
    "SRC",
    event.source,
    140
  );

  drawMac(
    "DST",
    event.destination,
    154
  );

  drawMac(
    "BSSID",
    event.bssid,
    168
  );
}

void WifiManagementEventScreen::drawMac(
  const char* label,
  const uint8_t* mac,
  int16_t y
)
{
  if (label == nullptr || mac == nullptr)
  {
    return;
  }

  constexpr int16_t LABEL_X = 28;
  constexpr int16_t VALUE_X = 108;

  char macBuffer[18];

  snprintf(
    macBuffer,
    sizeof(macBuffer),
    "%02X:%02X:%02X:%02X:%02X:%02X",
    mac[0],
    mac[1],
    mac[2],
    mac[3],
    mac[4],
    mac[5]
  );

  display->setTextSize(1);
  display->setTextColor(
    ILI9341_WHITE,
    ILI9341_BLACK
  );

  display->setCursor(
    LABEL_X,
    y
  );

  display->print(label);

  display->setCursor(
    VALUE_X,
    y
  );

  display->print(
    macBuffer
  );
}

void WifiManagementEventScreen::drawFooter()
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

void WifiManagementEventScreen::drawCentered(
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
    color
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

  int16_t x =
    (
      display->width() -
      width
    ) / 2;

  display->setCursor(
    x,
    y
  );

  display->print(
    text
  );
}
