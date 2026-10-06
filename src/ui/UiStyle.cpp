#include "ui/UiStyle.hpp"

#include <string.h>

namespace
{
  constexpr int16_t TITLE_X = 10;
  constexpr int16_t TITLE_Y = 12;

  constexpr int16_t RULE_Y = 34;
  constexpr int16_t RULE_MARGIN = 10;

  constexpr int16_t TAG_Y = 18;


  constexpr int16_t FOOTER_RULE_Y = 200;
  constexpr int16_t FOOTER_NAV_Y = 208;
  constexpr int16_t FOOTER_HINT_Y = 227;
  constexpr int16_t FOOTER_SIDE_X = 14;

  // Where source pixel `i` starts once scaled to 1.5x, and how wide it is.
  int16_t scaledStart(int16_t i)
  {
    return i + (i + 1) / 2;
  }

  int16_t scaledSpan(int16_t i)
  {
    return i % 2 == 0 ? 2 : 1;
  }

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
  const char* title
)
{
  display->setTextWrap(false);
  printBold(display, TITLE_X, TITLE_Y, title, 2, UiColor::TEXT);

  display->drawFastHLine(
    TITLE_X,
    RULE_Y,
    display->width() - RULE_MARGIN - TITLE_X,
    UiColor::ACCENT_SOFT
  );
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

void UiStyle::drawFooter(
  Adafruit_ILI9341* display,
  const char* action,
  const char* previous,
  const char* next,
  const char* hint
)
{
  const int16_t width = display->width();

  // The footer may be redrawn with different labels: start clean.
  display->fillRect(
    0,
    FOOTER_RULE_Y,
    width,
    display->height() - FOOTER_RULE_Y,
    UiColor::BACKGROUND
  );

  display->drawFastHLine(
    RULE_MARGIN,
    FOOTER_RULE_Y,
    width - 2 * RULE_MARGIN,
    UiColor::ACCENT_SOFT
  );

  display->setTextSize(1);
  display->setTextColor(UiColor::ACCENT_SOFT, UiColor::BACKGROUND);

  if (previous != nullptr)
  {
    display->setCursor(FOOTER_SIDE_X, FOOTER_NAV_Y);
    display->print(previous);
  }

  if (next != nullptr)
  {
    display->setCursor(width - FOOTER_SIDE_X - textWidth(display, next), FOOTER_NAV_Y);
    display->print(next);
  }

  if (action != nullptr)
  {
    UiStyle::drawFooterAction(display, action, FOOTER_NAV_Y);
  }

  if (hint != nullptr)
  {
    display->setTextSize(1);
    display->setTextColor(UiColor::TEXT_DIM, UiColor::BACKGROUND);
    display->setCursor((width - textWidth(display, hint)) / 2, FOOTER_HINT_Y);
    display->print(hint);
  }
}

// The built-in font only scales by whole numbers (size 2 is too big in
// places), so the text is rendered at size 1 off-screen and drawn at 1.5x:
// source rows and columns alternate between 2 and 1 pixels.
void UiStyle::drawLargeText(
  Adafruit_ILI9341* display,
  const char* text,
  int16_t x,
  int16_t y,
  uint16_t color,
  uint16_t background
)
{
  const int16_t sourceWidth = strlen(text) * 6;

  GFXcanvas1 canvas(sourceWidth, 8);

  if (canvas.getBuffer() == nullptr)
  {
    return;
  }

  canvas.setTextWrap(false);
  canvas.setTextSize(1);
  canvas.setTextColor(1);
  canvas.setCursor(0, 0);
  canvas.print(text);

  display->fillRect(x, y, largeTextWidth(text), LARGE_TEXT_HEIGHT, background);

  for (int16_t row = 0; row < 8; row++)
  {
    for (int16_t column = 0; column < sourceWidth; column++)
    {
      if (canvas.getPixel(column, row))
      {
        display->fillRect(
          x + scaledStart(column),
          y + scaledStart(row),
          scaledSpan(column),
          scaledSpan(row),
          color
        );
      }
    }
  }
}

int16_t UiStyle::largeTextWidth(const char* text)
{
  const int16_t length = strlen(text);

  return length == 0 ? 0 : scaledStart(length * 6 - 1) + 1;
}

void UiStyle::drawFooterAction(
  Adafruit_ILI9341* display,
  const char* text,
  int16_t y
)
{
  drawLargeText(
    display,
    text,
    (display->width() - largeTextWidth(text)) / 2,
    y + 3 - (LARGE_TEXT_HEIGHT - 1) / 2,
    UiColor::ACCENT,
    UiColor::BACKGROUND
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
