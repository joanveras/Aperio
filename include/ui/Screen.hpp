#pragma once

#include "../input/InputManager.hpp"

class Screen {
public:
  virtual void onEnter() = 0;
  virtual void onExit() {}
  virtual void handleInput(InputEvent event) = 0;
  virtual void update() = 0;
  virtual void render() = 0;

  // Whether the mascot may fall into its idle overlay while this screen is
  // on. Screens doing live work (capturing packets, transmitting) return
  // false so the device never "falls asleep" in the middle of an operation.
  virtual bool allowsIdle() const { return true; }

  virtual ~Screen() = default;
};
