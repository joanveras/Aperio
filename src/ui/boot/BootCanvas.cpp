#include "ui/boot/BootCanvas.hpp"

#include <cstdlib>
#include <cstring>

BootCanvas::BootCanvas(int16_t width, int16_t height)
  : canvasWidth(width),
    canvasHeight(height),
    pixels(nullptr)
{
}

BootCanvas::~BootCanvas()
{
  end();
}

bool BootCanvas::begin()
{
  if (pixels == nullptr && canvasWidth <= MAX_WIDTH)
  {
    pixels = static_cast<uint8_t*>(
      malloc(static_cast<size_t>(canvasWidth) * canvasHeight)
    );
  }

  clear();

  return pixels != nullptr;
}

void BootCanvas::end()
{
  free(pixels);
  pixels = nullptr;
}

bool BootCanvas::isReady() const
{
  return pixels != nullptr;
}

int16_t BootCanvas::width() const
{
  return canvasWidth;
}

int16_t BootCanvas::height() const
{
  return canvasHeight;
}

void BootCanvas::clear()
{
  if (pixels != nullptr)
  {
    memset(pixels, 0, static_cast<size_t>(canvasWidth) * canvasHeight);
  }
}

void BootCanvas::plot(int16_t x, int16_t y, uint8_t tone)
{
  if (pixels == nullptr
    || x < 0 || y < 0
    || x >= canvasWidth || y >= canvasHeight)
  {
    return;
  }

  uint8_t& pixel = pixels[y * canvasWidth + x];

  if (tone > pixel)
  {
    pixel = tone;
  }
}

void BootCanvas::fillRect(
  int16_t x,
  int16_t y,
  int16_t w,
  int16_t h,
  uint8_t tone
)
{
  for (int16_t row = y; row < y + h; row++)
  {
    for (int16_t column = x; column < x + w; column++)
    {
      plot(column, row, tone);
    }
  }
}

void BootCanvas::present(
  Adafruit_ILI9341* display,
  int16_t x,
  int16_t y,
  const uint16_t* palette
) const
{
  if (pixels == nullptr)
  {
    return;
  }

  static uint16_t line[MAX_WIDTH];

  display->startWrite();
  display->setAddrWindow(x, y, canvasWidth, canvasHeight);

  for (int16_t row = 0; row < canvasHeight; row++)
  {
    const uint8_t* source = pixels + row * canvasWidth;

    for (int16_t column = 0; column < canvasWidth; column++)
    {
      line[column] = palette[source[column]];
    }

    display->writePixels(line, canvasWidth);
  }

  display->endWrite();
}
