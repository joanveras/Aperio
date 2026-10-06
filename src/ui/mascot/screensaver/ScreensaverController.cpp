#include "ui/mascot/screensaver/ScreensaverController.hpp"

#include <Arduino.h>
#include <cmath>

#include "ui/mascot/MascotPalette.hpp"

namespace
{
  constexpr float PI_F = 3.14159265f;

  // The character's centre on the canvas -- where energy converges on wake.
  constexpr float CORE_X =
    (160 - CharacterSprites::STAND.width) / 2 + CharacterSprites::STAND.width / 2.0f;
  constexpr float CORE_Y = 74 + CharacterSprites::STAND.height / 2.0f;

  float clampf(float v, float lo, float hi)
  {
    return v < lo ? lo : (v > hi ? hi : v);
  }

  float lerp(float a, float b, float t)
  {
    return a + (b - a) * t;
  }

  // Smooth 0..1 ramp, zero slope at both ends.
  float easeInOut(float t)
  {
    t = clampf(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
  }
}

ScreensaverController::ScreensaverController(Adafruit_ILI9341* displayInstance)
  : display(displayInstance),
    canvas(CANVAS_W, CANVAS_H),
    view(displayInstance, PIXEL_SCALE),
    phase(Phase::AMBIENT),
    phaseStart(0),
    startedAt(0),
    lastFrameTime(0),
    lastNow(0),
    waking(false),
    wakeStart(0),
    wakeFlash(0.0f),
    charX(START_X),
    lastWalkSwap(0),
    walkToggle(false),
    ambientWavePhase(0.0f),
    nextAmbientWaveAt(0),
    ambientWaveProgress(0.0f),
    ambientWaveY(0),
    rng(0x5a17c0deu)
{
  for (auto& p : particles)
  {
    p.active = false;
  }
}

bool ScreensaverController::begin(uint32_t now)
{
  if (!canvas.begin())
  {
    return false;
  }

  rng ^= now * 2654435761u;

  phase = Phase::AMBIENT;
  phaseStart = now;
  startedAt = now;
  lastFrameTime = now - FRAME_INTERVAL_MS; // compose on the first update
  lastNow = now;

  waking = false;
  wakeStart = 0;
  wakeFlash = 0.0f;

  charX = START_X;
  lastWalkSwap = now;
  walkToggle = false;

  ambientWavePhase = 0.0f;
  ambientWaveProgress = 0.0f;
  ambientWaveY = CANVAS_H / 2;
  nextAmbientWaveAt = now + 1200;

  // A sparse scatter of ambient particles to start with.
  for (auto& p : particles)
  {
    p.active = false;
  }
  for (uint8_t i = 0; i < PARTICLE_COUNT / 2; ++i)
  {
    spawnAmbientParticle();
  }

  return true;
}

void ScreensaverController::end()
{
  canvas.end();
}

void ScreensaverController::wake(uint32_t now)
{
  if (waking)
  {
    return;
  }

  waking = true;
  wakeStart = now;
  burstFromCharacter();
}

bool ScreensaverController::isWaking() const
{
  return waking;
}

bool ScreensaverController::wakeFinished() const
{
  return waking && (lastNow - wakeStart) >= WAKE_MS;
}

void ScreensaverController::enterPhase(Phase next, uint32_t now)
{
  phase = next;
  phaseStart = now;
}

bool ScreensaverController::update(uint32_t now)
{
  lastNow = now;

  if ((now - lastFrameTime) < FRAME_INTERVAL_MS)
  {
    return false;
  }
  lastFrameTime = now;

  const float dt = FRAME_INTERVAL_MS / 1000.0f;

  if (!waking)
  {
    switch (phase)
    {
      case Phase::AMBIENT:
        if (now - phaseStart >= AMBIENT_MS)
        {
          enterPhase(Phase::ENTER, now);
        }
        break;

      case Phase::ENTER:
      {
        float t = clampf(
          static_cast<float>(now - phaseStart) / ENTER_MS, 0.0f, 1.0f);
        charX = lerp(START_X, CENTER_X, easeInOut(t));

        if (now - lastWalkSwap >= WALK_SWAP_MS)
        {
          walkToggle = !walkToggle;
          lastWalkSwap = now;
        }

        if (t >= 1.0f)
        {
          charX = CENTER_X;
          enterPhase(Phase::HOLD, now);
        }
        break;
      }

      case Phase::HOLD:
        charX = CENTER_X;
        break;

      case Phase::WAKE:
        break;
    }
  }

  // Occasionally send a discreet ring across the stage (not during wake).
  if (!waking)
  {
    if (ambientWaveProgress > 0.0f)
    {
      ambientWaveProgress += dt / 2.0f; // ~2 s to cross
      if (ambientWaveProgress >= 1.0f)
      {
        ambientWaveProgress = 0.0f;
        nextAmbientWaveAt = now + 3500 + (nextRandom() % 3000);
      }
    }
    else if (now >= nextAmbientWaveAt)
    {
      ambientWaveProgress = 0.0001f;
      ambientWaveY = 24 + static_cast<int16_t>(nextRandom() % (CANVAS_H - 48));
    }
  }

  updateParticles(dt);
  composeFrame(now);

  return true;
}

void ScreensaverController::updateParticles(float dt)
{
  float wt = waking
    ? clampf(static_cast<float>(lastNow - wakeStart) / WAKE_MS, 0.0f, 1.0f)
    : 0.0f;

  for (auto& p : particles)
  {
    if (!p.active)
    {
      continue;
    }

    // During the wake, particles are pulled into the core and fade fast.
    if (waking && wt > 0.3f)
    {
      p.vx += (CORE_X - p.x) * 3.0f * dt;
      p.vy += (CORE_Y - p.y) * 3.0f * dt;
      p.age += FRAME_INTERVAL_MS * 2;
    }
    else
    {
      p.age += FRAME_INTERVAL_MS;
    }

    p.x += p.vx * dt;
    p.y += p.vy * dt;

    bool offscreen = p.x < -2 || p.x > CANVAS_W + 2 || p.y < -2 || p.y > CANVAS_H + 2;

    if (p.age >= p.life || offscreen)
    {
      if (waking)
      {
        p.active = false;
      }
      else
      {
        resetAmbient(p); // recycle this slot as a fresh ambient mote
      }
    }
  }
}

void ScreensaverController::resetAmbient(Particle& p)
{
  p.active = true;
  p.x = randUnit() * CANVAS_W;
  p.y = randUnit() * CANVAS_H;
  p.vx = (randUnit() - 0.5f) * 5.0f;
  p.vy = -3.0f - randUnit() * 6.0f; // drift gently upward
  p.life = 2200 + static_cast<uint16_t>(randUnit() * 2600.0f);
  p.age = 0;
}

void ScreensaverController::spawnAmbientParticle()
{
  for (auto& p : particles)
  {
    if (!p.active)
    {
      resetAmbient(p);
      return;
    }
  }
}

void ScreensaverController::burstFromCharacter()
{
  for (auto& p : particles)
  {
    p.active = true;
    p.x = CENTER_X + randUnit() * CharacterSprites::STAND.width;
    p.y = CHAR_Y + randUnit() * CharacterSprites::STAND.height;
    p.vx = (randUnit() - 0.5f) * 60.0f;
    p.vy = (randUnit() - 0.5f) * 60.0f - 12.0f;
    p.life = 500 + static_cast<uint16_t>(randUnit() * 320.0f);
    p.age = 0;
  }
}

void ScreensaverController::composeFrame(uint32_t now)
{
  canvas.clear(MascotColor::BLACK);

  float wt = waking
    ? clampf(static_cast<float>(now - wakeStart) / WAKE_MS, 0.0f, 1.0f)
    : 0.0f;

  // Ambient ring fades out as the wake takes over.
  if (ambientWaveProgress > 0.0f && (!waking || wt < 0.4f))
  {
    drawAmbientWave();
  }

  drawParticles();

  if (!waking)
  {
    if (phase == Phase::ENTER || phase == Phase::HOLD)
    {
      drawCharacter(now, 0.0f);
    }
  }
  else if (wt < 0.4f)
  {
    // DISSOLVE: the character breaks apart as the particles take over.
    drawCharacter(now, clampf(wt / 0.4f, 0.0f, 1.0f));
  }

  if (waking)
  {
    drawWakeCore();

    // A short, bright flash around the middle of the wake, then gone.
    float f = 0.0f;
    if (wt > 0.6f)
    {
      float u = (wt - 0.6f) / 0.4f; // 0..1 across 0.6..1.0
      f = std::sin(PI_F * u);
      f = f * f;                    // sharpen: a brief peak, not a long wash
    }
    wakeFlash = f * 0.9f;
  }
  else
  {
    wakeFlash = 0.0f;
  }
}

void ScreensaverController::drawAmbientWave()
{
  // A faint ring expanding from the centre -- the world's quiet "signal".
  float r = ambientWaveProgress * 46.0f;
  if (r < 2.0f)
  {
    return;
  }

  float fade = 1.0f - ambientWaveProgress;
  uint8_t color = MascotColor::signal(0.18f + 0.22f * fade);
  if (color == MascotColor::BLACK)
  {
    return;
  }

  int16_t cx = CANVAS_W / 2;
  int16_t cy = ambientWaveY;
  float step = 0.5f / r;

  for (float a = -0.9f; a <= 0.9f; a += step)
  {
    // Dashed, and only the right+left flanks, so it reads as a ripple.
    int dash = static_cast<int>((a + 0.9f) * r / 3.0f) & 3;
    if (dash == 0)
    {
      continue;
    }

    canvas.setPixel(cx + static_cast<int16_t>(r * std::cos(a)),
                    cy + static_cast<int16_t>(r * std::sin(a)), color);
    canvas.setPixel(cx - static_cast<int16_t>(r * std::cos(a)),
                    cy + static_cast<int16_t>(r * std::sin(a)), color);
  }
}

void ScreensaverController::drawCharacter(uint32_t now, float dissolve)
{
  const CharacterSprites::Sprite* body = &CharacterSprites::STAND;

  if (phase == Phase::ENTER && !waking)
  {
    body = walkToggle ? &CharacterSprites::WALK_B : &CharacterSprites::WALK_A;
  }

  int16_t x = static_cast<int16_t>(charX + 0.5f);

  // A gentle 1px head-bob while walking.
  int16_t bob = 0;
  if (phase == Phase::ENTER && !waking)
  {
    bob = ((now / 170) % 2) ? -1 : 0;
  }
  int16_t y = CHAR_Y + bob;

  CharacterSprites::draw(canvas, *body, x, y, dissolve);
  CharacterSprites::draw(
    canvas,
    CharacterSprites::DEVICE,
    x + CharacterSprites::DEVICE_OFFSET_X,
    y + CharacterSprites::DEVICE_OFFSET_Y,
    dissolve);
}

void ScreensaverController::drawWakeCore()
{
  float wt = clampf(static_cast<float>(lastNow - wakeStart) / WAKE_MS, 0.0f, 1.0f);
  if (wt <= 0.4f)
  {
    return;
  }

  // Grows through CONVERGE, peaks at FLASH, shrinks through FADE.
  float u = (wt - 0.4f) / 0.6f; // 0..1 across 0.4..1.0
  float shape = std::sin(PI_F * u);
  float radius = 24.0f * shape;
  if (radius < 1.0f)
  {
    return;
  }

  int16_t cx = static_cast<int16_t>(CORE_X);
  int16_t cy = static_cast<int16_t>(CORE_Y);
  int16_t r = static_cast<int16_t>(radius);
  float r2 = radius * radius;
  float inner2 = (radius * 0.45f) * (radius * 0.45f);

  for (int16_t dy = -r; dy <= r; ++dy)
  {
    for (int16_t dx = -r; dx <= r; ++dx)
    {
      float d2 = static_cast<float>(dx * dx + dy * dy);
      if (d2 > r2)
      {
        continue;
      }
      // Bright white-ish core, cyan halo.
      uint8_t color = d2 < inner2 ? MascotColor::WHITE : MascotColor::SIGNAL_LAST;
      canvas.setPixel(cx + dx, cy + dy, color);
    }
  }
}

void ScreensaverController::drawParticles()
{
  float wt = waking
    ? clampf(static_cast<float>(lastNow - wakeStart) / WAKE_MS, 0.0f, 1.0f)
    : 0.0f;

  for (const auto& p : particles)
  {
    if (!p.active)
    {
      continue;
    }

    float remain = 1.0f - static_cast<float>(p.age) / static_cast<float>(p.life);
    float intensity = 0.25f + 0.45f * clampf(remain, 0.0f, 1.0f);
    if (waking)
    {
      intensity = clampf(intensity + 0.3f, 0.0f, 1.0f) * (1.0f - wt * 0.5f);
    }

    uint8_t color = MascotColor::signal(intensity);
    if (color != MascotColor::BLACK)
    {
      canvas.setPixel(static_cast<int16_t>(p.x), static_cast<int16_t>(p.y), color);
    }
  }
}

void ScreensaverController::present()
{
  view.present(canvas, 0, 0, wakeFlash, 0.0f);
}

uint32_t ScreensaverController::nextRandom()
{
  rng = rng * 1664525u + 1013904223u;
  return rng;
}

float ScreensaverController::randUnit()
{
  return (nextRandom() >> 8) / 16777216.0f;
}
