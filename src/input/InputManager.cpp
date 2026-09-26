#include "../../include/input/InputManager.hpp"
#include <Arduino.h>

InputManager::InputManager(
  uint8_t previousPin,
  uint8_t selectPin,
  uint8_t nextPin,
  InputEvent event
) : previousButton(previousPin),
    selectButton(selectPin),
    nextButton(nextPin),
    pendingEvent(event)
{
}

void InputManager::begin()
{
  previousButton.begin();
  selectButton.begin();
  nextButton.begin();
}

InputEvent InputManager::getEvent()
{
  InputEvent event = pendingEvent;
  pendingEvent = InputEvent::NONE;

  return event;
}

void InputManager::update()
{
  uint32_t now = millis();

  previousButton.update(now);
  selectButton.update(now);
  nextButton.update(now);

  if (previousButton.wasPressed())
    pendingEvent = InputEvent::PREVIOUS;
  else if (nextButton.wasPressed())
    pendingEvent = InputEvent::NEXT;
  else if (selectButton.wasLongPressed())
    pendingEvent = InputEvent::BACK;
  else if (selectButton.wasPressed())
    pendingEvent = InputEvent::SELECT;
}
