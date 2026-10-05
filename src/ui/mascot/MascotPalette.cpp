#include "ui/mascot/MascotPalette.hpp"

namespace
{
  struct Rgb
  {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  // Black, white and cyan/blue only, as in the storyboard.
  constexpr Rgb BASE_COLORS[MascotColor::COUNT] = {
    {   0,   0,   0 }, // BLACK
    {   6,  42,  51 }, // SIGNAL 1
    {  11,  74,  90 }, // SIGNAL 2
    {  17, 112, 133 }, // SIGNAL 3
    {  24, 152, 176 }, // SIGNAL 4
    {  54, 196, 220 }, // SIGNAL 5
    { 122, 232, 245 }, // SIGNAL 6
    { 240, 251, 255 }, // WHITE
    {   5,  12,  24 }, // SCLERA
    {  11,  44, 120 }, // IRIS_DARK
    {  23, 100, 214 }, // IRIS_MID
    {  79, 176, 255 }, // IRIS_LIGHT
  };

  uint8_t towardsWhite(uint8_t channel, float amount)
  {
    return channel + static_cast<uint8_t>((255 - channel) * amount);
  }

  uint16_t toRgb565(uint8_t r, uint8_t g, uint8_t b)
  {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }
}

uint8_t MascotColor::signal(float intensity)
{
  if (intensity < 0.06f)
  {
    return BLACK;
  }

  if (intensity >= 1.0f)
  {
    return SIGNAL_LAST;
  }

  constexpr int STEPS = SIGNAL_LAST - SIGNAL_FIRST + 1;

  return SIGNAL_FIRST + static_cast<uint8_t>(intensity * STEPS);
}

void MascotPalette::build(uint16_t out[MascotColor::COUNT], float flash)
{
  if (flash < 0.0f)
  {
    flash = 0.0f;
  }

  if (flash > 1.0f)
  {
    flash = 1.0f;
  }

  // The background stays black, otherwise the canvas rectangle would show.
  out[MascotColor::BLACK] = 0;

  for (uint8_t i = MascotColor::BLACK + 1; i < MascotColor::COUNT; i++)
  {
    const Rgb& color = BASE_COLORS[i];

    out[i] = toRgb565(
      towardsWhite(color.r, flash),
      towardsWhite(color.g, flash),
      towardsWhite(color.b, flash)
    );
  }
}
