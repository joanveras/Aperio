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
// It gives the eye its personality. The overlay runs a small state machine:
//
//   ENTERING  the eye materializes from scattered signal pixels (~1.5 s)
//   IDLE      calm breathing, with occasional micro-animations picked by a
//             weighted, non-repeating scheduler: blink, double blink, look
//             to the side, a "curious" glance, a strong pulse, a rare glitch
//   SLEEPING  after a long quiet stretch the waves fade and the eye closes
//   WAKING    any button makes it wake (a signal, the eye opens, a pulse),
//             after which the AperioApp returns to the previous screen
//
// Ambient signal particles drift around it the whole time, and the iris
// throws the odd bright glint, so the screen never looks static.
//
// Everything is driven by building a MascotPose each frame and handing it to
// the shared MascotRenderer -- the same pipeline as the boot animation, so
// no renderer changes are needed. Nothing blocks; all timing is millis().
class MascotIdleAnimator
{
public:
  explicit MascotIdleAnimator(Adafruit_ILI9341* displayInstance);

  // Allocates the canvas, clears the screen and starts the entry transition.
  // Returns false if the canvas could not be allocated (the AperioApp then
  // simply stays on the current screen and tries again later).
  bool begin(uint32_t now);
  void end();

  // Any button was pressed: play the wake sequence once, from whatever state
  // the mascot is in. The button itself is swallowed by the AperioApp, so it
  // only wakes and does not act on the screen underneath.
  void wake(uint32_t now);
  bool isWaking() const;

  // True once the wake sequence has fully played, so the AperioApp can end
  // the overlay and repaint the screen underneath.
  bool wakeFinished() const;

  // Advances the animation to `now`. Returns true when a new frame is due
  // (~30 fps), so the caller can throttle present().
  bool update(uint32_t now);

  // Draws the current frame. Cheap; safe to call every frame.
  void present();

private:
  // 150 x 116 logical px shown at 2x: 300 x 232 on the 320 x 240 TFT, with a
  // small margin all around for the eye to float into. The eye (~68 wide)
  // sits centered, with room for generous waves and drifting particles.
  static constexpr int16_t CANVAS_WIDTH = 150;
  static constexpr int16_t CANVAS_HEIGHT = 116;
  static constexpr uint8_t PIXEL_SCALE = 2;

  static constexpr uint32_t FRAME_INTERVAL_MS = 33;  // ~30 fps

  // Entry transition: dark -> pixels -> fragments -> eye -> waves -> idle.
  static constexpr uint32_t ENTER_MS = 1500;

  // Idle breathing.
  static constexpr float IDLE_WAVE_INTENSITY = 0.6f;
  static constexpr float IDLE_WAVE_RADIUS = 40.0f;
  static constexpr float IDLE_WAVE_RADIUS_BREATH = 4.0f;
  static constexpr float IDLE_WAVE_SPACING = 6.0f;
  static constexpr float IDLE_WAVE_SPAN = 0.5f;
  static constexpr float IDLE_WAVE_SPAN_VAR = 0.45f;  // how much arc lengths vary
  static constexpr float WAVE_PHASE_PER_MS = 1.0f / 3000.0f;  // ~3 s breath
  static constexpr float RING_TURNS_PER_MS = 0.00003f;

  // Gentle float: the eye wanders a few pixels within the screen margin.
  static constexpr float FLOAT_X_AMP = 3.0f;   // display px
  static constexpr float FLOAT_Y_AMP = 4.0f;   // display px
  static constexpr float FLOAT_X_PER_MS = 1.0f / 9000.0f;
  static constexpr float FLOAT_Y_PER_MS = 1.0f / 6500.0f;

  // Micro-animations: gap between one gesture ending and the next starting.
  static constexpr uint32_t GESTURE_MIN_GAP_MS = 1500;
  static constexpr uint32_t GESTURE_VAR_MS = 3200;

  static constexpr uint32_t BLINK_MS = 360;
  static constexpr uint32_t DBLBLINK_MS = 760;
  static constexpr uint32_t LOOK_MS = 1100;
  static constexpr uint32_t CURIOUS_MS = 1700;
  static constexpr uint32_t PULSE_MS = 1000;
  static constexpr uint32_t GLITCH_MS = 320;

  // Sleep: after this long with no button in IDLE, the eye drifts off.
  static constexpr uint32_t IDLE_TO_SLEEP_MS = 120000;  // ~2 min
  static constexpr uint32_t SLEEP_TRANS_MS = 2200;

  // Wake sequence length (covers waking from sleep, i.e. eye closed -> open).
  static constexpr uint32_t WAKE_MS = 620;

  // Particles: a small pool of drifting signal pixels and iris glints.
  static constexpr int PARTICLE_COUNT = 22;

  enum class Phase : uint8_t { ENTERING, IDLE, SLEEPING, WAKING };

  enum class Gesture : uint8_t
  {
    NONE, BLINK, DOUBLE_BLINK, LOOK, CURIOUS, PULSE, GLITCH
  };

  struct Particle
  {
    bool active;
    uint8_t kind;      // 0 = ambient signal mote, 1 = iris glint
    int16_t x;
    int16_t y;
    uint32_t born;
    uint16_t life;     // ms
    float peak;        // 0..1 brightness at the middle of its life
  };

  Adafruit_ILI9341* display;

  int16_t baseX;
  int16_t baseY;
  int16_t posX;
  int16_t posY;

  MascotCanvas canvas;
  MascotRenderer renderer;
  MascotView view;

  uint32_t lastUpdate;
  uint32_t lastFrame;

  Phase phase;
  uint32_t phaseStart;    // when ENTERING / SLEEPING began
  uint32_t idleEnteredAt; // when IDLE began, for the sleep timer

  Gesture gesture;
  Gesture lastGesture;
  uint32_t gestureStart;
  uint32_t gestureDur;
  uint32_t nextGestureAt;
  int gestureDir;         // -1 / +1 for look / curious

  bool waking;
  uint32_t wakeStart;
  float wakeFromOpen;     // eyeOpen when the wake began

  Particle particles[PARTICLE_COUNT];
  uint32_t rng;

  MascotPose pose;        // rebuilt every update()

  // Pose builders for each phase / gesture.
  void buildBreathingPose(uint32_t now);
  void buildEntryPose(uint32_t now);
  void buildSleepPose(uint32_t now);
  void buildWakePose(uint32_t now);
  void applyGesture(uint32_t now);

  // Micro-animation scheduler.
  void startGesture(uint32_t now);
  void scheduleNextGesture(uint32_t now);
  uint32_t gestureDuration(Gesture g) const;

  // Float + particles.
  void updateFloat(uint32_t now);
  void updateParticles(uint32_t now);
  void spawnParticle(uint32_t now, uint8_t kind);
  void drawParticles(uint32_t now);

  uint32_t nextRandom();
  float randUnit();  // 0..1
};
