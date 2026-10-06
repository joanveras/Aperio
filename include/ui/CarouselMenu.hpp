#pragma once

#include <Adafruit_ILI9341.h>
#include <cstdint>

#include "ui/MenuItem.hpp"

// The menus' look: the selected item sits in the middle of a HUD ring,
// with the previous item above and the next one below, both dimmed and
// soft. The list is circular, so there is always one above and one below.
//
// Screens keep their own selection; these functions only draw.
namespace CarouselMenu
{
  // Clears the screen and draws everything that does not change with the
  // selection: the title (with `subtitle` under it, or a short underline
  // when it is null), the version and the PREV / OK / NEXT footer.
  void drawFrame(
    Adafruit_ILI9341* display,
    const char* title,
    const char* subtitle = nullptr
  );

  // Redraws the middle of the screen for the given selection.
  void drawItems(
    Adafruit_ILI9341* display,
    const MenuItem* items,
    int count,
    int selected
  );
}
