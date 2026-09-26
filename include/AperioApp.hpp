#pragma once

#include <Arduino.h>
#include <Adafruit_ILI9341.h>

#include "input/InputManager.hpp"
#include "ui/ScreenManager.hpp"
#include "ui/screens/MainMenuScreen.hpp"
#include "ui/screens/PlaceholderScreen.hpp"
#include "ui/screens/AboutScreen.hpp"

class AperioApp
{
public:
    AperioApp(
        Adafruit_ILI9341* displayInstance,
        uint8_t previousButtonPin,
        uint8_t selectButtonPin,
        uint8_t nextButtonPin
    );

    void begin();
    void update();

private:
    Adafruit_ILI9341* display;

    InputManager input;
    ScreenManager screenManager;
    MainMenuScreen mainMenu;
    AboutScreen aboutScreen;
    PlaceholderScreen placeholderScreen;

    void showBootScreen();
    void handleNavigation(ScreenId screenId);
};
