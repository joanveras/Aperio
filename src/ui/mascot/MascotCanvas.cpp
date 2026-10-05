#include "ui/mascot/MascotCanvas.hpp"

#include <cstdlib>
#include <cstring>

MascotCanvas::MascotCanvas(int16_t width, int16_t height)
  : canvasWidth(width),
    canvasHeight(height),
    pixels(nullptr)
{
}

MascotCanvas::~MascotCanvas()
{
  end();
}

bool MascotCanvas::begin()
{
  if (pixels == nullptr)
  {
    pixels = static_cast<uint8_t*>(
      malloc(static_cast<size_t>(canvasWidth) * canvasHeight)
    );
  }

  return pixels != nullptr;
}

void MascotCanvas::end()
{
  free(pixels);
  pixels = nullptr;
}

bool MascotCanvas::isReady() const
{
  return pixels != nullptr;
}

int16_t MascotCanvas::width() const
{
  return canvasWidth;
}

int16_t MascotCanvas::height() const
{
  return canvasHeight;
}

void MascotCanvas::clear(uint8_t color)
{
  if (pixels == nullptr)
  {
    return;
  }

  memset(pixels, color, static_cast<size_t>(canvasWidth) * canvasHeight);
}

void MascotCanvas::setPixel(int16_t x, int16_t y, uint8_t color)
{
  if (pixels == nullptr)
  {
    return;
  }

  if (x < 0 || y < 0 || x >= canvasWidth || y >= canvasHeight)
  {
    return;
  }

  pixels[y * canvasWidth + x] = color;
}

void MascotCanvas::drawVerticalSpan(
  int16_t x,
  int16_t y0,
  int16_t y1,
  uint8_t color
)
{
  if (y0 > y1)
  {
    int16_t swap = y0;
    y0 = y1;
    y1 = swap;
  }

  for (int16_t y = y0; y <= y1; y++)
  {
    setPixel(x, y, color);
  }
}

const uint8_t* MascotCanvas::row(int16_t y) const
{
  if (pixels == nullptr || y < 0 || y >= canvasHeight)
  {
    return nullptr;
  }

  return pixels + y * canvasWidth;
}
