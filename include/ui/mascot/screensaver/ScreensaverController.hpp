#pragma once

#include <Adafruit_ILI9341.h>

#include "../MascotCanvas.hpp"
#include "../MascotView.hpp"
#include "CharacterSprites.hpp"

// The Aperio screensaver: a small narrative that plays after a stretch of
// inactivity, told as a non-blocking state machine over one off-screen
// canvas (composited each frame, then sent to the TFT in one go).
//
// ETAPA 2 covers the opening: an almost-black AMBIENT, the character walking
// IN from the left, a HOLD once he reaches the centre, and the WAKE
// transition that any button triggers to hand the screen back.
//
// It is a global overlay, not a Screen: AperioApp owns it next to the boot
// animation, and it never touches the ScreenManager's history, so waking
// returns to exactly the screen the user left.
class ScreensaverController
{
public:
  explicit ScreensaverController(Adafruit_ILI9341* displayInstance);

  // Allocates the canvas and starts the scene. Returns false if there was
  // not enough memory, in which case nothing is shown.
  bool begin(uint32_t now);
  void end();

  // Asks the scene to wake up (a button was pressed). The first call starts
  // the ~800 ms wake transition; later calls are ignored.
  void wake(uint32_t now);
  bool isWaking() const;
  bool wakeFinished() const;

  // Advances the scene. Returns true when a new frame was composed and
  // present() should be called.
  bool update(uint32_t now);
  void present();

private:
  // 160 x 120 logical pixels at 2x = the full 320 x 240 TFT.
  static constexpr int16_t CANVAS_W = 160;
  static constexpr int16_t CANVAS_H = 120;
  static constexpr uint8_t PIXEL_SCALE = 2;

  static constexpr uint32_t FRAME_INTERVAL_MS = 33; // ~30 fps

  // Scene timing.
  static constexpr uint32_t AMBIENT_MS = 2000;
  static constexpr uint32_t ENTER_MS = 2800;
  static constexpr uint32_t WAKE_MS = 800;

  // Character placement (sprite top-left, logical pixels).
  static constexpr int16_t START_X = -CharacterSprites::STAND.width - 2;
  static constexpr int16_t CENTER_X = (CANVAS_W - CharacterSprites::STAND.width) / 2;
  static constexpr int16_t CHAR_Y = 74;
  static constexpr uint32_t WALK_SWAP_MS = 170;

  static constexpr uint8_t PARTICLE_COUNT = 18;

  enum class Phase : uint8_t
  {
    AMBIENT,  // sparse particles, the stage almost empty
    ENTER,    // the character walks in from the left
    HOLD,     // he stands at the centre (end of ETAPA 2)
    WAKE      // a button was pressed: dissolve, converge, flash, hand back
  };

  struct Particle
  {
    bool active;
    float x, y, vx, vy;
    uint16_t life, age;
  };

  Adafruit_ILI9341* display;
  MascotCanvas canvas;
  MascotView view;

  Phase phase;
  uint32_t phaseStart;
  uint32_t startedAt;
  uint32_t lastFrameTime;
  uint32_t lastNow;

  bool waking;
  uint32_t wakeStart;
  float wakeFlash;

  float charX;          // current sprite top-left X
  uint32_t lastWalkSwap;
  bool walkToggle;

  float ambientWavePhase;
  uint32_t nextAmbientWaveAt;
  float ambientWaveProgress; // 0 = idle, >0 = a ring is crossing
  int16_t ambientWaveY;

  Particle particles[PARTICLE_COUNT];
  uint32_t rng;

  void enterPhase(Phase next, uint32_t now);

  void composeFrame(uint32_t now);
  void drawAmbientWave();
  void drawCharacter(uint32_t now, float dissolve);
  void drawWakeCore();
  void drawParticles();

  void updateParticles(float dt);
  void resetAmbient(Particle& p);
  void spawnAmbientParticle();
  void burstFromCharacter();

  uint32_t nextRandom();
  float randUnit();
};
