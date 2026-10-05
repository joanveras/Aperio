#include "ui/mascot/MascotMenuBadge.hpp"
#include "ui/mascot/MascotPalette.hpp"

#include <cmath>

namespace
{
  constexpr float PI_F = 3.14159265f;

  float clampf(float v, float lo, float hi)
  {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
  }
}

MascotMenuBadge::MascotMenuBadge(
  Adafruit_ILI9341* displayInstance,
  int16_t x,
  int16_t y
)
  : display(displayInstance),
    posX(x),
    posY(y),
    canvas(CANVAS_WIDTH, CANVAS_HEIGHT),
    renderer(),
    view(displayInstance, PIXEL_SCALE),
    lastUpdate(0),
    lastFrame(0),
    gaze(0.0f),
    gazeTarget(0.0f),
    gazeHoldUntil(0),
    blinking(false),
    blinkStart(0),
    nextBlink(0),
    rng(0),
    pose()
{
}

bool MascotMenuBadge::begin(uint32_t now)
{
  lastUpdate = now;
  lastFrame = now - FRAME_INTERVAL_MS;  // make the first frame due

  gaze = 0.0f;
  gazeTarget = 0.0f;
  gazeHoldUntil = 0;

  blinking = false;
  blinkStart = 0;

  rng = now ? now : 1u;
  scheduleNextBlink(now);

  return canvas.begin();
}

void MascotMenuBadge::end()
{
  canvas.end();
}

void MascotMenuBadge::glance(int direction, uint32_t now)
{
  float side = (direction < 0) ? -1.0f : (direction > 0 ? 1.0f : 0.0f);

  gazeTarget = side * GAZE_MAX;
  gazeHoldUntil = now + GAZE_HOLD_MS;
}

bool MascotMenuBadge::update(uint32_t now)
{
  float dt = (now >= lastUpdate) ? static_cast<float>(now - lastUpdate) : 0.0f;
  lastUpdate = now;

  // Gaze eases towards its target; once the dwell is over, back to center.
  if (now >= gazeHoldUntil)
  {
    gazeTarget = 0.0f;
  }

  float ease = 1.0f - expf(-dt / GAZE_TAU_MS);
  gaze += (gazeTarget - gaze) * ease;

  // Blink state machine.
  if (!blinking && now >= nextBlink)
  {
    blinking = true;
    blinkStart = now;
  }

  if (blinking && (now - blinkStart) >= BLINK_MS)
  {
    blinking = false;
    scheduleNextBlink(now);
  }

  float eyeOpen = 1.0f;
  if (blinking)
  {
    float t = static_cast<float>(now - blinkStart) / BLINK_MS;
    eyeOpen = std::fabs(std::cos(PI_F * t));  // 1 -> 0 -> 1
  }

  // Rebuild the pose: a fully-formed eye, no waves, no marks.
  pose = MascotPose();
  pose.waveIntensity = 0.0f;
  pose.waveAmpLeft = 0.0f;
  pose.waveAmpRight = 0.0f;
  pose.marksReveal = 0.0f;

  pose.eyeOpen = eyeOpen;
  pose.gazeX = clampf(gaze, -1.0f, 1.0f);
  pose.gazeY = 0.0f;
  pose.ringRotation = now * RING_TURNS_PER_MS;

  return (now - lastFrame) >= FRAME_INTERVAL_MS;
}

void MascotMenuBadge::present()
{
  if (!canvas.isReady())
  {
    return;
  }

  canvas.clear(MascotColor::BLACK);
  renderer.draw(canvas, pose);
  view.present(canvas, posX, posY, pose.flash, pose.glitch);

  lastFrame = lastUpdate;
}

void MascotMenuBadge::scheduleNextBlink(uint32_t now)
{
  nextBlink = now + BLINK_MIN_MS + (nextRandom() % BLINK_VAR_MS);
}

uint32_t MascotMenuBadge::nextRandom()
{
  rng = rng * 1664525u + 1013904223u;
  return rng >> 16;
}
