#pragma once

#include <Adafruit_ILI9341.h>

#include "MascotCanvas.hpp"
#include "MascotRenderer.hpp"
#include "MascotView.hpp"
#include "MascotPose.hpp"

// A small, self-contained mascot for the menu headers: the eye with its
// signal waves around it (no marks), drawn at 1x in the top-right corner.
//
// It lives on its own: it blinks by itself every few seconds and flicks its
// gaze to the side when the user navigates, so the menus feel alive without
// the screens having to know anything about how the eye is drawn. A screen:
//   - calls begin() when it is shown,
//   - tells the badge the user moved (glance),
//   - calls update() from its update() and present() at the END of its
//     render(), so a full-screen redraw never erases the eye.
//
// Nothing here blocks; all timing is millis()-based, like the boot anim.
class MascotMenuBadge
{
public:
  // The eye is ~68x34 logical pixels; the canvas is wider and taller so the
  // signal waves have room on both sides. Drawn at 1x, so canvas px ==
  // display px. Kept short enough to clear the first menu row below.
  static constexpr int16_t CANVAS_WIDTH = 112;
  static constexpr int16_t CANVAS_HEIGHT = 47;
  static constexpr uint8_t PIXEL_SCALE = 1;

  MascotMenuBadge(
    Adafruit_ILI9341* displayInstance,
    int16_t x,
    int16_t y
  );

  // Allocates the canvas and resets the animation. Returns false if the
  // canvas could not be allocated (the screen then simply shows no mascot).
  bool begin(uint32_t now);
  void end();

  // The user moved the selection: -1 = previous (left button),
  // +1 = next (right button). The eye flicks that way and eases back.
  void glance(int direction, uint32_t now);

  // Advances the animation to `now`. Returns true when enough time has
  // passed for a new frame (~30 fps), so the caller can throttle present().
  bool update(uint32_t now);

  // Draws the current pose. Safe to call every frame; cheap.
  void present();

private:
  static constexpr uint32_t FRAME_INTERVAL_MS = 33;  // ~30 fps

  // Gaze flick.
  static constexpr float GAZE_MAX = 0.85f;     // how far to the side
  static constexpr float GAZE_TAU_MS = 70.0f;  // easing time constant
  static constexpr uint32_t GAZE_HOLD_MS = 260; // dwell before returning

  // Blink.
  static constexpr uint32_t BLINK_MS = 130;
  static constexpr uint32_t BLINK_MIN_MS = 2600;
  static constexpr uint32_t BLINK_VAR_MS = 3400;

  // Idle: a very slow iris-ring rotation, for a quiet "scanning" feel.
  static constexpr float RING_TURNS_PER_MS = 0.00003f;

  // Signal waves: compact arcs that breathe around the eye. The radius and
  // span are tuned so three arcs per side fit inside the canvas.
  static constexpr float WAVE_INTENSITY = 0.6f;
  static constexpr float WAVE_RADIUS = 38.0f;
  static constexpr float WAVE_SPACING = 5.0f;
  static constexpr float WAVE_SPAN = 0.42f;
  static constexpr float WAVE_PHASE_PER_MS = 0.0004f;  // ~2.5 s per breath

  Adafruit_ILI9341* display;
  int16_t posX;
  int16_t posY;

  MascotCanvas canvas;
  MascotRenderer renderer;
  MascotView view;

  uint32_t lastUpdate;
  uint32_t lastFrame;

  float gaze;        // current horizontal gaze, -1..1
  float gazeTarget;  // where the gaze is easing to
  uint32_t gazeHoldUntil;

  bool blinking;
  uint32_t blinkStart;
  uint32_t nextBlink;

  uint32_t rng;      // tiny LCG, so blinks are not perfectly periodic

  MascotPose pose;   // rebuilt every update()

  void scheduleNextBlink(uint32_t now);
  uint32_t nextRandom();
};
