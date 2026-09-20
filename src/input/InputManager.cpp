#include "../../include/input/InputManager.hpp"
#include <Arduino.h>

void InputManager::update() {
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
