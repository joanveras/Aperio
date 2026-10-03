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
    )
{
}

void AperioApp::begin()
{
  input.begin();

  display->begin(TFT_SPI_FREQUENCY);
  display->setRotation(1);

  showBootScreen();

  screenManager.begin(&mainMenu);

  wifiScanner.begin();
}

void AperioApp::update()
{
  input.update();

  InputEvent event = input.getEvent();

  if (event != InputEvent::NONE)
  {
    screenManager.handleInput(event);
  }

  screenManager.update();
  screenManager.render();
}

void AperioApp::showBootScreen()
{
  display->fillScreen(ILI9341_BLACK);
  display->setTextWrap(false);

  auto drawCentered = [this](
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

    int16_t x = (display->width() - width) / 2;

    display->setCursor(x, y);
    display->print(text);
  };

  drawCentered(
    "Aperio",
    90,
    4,
    ILI9341_WHITE
  );

  drawCentered(
    "Quod Latet",
    140,
    2,
    ILI9341_WHITE
  );

  delay(5000);
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

      wifiNetworkListScreen.resetSelection();

      screenManager.setScreen(&wifiNetworkListScreen);

      break;
    }

    case ScreenId::WIFI_NETWORKS:
      wifiNetworkListScreen.setTitle("NETWORKS");

      wifiNetworkListScreen.resetSelection();

      screenManager.setScreen(&wifiNetworkListScreen);

      break;

    case ScreenId::WIFI_NETWORK_DETAILS:
    {
      const WifiNetwork* network =
        wifiNetworkListScreen.getSelectedNetwork();

      wifiNetworkDetailScreen.setNetwork(
        network
      );

      screenManager.setScreen(
        &wifiNetworkDetailScreen
      );

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
