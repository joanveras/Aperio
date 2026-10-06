#pragma once

#include <Adafruit_ILI9341.h>
#include <cstdint>

#include "BootCanvas.hpp"

// The retro green boot screen: a CRT turning on, corner brackets, the
// signal emblem, the APERIO logo, an init log with a progress bar and a
// dotted wave terrain at the bottom.
//
// Non-blocking: call update() from the main loop until isFinished().
// Static parts are drawn once straight on the display; only the emblem and
// the terrain are redrawn every frame, through two small canvases.
class RetroBoot
{
public:
  explicit RetroBoot(Adafruit_ILI9341* displayInstance);

  void begin(uint32_t now);
  void update(uint32_t now);

  // Ends the animation right away (e.g. a button was pressed).
  void skip();

  bool isFinished() const;

private:
  static constexpr uint8_t LOG_LINES = 5;

  // ~30 fps
  static constexpr uint32_t FRAME_INTERVAL_MS = 33;

  Adafruit_ILI9341* display;

  BootCanvas emblemCanvas;
  BootCanvas terrainCanvas;

  uint32_t startTime;
  uint32_t lastFrameTime;
  bool finished;

  // What has already been drawn on the display, so each frame only adds
  // what is new.
  bool screenCleared;
  float bracketsShown;
  uint8_t versionChars;
  uint8_t logoStates[6];
  float dashesShown;
  uint8_t subtitleChars;
  bool barFrameShown;
  uint8_t logChars[LOG_LINES];
  bool logOkShown[LOG_LINES];
  uint8_t loadingDots;
  uint8_t segmentsShown;
  uint8_t segmentsTone;

  // Performance numbers, printed to Serial when the boot ends.
  uint32_t frameCount;
  uint32_t totalFrameMicros;

  void drawPowerOn(uint32_t elapsed);
  void drawFrame(uint32_t elapsed);
  void drawBrackets(uint32_t elapsed);
  void drawVersion(uint32_t elapsed);
  void drawLogo(uint32_t elapsed);
  void drawSubtitle(uint32_t elapsed);
  void drawLog(uint32_t elapsed);
  void drawProgress(uint32_t elapsed);
  void drawEmblem(uint32_t elapsed);
  void drawTerrain(uint32_t elapsed);

  void finish();
};
