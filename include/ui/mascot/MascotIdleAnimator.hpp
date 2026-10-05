#pragma once

#include <Adafruit_ILI9341.h>

#include "MascotCanvas.hpp"
#include "MascotRenderer.hpp"
#include "MascotView.hpp"
#include "MascotPose.hpp"

// The mascot's home: a full-screen idle animation it falls into after a
// while with no input. This is NOT a Screen -- the AperioApp runs it as a
// global inactivity overlay, on top of (and without touching) whatever
// screen the user was on, exactly like the boot animation.
//
// This first version is deliberately small, to prove the plumbing:
//   - a calm "breathing" idle: the eye is open, pupil centered, with faint
//     signal waves that expand and contract slowly;
//   - a very gentle float, so the eye drifts a few pixels instead of
//     sitting perfectly still;
//   - a "wake" pulse: any button makes the mascot react with a bright
//     pulse, after which the AperioApp returns to the previous screen.
//
// The microanimations (blink, look, pulse, curious), sleep and the Wi-Fi
// scan animation come as later, separate steps -- they are just new ways to
// drive the same MascotPose, so nothing here needs to change for them.
//
// Nothing blocks: all timing is millis()-based, like the boot animation.
class MascotIdleAnimator
{
public:
  explicit MascotIdleAnimator(Adafruit_ILI9341* displayInstance);

  // Allocates the canvas, clears the screen and starts the idle breathing.
  // Returns false if the canvas could not be allocated (the AperioApp then
  // simply stays on the current screen and tries again later).
  bool begin(uint32_t now);
  void end();

  // Any button was pressed: play the wake pulse once. The button itself is
  // swallowed by the AperioApp, so it only wakes and does not act.
  void wake(uint32_t now);
  bool isWaking() const;

  // True once the wake pulse has fully played, so the AperioApp can end the
  // overlay and repaint the screen underneath.
  bool wakeFinished() const;

  // Advances the animation to `now`. Returns true when a new frame is due
  // (~30 fps), so the caller can throttle present().
  bool update(uint32_t now);

  // Draws the current pose. Cheap; safe to call every frame.
  void present();

private:
  // 150 x 116 logical px shown at 2x: 300 x 232 on the 320 x 240 TFT, which
  // leaves a small margin all around for the eye to float into without ever
  // running off the panel. The eye (~68 wide) sits centered, with plenty of
  // black room around it for generous waves.
  static constexpr int16_t CANVAS_WIDTH = 150;
  static constexpr int16_t CANVAS_HEIGHT = 116;
  static constexpr uint8_t PIXEL_SCALE = 2;

  static constexpr uint32_t FRAME_INTERVAL_MS = 33;  // ~30 fps

  // Idle breathing. Faint waves, a slow radius breath (~3 s cycle) and a
  // barely-there iris-ring rotation for a quiet "alive" feel.
  static constexpr float IDLE_WAVE_INTENSITY = 0.6f;
  static constexpr float IDLE_WAVE_RADIUS = 40.0f;
  static constexpr float IDLE_WAVE_RADIUS_BREATH = 4.0f;
  static constexpr float IDLE_WAVE_SPACING = 6.0f;
  static constexpr float IDLE_WAVE_SPAN = 0.5f;
  static constexpr float WAVE_PHASE_PER_MS = 1.0f / 3000.0f;  // ~3 s breath
  static constexpr float RING_TURNS_PER_MS = 0.00003f;

  // Gentle float: the eye drifts within the screen margin, two slow sines
  // at different periods so it wanders instead of bobbing in a line.
  static constexpr float FLOAT_X_AMP = 2.0f;   // display px
  static constexpr float FLOAT_Y_AMP = 3.0f;   // display px
  static constexpr float FLOAT_X_PER_MS = 1.0f / 8000.0f;
  static constexpr float FLOAT_Y_PER_MS = 1.0f / 6000.0f;

  // Wake pulse: a short, bright flash with the waves blooming outward.
  static constexpr uint32_t WAKE_MS = 450;
  static constexpr float WAKE_FLASH = 0.8f;
  static constexpr float WAKE_WAVE_EXPAND = 20.0f;
  static constexpr float WAKE_SPACING_EXPAND = 3.0f;

  Adafruit_ILI9341* display;

  int16_t baseX;  // centered origin of the canvas on the TFT
  int16_t baseY;
  int16_t posX;   // current (floated) origin
  int16_t posY;

  MascotCanvas canvas;
  MascotRenderer renderer;
  MascotView view;

  uint32_t lastUpdate;
  uint32_t lastFrame;

  bool waking;
  uint32_t wakeStart;

  MascotPose pose;  // rebuilt every update()

  void buildIdlePose(uint32_t now);
  void buildWakePose(uint32_t now);
};
