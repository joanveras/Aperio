#pragma once

#include <cstdint>

// Tones of the retro green boot screen, from black to almost white.
//
// The order matters: a higher tone is always brighter, so when two shapes
// land on the same pixel the canvas keeps the brighter one.
namespace BootTone
{
  constexpr uint8_t BLACK = 0;
  constexpr uint8_t FAINT = 1;  // far particles, glow halos
  constexpr uint8_t DIM = 2;    // distant terrain
  constexpr uint8_t MID = 3;    // log text, mid terrain
  constexpr uint8_t GREEN = 4;  // lines, brackets, near terrain
  constexpr uint8_t BRIGHT = 5; // title, wave crest, emblem core
  constexpr uint8_t WHITE = 6;  // highlights and sparks

  constexpr uint8_t COUNT = 7;
}

namespace BootPalette
{
  // Fills `palette` with RGB565 colors for every tone. `level` (0..1)
  // dims everything at once, used to fade the canvases in.
  void build(uint16_t* palette, float level = 1.0f);

  // RGB565 color of one tone at full brightness, for direct drawing.
  uint16_t color(uint8_t tone);
}
