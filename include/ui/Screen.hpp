#pragma once

#include "../input/InputManager.hpp"

class Screen {
public:
  virtual void onEnter() = 0;
  virtual void handleInput(InputEvent event) = 0;
  virtual void update() = 0;
  virtual void render() = 0;

  virtual ~Screen() = default;
};
