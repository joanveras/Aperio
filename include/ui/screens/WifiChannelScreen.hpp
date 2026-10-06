#pragma once

#include <Adafruit_ILI9341.h>
#include <functional>

#include "../Screen.hpp"
#include "../MenuItem.hpp"
#include "../../wifi/WifiScanner.hpp"
#include "../../wifi/WifiChannelStats.hpp"

class WifiChannelScreen : public Screen
{
public:
  WifiChannelScreen(
    Adafruit_ILI9341* displayInstance,
    WifiScanner* scannerInstance,
    std::function<void(ScreenId)> navigationCallback
  );

  uint8_t getSelectedChannel() const;

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void update() override;
  void render() override;

private:
  static constexpr uint8_t MIN_CHANNEL = 1;
  static constexpr uint8_t MAX_CHANNEL = 11;
  static constexpr size_t CHANNEL_COUNT = 11;
  static constexpr size_t VISIBLE_ITEM_COUNT = 5;

  Adafruit_ILI9341* display;
  WifiScanner* scanner;

  std::function<void(ScreenId)> navigationCallback;

  WifiChannelStats channelStats[CHANNEL_COUNT];

  uint8_t selectedChannel = MIN_CHANNEL;
  uint8_t firstVisibleChannel = MIN_CHANNEL;

  bool needsRedraw = true;

  void buildChannelStats();

  void moveSelection(int direction);
  void openSelectedChannel();

  void drawHeader();
  void drawChannels();

  void drawChannelItem(
    uint8_t channel,
    int16_t y,
    bool selected
  );

  void drawFooter();
};
