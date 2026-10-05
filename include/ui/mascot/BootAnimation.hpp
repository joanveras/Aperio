#pragma once

#include <Adafruit_ILI9341.h>

#include "BootTimeline.hpp"
#include "MascotCanvas.hpp"
#include "MascotRenderer.hpp"
#include "MascotView.hpp"

// Plays the boot sequence without blocking: call update() from the main
// loop until isFinished() returns true.
//
// The story itself lives in BootTimeline; this class only turns it into
// frames on the display and types the title below the eye.
class BootAnimation
{
public:
  explicit BootAnimation(Adafruit_ILI9341* displayInstance);

  void begin(uint32_t now);
  void update(uint32_t now);

  // Ends the animation right away (e.g. a button was pressed).
  void skip();

  bool isFinished() const;

private:
  // 120 x 72 logical pixels, shown at 2x: 240 x 144 on the TFT.
  static constexpr int16_t CANVAS_WIDTH = 120;
  static constexpr int16_t CANVAS_HEIGHT = 72;
  static constexpr uint8_t PIXEL_SCALE = 2;
  static constexpr int16_t CANVAS_TOP = 12;

  static constexpr int16_t TITLE_TOP = 170;
  static constexpr int16_t SUBTITLE_TOP = 212;

  // ~30 fps
  static constexpr uint32_t FRAME_INTERVAL_MS = 33;

  Adafruit_ILI9341* display;

  MascotCanvas canvas;
  MascotRenderer renderer;
  MascotView view;

  uint32_t startTime;
  uint32_t lastFrameTime;
  bool finished;

  uint8_t titleCharsShown;
  uint8_t subtitleCharsShown;

  // Performance numbers, printed to Serial when the boot ends.
  uint32_t frameCount;
  uint32_t totalRenderMicros;
  uint32_t totalPresentMicros;

  void drawText(const BootFrame& frame);

  void typeText(
    const char* text,
    int16_t y,
    uint8_t size,
    uint16_t color,
    float progress,
    uint8_t& charsShown
  );

  void finish();
};
