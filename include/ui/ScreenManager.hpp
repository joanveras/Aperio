#pragma once

#include "Screen.hpp"
#include <cstddef>

class ScreenManager {
public:
  void begin(Screen* initialScreen);

  void setScreen(Screen* screen);
  void goBack();

  void handleInput(InputEvent event);
  void update();
  void render();

private:
  static constexpr size_t MAX_HISTORY = 8;
  size_t historySize = 0;

  Screen* history[MAX_HISTORY] = {};
  Screen* currentScreen = nullptr;
};
