#include "ui/screens/PlaceholderScreen.hpp"
#include "ui/UiStyle.hpp"

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

  display->fillScreen(UiColor::BACKGROUND);

  // Header
  UiStyle::drawTitle(display, title);

  // Content
  drawCentered(
    "UNDER DEVELOPMENT",
    92,
    2,
    UiColor::ACCENT
  );

  drawCentered(
    "Feature not yet",
    132,
    1,
    UiColor::TEXT_MUTED
  );

  drawCentered(
    "implemented",
    148,
    1,
    UiColor::TEXT_MUTED
  );

  // Footer
  UiStyle::drawDivider(display, 199);

  drawCentered(
    "HOLD OK : BACK",
    216,
    1,
    UiColor::TEXT_DIM
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