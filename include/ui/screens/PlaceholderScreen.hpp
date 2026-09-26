#pragma once

#include <Adafruit_ILI9341.h>

#include "../Screen.hpp"

class PlaceholderScreen : public Screen
{
public:
  explicit PlaceholderScreen(Adafruit_ILI9341* displayInstance);

  void setTitle(const char* title);

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void update() override;
  void render() override;

private:
  Adafruit_ILI9341* display;

  const char* title;
  bool needsRedraw;

  void drawCentered(
    const char* text,
    int y,
    uint8_t textSize,
    uint16_t color
  );
};
