#include "ui/mascot/MascotIdleAnimator.hpp"
#include "ui/mascot/MascotPalette.hpp"

#include <cmath>

namespace
{
  constexpr float PI_F = 3.14159265f;
  constexpr float TWO_PI_F = 6.28318531f;

  float clampf(float v, float lo, float hi)
  {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
  }

  float lerp(float from, float to, float amount)
  {
    return from + (to - from) * amount;
  }

  // Classic smoothstep: 0 below edge0, 1 above edge1, eased in between.
  float smoothstep(float edge0, float edge1, float x)
  {
    if (edge1 <= edge0) return x < edge1 ? 0.0f : 1.0f;
    float t = clampf((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
  }

  // 0 at the ends, 1 in the middle -- a particle's fade in and out.
  float triangle(float x)
  {
    return 1.0f - std::fabs(2.0f * x - 1.0f);
  }

  int16_t roundToInt(float value)
  {
    return static_cast<int16_t>(std::floor(value + 0.5f));
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
    phase(Phase::ENTERING),
    phaseStart(0),
    idleEnteredAt(0),
    gesture(Gesture::NONE),
    lastGesture(Gesture::NONE),
    gestureStart(0),
    gestureDur(0),
    nextGestureAt(0),
    gestureDir(1),
    waking(false),
    wakeStart(0),
    wakeFromOpen(1.0f),
    particles{},
    rng(1),
    pose()
{
}

bool MascotIdleAnimator::begin(uint32_t now)
{
  baseX = (display->width() - CANVAS_WIDTH * PIXEL_SCALE) / 2;
  baseY = (display->height() - CANVAS_HEIGHT * PIXEL_SCALE) / 2;
  posX = baseX;
  posY = baseY;

  lastUpdate = now;
  lastFrame = now - FRAME_INTERVAL_MS;

  phase = Phase::ENTERING;
  phaseStart = now;
  idleEnteredAt = now;

  gesture = Gesture::NONE;
  lastGesture = Gesture::NONE;

  waking = false;
  wakeStart = 0;
  wakeFromOpen = 1.0f;

  rng = now ? now : 1u;

  for (int i = 0; i < PARTICLE_COUNT; i++)
  {
    particles[i].active = false;
  }

  pose = MascotPose();

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
    phase = Phase::WAKING;
    wakeStart = now;
    wakeFromOpen = pose.eyeOpen;  // may be ~0 if it was asleep

    // A little signal flares up as it wakes.
    for (int i = 0; i < 6; i++)
    {
      spawnParticle(now, 0);
    }
  }
}

bool MascotIdleAnimator::isWaking() const
{
  return waking;
}

bool MascotIdleAnimator::wakeFinished() const
{
  return waking && (lastUpdate - wakeStart) >= WAKE_MS;
}

bool MascotIdleAnimator::update(uint32_t now)
{
  lastUpdate = now;

  switch (phase)
  {
    case Phase::ENTERING:
      buildEntryPose(now);
      if (now - phaseStart >= ENTER_MS)
      {
        phase = Phase::IDLE;
        idleEnteredAt = now;
        lastGesture = Gesture::NONE;
        scheduleNextGesture(now);
      }
      break;

    case Phase::IDLE:
      buildBreathingPose(now);

      if (gesture == Gesture::NONE)
      {
        if (now - idleEnteredAt >= IDLE_TO_SLEEP_MS)
        {
          phase = Phase::SLEEPING;
          phaseStart = now;
        }
        else if (now >= nextGestureAt)
        {
          startGesture(now);
        }
      }
      else if (now - gestureStart >= gestureDur)
      {
        lastGesture = gesture;
        gesture = Gesture::NONE;
        scheduleNextGesture(now);
      }

      if (gesture != Gesture::NONE)
      {
        applyGesture(now);
      }
      break;

    case Phase::SLEEPING:
      buildSleepPose(now);
      break;

    case Phase::WAKING:
      buildWakePose(now);
      break;
  }

  updateFloat(now);
  updateParticles(now);

  return (now - lastFrame) >= FRAME_INTERVAL_MS;
}

void MascotIdleAnimator::present()
{
  if (!canvas.isReady())
  {
    return;
  }

  renderer.draw(canvas, pose);  // clears the canvas, then draws the eye
  drawParticles(lastUpdate);    // motes and glints layered on top

  view.present(canvas, posX, posY, pose.flash, pose.glitch);

  lastFrame = lastUpdate;
}

// ---- Pose builders -------------------------------------------------------

// The calm resting state: eye open, pupil centered, faint waves breathing.
void MascotIdleAnimator::buildBreathingPose(uint32_t now)
{
  const float phaseT = now * WAVE_PHASE_PER_MS;
  const float breath = std::sin(TWO_PI_F * phaseT);

  pose = MascotPose();

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
  pose.wavePhase = phaseT;
}

// The eye is born from scattered signal: spark, rings, iris, pupil, outline,
// then the waves settle -- a short cousin of the boot animation.
void MascotIdleAnimator::buildEntryPose(uint32_t now)
{
  const float t = clampf(
    static_cast<float>(now - phaseStart) / static_cast<float>(ENTER_MS),
    0.0f, 1.0f
  );

  pose = MascotPose();

  pose.coreIntensity = smoothstep(0.0f, 0.20f, t) * (1.0f - smoothstep(0.35f, 0.55f, t));
  pose.ringReveal = smoothstep(0.30f, 0.72f, t);
  pose.irisReveal = smoothstep(0.42f, 0.82f, t);
  pose.pupilReveal = smoothstep(0.60f, 0.92f, t);
  pose.contourReveal = smoothstep(0.50f, 0.86f, t);
  pose.marksReveal = smoothstep(0.65f, 1.0f, t);
  pose.eyeOpen = smoothstep(0.45f, 0.80f, t);
  pose.ringRotation = now * RING_TURNS_PER_MS;

  const float waves = smoothstep(0.40f, 1.0f, t);
  pose.waveMode = WaveMode::BREATHE;
  pose.waveIntensity = IDLE_WAVE_INTENSITY * waves;
  pose.waveRadius = lerp(12.0f, IDLE_WAVE_RADIUS, waves);
  pose.waveSpacing = IDLE_WAVE_SPACING;
  pose.waveSpan = IDLE_WAVE_SPAN;
  pose.wavePhase = now * WAVE_PHASE_PER_MS;
}

// Drifting off: the waves fade and shrink, then the eye closes, until the
// screen is almost black.
void MascotIdleAnimator::buildSleepPose(uint32_t now)
{
  buildBreathingPose(now);

  const float k = clampf(
    static_cast<float>(now - phaseStart) / static_cast<float>(SLEEP_TRANS_MS),
    0.0f, 1.0f
  );

  pose.waveIntensity = IDLE_WAVE_INTENSITY * (1.0f - k);
  pose.waveRadius = lerp(pose.waveRadius, 14.0f, k);
  pose.marksReveal = 1.0f - k;
  pose.eyeOpen = 1.0f - smoothstep(0.35f, 1.0f, k);
}

// Waking up: the eye opens from wherever it was, with a flash and the waves
// blooming outward -- "o mascote reage/acorda".
void MascotIdleAnimator::buildWakePose(uint32_t now)
{
  buildBreathingPose(now);

  const float wt = clampf(
    static_cast<float>(now - wakeStart) / static_cast<float>(WAKE_MS),
    0.0f, 1.0f
  );

  pose.eyeOpen = lerp(wakeFromOpen, 1.0f, smoothstep(0.0f, 0.5f, wt));

  const float e = std::sin(PI_F * clampf((wt - 0.15f) / 0.85f, 0.0f, 1.0f));
  pose.flash = 0.8f * e;
  pose.waveIntensity = lerp(IDLE_WAVE_INTENSITY, 1.0f, e);
  pose.waveRadius = IDLE_WAVE_RADIUS + 22.0f * e;
  pose.waveSpacing = IDLE_WAVE_SPACING + 3.0f * e;
}

// Layers the active gesture on top of the breathing pose.
void MascotIdleAnimator::applyGesture(uint32_t now)
{
  const float gp = clampf(
    static_cast<float>(now - gestureStart) / static_cast<float>(gestureDur),
    0.0f, 1.0f
  );
  const float dir = static_cast<float>(gestureDir);

  switch (gesture)
  {
    case Gesture::BLINK:
      pose.eyeOpen = std::fabs(std::cos(PI_F * gp));
      break;

    case Gesture::DOUBLE_BLINK:
      if (gp < 0.38f)
        pose.eyeOpen = std::fabs(std::cos(PI_F * (gp / 0.38f)));
      else if (gp < 0.50f)
        pose.eyeOpen = 1.0f;
      else if (gp < 0.88f)
        pose.eyeOpen = std::fabs(std::cos(PI_F * ((gp - 0.50f) / 0.38f)));
      else
        pose.eyeOpen = 1.0f;
      break;

    case Gesture::LOOK:
    {
      const float e = std::sin(PI_F * gp);  // out and back
      pose.gazeX = dir * 0.8f * e;
      // The waves on the side it looks at compress; the far side swells.
      if (gestureDir > 0)
      {
        pose.waveAmpRight *= (1.0f - 0.45f * e);
        pose.waveAmpLeft *= (1.0f + 0.15f * e);
      }
      else
      {
        pose.waveAmpLeft *= (1.0f - 0.45f * e);
        pose.waveAmpRight *= (1.0f + 0.15f * e);
      }
      break;
    }

    case Gesture::CURIOUS:
    {
      // Quick look one way, a beat to "analyse", a look across, then back.
      float g;
      if (gp < 0.18f)
        g = dir * smoothstep(0.0f, 0.18f, gp);
      else if (gp < 0.42f)
        g = dir;
      else if (gp < 0.62f)
        g = dir * (1.0f - 2.0f * smoothstep(0.42f, 0.62f, gp));
      else if (gp < 0.80f)
        g = -dir;
      else
        g = -dir * (1.0f - smoothstep(0.80f, 1.0f, gp));

      pose.gazeX = 0.85f * g;
      pose.pupilScale = 1.0f - 0.12f * std::fabs(g);  // a focused squint

      if (g > 0.0f)
        pose.waveAmpRight *= (1.0f - 0.4f * std::fabs(g));
      else if (g < 0.0f)
        pose.waveAmpLeft *= (1.0f - 0.4f * std::fabs(g));
      break;
    }

    case Gesture::PULSE:
    {
      const float e = std::sin(PI_F * gp);
      pose.flash = 0.7f * e;
      pose.waveIntensity = lerp(IDLE_WAVE_INTENSITY, 1.0f, e);
      pose.waveRadius += 22.0f * e;
      pose.waveSpacing += 3.0f * e;
      break;
    }

    case Gesture::GLITCH:
      // A brief, flickery row-shift distortion that fades out, plus a tiny
      // jitter of the gaze.
      pose.glitch = clampf(
        (1.0f - gp) * (0.4f + 0.4f * std::fabs(std::sin(gp * 18.0f))),
        0.0f, 1.0f
      );
      pose.gazeX += 0.10f * std::sin(gp * 37.0f);
      break;

    case Gesture::NONE:
      break;
  }
}

// ---- Micro-animation scheduler ------------------------------------------

void MascotIdleAnimator::startGesture(uint32_t now)
{
  Gesture picked = Gesture::BLINK;

  // Weighted pick, re-rolled a few times so the same gesture rarely repeats.
  for (int tries = 0; tries < 6; tries++)
  {
    const uint32_t r = nextRandom() % 100u;

    if (r < 34u)       picked = Gesture::BLINK;
    else if (r < 58u)  picked = Gesture::LOOK;
    else if (r < 72u)  picked = Gesture::PULSE;
    else if (r < 84u)  picked = Gesture::DOUBLE_BLINK;
    else if (r < 94u)  picked = Gesture::CURIOUS;
    else               picked = Gesture::GLITCH;

    if (picked != lastGesture)
    {
      break;
    }
  }

  gesture = picked;
  gestureStart = now;
  gestureDur = gestureDuration(picked);
  gestureDir = (nextRandom() & 1u) ? 1 : -1;
}

void MascotIdleAnimator::scheduleNextGesture(uint32_t now)
{
  nextGestureAt = now + GESTURE_MIN_GAP_MS + (nextRandom() % GESTURE_VAR_MS);
}

uint32_t MascotIdleAnimator::gestureDuration(Gesture g) const
{
  switch (g)
  {
    case Gesture::BLINK:        return BLINK_MS;
    case Gesture::DOUBLE_BLINK: return DBLBLINK_MS;
    case Gesture::LOOK:         return LOOK_MS;
    case Gesture::CURIOUS:      return CURIOUS_MS;
    case Gesture::PULSE:        return PULSE_MS;
    case Gesture::GLITCH:       return GLITCH_MS;
    case Gesture::NONE:         return 0;
  }
  return 0;
}

// ---- Float + particles ---------------------------------------------------

void MascotIdleAnimator::updateFloat(uint32_t now)
{
  if (phase == Phase::SLEEPING)
  {
    posX = baseX;  // it lies still while asleep
    posY = baseY;
    return;
  }

  const float t = static_cast<float>(now);
  posX = baseX + roundToInt(FLOAT_X_AMP * std::sin(TWO_PI_F * t * FLOAT_X_PER_MS));
  posY = baseY + roundToInt(FLOAT_Y_AMP * std::sin(TWO_PI_F * t * FLOAT_Y_PER_MS + 1.3f));
}

void MascotIdleAnimator::updateParticles(uint32_t now)
{
  for (int i = 0; i < PARTICLE_COUNT; i++)
  {
    if (particles[i].active && (now - particles[i].born) >= particles[i].life)
    {
      particles[i].active = false;
    }
  }

  switch (phase)
  {
    case Phase::ENTERING:
    {
      const float t = static_cast<float>(now - phaseStart) / static_cast<float>(ENTER_MS);
      if (t < 0.7f && randUnit() < 0.6f)
      {
        spawnParticle(now, 0);
      }
      break;
    }

    case Phase::IDLE:
      if (randUnit() < 0.18f) spawnParticle(now, 0);
      if (randUnit() < 0.015f) spawnParticle(now, 1);  // the rare glint
      break;

    case Phase::WAKING:
      if (randUnit() < 0.25f) spawnParticle(now, 0);
      break;

    case Phase::SLEEPING:
      if (randUnit() < 0.01f) spawnParticle(now, 0);
      break;
  }
}

void MascotIdleAnimator::spawnParticle(uint32_t now, uint8_t kind)
{
  int slot = -1;
  for (int i = 0; i < PARTICLE_COUNT; i++)
  {
    if (!particles[i].active)
    {
      slot = i;
      break;
    }
  }

  if (slot < 0)
  {
    return;
  }

  Particle& p = particles[slot];
  p.active = true;
  p.kind = kind;
  p.born = now;

  const int16_t cx = CANVAS_WIDTH / 2;
  const int16_t cy = CANVAS_HEIGHT / 2;

  if (kind == 1)
  {
    // Glint: a short, bright sparkle near the iris highlight.
    p.x = cx - 2 + static_cast<int16_t>(nextRandom() % 9u) - 4;
    p.y = cy - 2 + static_cast<int16_t>(nextRandom() % 9u) - 4;
    p.life = 140 + static_cast<uint16_t>(nextRandom() % 160u);
    p.peak = 0.9f;
    return;
  }

  // Ambient mote: somewhere in the field, but not on top of the eye itself.
  int16_t x = 2;
  int16_t y = 2;
  for (int tries = 0; tries < 5; tries++)
  {
    x = 2 + static_cast<int16_t>(nextRandom() % (CANVAS_WIDTH - 4));
    y = 2 + static_cast<int16_t>(nextRandom() % (CANVAS_HEIGHT - 4));

    const float nx = (x - cx) / 42.0f;
    const float ny = (y - cy) / 26.0f;
    if (nx * nx + ny * ny >= 1.0f)
    {
      break;  // clear of the eye
    }
  }

  p.x = x;
  p.y = y;
  p.life = 600 + static_cast<uint16_t>(nextRandom() % 900u);
  p.peak = 0.35f + randUnit() * 0.4f;
}

void MascotIdleAnimator::drawParticles(uint32_t now)
{
  for (int i = 0; i < PARTICLE_COUNT; i++)
  {
    const Particle& p = particles[i];
    if (!p.active)
    {
      continue;
    }

    const float age = static_cast<float>(now - p.born) / static_cast<float>(p.life);
    const float brightness = triangle(clampf(age, 0.0f, 1.0f)) * p.peak;
    if (brightness <= 0.02f)
    {
      continue;
    }

    if (p.kind == 1)
    {
      // A little white "+" twinkle.
      canvas.setPixel(p.x, p.y, MascotColor::WHITE);
      canvas.setPixel(p.x + 1, p.y, MascotColor::WHITE);
      canvas.setPixel(p.x - 1, p.y, MascotColor::WHITE);
      canvas.setPixel(p.x, p.y + 1, MascotColor::WHITE);
      canvas.setPixel(p.x, p.y - 1, MascotColor::WHITE);
    }
    else
    {
      const uint8_t color = MascotColor::signal(brightness);
      if (color != MascotColor::BLACK)
      {
        canvas.setPixel(p.x, p.y, color);
      }
    }
  }
}

uint32_t MascotIdleAnimator::nextRandom()
{
  rng = rng * 1664525u + 1013904223u;
  return rng >> 16;
}

float MascotIdleAnimator::randUnit()
{
  return static_cast<float>(nextRandom() & 0xFFFFu) / 65535.0f;
}
