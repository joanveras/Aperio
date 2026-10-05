#include "ui/mascot/MascotView.hpp"
#include "ui/mascot/MascotPalette.hpp"

MascotView::MascotView(Adafruit_ILI9341* displayInstance, uint8_t pixelScale)
  : display(displayInstance),
    scale(pixelScale),
    frameCounter(0),
    line{}
{
}

void MascotView::present(
  const MascotCanvas& canvas,
  int16_t x,
  int16_t y,
  float flash,
  float glitch
)
{
  if (!canvas.isReady())
  {
    return;
  }

  const int16_t lineWidth = canvas.width() * scale;

  if (lineWidth > MAX_LINE_WIDTH)
  {
    return;
  }

  uint16_t palette[MascotColor::COUNT];
  MascotPalette::build(palette, flash);

  frameCounter++;

  display->startWrite();
  display->setAddrWindow(x, y, lineWidth, canvas.height() * scale);

  for (int16_t row = 0; row < canvas.height(); row++)
  {
    const uint8_t* pixels = canvas.row(row);
    int8_t shift = glitchShift(row, glitch);

    for (int16_t column = 0; column < canvas.width(); column++)
    {
      int16_t source = column - shift;

      uint8_t index = (source >= 0 && source < canvas.width())
        ? pixels[source]
        : MascotColor::BLACK;

      uint16_t color = palette[index];

      for (uint8_t repeat = 0; repeat < scale; repeat++)
      {
        line[column * scale + repeat] = color;
      }
    }

    // The same line is sent `scale` times to scale vertically.
    for (uint8_t repeat = 0; repeat < scale; repeat++)
    {
      display->writePixels(line, lineWidth);
    }
  }

  display->endWrite();
}

// Rows are shifted in bands of two, picked by a cheap hash so the pattern
// changes every couple of frames.
int8_t MascotView::glitchShift(int16_t row, float glitch) const
{
  if (glitch <= 0.0f)
  {
    return 0;
  }

  uint32_t hash = static_cast<uint32_t>(row / 2) * 2654435761u
    ^ (frameCounter / 2) * 2246822519u;

  hash ^= hash >> 15;
  hash *= 2654435761u;
  hash ^= hash >> 13;

  if ((hash & 0xFF) >= glitch * 50.0f)
  {
    return 0;
  }

  return static_cast<int8_t>((hash >> 8) % 5) - 2;
}
