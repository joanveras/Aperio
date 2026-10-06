#pragma once

#include <Adafruit_ILI9341.h>
#include <cstdint>

// Colors and small drawing helpers shared by every screen.
//
// Retro green CRT theme: black background, pale green text and a phosphor
// green accent for rules, selection and hints. Warning and danger exist for
// the few places that must stand out (TX Lab, errors) and should stay rare.
namespace UiColor
{
  constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b)
  {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }

  constexpr uint16_t BACKGROUND = rgb(0, 0, 0);

  constexpr uint16_t TEXT = rgb(226, 255, 230);       // titles, values, selection
  constexpr uint16_t TEXT_MUTED = rgb(146, 182, 152); // labels, unselected items
  constexpr uint16_t TEXT_DIM = rgb(102, 132, 108);   // hints, secondary info

  constexpr uint16_t ACCENT = rgb(84, 255, 132);      // phosphor green
  constexpr uint16_t ACCENT_SOFT = rgb(62, 196, 104); // rules, bars, quieter highlights
  constexpr uint16_t LINE = rgb(28, 92, 48);          // thin dividers
  constexpr uint16_t SELECTION = rgb(14, 56, 26);     // selected row background

  constexpr uint16_t WARNING = rgb(255, 190, 70);
  constexpr uint16_t DANGER = rgb(240, 84, 96);
}

namespace UiStyle
{
  // Screen title at the top left (text size 2, drawn bold), underlined by a
  // green rule across the screen.
  //
  // `ruleEndX` is where the rule stops on the right. Leave it at -1 for the
  // full width.
  void drawTitle(
    Adafruit_ILI9341* display,
    const char* title,
    int16_t ruleEndX = -1
  );

  // Small text on the right of the title line, e.g. the firmware version.
  void drawTitleTag(Adafruit_ILI9341* display, const char* text);

  // Thin horizontal divider across the screen.
  void drawDivider(Adafruit_ILI9341* display, int16_t y);

  // The menus' footer: a green rule, "< PREV   OK   NEXT >" and, below,
  // "HOLD OK : BACK".
  void drawMenuFooter(Adafruit_ILI9341* display);

  // Background of a selected row: dark green with a bright bar on the left.
  // Text drawn on it should use UiColor::TEXT on UiColor::SELECTION.
  void drawSelection(
    Adafruit_ILI9341* display,
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height
  );
}
