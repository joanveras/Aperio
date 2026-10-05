#include "ui/mascot/MascotIdleAnimator.hpp"

#include <cmath>

namespace
{
  constexpr float PI_F = 3.14159265f;
  constexpr float TWO_PI_F = 6.28318531f;

  float lerp(float from, float to, float amount)
  {
    return from + (to - from) * amount;
  }

  int16_t roundToInt(float value)
  {
    return static_cast<int16_t>(floorf(value + 0.5f));
  }
}

MascotIdleAnimator::MascotIdleAnimator(Adafruit_ILI9341* displayInstance)
  : display(displayInstance),
    baseX(0),
    baseY(0),
    posX(0),
    posY(0),
    canvas(CANVAS_WIDTH, CANVAS_HEIGHT),
    renderer(),
    view(displayInstance, PIXEL_SCALE),
    lastUpdate(0),
    lastFrame(0),
    waking(false),
    wakeStart(0),
    pose()
{
}

bool MascotIdleAnimator::begin(uint32_t now)
{
  // The display is initialized by the time this runs, so the panel size is
  // known: center the canvas and leave an equal margin all around.
  baseX = (display->width() - CANVAS_WIDTH * PIXEL_SCALE) / 2;
  baseY = (display->height() - CANVAS_HEIGHT * PIXEL_SCALE) / 2;
  posX = baseX;
  posY = baseY;

  lastUpdate = now;
  lastFrame = now - FRAME_INTERVAL_MS;  // make the first frame due

  waking = false;
  wakeStart = 0;

  if (!canvas.begin())
  {
    return false;
  }

  display->fillScreen(ILI9341_BLACK);

  return true;
}

void MascotIdleAnimator::end()
{
  canvas.end();
}

void MascotIdleAnimator::wake(uint32_t now)
{
  if (!waking)
  {
    waking = true;
    wakeStart = now;
  }
}

bool MascotIdleAnimator::isWaking() const
{
  return waking;
}

bool MascotIdleAnimator::wakeFinished() const
{
  // lastUpdate is advanced every frame by update(), so this reads the time
  // of the most recent tick without needing `now` passed in again.
  return waking && (lastUpdate - wakeStart) >= WAKE_MS;
}

bool MascotIdleAnimator::update(uint32_t now)
{
  lastUpdate = now;

  if (waking)
  {
    buildWakePose(now);
  }
  else
  {
    buildIdlePose(now);
  }

  // The gentle float: drift the whole canvas a few pixels. The eye sits far
  // from the canvas edges, so the black margin keeps it from ever clipping
  // or leaving a trail.
  float t = static_cast<float>(now);
  posX = baseX + roundToInt(FLOAT_X_AMP * sinf(TWO_PI_F * t * FLOAT_X_PER_MS));
  posY = baseY + roundToInt(FLOAT_Y_AMP * sinf(TWO_PI_F * t * FLOAT_Y_PER_MS + 1.3f));

  return (now - lastFrame) >= FRAME_INTERVAL_MS;
}

void MascotIdleAnimator::present()
{
  if (!canvas.isReady())
  {
    return;
  }

  renderer.draw(canvas, pose);  // draw() clears the canvas first
  view.present(canvas, posX, posY, pose.flash, pose.glitch);

  lastFrame = lastUpdate;
}

// The calm resting state: eye open, pupil centered, faint waves breathing
// in and out. The two sides already drift apart a little inside the
// renderer, so the ring never looks perfectly mechanical.
void MascotIdleAnimator::buildIdlePose(uint32_t now)
{
  const float phase = now * WAVE_PHASE_PER_MS;
  const float breath = sinf(TWO_PI_F * phase);

  pose = MascotPose();  // fully formed eye, looking forward

  pose.eyeOpen = 1.0f;
  pose.gazeX = 0.0f;
  pose.gazeY = 0.0f;
  pose.ringRotation = now * RING_TURNS_PER_MS;

  pose.waveMode = WaveMode::BREATHE;
  pose.waveIntensity = IDLE_WAVE_INTENSITY;
  pose.waveRadius = IDLE_WAVE_RADIUS + IDLE_WAVE_RADIUS_BREATH * breath;
  pose.waveSpacing = IDLE_WAVE_SPACING;
  pose.waveSpan = IDLE_WAVE_SPAN;
  pose.waveAmpLeft = 1.0f;
  pose.waveAmpRight = 1.0f;
  pose.wavePhase = phase;
}

// The wake reaction: a single bright pulse. The eye is already open here
// (we only sleep in a later step), so waking reads as the waves blooming
// outward with a flash, then settling -- "o mascote reage/acorda".
void MascotIdleAnimator::buildWakePose(uint32_t now)
{
  const float elapsed = static_cast<float>(now - wakeStart);
  float t = elapsed / static_cast<float>(WAKE_MS);

  if (t > 1.0f)
  {
    t = 1.0f;
  }

  const float env = sinf(PI_F * t);  // 0 -> 1 -> 0

  buildIdlePose(now);  // start from the calm pose, then push the pulse

  pose.eyeOpen = 1.0f;
  pose.flash = WAKE_FLASH * env;
  pose.waveIntensity = lerp(IDLE_WAVE_INTENSITY, 1.0f, env);
  pose.waveRadius = IDLE_WAVE_RADIUS + WAKE_WAVE_EXPAND * env;
  pose.waveSpacing = IDLE_WAVE_SPACING + WAKE_SPACING_EXPAND * env;
}
