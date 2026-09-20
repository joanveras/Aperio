#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include "input/InputManager.hpp"
#include "ui/ScreenManager.hpp"
#include "ui/MainMenuScreen.hpp"

class AperioApp
{
public:
    AperioApp(
        Arduino_GFX* displayInstance,
        uint8_t previousButtonPin,
        uint8_t selectButtonPin,
        uint8_t nextButtonPin
    );

    void begin();
    void update();

private:
    Arduino_GFX* display;

    InputManager input;
    ScreenManager screenManager;
    MainMenuScreen mainMenu;

    void showBootScreen();
};