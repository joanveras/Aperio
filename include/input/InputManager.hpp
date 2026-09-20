#pragma once

#include "Button.hpp"

enum class InputEvent {
  NONE,
  PREVIOUS,
  NEXT,
  SELECT,
  BACK
};

class InputManager {
public:
  InputManager(uint8_t previousPin,
    uint8_t selectPin,
    uint8_t nextPin);

  void begin();
  void update();

  InputEvent getEvent();

private:
  Button previousButton;
  Button selectButton;
  Button nextButton;

  InputEvent pendingEvent;
};
