#include "ui/boot/BootPalette.hpp"

namespace
{
  struct Rgb
  {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  constexpr Rgb TONES[BootTone::COUNT] = {
    {0, 0, 0},
    {8, 44, 20},
    {18, 92, 44},
    {40, 160, 78},
    {70, 224, 112},
    {140, 255, 168},
    {214, 255, 224}
  };

  uint16_t toRgb565(float r, float g, float b)
  {
    const uint8_t red = static_cast<uint8_t>(r);
    const uint8_t green = static_cast<uint8_t>(g);
    const uint8_t blue = static_cast<uint8_t>(b);

    return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
  }
}

void BootPalette::build(uint16_t* palette, float level)
{
  if (level < 0.0f)
  {
    level = 0.0f;
  }

  if (level > 1.0f)
  {
    level = 1.0f;
  }

  for (uint8_t tone = 0; tone < BootTone::COUNT; tone++)
  {
    palette[tone] = toRgb565(
      TONES[tone].r * level,
      TONES[tone].g * level,
      TONES[tone].b * level
    );
  }
}

uint16_t BootPalette::color(uint8_t tone)
{
  if (tone >= BootTone::COUNT)
  {
    tone = BootTone::WHITE;
  }

  return toRgb565(TONES[tone].r, TONES[tone].g, TONES[tone].b);
}
