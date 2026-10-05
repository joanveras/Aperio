#include "ui/UiStyle.hpp"

namespace
{
  constexpr int16_t TITLE_X = 10;
  constexpr int16_t TITLE_Y = 10;

  constexpr int16_t RULE_Y = 33;
  constexpr int16_t RULE_ACCENT_WIDTH = 18;
  constexpr int16_t RULE_MARGIN = 10;

  constexpr int16_t DIVIDER_MARGIN = 8;

  constexpr int16_t SELECTION_BAR_WIDTH = 3;
}

void UiStyle::drawTitle(
  Adafruit_ILI9341* display,
  const char* title,
  int16_t ruleEndX
)
{
  display->setTextWrap(false);
  display->setTextSize(2);
  display->setTextColor(UiColor::TEXT, UiColor::BACKGROUND);
  display->setCursor(TITLE_X, TITLE_Y);
  display->print(title);

  display->fillRect(
    TITLE_X,
    RULE_Y,
    RULE_ACCENT_WIDTH,
    2,
    UiColor::ACCENT
  );

  const int16_t lineStart = TITLE_X + RULE_ACCENT_WIDTH + 2;
  const int16_t lineEnd = (ruleEndX >= 0)
    ? ruleEndX
    : display->width() - RULE_MARGIN;

  if (lineEnd > lineStart)
  {
    display->drawFastHLine(
      lineStart,
      RULE_Y + 1,
      lineEnd - lineStart,
      UiColor::LINE
    );
  }
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
