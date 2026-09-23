#include "../../include/input/Button.hpp"
#include <Arduino.h>

Button::Button(uint8_t pin, bool activeLow)
  : pin(pin),
  activeLow(activeLow),
  currentState(false),
  previousState(false),
  pressEvent(false),
  longPressEvent(false),
  longPressTriggered(false),
  lastChangeTime(0),
  pressStartTime(0)
{
}

void Button::begin()
{
  if (activeLow)
    pinMode(pin, INPUT_PULLUP);
  else
    pinMode(pin, INPUT_PULLDOWN);

  bool rawState = digitalRead(pin);

  currentState = activeLow ? rawState == LOW : rawState == HIGH;

  previousState = currentState;

  pressEvent = false;
  longPressEvent = false;
  longPressTriggered = false;

  lastChangeTime = millis();
  pressStartTime = 0;
}

void Button::update(uint32_t now)
{
  bool rawState = digitalRead(pin);

  bool reading = activeLow ? rawState == LOW : rawState == HIGH;

  if (reading != previousState)
  {
    lastChangeTime = now;
    previousState = reading;
  }

  if (now - lastChangeTime >= DEBOUNCE_TIME)
  {
    if (reading != currentState)
    {
        currentState = reading;
  
        if (currentState)
        {
          pressStartTime = now;
          longPressTriggered = false;
        }
        else if (!longPressTriggered)
        {
          pressEvent = true;
        }
    }
  }

  if (currentState && !longPressTriggered)
  {
    uint32_t timePressed = now - pressStartTime;
    if (timePressed >= LONG_PRESS_TIME)
    {
      longPressEvent = true;
      longPressTriggered = true;
    }
  }
}

bool Button::wasPressed()
{
  bool event = pressEvent;
  pressEvent = false;

  return event;
}

bool Button::isPressed() const
{
  return currentState;
}

bool Button::wasLongPressed()
{
  bool event = longPressEvent;

  longPressEvent = false;
  return event;
}
