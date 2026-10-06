#include "ui/screens/TxLabScreen.hpp"
#include "ui/UiStyle.hpp"

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
    UiColor::BACKGROUND
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
  UiStyle::drawTitle(display, "TX LAB");

  display->setTextSize(1);
  display->setTextColor(
    UiColor::TEXT_MUTED,
    UiColor::BACKGROUND
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


}

void TxLabScreen::drawContent()
{
  constexpr int LABEL_X = 20;
  constexpr int VALUE_X = 92;

  display->setTextSize(1);

  /*
   * Network
  */
  display->setCursor(
    LABEL_X,
    50
  );

  display->setTextColor(UiColor::TEXT_MUTED, UiColor::BACKGROUND);
  display->print(
    "Network"
  );

  String ssid = targetNetwork.ssid;

  constexpr size_t MAX_SSID_LENGTH = 30;

  if (ssid.length() > MAX_SSID_LENGTH)
  {
    ssid = ssid.substring( 0, MAX_SSID_LENGTH - 3) + "...";
  }

  display->setTextColor(UiColor::TEXT, UiColor::BACKGROUND);
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

  display->setTextColor(UiColor::TEXT_MUTED, UiColor::BACKGROUND);
  display->print(
    "BSSID"
  );

  display->setTextColor(UiColor::TEXT, UiColor::BACKGROUND);
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

  display->setTextColor(UiColor::TEXT_MUTED, UiColor::BACKGROUND);
  display->print(
    "Payload"
  );

  display->setTextColor(UiColor::TEXT, UiColor::BACKGROUND);
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

    display->setTextColor(UiColor::TEXT_DIM, UiColor::BACKGROUND);
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
    UiColor::TEXT_MUTED,
    UiColor::BACKGROUND
  );

  display->setCursor(
    LABEL_X,
    138
  );

  display->print(
    "Status"
  );

  display->setTextColor(statusColor(), UiColor::BACKGROUND);
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

  display->setTextColor(UiColor::TEXT_MUTED, UiColor::BACKGROUND);
  display->print(
    "Sent"
  );

  display->setTextColor(UiColor::TEXT, UiColor::BACKGROUND);
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

  display->setTextColor(UiColor::TEXT_MUTED, UiColor::BACKGROUND);
  display->print(
    "Failed"
  );

  display->setTextColor(UiColor::TEXT, UiColor::BACKGROUND);
  display->setCursor(
    235,
    160
  );

  display->print(
    stats.failedFrames
  );
}

uint16_t TxLabScreen::statusColor() const
{
  if (hasSendResult && !lastSendSucceeded)
  {
    return UiColor::DANGER;
  }

  if (hasSendResult && lastSendSucceeded)
  {
    return UiColor::ACCENT;
  }

  return UiColor::TEXT;
}

void TxLabScreen::drawReasonCode(int16_t y, bool selected)
{
  constexpr int ITEM_X = 12;
  constexpr int ITEM_WIDTH = 296;
  constexpr int ITEM_HEIGHT = 20;

  constexpr int LABEL_X = 20;
  constexpr int VALUE_X = 92;

  const WifiReasonCode& reason = getSelectedReasonCode();

  if (selected)
  {
    UiStyle::drawSelection(
      display,
      ITEM_X,
      y,
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
      UiColor::ACCENT,
      UiColor::BACKGROUND
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
    UiColor::TEXT,
    UiColor::BACKGROUND
  );
}

void TxLabScreen::drawFooter()
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
    UiColor::ACCENT
  );

  // drawCentered() left the accent color set; NEXT matches PREV.
  display->setTextColor(
    UiColor::TEXT_MUTED,
    UiColor::BACKGROUND
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

void TxLabScreen::drawError()
{
  if (invalidTarget)
  {
    drawCentered(
      "INVALID TARGET",
      88,
      2,
      UiColor::DANGER
    );

    if (hasTarget && targetNetwork.ssid.isEmpty())
    {
      drawCentered(
        "HIDDEN SSID NOT SUPPORTED",
        120,
        1,
        UiColor::TEXT_MUTED
      );
    }
    else
    {
      drawCentered(
        "SELECT A VALID NETWORK",
        120,
        1,
        UiColor::TEXT_MUTED
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
      UiColor::DANGER
    );

    drawCentered(
      "FAILED TO INITIALIZE",
      120,
      1,
      UiColor::TEXT_MUTED
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
