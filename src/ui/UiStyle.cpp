#include "ui/UiStyle.hpp"

#include "ui/fonts/CascadiaMono.h"

// Defined here only, so the font data lives in one place in flash.
namespace UiFont
{
  const GFXfont* TITLE = &Cascadia_Title;  // Bold, screen titles
  const GFXfont* ITEM = &Cascadia_Item;    // SemiBold, menu items
  const GFXfont* BODY = &Cascadia_Body;    // Regular, larger body text
}

namespace
{
  constexpr int16_t TITLE_X = 10;
  constexpr int16_t TITLE_Y = 9;

  constexpr int16_t RULE_Y = 33;
  constexpr int16_t RULE_ACCENT_WIDTH = 18;
  constexpr int16_t RULE_MARGIN = 10;

  constexpr int16_t DIVIDER_MARGIN = 8;

  constexpr int16_t SELECTION_BAR_WIDTH = 3;
}

void UiStyle::text(
  Adafruit_ILI9341* display,
  const GFXfont* font,
  int16_t x,
  int16_t topY,
  uint16_t color,
  const char* text
)
{
  display->setFont(font);
  display->setTextColor(color);
  display->setTextWrap(false);

  // A custom font places the cursor at the baseline, not the top-left.
  // getTextBounds reports how far above the cursor the glyphs reach (y1 is
  // negative), so topY - y1 puts the top of the text exactly at topY.
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display->getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  display->setCursor(x - x1, topY - y1);
  display->print(text);

  display->setFont(nullptr);
}

void UiStyle::textCentered(
  Adafruit_ILI9341* display,
  const GFXfont* font,
  int16_t topY,
  uint16_t color,
  const char* text
)
{
  display->setFont(font);
  display->setTextColor(color);
  display->setTextWrap(false);

  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display->getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  display->setCursor((display->width() - width) / 2 - x1, topY - y1);
  display->print(text);

  display->setFont(nullptr);
}

void UiStyle::drawTitle(Adafruit_ILI9341* display, const char* title)
{
  text(display, UiFont::TITLE, TITLE_X, TITLE_Y, UiColor::TEXT, title);

  display->fillRect(
    TITLE_X,
    RULE_Y,
    RULE_ACCENT_WIDTH,
    2,
    UiColor::ACCENT
  );

  display->drawFastHLine(
    TITLE_X + RULE_ACCENT_WIDTH + 2,
    RULE_Y + 1,
    display->width() - TITLE_X - RULE_ACCENT_WIDTH - 2 - RULE_MARGIN,
    UiColor::LINE
  );
}

void UiStyle::drawDivider(Adafruit_ILI9341* display, int16_t y)
{
  display->drawFastHLine(
    DIVIDER_MARGIN,
    y,
    display->width() - 2 * DIVIDER_MARGIN,
    UiColor::LINE
  );
}

void UiStyle::drawSelection(
  Adafruit_ILI9341* display,
  int16_t x,
  int16_t y,
  int16_t width,
  int16_t height
)
{
  display->fillRect(x, y, width, height, UiColor::SELECTION);
  display->fillRect(x, y, SELECTION_BAR_WIDTH, height, UiColor::ACCENT);
}
