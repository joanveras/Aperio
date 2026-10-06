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

  // Whether the current screen allows the screensaver to take over.
  bool currentAllowsScreensaver() const;

  // Re-enters the current screen so it repaints itself. Used after the
  // screensaver ends, to bring the frozen screen back exactly where it was.
  void refresh();

private:
  static constexpr size_t MAX_HISTORY = 8;
  size_t historySize = 0;

  Screen* history[MAX_HISTORY] = {};
  Screen* currentScreen = nullptr;
};
