#pragma once

#include <cstdint>

// The mascot is drawn with color indices instead of real colors.
// Indices are turned into RGB565 only when the frame is sent to the TFT,
// which makes fades, flashes and future palette swaps (e.g. red for ERROR)
// a matter of changing a small table.
namespace MascotColor
{
  constexpr uint8_t BLACK = 0;

  // Signal ramp: dark cyan (1) to bright cyan (6).
  // Used for waves, rings and marks; brightness = index.
  constexpr uint8_t SIGNAL_FIRST = 1;
  constexpr uint8_t SIGNAL_LAST = 6;

  constexpr uint8_t WHITE = 7;
  constexpr uint8_t SCLERA = 8;
  constexpr uint8_t IRIS_DARK = 9;
  constexpr uint8_t IRIS_MID = 10;
  constexpr uint8_t IRIS_LIGHT = 11;

  constexpr uint8_t COUNT = 12;

  // Maps an intensity (0..1) to a signal ramp index.
  // Returns BLACK when the intensity is too low to be seen.
  uint8_t signal(float intensity);
}

namespace MascotPalette
{
  // Fills `out` with the RGB565 value of every color index.
  // `flash` (0..1) pushes every color except the black background
  // towards white.
  void build(uint16_t out[MascotColor::COUNT], float flash);
}
