#pragma once

#include <Adafruit_ILI9341.h>
#include <cstdint>

// Colors and small drawing helpers shared by every screen.
//
// The palette comes from the mascot: black background, near-white text and
// cyan as the only accent. Warning and danger exist for the few places that
// must stand out (TX Lab, errors) and should stay rare.
namespace UiColor
{
  constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b)
  {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }

  constexpr uint16_t BACKGROUND = rgb(0, 0, 0);

  constexpr uint16_t TEXT = rgb(240, 251, 255);       // titles, values, selection
  constexpr uint16_t TEXT_MUTED = rgb(138, 166, 180); // labels, unselected items
  constexpr uint16_t TEXT_DIM = rgb(74, 98, 112);     // hints, secondary info

  constexpr uint16_t ACCENT = rgb(54, 196, 220);      // the mascot's signal cyan
  constexpr uint16_t ACCENT_SOFT = rgb(24, 152, 176); // bars, quieter highlights
  constexpr uint16_t LINE = rgb(17, 60, 75);          // dividers
  constexpr uint16_t SELECTION = rgb(8, 38, 50);      // selected row background

  constexpr uint16_t WARNING = rgb(255, 190, 70);
  constexpr uint16_t DANGER = rgb(240, 84, 96);
}

namespace UiStyle
{
  // Screen title at the top left (text size 2), underlined by a short cyan
  // dash that fades into a thin divider across the screen.
  //
  // `ruleEndX` is where the thin divider line stops on the right. Leave it
  // at -1 for the full width; the menu screens pass a smaller value so the
  // line does not run under the mascot in the corner.
  void drawTitle(
    Adafruit_ILI9341* display,
    const char* title,
    int16_t ruleEndX = -1
  );

  // Thin horizontal divider across the screen.
  void drawDivider(Adafruit_ILI9341* display, int16_t y);

  // Background of a selected row: dark teal with a cyan bar on the left.
  // Text drawn on it should use UiColor::TEXT on UiColor::SELECTION.
  void drawSelection(
    Adafruit_ILI9341* display,
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height
  );
}
