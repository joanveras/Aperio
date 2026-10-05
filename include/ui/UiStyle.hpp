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

// Fonts used by the "chrome" of the UI: titles, menu items and the larger
// messages. They are defined once in UiStyle.cpp. Dense tabular screens
// (lists, the monitor stats grid) keep the built-in fixed-width font,
// because these proportional fonts are about twice as wide and would
// overflow those tight columns.
namespace UiFont
{
  extern const GFXfont* TITLE;  // screen titles
  extern const GFXfont* ITEM;   // menu item labels, emphasis
  extern const GFXfont* BODY;   // larger body text with room to breathe
}

namespace UiStyle
{
  // Draws `text` in `font` with its top-left corner at (x, topY). Restores
  // the built-in font afterwards, so callers can keep using the default.
  void text(
    Adafruit_ILI9341* display,
    const GFXfont* font,
    int16_t x,
    int16_t topY,
    uint16_t color,
    const char* text
  );

  // Same, horizontally centered on the screen.
  void textCentered(
    Adafruit_ILI9341* display,
    const GFXfont* font,
    int16_t topY,
    uint16_t color,
    const char* text
  );

  // Screen title at the top left, underlined by a short cyan dash that
  // fades into a thin divider across the screen.
  void drawTitle(Adafruit_ILI9341* display, const char* title);

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
