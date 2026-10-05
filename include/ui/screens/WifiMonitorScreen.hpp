#pragma once

#include <Adafruit_ILI9341.h>

#include "../Screen.hpp"
#include "../../wifi/WifiMonitor.hpp"

class WifiMonitorScreen : public Screen
{
public:
  WifiMonitorScreen(
    Adafruit_ILI9341* displayInstance,
    WifiMonitor* monitorInstance
  );

  void onEnter() override;
  void onExit() override;

  void handleInput(InputEvent event) override;
  void update() override;
  void render() override;

  // Never sleep while the monitor is capturing live traffic.
  bool allowsIdle() const override { return false; }

private:
  static constexpr uint32_t REFRESH_INTERVAL = 500;

  Adafruit_ILI9341* display;
  WifiMonitor* monitor;

  WifiMonitorStats stats;

  uint32_t lastRefreshTime = 0;

  bool monitorStartFailed = false;

  bool needsFullRedraw = true;
  bool needsHeaderRedraw = true;
  bool needsStatsRedraw = true;
  bool needsFooterRedraw = true;

  void drawScreen();
  void drawHeader();
  void drawStats();
  void drawError();
  void drawFooter();

  void clearHeaderArea();
  void clearStatsArea();
  void clearFooterArea();

  void handlePrevious();
  void handleNext();
  void handleSelect();

  void drawLastManagementEvent();

  void drawCentered(
    const char* text,
    int16_t y,
    uint8_t textSize,
    uint16_t color
  );
};
