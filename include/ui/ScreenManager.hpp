#pragma once

#include "Screen.hpp"

class ScreenManager {
public:
  void begin(Screen* initialScreen);

  void setScreen(Screen* screen);
  void goBack();

  void handleInput(InputEvent event);
  void update();
  void render();

private:
  Screen* currentScreen = nullptr;
  Screen* previousScreen = nullptr;
};
