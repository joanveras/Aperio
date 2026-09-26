#include "../../include/ui/ScreenManager.hpp"

void ScreenManager::begin(Screen* initialScreen)
{
  currentScreen = initialScreen;

  historySize = 0;

  if (currentScreen)
  {
    currentScreen->onEnter();
  }
}

void ScreenManager::setScreen(Screen* screen) {
  if (screen == nullptr)
  {
    return;
  }

  if (screen == currentScreen)
  {
    return;
  }

  if (currentScreen)
  {
    if (historySize >= MAX_HISTORY)
    {
      return;
    }

    history[historySize] = currentScreen;
    historySize++;
  }

  currentScreen = screen;
  currentScreen->onEnter();
}

void ScreenManager::goBack()
{
  if (historySize == 0)
  {
    return;
  }

  historySize--;

  Screen* priorScreen = history[historySize];

  currentScreen = priorScreen;

  history[historySize] = nullptr;

  currentScreen->onEnter();
}

void ScreenManager::handleInput(InputEvent event)
{
  if (currentScreen == nullptr)
  {
    return;
  }

  if (event == InputEvent::BACK)
  {
    goBack();

    return;
  }

  currentScreen->handleInput(event);
}

void ScreenManager::update()
{
  if (currentScreen)
  {
    currentScreen->update();
  }
}

void ScreenManager::render()
{
  if (currentScreen)
  {
    currentScreen->render();
  }
}
