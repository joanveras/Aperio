#include "ui/screens/WifiMonitorScreen.hpp"
#include "wifi/WifiManagementUtils.hpp"

WifiMonitorScreen::WifiMonitorScreen(
  Adafruit_ILI9341* displayInstance,
  WifiMonitor* monitorInstance
)
  : display(displayInstance),
    monitor(monitorInstance)
{
}

void WifiMonitorScreen::onEnter()
{
  if (monitor == nullptr)
  {
    return;
  }

  monitorStartFailed = !monitor->start(monitor->getChannel());

  stats = monitor->getStats();

  lastRefreshTime = millis();

  needsFullRedraw = true;
  needsHeaderRedraw = true;
  needsStatsRedraw = true;
  needsFooterRedraw = true;
}

void WifiMonitorScreen::onExit()
{
  if (monitor == nullptr)
  {
    return;
  }

  monitor->stop();
}

void WifiMonitorScreen::handleInput(
  InputEvent event
)
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

void WifiMonitorScreen::update()
{
  if (monitor == nullptr || monitorStartFailed)
  {
    return;
  }

  monitor->update();

  uint32_t now = millis();

  if (now - lastRefreshTime < REFRESH_INTERVAL)
  {
    return;
  }

  lastRefreshTime = now;

  stats = monitor->getStats();

  needsStatsRedraw = true;
}

void WifiMonitorScreen::render()
{
  if (display == nullptr)
  {
    return;
  }

  if (needsFullRedraw)
  {
    drawScreen();

    needsFullRedraw = false;
    needsHeaderRedraw = false;
    needsStatsRedraw = false;
    needsFooterRedraw = false;

    return;
  }

  if (needsHeaderRedraw)
  {
    clearHeaderArea();
    drawHeader();

    needsHeaderRedraw = false;
  }

  if (needsStatsRedraw)
  {
    clearStatsArea();
    drawStats();

    needsStatsRedraw = false;
  }

  if (needsFooterRedraw)
  {
    clearFooterArea();
    drawFooter();

    needsFooterRedraw = false;
  }
}

void WifiMonitorScreen::handlePrevious()
{
  if (monitor == nullptr || monitorStartFailed)
  {
    return;
  }

  if (monitor->previousChannel())
  {
    stats = monitor->getStats();

    needsFullRedraw = true;
  }
}

void WifiMonitorScreen::handleNext()
{
  if (monitor == nullptr || monitorStartFailed)
  {
    return;
  }

  if (monitor->nextChannel())
  {
    stats = monitor->getStats();

    needsFullRedraw = true;
  }
}

void WifiMonitorScreen::handleSelect()
{
  if (monitor == nullptr || monitorStartFailed)
  {
    return;
  }

  if (monitor->isPaused())
  {
    monitor->resume();
  }
  else
  {
    monitor->pause();
  }

  stats = monitor->getStats();

  needsHeaderRedraw = true;
  needsStatsRedraw = true;
  needsFooterRedraw = true;
}

void WifiMonitorScreen::drawScreen()
{
  display->fillScreen(ILI9341_BLACK);

  display->setTextWrap(false);

  drawHeader();

  if (monitorStartFailed)
  {
    drawError();
  }
  else
  {
    drawStats();
  }

  drawFooter();
}

void WifiMonitorScreen::drawHeader()
{
  display->setTextSize(2);
  display->setTextColor(
    ILI9341_WHITE,
    ILI9341_BLACK
  );

  display->setCursor(10, 10);
  display->print("802.11 MONITOR");

  display->setTextSize(1);

  if (monitor != nullptr && monitor->isPaused())
  {
    display->setCursor(205, 14);
    display->print("PAUSED");
  }

  char channelText[12];

  snprintf(
    channelText,
    sizeof(channelText),
    "CH %u",
    static_cast<unsigned>(
      stats.channel
    )
  );

  display->setCursor(278, 14);
  display->print(channelText);

  display->drawFastHLine(
    8,
    36,
    display->width() - 16,
    ILI9341_WHITE
  );
}

void WifiMonitorScreen::drawStats()
{
  display->setTextColor(
    ILI9341_WHITE,
    ILI9341_BLACK
  );

  display->setTextSize(1);

  // Frames/s
  display->setCursor(16, 50);
  display->print("Frames/s");

  display->setTextSize(2);

  display->setCursor(16, 64);
  display->print(stats.framesPerSecond);

  // Last event
  drawLastManagementEvent();

  display->setTextSize(1);

  // Left column
  display->setCursor(16, 100);
  display->print("Total");

  display->setCursor(105, 100);
  display->print(stats.totalFrames);

  display->setCursor(16, 116);
  display->print("Management");

  display->setCursor(105, 116);
  display->print(stats.managementFrames);

  display->setCursor(16, 132);
  display->print("Control");

  display->setCursor(105, 132);
  display->print(stats.controlFrames);

  display->setCursor(16, 148);
  display->print("Data");

  display->setCursor(105, 148);
  display->print(stats.dataFrames);

  // Right column
  display->setCursor(178, 100);
  display->print("Beacon");

  display->setCursor(265, 100);
  display->print(stats.beaconFrames);

  display->setCursor(178, 116);
  display->print("Probe Req");

  display->setCursor(265, 116);
  display->print(stats.probeRequestFrames);

  display->setCursor(178, 132);
  display->print("Probe Resp");

  display->setCursor(265, 132);
  display->print(stats.probeResponseFrames);

  display->setCursor(178, 148);
  display->print("Deauth");

  display->setCursor(265, 148);
  display->print(stats.deauthFrames);

  display->setCursor(178, 164);
  display->print("Disassoc");

  display->setCursor(265, 164);
  display->print(stats.disassociationFrames);
}

void WifiMonitorScreen::drawFooter()
{
  display->drawFastHLine(
    8,
    184,
    display->width() - 16,
    ILI9341_WHITE
  );

  if (monitorStartFailed)
  {
    drawCentered(
      "HOLD OK : BACK",
      220,
      1,
      ILI9341_WHITE
    );

    return;
  }

  display->setTextSize(1);
  display->setTextColor(
    ILI9341_WHITE,
    ILI9341_BLACK
  );

  display->setCursor(12, 198);
  display->print("< CH-");

  const char* centerText =
    monitor != nullptr && monitor->isPaused() ?
      "RESUME" : "PAUSE";

  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display->getTextBounds(
    centerText,
    0,
    0,
    &x1,
    &y1,
    &width,
    &height
  );

  display->setCursor(
    (display->width() - width) / 2,
    198
  );

  display->print(centerText);

  display->setCursor(278, 198);
  display->print("CH+ >");

  drawCentered(
    "HOLD OK : BACK",
    220,
    1,
    ILI9341_WHITE
  );
}

void WifiMonitorScreen::clearHeaderArea()
{
  display->fillRect(
    0,
    0,
    display->width(),
    40,
    ILI9341_BLACK
  );
}

void WifiMonitorScreen::clearStatsArea()
{
  display->fillRect(
    0,
    40,
    display->width(),
    142,
    ILI9341_BLACK
  );
}

void WifiMonitorScreen::clearFooterArea()
{
  display->fillRect(
    0,
    185,
    display->width(),
    55,
    ILI9341_BLACK
  );
}

void WifiMonitorScreen::drawError()
{
  drawCentered(
    "MONITOR ERROR",
    92,
    2,
    ILI9341_WHITE
  );

  drawCentered(
    "FAILED TO START",
    124,
    1,
    ILI9341_WHITE
  );
}

void WifiMonitorScreen::drawLastManagementEvent()
{
  display->setTextSize(1);
  display->setTextColor(
    ILI9341_WHITE,
    ILI9341_BLACK
  );

  display->setCursor(178, 50);
  display->print("LAST EVENT");

  const WifiManagementEvent& event = stats.lastManagementEvent;

  if (!event.valid)
  {
    display->setCursor(178, 66);
    display->print("--");
    return;
  }

  bool belongsToCurrentChannel = false;

  if (event.networkChannelKnown)
  {
    belongsToCurrentChannel = event.networkChannel == stats.channel;
  }
  else
  {
    belongsToCurrentChannel = event.receivedChannel == stats.channel;
  }

  if (!belongsToCurrentChannel)
  {
    display->setCursor(178, 66);
    display->print("--");
    return;
  }

  WifiManagementDirection direction =
    getManagementEventDirection(event);

  display->setCursor(178, 66);

  display->print(
    wifiManagementEventTypeToString(
      event.type
    )
  );

  display->print(" R");
  display->print(event.reasonCode);

  display->setCursor(178, 80);

  display->print(
    wifiManagementDirectionToString(
      direction
    )
  );

  display->print(" ");

  display->print(event.rssi);
  display->print("dBm");
}

void WifiMonitorScreen::drawCentered(
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
    (display->width() - width) / 2;

  display->setCursor(x, y);
  display->print(text);
}
