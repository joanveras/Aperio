#include "ui/screens/PlaceholderScreen.hpp"

PlaceholderScreen::PlaceholderScreen(
  Adafruit_ILI9341* displayInstance
)
  : display(displayInstance),
    title(""),
    needsRedraw(true)
{
}

void PlaceholderScreen::setTitle(const char* newTitle)
{
  title = newTitle;
  needsRedraw = true;
}

void PlaceholderScreen::onEnter()
{
  needsRedraw = true;
}

void PlaceholderScreen::handleInput(InputEvent event)
{
  (void) event;
}

void PlaceholderScreen::update()
{
}

void PlaceholderScreen::render()
{
  if (!needsRedraw)
  {
    return;
  }

  display->fillScreen(ILI9341_BLACK);
  display->setTextWrap(false);

  // Header
  display->setTextSize(2);
  display->setTextColor(ILI9341_WHITE);
  display->setCursor(10, 10);
  display->print(title);

  display->drawFastHLine(
    8,
    36,
    display->width() - 16,
    ILI9341_WHITE
  );

  // Content
  drawCentered(
    "UNDER DEVELOPMENT",
    92,
    2,
    ILI9341_WHITE
  );

  drawCentered(
    "Feature not yet",
    132,
    1,
    ILI9341_WHITE
  );

  drawCentered(
    "implemented",
    148,
    1,
    ILI9341_WHITE
  );

  // Footer
  display->drawFastHLine(
    8,
    199,
    display->width() - 16,
    ILI9341_WHITE
  );

  drawCentered(
    "HOLD OK : BACK",
    216,
    1,
    ILI9341_WHITE
  );

  needsRedraw = false;
}

void PlaceholderScreen::drawCentered(
  const char* text,
  int y,
  uint8_t textSize,
  uint16_t color
)
{
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display->setTextSize(textSize);
  display->setTextColor(color);

  display->getTextBounds(
    text,
    0,
    0,
    &x1,
    &y1,
    &width,
    &height
  );

  int16_t x = (display->width() - width) / 2;

  display->setCursor(x, y);
  display->print(text);
}