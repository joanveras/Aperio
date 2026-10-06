#include "ui/UiStyle.hpp"

namespace
{
  constexpr int16_t TITLE_X = 10;
  constexpr int16_t TITLE_Y = 12;

  constexpr int16_t RULE_Y = 34;
  constexpr int16_t RULE_MARGIN = 10;

  constexpr int16_t TAG_Y = 18;

  constexpr int16_t DIVIDER_MARGIN = 8;

  constexpr int16_t FOOTER_RULE_Y = 200;
  constexpr int16_t FOOTER_NAV_Y = 208;
  constexpr int16_t FOOTER_HINT_Y = 227;
  constexpr int16_t FOOTER_SIDE_X = 14;

  int16_t textWidth(Adafruit_ILI9341* display, const char* text)
  {
    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;

    display->getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

    return width;
  }

  // The built-in font, thickened by drawing it twice one pixel apart. The
  // first pass has a solid background (so it replaces what was there), the
  // second one is transparent (so it does not erase the first).
  void printBold(
    Adafruit_ILI9341* display,
    int16_t x,
    int16_t y,
    const char* text,
    uint8_t size,
    uint16_t color
  )
  {
    display->setTextSize(size);

    display->setTextColor(color, UiColor::BACKGROUND);
    display->setCursor(x, y);
    display->print(text);

    display->setTextColor(color);
    display->setCursor(x + 1, y);
    display->print(text);
  }
}

void UiStyle::drawTitle(
  Adafruit_ILI9341* display,
  const char* title,
  int16_t ruleEndX
)
{
  display->setTextWrap(false);
  printBold(display, TITLE_X, TITLE_Y, title, 2, UiColor::TEXT);

  const int16_t lineEnd = (ruleEndX >= 0)
    ? ruleEndX
    : display->width() - RULE_MARGIN;

  if (lineEnd > TITLE_X)
  {
    display->drawFastHLine(
      TITLE_X,
      RULE_Y,
      lineEnd - TITLE_X,
      UiColor::ACCENT_SOFT
    );
  }
}

void UiStyle::drawTitleTag(Adafruit_ILI9341* display, const char* text)
{
  display->setTextSize(1);
  display->setTextColor(UiColor::ACCENT_SOFT, UiColor::BACKGROUND);
  display->setCursor(
    display->width() - RULE_MARGIN - textWidth(display, text),
    TAG_Y
  );
  display->print(text);
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

void UiStyle::drawMenuFooter(Adafruit_ILI9341* display)
{
  const int16_t width = display->width();

  display->drawFastHLine(
    RULE_MARGIN,
    FOOTER_RULE_Y,
    width - 2 * RULE_MARGIN,
    UiColor::ACCENT_SOFT
  );

  display->setTextSize(1);
  display->setTextColor(UiColor::ACCENT_SOFT, UiColor::BACKGROUND);

  display->setCursor(FOOTER_SIDE_X, FOOTER_NAV_Y);
  display->print("< PREV");

  const char* next = "NEXT >";
  display->setCursor(width - FOOTER_SIDE_X - textWidth(display, next), FOOTER_NAV_Y);
  display->print(next);

  UiStyle::drawFooterAction(display, "OK", FOOTER_NAV_Y);

  const char* hint = "HOLD OK : BACK";

  display->setTextSize(1);
  display->setTextColor(UiColor::TEXT_DIM, UiColor::BACKGROUND);
  display->setCursor((width - textWidth(display, hint)) / 2, FOOTER_HINT_Y);
  display->print(hint);
}

void UiStyle::drawFooterAction(
  Adafruit_ILI9341* display,
  const char* text,
  int16_t y
)
{
  display->setTextSize(2);
  const int16_t width = textWidth(display, text) + 1;

  printBold(
    display,
    (display->width() - width) / 2,
    y - 4,
    text,
    2,
    UiColor::ACCENT
  );

  display->setTextSize(1);
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
}
