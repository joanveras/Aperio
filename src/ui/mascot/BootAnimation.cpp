#include "ui/mascot/BootAnimation.hpp"
#include "ui/mascot/MascotPalette.hpp"

#include <Arduino.h>
#include <cstring>

namespace
{
  constexpr const char* TITLE = "APERIO";
  constexpr const char* SUBTITLE = "Quod Latet";

  // Adafruit GFX built-in font: 5x7 glyphs on a 6x8 grid, times the size.
  constexpr int16_t GLYPH_ADVANCE = 6;
}

BootAnimation::BootAnimation(Adafruit_ILI9341* displayInstance)
  : display(displayInstance),
    canvas(CANVAS_WIDTH, CANVAS_HEIGHT),
    view(displayInstance, PIXEL_SCALE),
    startTime(0),
    lastFrameTime(0),
    finished(false),
    titleCharsShown(0),
    subtitleCharsShown(0),
    frameCount(0),
    totalRenderMicros(0),
    totalPresentMicros(0)
{
}

void BootAnimation::begin(uint32_t now)
{
  display->fillScreen(ILI9341_BLACK);

  startTime = now;
  lastFrameTime = now - FRAME_INTERVAL_MS;
  finished = false;
  titleCharsShown = 0;
  subtitleCharsShown = 0;
  frameCount = 0;
  totalRenderMicros = 0;
  totalPresentMicros = 0;

  if (!canvas.begin())
  {
    Serial.println("[boot] not enough memory for the mascot, skipping");
    finished = true;
  }
}

void BootAnimation::update(uint32_t now)
{
  if (finished)
  {
    return;
  }

  if (now - lastFrameTime < FRAME_INTERVAL_MS)
  {
    return;
  }

  lastFrameTime = now;

  BootFrame frame = BootTimeline::at(now - startTime);

  if (frame.finished)
  {
    finish();
    return;
  }

  uint32_t renderStart = micros();
  renderer.draw(canvas, frame.pose);

  uint32_t presentStart = micros();
  view.present(
    canvas,
    (display->width() - CANVAS_WIDTH * PIXEL_SCALE) / 2,
    CANVAS_TOP,
    frame.pose.flash,
    frame.pose.glitch
  );

  uint32_t presentEnd = micros();

  frameCount++;
  totalRenderMicros += presentStart - renderStart;
  totalPresentMicros += presentEnd - presentStart;

  drawText(frame);
}

void BootAnimation::skip()
{
  if (!finished)
  {
    finish();
  }
}

bool BootAnimation::isFinished() const
{
  return finished;
}

void BootAnimation::drawText(const BootFrame& frame)
{
  uint16_t palette[MascotColor::COUNT];
  MascotPalette::build(palette, 0.0f);

  typeText(
    TITLE,
    TITLE_TOP,
    4,
    palette[MascotColor::WHITE],
    frame.titleProgress,
    titleCharsShown
  );

  typeText(
    SUBTITLE,
    SUBTITLE_TOP,
    2,
    palette[MascotColor::SIGNAL_LAST - 1],
    frame.subtitleProgress,
    subtitleCharsShown
  );
}

// Typewriter effect: draws only the characters that became visible since
// the last frame, so the text never needs to be redrawn.
void BootAnimation::typeText(
  const char* text,
  int16_t y,
  uint8_t size,
  uint16_t color,
  float progress,
  uint8_t& charsShown
)
{
  const uint8_t length = strlen(text);
  const uint8_t visible = static_cast<uint8_t>(progress * length);

  const int16_t advance = GLYPH_ADVANCE * size;
  const int16_t textWidth = length * advance - size;
  const int16_t left = (display->width() - textWidth) / 2;

  while (charsShown < visible && charsShown < length)
  {
    display->drawChar(
      left + charsShown * advance,
      y,
      text[charsShown],
      color,
      ILI9341_BLACK,
      size
    );

    charsShown++;
  }
}

void BootAnimation::finish()
{
  finished = true;
  canvas.end();

  if (frameCount == 0)
  {
    return;
  }

  Serial.printf(
    "[boot] %lu frames, avg render %lu us, avg present %lu us\n",
    static_cast<unsigned long>(frameCount),
    static_cast<unsigned long>(totalRenderMicros / frameCount),
    static_cast<unsigned long>(totalPresentMicros / frameCount)
  );
}
