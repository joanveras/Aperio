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
    aboutScreen(displayInstance),
    placeholderScreen(displayInstance)
{
}

void AperioApp::begin()
{
  input.begin();

  display->begin(TFT_SPI_FREQUENCY);
  display->setRotation(1);

  showBootScreen();

  screenManager.begin(&mainMenu);
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
      placeholderScreen.setTitle("WI-FI");
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
