#pragma once

#include <Arduino.h>
#include <Adafruit_ILI9341.h>

#include "input/InputManager.hpp"
#include "ui/ScreenManager.hpp"
#include "ui/screens/MainMenuScreen.hpp"
#include "ui/screens/PlaceholderScreen.hpp"
#include "ui/screens/AboutScreen.hpp"
#include "ui/screens/WifiMenuScreen.hpp"
#include "ui/screens/WifiMonitorMenuScreen.hpp"
#include "ui/screens/WifiNetworkListScreen.hpp"
#include "ui/screens/WifiNetworkDetailScreen.hpp"
#include "ui/screens/WifiChannelScreen.hpp"
#include "ui/screens/WifiMonitorScreen.hpp"
#include "ui/screens/WifiManagementEventScreen.hpp"
#include "ui/screens/WifiManagementEventsScreen.hpp"
#include "ui/screens/TxLabScreen.hpp"
#include "ui/mascot/BootAnimation.hpp"
#include "ui/mascot/screensaver/ScreensaverController.hpp"
#include "wifi/WifiTransmitter.hpp"
#include "wifi/WifiScanner.hpp"
#include "wifi/WifiMonitor.hpp"

class AperioApp
{
public:
    AperioApp(
        Adafruit_ILI9341* displayInstance,
        uint8_t previousButtonPin,
        uint8_t selectButtonPin,
        uint8_t nextButtonPin
    );

    void begin();
    void update();

private:
    Adafruit_ILI9341* display;

    BootAnimation bootAnimation;
    ScreensaverController screensaver;

    InputManager input;
    ScreenManager screenManager;
    MainMenuScreen mainMenu;
    PlaceholderScreen placeholderScreen;
    AboutScreen aboutScreen;

    WifiMenuScreen wifiScreen;
    WifiMonitorMenuScreen wifiMonitorMenuScreen;

    WifiScanner wifiScanner;

    WifiNetworkListScreen wifiNetworkListScreen;
    WifiNetworkDetailScreen wifiNetworkDetailScreen;
    WifiChannelScreen wifiChannelScreen;

    WifiMonitor wifiMonitor;
    WifiMonitorScreen wifiMonitorScreen;
    WifiManagementEventScreen wifiManagementEventScreen;
    WifiManagementEventsScreen wifiManagementEventsScreen;

    WifiTransmitter wifiTransmitter;
    TxLabScreen txLabScreen;

    // The screensaver takes over after this long with no input.
    static constexpr uint32_t SCREENSAVER_ENTER_MS = 20000;

    enum class UiState : uint8_t
    {
      ACTIVE,      // a normal screen is on; watching for inactivity
      SCREENSAVER  // the screensaver owns the display until a button wakes it
    };

    UiState uiState = UiState::ACTIVE;
    uint32_t lastInteractionMs = 0;

    void updateBoot(InputEvent event);

    // Drives the screensaver while it owns the display. Any button wakes it
    // and is swallowed here, so the first press only wakes and never acts on
    // the screen underneath.
    void updateScreensaver(InputEvent event, uint32_t now);

    void handleNavigation(ScreenId screenId);

    WifiPmfMode detectNetworkPmf(const WifiNetwork& network);
};
