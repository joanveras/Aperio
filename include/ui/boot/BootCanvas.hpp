#pragma once

#include <Adafruit_ILI9341.h>
#include <cstdint>

// A small off-screen buffer of BootTone values (1 byte per pixel).
//
// The animated parts of the boot screen (the emblem and the wave terrain)
// are drawn here every frame and then sent to the display in one go, so
// they never flicker. The memory only exists between begin() and end().
class BootCanvas
{
public:
  BootCanvas(int16_t width, int16_t height);
  ~BootCanvas();

  BootCanvas(const BootCanvas&) = delete;
  BootCanvas& operator=(const BootCanvas&) = delete;

  bool begin();
  void end();

  void clear();

  // Lights one pixel. A pixel never gets darker: the brighter tone wins.
  void plot(int16_t x, int16_t y, uint8_t tone);

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t tone);

  // Sends the whole canvas to the display with its top left at (x, y).
  void present(
    Adafruit_ILI9341* display,
    int16_t x,
    int16_t y,
    const uint16_t* palette
  ) const;

private:
  static constexpr int16_t MAX_WIDTH = 320;

  int16_t canvasWidth;
  int16_t canvasHeight;
  uint8_t* pixels;
};
