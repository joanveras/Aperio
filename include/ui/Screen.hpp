#pragma once

#include "../input/InputManager.hpp"

class Screen {
public:
  virtual void onEnter() = 0;
  virtual void onExit() {}
  virtual void handleInput(InputEvent event) = 0;
  virtual void update() = 0;
  virtual void render() = 0;

  // Whether the screensaver may take over while this screen is on. Screens
  // doing live work (capturing packets, transmitting) return false so the
  // device never drifts into the screensaver mid-operation.
  virtual bool allowsScreensaver() const { return true; }

  virtual ~Screen() = default;
};
