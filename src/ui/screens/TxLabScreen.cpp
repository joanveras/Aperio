#include "ui/screens/TxLabScreen.hpp"

constexpr WifiReasonCode TxLabScreen::REASON_CODES[];

constexpr size_t TxLabScreen::REASON_CODE_COUNT;

TxLabScreen::TxLabScreen(
  Adafruit_ILI9341* displayInstance,
  WifiTransmitter* transmitterInstance
)
  : display(displayInstance),
    transmitter(transmitterInstance)
{
}

void TxLabScreen::setTarget(
  const WifiNetwork& network
)
{
  targetNetwork = network;
  hasTarget = true;

  needsRedraw = true;
}

void TxLabScreen::onEnter()
{
  transmitterStartFailed = false;
  invalidTarget = false;

  hasSendResult = false;
  lastSendSucceeded = false;

  selectedPayload = TxPayloadType::PROBE_REQUEST;

  mode = TxLabMode::PAYLOAD_SELECT;

  selectedReasonIndex = 2;

  if (transmitter == nullptr || !isTargetValid())
  {
    invalidTarget = true;
    needsRedraw = true;

    return;
  }

  if (!transmitter->begin())
  {
    transmitterStartFailed = true;
    needsRedraw = true;

    return;
  }

  if (!transmitter->setChannel(targetNetwork.channel))
  {
    transmitterStartFailed = true;
    needsRedraw = true;

    return;
  }

  transmitter->resetStats();

  refreshStats();

  needsRedraw = true;
}

void TxLabScreen::handleInput(InputEvent event)
{
  switch (event)
  {
    case InputEvent::PREVIOUS:
      handlePrevious();
      break;

    case InputEvent::NEXT:
      handleNext();
      break;

    case InputEvent::SELECT:
      handleSelect();
      break;

    default:
      break;
  }
}

void TxLabScreen::update()
{
}

void TxLabScreen::render()
{
  if (!needsRedraw || display == nullptr)
  {
    return;
  }

  drawScreen();

  needsRedraw = false;
}

void TxLabScreen::handlePrevious()
{
  if (invalidTarget || transmitterStartFailed)
  {
    return;
  }

  if (mode == TxLabMode::CONFIGURE)
  {
    moveReasonCode(-1);
  }
  else
  {
    /*
     * If we had already confirmed a
     * configuration, PREV/NEXT goes back
     * to the payload selection mode.
    */
    mode = TxLabMode::PAYLOAD_SELECT;

    movePayload(-1);
  }

  hasSendResult = false;
  needsRedraw = true;
}

void TxLabScreen::handleNext()
{
  if (invalidTarget || transmitterStartFailed)
  {
    return;
  }

  if (mode == TxLabMode::CONFIGURE)
  {
    moveReasonCode(1);
  }
  else
  {
    mode = TxLabMode::PAYLOAD_SELECT;

    movePayload(1);
  }

  hasSendResult = false;
  needsRedraw = true;
}

void TxLabScreen::handleSelect()
{
  if (
    transmitter == nullptr ||
    invalidTarget ||
    transmitterStartFailed ||
    !transmitter->isReady()
  )
  {
    return;
  }

  switch (mode)
  {
    case TxLabMode::PAYLOAD_SELECT:
    {
      if (payloadRequiresConfiguration())
      {
        enterConfiguration();
      }
      else
      {
        executeSelectedPayload();
      }

      break;
    }

    case TxLabMode::CONFIGURE:
      confirmConfiguration();
      break;

    case TxLabMode::READY:
      executeSelectedPayload();
      break;
  }

  needsRedraw = true;
}

void TxLabScreen::movePayload(int direction)
{
  int current = static_cast<int>(selectedPayload);

  current += direction;

  if (current < 0)
  {
    current = static_cast<int>(PAYLOAD_COUNT) - 1;
  }

  if (current >= static_cast<int>(PAYLOAD_COUNT))
  {
    current = 0;
  }

  selectedPayload = static_cast<TxPayloadType>(current);
}

void TxLabScreen::moveReasonCode(int direction)
{
  if (REASON_CODE_COUNT == 0)
  {
    return;
  }

  if (direction < 0)
  {
    if (selectedReasonIndex == 0)
    {
      selectedReasonIndex = REASON_CODE_COUNT - 1;
    }
    else
    {
      selectedReasonIndex--;
    }
  }
  else
  {
    selectedReasonIndex++;

    if (selectedReasonIndex >= REASON_CODE_COUNT)
    {
      selectedReasonIndex = 0;
    }
  }
}

void TxLabScreen::enterConfiguration()
{
  mode = TxLabMode::CONFIGURE;

  hasSendResult = false;
}

void TxLabScreen::confirmConfiguration()
{
  mode = TxLabMode::READY;

  hasSendResult = false;
}

void TxLabScreen::executeSelectedPayload()
{
  switch (selectedPayload)
  {
    case TxPayloadType::PROBE_REQUEST:
      executeProbeRequest();
      break;

    case TxPayloadType::DEAUTH_TEST:
      executeDeauthTest();
      break;
  }
}

void TxLabScreen::executeProbeRequest()
{
  if (transmitter == nullptr)
  {
    return;
  }

  lastSendSucceeded =
    transmitter->sendProbeRequest(targetNetwork.ssid.c_str());

  hasSendResult = true;

  refreshStats();
}

void TxLabScreen::executeDeauthTest()
{
  if (transmitter == nullptr || !transmitter->isReady())
  {
    hasSendResult = true;
    lastSendSucceeded = false;
    return;
  }

  uint8_t bssid[6];

  if (!WifiTransmitter::parseMac(targetNetwork.bssid, bssid))
  {
    hasSendResult = true;
    lastSendSucceeded = false;
    return;
  }

  const WifiReasonCode& reason = getSelectedReasonCode();

  /*
   * Deauth burst.
   *
   * 15 frames with 10 ms of
   * interval ≈ 150 ms of
   * continuous transmission.
   *
   * It is enough for the client
   * driver to process and
   * disassociate, without freezing the UI
   * for a perceptible amount of time.
  */
  constexpr uint8_t BURST_COUNT = 15;

  constexpr uint16_t INTER_FRAME_DELAY_MS = 10;

  uint8_t successCount = 0;

  for (uint8_t i = 0; i < BURST_COUNT; i++)
  {
    if (transmitter->sendDeauth(bssid, reason.code))
    {
      successCount++;
    }

    delay(INTER_FRAME_DELAY_MS);
  }

  lastSendSucceeded = (successCount == BURST_COUNT);

  hasSendResult = true;

  refreshStats();
}

void TxLabScreen::refreshStats()
{
  if (transmitter == nullptr)
  {
    return;
  }

  stats = transmitter->getStats();
}

bool TxLabScreen::isTargetValid() const
{
  if (!hasTarget)
  {
    return false;
  }

  if (targetNetwork.ssid.isEmpty())
  {
    return false;
  }

  if (targetNetwork.channel < 1 || targetNetwork.channel > 11)
  {
    return false;
  }

  return true;
}

bool TxLabScreen::payloadRequiresConfiguration() const
{
  switch (selectedPayload)
  {
    case TxPayloadType::DEAUTH_TEST:
      return true;

    case TxPayloadType::PROBE_REQUEST:
    default:
      return false;
  }
}

const char* TxLabScreen::getPayloadName() const
{
  switch (selectedPayload)
  {
    case TxPayloadType::PROBE_REQUEST:
      return "PROBE REQUEST";

    case TxPayloadType::DEAUTH_TEST:
      return "DEAUTH";

    default:
      return "UNKNOWN";
  }
}

const WifiReasonCode& TxLabScreen::getSelectedReasonCode() const
{
  return REASON_CODES[selectedReasonIndex];
}

void TxLabScreen::drawScreen()
{
  display->fillScreen(
    ILI9341_BLACK
  );

  display->setTextWrap(false);

  drawHeader();

  if (invalidTarget || transmitterStartFailed)
  {
    drawError();
  }
  else
  {
    drawContent();
  }

  drawFooter();
}

void TxLabScreen::drawHeader()
{
  display->setTextSize(2);

  display->setTextColor(
    ILI9341_WHITE
  );

  display->setCursor(
    10,
    10
  );

  display->print(
    "TX LAB"
  );

  if (
    hasTarget &&
    targetNetwork.channel >= 1 &&
    targetNetwork.channel <= 11
  )
  {
    char channelText[16];

    snprintf(
      channelText,
      sizeof(channelText),
      "CH %u",
      static_cast<unsigned>(
        targetNetwork.channel
      )
    );

    int16_t x1;
    int16_t y1;

    uint16_t width;
    uint16_t height;

    display->getTextBounds(
      channelText,
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
      10
    );

    display->print(
      channelText
    );
  }

  display->drawFastHLine(
    8,
    36,
    display->width() - 16,
    ILI9341_WHITE
  );
}

void TxLabScreen::drawContent()
{
  constexpr int LABEL_X = 14;
  constexpr int VALUE_X = 90;

  display->setTextSize(1);

  display->setTextColor(
    ILI9341_WHITE
  );

  /*
   * Network
  */
  display->setCursor(
    LABEL_X,
    50
  );

  display->print(
    "Network"
  );

  String ssid = targetNetwork.ssid;

  constexpr size_t MAX_SSID_LENGTH = 30;

  if (ssid.length() > MAX_SSID_LENGTH)
  {
    ssid = ssid.substring( 0, MAX_SSID_LENGTH - 3) + "...";
  }

  display->setCursor(
    VALUE_X,
    50
  );

  display->print(
    ssid
  );

  /*
   * BSSID
  */
  display->setCursor(
    LABEL_X,
    68
  );

  display->print(
    "BSSID"
  );

  display->setCursor(
    VALUE_X,
    68
  );

  display->print(
    targetNetwork.bssid
  );

  /*
   * Payload
  */
  display->setCursor(
    LABEL_X,
    90
  );

  display->print(
    "Payload"
  );

  display->setCursor(
    VALUE_X,
    90
  );

  display->print(
    getPayloadName()
  );

  /*
   * Reason Code
  */
  if (selectedPayload == TxPayloadType::DEAUTH_TEST)
  {
    drawReasonCode(
      108,
      mode == TxLabMode::CONFIGURE
    );
  }
  else
  {
    display->setCursor(
      LABEL_X,
      113
    );

    display->print(
      "Reason"
    );

    display->setCursor(
      VALUE_X,
      113
    );

    display->print(
      "--"
    );
  }

  /*
   * Status
  */
  display->setTextColor(
    ILI9341_WHITE,
    ILI9341_BLACK
  );

  display->setCursor(
    LABEL_X,
    138
  );

  display->print(
    "Status"
  );

  display->setCursor(
    VALUE_X,
    138
  );

  if (mode == TxLabMode::CONFIGURE)
  {
    display->print(
      "CONFIGURING"
    );
  }
  else if (mode == TxLabMode::READY && !hasSendResult)
  {
    display->print(
      "READY"
    );
  }
  else if (!hasSendResult)
  {
    display->print(
      "READY"
    );
  }
  else if (
    selectedPayload == TxPayloadType::DEAUTH_TEST && lastSendSucceeded
  )
  {
    display->print("DEAUTH SENT");
  }
  else if (lastSendSucceeded)
  {
    display->print(
      "SENT"
    );
  }
  else
  {
    display->print(
      "FAILED"
    );
  }

  /*
   * TX stats
  */
  display->setCursor(
    LABEL_X,
    160
  );

  display->print(
    "Sent"
  );

  display->setCursor(
    VALUE_X,
    160
  );

  display->print(
    stats.sentFrames
  );

  display->setCursor(
    180,
    160
  );

  display->print(
    "Failed"
  );

  display->setCursor(
    235,
    160
  );

  display->print(
    stats.failedFrames
  );
}

void TxLabScreen::drawReasonCode(int16_t y, bool selected)
{
  constexpr int ITEM_X = 10;
  constexpr int ITEM_WIDTH = 300;
  constexpr int ITEM_HEIGHT = 20;

  constexpr int LABEL_X = 14;
  constexpr int VALUE_X = 90;

  const WifiReasonCode& reason = getSelectedReasonCode();

  if (selected)
  {
    display->fillRect(
      ITEM_X,
      y,
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
    LABEL_X,
    y + 5
  );

  display->print(
    "Reason"
  );

  char reasonText[40];

  if (selected)
  {
    snprintf(
      reasonText,
      sizeof(reasonText),
      "< %u - %s >",
      static_cast<unsigned>(
        reason.code
      ),
      reason.label
    );
  }
  else
  {
    snprintf(
      reasonText,
      sizeof(reasonText),
      "%u - %s",
      static_cast<unsigned>(
        reason.code
      ),
      reason.label
    );
  }

  display->setCursor(
    VALUE_X,
    y + 5
  );

  display->print(
    reasonText
  );

  display->setTextColor(
    ILI9341_WHITE,
    ILI9341_BLACK
  );
}

void TxLabScreen::drawFooter()
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

  display->setCursor(
    12,
    198
  );

  display->print(
    "< PREV"
  );

  const char* centerText = "SEND";

  if (mode == TxLabMode::CONFIGURE)
  {
    centerText = "CONFIRM";
  }
  else if (selectedPayload == TxPayloadType::DEAUTH_TEST)
  {
    if (mode == TxLabMode::PAYLOAD_SELECT)
    {
      centerText = "CONFIG";
    }
    else
    {
      centerText = "TEST";
    }
  }

  drawCentered(
    centerText,
    198,
    1,
    ILI9341_WHITE
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

void TxLabScreen::drawError()
{
  if (invalidTarget)
  {
    drawCentered(
      "INVALID TARGET",
      88,
      2,
      ILI9341_WHITE
    );

    if (hasTarget && targetNetwork.ssid.isEmpty())
    {
      drawCentered(
        "HIDDEN SSID NOT SUPPORTED",
        120,
        1,
        ILI9341_WHITE
      );
    }
    else
    {
      drawCentered(
        "SELECT A VALID NETWORK",
        120,
        1,
        ILI9341_WHITE
      );
    }

    return;
  }

  if (transmitterStartFailed)
  {
    drawCentered(
      "TX ERROR",
      88,
      2,
      ILI9341_WHITE
    );

    drawCentered(
      "FAILED TO INITIALIZE",
      120,
      1,
      ILI9341_WHITE
    );
  }
}

void TxLabScreen::drawCentered(
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

  int16_t x = (display->width() - width) / 2;

  display->setCursor(
    x,
    y
  );

  display->print(
    text
  );
}
