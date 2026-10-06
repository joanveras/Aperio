#include "../include/AperioApp.hpp"
#include "../include/Config.hpp"
#include <Arduino.h>

AperioApp::AperioApp(
  Adafruit_ILI9341* displayInstance,
  uint8_t previousButtonPin,
  uint8_t selectButtonPin,
  uint8_t nextButtonPin
)
  : display(displayInstance),
    bootAnimation(displayInstance),
    input(
      previousButtonPin,
      selectButtonPin,
      nextButtonPin
    ),
    mainMenu(
      displayInstance,
      [this](ScreenId screenId)
      {
        handleNavigation(screenId);
      }
    ),
    placeholderScreen(displayInstance),
    aboutScreen(displayInstance),
    wifiScreen(
      displayInstance,
      [this](ScreenId screenId)
      {
        handleNavigation(screenId);
      }
    ),
    wifiMonitorMenuScreen(
      displayInstance,
      [this](ScreenId screenId)
      {
        handleNavigation(screenId);
      }
    ),
    wifiNetworkListScreen(
      displayInstance,
      &wifiScanner,
      [this](ScreenId screenId)
      {
        handleNavigation(screenId);
      }
    ),
    wifiNetworkDetailScreen(
      displayInstance
    ),
    wifiChannelScreen(
      displayInstance,
      &wifiScanner,
      [this](ScreenId screenId)
      {
        if (screenId == ScreenId::WIFI_MONITOR)
        {
          wifiMonitor.setChannel(
            wifiChannelScreen.getSelectedChannel()
          );
        }

        handleNavigation(screenId);
      }
    ),
    wifiMonitorScreen(
      displayInstance,
      &wifiMonitor
    ),
    wifiManagementEventScreen(
      displayInstance
    ),
    wifiManagementEventsScreen(
      displayInstance,
      &wifiMonitor,
      [this](ScreenId screenId)
      {
        handleNavigation(screenId);
      }
    ),
    txLabScreen(
      displayInstance,
      &wifiTransmitter
    )
{
}

void AperioApp::begin()
{
  input.begin();

  display->begin(TFT_SPI_FREQUENCY);
  display->setRotation(1);

  wifiScanner.begin();

  bootAnimation.begin(millis());

  if (bootAnimation.isFinished())
  {
    screenManager.begin(&mainMenu);
  }
}

void AperioApp::update()
{
  input.update();

  InputEvent event = input.getEvent();

  if (!bootAnimation.isFinished())
  {
    updateBoot(event);
    return;
  }

  if (event != InputEvent::NONE)
  {
    screenManager.handleInput(event);
  }

  screenManager.update();
  screenManager.render();
}

// The main menu only starts once the boot animation is over. Any button
// skips the animation.
void AperioApp::updateBoot(InputEvent event)
{
  if (event != InputEvent::NONE)
  {
    bootAnimation.skip();
  }

  bootAnimation.update(millis());

  if (bootAnimation.isFinished())
  {
    screenManager.begin(&mainMenu);
  }
}

void AperioApp::handleNavigation(ScreenId screenId)
{
  switch (screenId)
  {
    case ScreenId::WIFI_MENU:
      screenManager.setScreen(&wifiScreen);
      break;

    case ScreenId::WIFI_SCAN:
    {
      wifiScanner.scan();

      size_t networkCount = wifiScanner.getNetworkCount();

      for (size_t i = 0; i < networkCount; i++)
      {
        const WifiNetwork& network =
          wifiScanner.getNetwork(i);

        wifiMonitor.rememberApChannel(
          network.bssid,
          network.channel
        );
      }

      wifiNetworkListScreen.setTitle("WI-FI SCAN");

      wifiNetworkListScreen.setSelectDestination(
        ScreenId::WIFI_NETWORK_DETAILS
      );

      wifiNetworkListScreen.resetSelection();

      screenManager.setScreen(&wifiNetworkListScreen);

      break;
    }

    case ScreenId::WIFI_NETWORKS:
      wifiNetworkListScreen.setTitle("NETWORKS");

      wifiNetworkListScreen.setSelectDestination(
        ScreenId::WIFI_NETWORK_DETAILS
      );

      wifiNetworkListScreen.resetSelection();

      screenManager.setScreen(&wifiNetworkListScreen);

      break;

    case ScreenId::WIFI_NETWORK_DETAILS:
    {
      const WifiNetwork* network =
        wifiNetworkListScreen.getSelectedNetwork();

      if (network == nullptr)
      {
        break;
      }

      /*
        * We make a copy because we will
        * enrich it with the detected PMF.
      */
      WifiNetwork detailedNetwork = *network;

      detailedNetwork.pmf = detectNetworkPmf(detailedNetwork);

      wifiNetworkDetailScreen.setNetwork(detailedNetwork);

      screenManager.setScreen(&wifiNetworkDetailScreen);

      break;
    }

    case ScreenId::WIFI_CHANNELS:
      screenManager.setScreen(&wifiChannelScreen);
      break;

    case ScreenId::WIFI_MONITOR_MENU:
      screenManager.setScreen(&wifiMonitorMenuScreen);
      break;

    case ScreenId::WIFI_MONITOR:
      screenManager.setScreen(&wifiMonitorScreen);
      break;

    case ScreenId::WIFI_MANAGEMENT_EVENTS:
      screenManager.setScreen(&wifiManagementEventsScreen);
      break;

    case ScreenId::WIFI_MANAGEMENT_EVENT:
    {
      WifiManagementEvent event;

      if (wifiManagementEventsScreen.getSelectedEvent(event))
      {
        wifiManagementEventScreen.setEvent(event);

        screenManager.setScreen(&wifiManagementEventScreen);
      }

      break;
    }

    case ScreenId::WIFI_ACTIVE_SURVEY:
      placeholderScreen.setTitle("ACTIVE SURVEY");

      screenManager.setScreen(&placeholderScreen);

      break;

    case ScreenId::WIFI_TX_SELECT:
    {
      wifiScanner.scan();

      size_t networkCount =
        wifiScanner.getNetworkCount();

      for (size_t i = 0; i < networkCount; i++)
      {
        const WifiNetwork& network =
          wifiScanner.getNetwork(i);

        wifiMonitor.rememberApChannel(
          network.bssid,
          network.channel
        );
      }

      wifiNetworkListScreen.setTitle(
        "TX TARGET"
      );

      wifiNetworkListScreen.setSelectDestination(
        ScreenId::WIFI_TX_LAB
      );

      wifiNetworkListScreen.resetSelection();

      screenManager.setScreen(
        &wifiNetworkListScreen
      );

      break;
    }

    case ScreenId::WIFI_TX_LAB:
    {
      const WifiNetwork* network =
        wifiNetworkListScreen.getSelectedNetwork();

      if (network == nullptr)
      {
        break;
      }

      txLabScreen.setTarget(*network);

      screenManager.setScreen(&txLabScreen);

      break;
    }

    case ScreenId::BLUETOOTH_MENU:
      placeholderScreen.setTitle("BLUETOOTH");
      screenManager.setScreen(&placeholderScreen);
      break;

    case ScreenId::SYSTEM_INFO:
      placeholderScreen.setTitle("SYSTEM");
      screenManager.setScreen(&placeholderScreen);
      break;

    case ScreenId::ABOUT:
      screenManager.setScreen(&aboutScreen);
      break;

    case ScreenId::MAIN_MENU:
    default:
      break;
  }
}

WifiPmfMode AperioApp::detectNetworkPmf(const WifiNetwork& network)
{
  /*
   * Does not reuse old result.
  */
  wifiMonitor.forgetKnownApPmf( network.bssid);

  /*
   * Listens on exactly the channel
   * of the selected network.
  */
  if (!wifiMonitor.start( network.channel))
  {
    return
      WifiPmfMode::PMF_UNKNOWN;
  }

  WifiPmfMode detectedPmf = WifiPmfMode::PMF_UNKNOWN;

  constexpr uint32_t PMF_CHECK_TIMEOUT = 1000;

  uint32_t startTime = millis();

  while (millis() - startTime < PMF_CHECK_TIMEOUT)
  {
    if (wifiMonitor.getKnownApPmf(network.bssid, detectedPmf))
    {
      break;
    }

    delay(10);
  }

  wifiMonitor.stop();

  return detectedPmf;
}
