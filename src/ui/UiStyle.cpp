#include "ui/UiStyle.hpp"

#include <string.h>

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

// The built-in font only scales by whole numbers (size 2 is too big here),
// so the text is rendered at size 1 off-screen and drawn at 1.5x: source
// rows and columns alternate between 2 and 1 pixels.
void UiStyle::drawFooterAction(
  Adafruit_ILI9341* display,
  const char* text,
  int16_t y
)
{
  const int16_t length = strlen(text);
  const int16_t sourceWidth = length * 6;
  constexpr int16_t SOURCE_HEIGHT = 7;

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

  // Where source pixel `i` starts once scaled, and how wide it is.
  auto start = [](int16_t i) { return static_cast<int16_t>(i + (i + 1) / 2); };
  auto span = [](int16_t i) { return static_cast<int16_t>(i % 2 == 0 ? 2 : 1); };

  const int16_t width = start(sourceWidth - 1) + 1;
  const int16_t height = start(SOURCE_HEIGHT);
  const int16_t left = (display->width() - width) / 2;
  const int16_t top = y + 3 - height / 2;

  display->fillRect(left, top, width, height, UiColor::BACKGROUND);

  for (int16_t row = 0; row < SOURCE_HEIGHT; row++)
  {
    for (int16_t column = 0; column < sourceWidth; column++)
    {
      if (canvas.getPixel(column, row))
      {
        display->fillRect(
          left + start(column),
          top + start(row),
          span(column),
          span(row),
          UiColor::ACCENT
        );
      }
    }
  }

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
