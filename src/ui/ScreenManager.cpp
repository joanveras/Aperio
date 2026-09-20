#include "../../include/ui/ScreenManager.hpp"

void ScreenManager::setScreen(Screen* screen) {
  previousScreen = currentScreen;
  currentScreen = screen;

  if (currentScreen != nullptr)
    currentScreen->onEnter();
}
