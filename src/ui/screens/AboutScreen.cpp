#include "ui/screens/AboutScreen.hpp"

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

  display->fillScreen(ILI9341_BLACK);
  display->setTextWrap(false);

  // Header
  display->setTextSize(2);
  display->setTextColor(ILI9341_WHITE);
  display->setCursor(10, 10);
  display->print("ABOUT");

  display->drawFastHLine(
    8,
    36,
    display->width() - 16,
    ILI9341_WHITE
  );

  // Identidade
  drawCentered(
    "APERIO",
    58,
    2,
    ILI9341_WHITE
  );

  drawCentered(
    "Quod Latet",
    82,
    1,
    ILI9341_WHITE
  );

  // Informações
  display->setTextSize(1);
  display->setTextColor(ILI9341_WHITE);

  display->setCursor(34, 112);
  display->print("Firmware");

  display->setCursor(180, 112);
  display->print("v0.1");

  display->setCursor(34, 132);
  display->print("Platform");

  display->setCursor(180, 132);
  display->print("ESP32");

  display->setCursor(34, 152);
  display->print("Display");

  display->setCursor(180, 152);
  display->print("ILI9341 320x240");

  drawCentered(
    "github.com/joanveras/Aperio",
    178,
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
