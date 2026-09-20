#pragma once

#include <stdint.h>

class Button {
public:
  Button(uint8_t pin, bool activeLow = true);

  void begin();
  void update(uint32_t now);

  bool wasPressed();
  bool wasLongPressed();
  bool isPressed() const;

private:
  uint8_t pin;
  bool activeLow;

  bool currentState;
  bool previousState;
  bool pressEvent;
  bool longPressEvent;
  bool longPressTriggered;

  uint32_t lastChangeTime;
  uint32_t pressStartTime;

  static constexpr uint32_t DEBOUNCE_TIME = 30;
  static constexpr uint32_t LONG_PRESS_TIME = 700;
};
