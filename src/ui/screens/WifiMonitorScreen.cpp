#include "ui/screens/WifiMonitorScreen.hpp"

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

  monitor->start(monitor->getChannel());

  stats = monitor->getStats();

  lastRefreshTime = millis();

  needsFullRedraw = true;
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
  if (monitor == nullptr)
  {
    return;
  }

  monitor->update();

  uint32_t now = millis();

  if (now - lastRefreshTime <REFRESH_INTERVAL)
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
    needsStatsRedraw = false;
    needsFooterRedraw = false;

    return;
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
  if (monitor == nullptr)
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
  if (monitor == nullptr)
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
  if (monitor == nullptr)
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

  needsStatsRedraw = true;
  needsFooterRedraw = true;
}

void WifiMonitorScreen::drawScreen()
{
  display->fillScreen(ILI9341_BLACK);

  display->setTextWrap(false);

  drawHeader();
  drawStats();
  drawFooter();
}

void WifiMonitorScreen::drawHeader()
{
  display->setTextSize(2);
  display->setTextColor(ILI9341_WHITE);

  display->setCursor(10, 10);
  display->print("802.11 MONITOR");

  display->setTextSize(1);

  char channelText[16];

  snprintf(
    channelText,
    sizeof(channelText),
    "CH %u",
    static_cast<unsigned>(
      stats.channel
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

  display->setCursor(display->width() - width - 10,14);

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

  display->setCursor(16, 50);
  display->print("Frames/s");

  display->setTextSize(2);

  display->setCursor(16, 64);
  display->print(
    stats.framesPerSecond
  );

  display->setTextSize(1);

  // Left column

  display->setCursor(16, 98);
  display->print("Total");

  display->setCursor(105, 98);
  display->print(
    stats.totalFrames
  );

  display->setCursor(16, 116);
  display->print("Management");

  display->setCursor(105, 116);
  display->print(
    stats.managementFrames
  );

  display->setCursor(16, 134);
  display->print("Control");

  display->setCursor(105, 134);
  display->print(
    stats.controlFrames
  );

  display->setCursor(16, 152);
  display->print("Data");

  display->setCursor(105, 152);
  display->print(
    stats.dataFrames
  );

  // Right column

  display->setCursor(178, 98);
  display->print("Beacon");

  display->setCursor(265, 98);
  display->print(
    stats.beaconFrames
  );

  display->setCursor(178, 116);
  display->print("Probe Req");

  display->setCursor(265, 116);
  display->print(
    stats.probeRequestFrames
  );

  display->setCursor(178, 134);
  display->print("Probe Resp");

  display->setCursor(265, 134);
  display->print(
    stats.probeResponseFrames
  );

  display->setCursor(178, 152);
  display->print("Deauth");

  display->setCursor(265, 152);
  display->print(
    stats.deauthFrames
  );

  if (monitor != nullptr && monitor->isPaused())
  {
    drawCentered(
      "PAUSED",
      170,
      1,
      ILI9341_WHITE
    );
  }
}

void WifiMonitorScreen::drawFooter()
{
  display->drawFastHLine(
    8,
    184,
    display->width() - 16,
    ILI9341_WHITE
  );

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
