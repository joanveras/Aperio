#include "ui/screens/AboutScreen.hpp"
#include "ui/UiStyle.hpp"

AboutScreen::AboutScreen(Adafruit_ILI9341* displayInstance)
  : display(displayInstance),
    needsRedraw(true)
{
}

void AboutScreen::onEnter()
{
  needsRedraw = true;
}

void AboutScreen::handleInput(InputEvent event)
{
  (void) event;
}

void AboutScreen::update()
{
}

void AboutScreen::render()
{
  if (!needsRedraw)
  {
    return;
  }

  display->fillScreen(UiColor::BACKGROUND);

  UiStyle::drawTitle(display, "ABOUT");

  drawCentered(
    "APERIO",
    58,
    2,
    UiColor::TEXT
  );

  drawCentered(
    "Quod Latet",
    82,
    1,
    UiColor::ACCENT
  );

  display->setTextSize(1);

  display->setTextColor(UiColor::TEXT_MUTED);

  display->setCursor(34, 112);
  display->print("Firmware");

  display->setCursor(34, 132);
  display->print("Platform");

  display->setCursor(34, 152);
  display->print("Display");

  display->setTextColor(UiColor::TEXT);

  display->setCursor(180, 112);
  display->print("v0.1");

  display->setCursor(180, 132);
  display->print("ESP32");

  display->setCursor(180, 152);
  display->print("ILI9341 320x240");

  drawCentered(
    "github.com/joanveras/Aperio",
    184,
    1,
    UiColor::TEXT_DIM
  );

  UiStyle::drawFooter(display, nullptr, nullptr, nullptr);

  needsRedraw = false;
}

void AboutScreen::drawCentered(
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
