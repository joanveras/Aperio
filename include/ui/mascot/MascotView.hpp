#pragma once

#include <Adafruit_ILI9341.h>

#include "MascotCanvas.hpp"

// Sends a MascotCanvas to the TFT, with its top-left corner at (x, y).
//
// Each logical pixel becomes a `scale` x `scale` block, which keeps the
// pixel art look and makes the canvas (scale * scale) times cheaper to
// draw and store.
// Colors are resolved through MascotPalette one row at a time, so the only
// extra memory is a single line buffer.
class MascotView
{
public:
  static constexpr int16_t MAX_LINE_WIDTH = 320;

  MascotView(Adafruit_ILI9341* displayInstance, uint8_t pixelScale);

  // `flash` (0..1) brightens every color, `glitch` (0..1) shifts random
  // rows sideways for that frame.
  void present(
    const MascotCanvas& canvas,
    int16_t x,
    int16_t y,
    float flash,
    float glitch
  );

private:
  Adafruit_ILI9341* display;
  uint8_t scale;
  uint32_t frameCounter;

  uint16_t line[MAX_LINE_WIDTH];

  int8_t glitchShift(int16_t row, float glitch) const;
};
