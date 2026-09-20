#include "../include/AperioApp.hpp"
#include <Arduino.h>

AperioApp::AperioApp(
  Arduino_GFX* displayInstance,
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
    mainMenu(displayInstance)
{
}

void AperioApp::begin()
{
  input.begin();

  display->begin();
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