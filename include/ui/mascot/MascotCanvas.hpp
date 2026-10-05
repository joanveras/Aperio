#pragma once

#include <cstdint>

// A small off-screen image where every pixel is a color index
// (see MascotPalette). The mascot is drawn here first and then sent to
// the TFT in one go by MascotView, which avoids flicker.
//
// The canvas works in "logical" pixels: MascotView scales each one up
// (2x by default) when it reaches the display.
class MascotCanvas
{
public:
  MascotCanvas(int16_t width, int16_t height);
  ~MascotCanvas();

  MascotCanvas(const MascotCanvas&) = delete;
  MascotCanvas& operator=(const MascotCanvas&) = delete;

  // Allocates / frees the pixel buffer (width * height bytes).
  bool begin();
  void end();

  bool isReady() const;

  int16_t width() const;
  int16_t height() const;

  void clear(uint8_t color);

  // Out-of-bounds pixels are ignored, so callers never need to clip.
  void setPixel(int16_t x, int16_t y, uint8_t color);
  void drawVerticalSpan(int16_t x, int16_t y0, int16_t y1, uint8_t color);

  const uint8_t* row(int16_t y) const;

private:
  int16_t canvasWidth;
  int16_t canvasHeight;
  uint8_t* pixels;
};
