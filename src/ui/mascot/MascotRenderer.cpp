#include "ui/mascot/MascotRenderer.hpp"
#include "ui/mascot/MascotPalette.hpp"

#include <cmath>

namespace
{
  constexpr float TWO_PI_F = 6.28318531f;

  // How far (logical px) each layer moves when the eye looks around.
  // Layers closer to the center move more: a cheap illusion of depth.
  constexpr float GAZE_RANGE_X = 7.0f;
  constexpr float GAZE_RANGE_Y = 4.0f;
  constexpr float RING_PARALLAX = 0.6f;
  constexpr float IRIS_PARALLAX = 0.8f;
  constexpr float PUPIL_PARALLAX = 1.0f;

  // Closed eye: both lids meet on a soft downward curve ("sleepy smile").
  constexpr float CLOSED_CURVE = 0.3f;

  // Segmented ring: 12 segments, two of them missing for asymmetry.
  constexpr int RING_SEGMENTS = 12;
  constexpr float RING_SEGMENT_FILL = 0.75f;
  constexpr int RING_MISSING_A = 2;
  constexpr int RING_MISSING_B = 7;

  // Waves: three arcs per side, each with its own dash pattern so the two
  // sides are never perfect mirrors of each other.
  constexpr int WAVES_PER_SIDE = 3;
  constexpr float WAVE_DASH_LENGTH = 3.0f;
  constexpr uint16_t RIGHT_DASHES[WAVES_PER_SIDE] = { 0xF7BD, 0xEDB7, 0xDF7B };
  constexpr uint16_t LEFT_DASHES[WAVES_PER_SIDE] = { 0xBDEF, 0x7BDE, 0xF6DB };

  int16_t roundToInt(float value)
  {
    return static_cast<int16_t>(floorf(value + 0.5f));
  }

  float lerp(float from, float to, float amount)
  {
    return from + (to - from) * amount;
  }

  float fraction(float value)
  {
    return value - floorf(value);
  }

  // 0 at the corners of the eye, 1 at the middle.
  float eyeProfile(float x)
  {
    float u = x / MascotRenderer::EYE_HALF_WIDTH;
    float profile = 1.0f - u * u;

    return profile > 0.0f ? profile : 0.0f;
  }
}

void MascotRenderer::draw(MascotCanvas& canvas, const MascotPose& pose) const
{
  canvas.clear(MascotColor::BLACK);

  Center eye = {
    static_cast<int16_t>(canvas.width() / 2),
    static_cast<int16_t>(canvas.height() / 2)
  };

  drawSclera(canvas, eye, pose);
  drawCore(canvas, eye, pose);
  drawIris(canvas, eye, pose);
  applyLids(canvas, eye, pose);
  drawOutline(canvas, eye, pose);
  drawMarks(canvas, eye, pose);
  drawWaves(canvas, eye, pose);
}

float MascotRenderer::lidTop(float x, float eyeOpen) const
{
  float profile = eyeProfile(x);
  float closed = EYE_HALF_HEIGHT * CLOSED_CURVE * profile;

  return lerp(closed, -EYE_HALF_HEIGHT * profile, eyeOpen);
}

float MascotRenderer::lidBottom(float x, float eyeOpen) const
{
  float profile = eyeProfile(x);
  float closed = EYE_HALF_HEIGHT * CLOSED_CURVE * profile;

  return lerp(closed, EYE_HALF_HEIGHT * profile, eyeOpen);
}

// The outline materializes from both corners towards the center.
bool MascotRenderer::isColumnRevealed(int x, float contourReveal) const
{
  if (contourReveal <= 0.0f)
  {
    return false;
  }

  if (contourReveal >= 1.0f)
  {
    return true;
  }

  return fabsf(static_cast<float>(x)) >= EYE_HALF_WIDTH * (1.0f - contourReveal);
}

void MascotRenderer::drawSclera(
  MascotCanvas& canvas,
  Center eye,
  const MascotPose& pose
) const
{
  for (int x = -EYE_HALF_WIDTH; x <= EYE_HALF_WIDTH; x++)
  {
    if (!isColumnRevealed(x, pose.contourReveal))
    {
      continue;
    }

    int16_t top = roundToInt(eye.y + lidTop(x, pose.eyeOpen));
    int16_t bottom = roundToInt(eye.y + lidBottom(x, pose.eyeOpen));

    if (bottom - top < 2)
    {
      continue;
    }

    canvas.drawVerticalSpan(eye.x + x, top + 1, bottom - 1, MascotColor::SCLERA);
  }
}

// The "+" spark the eye is born from. The pupil covers it later.
void MascotRenderer::drawCore(
  MascotCanvas& canvas,
  Center eye,
  const MascotPose& pose
) const
{
  uint8_t color = MascotColor::signal(pose.coreIntensity);

  if (color == MascotColor::BLACK)
  {
    return;
  }

  canvas.setPixel(eye.x - 1, eye.y, color);
  canvas.setPixel(eye.x + 1, eye.y, color);
  canvas.setPixel(eye.x, eye.y - 1, color);
  canvas.setPixel(eye.x, eye.y + 1, color);

  canvas.setPixel(
    eye.x,
    eye.y,
    pose.coreIntensity > 0.7f ? MascotColor::WHITE : color
  );
}

void MascotRenderer::drawIris(
  MascotCanvas& canvas,
  Center eye,
  const MascotPose& pose
) const
{
  float gazeX = pose.gazeX * GAZE_RANGE_X;
  float gazeY = pose.gazeY * GAZE_RANGE_Y;

  Center ring = {
    roundToInt(eye.x + gazeX * RING_PARALLAX),
    roundToInt(eye.y + gazeY * RING_PARALLAX)
  };

  Center iris = {
    roundToInt(eye.x + gazeX * IRIS_PARALLAX),
    roundToInt(eye.y + gazeY * IRIS_PARALLAX)
  };

  Center pupil = {
    roundToInt(eye.x + gazeX * PUPIL_PARALLAX),
    roundToInt(eye.y + gazeY * PUPIL_PARALLAX)
  };

  // Segmented ring: sweeps in clockwise from the top while revealing.
  if (pose.ringReveal > 0.0f)
  {
    const float inner = RING_RADIUS - 0.5f;
    const float outer = RING_RADIUS + 0.5f;

    for (int dy = -RING_RADIUS - 1; dy <= RING_RADIUS + 1; dy++)
    {
      for (int dx = -RING_RADIUS - 1; dx <= RING_RADIUS + 1; dx++)
      {
        float distanceSquared = static_cast<float>(dx * dx + dy * dy);

        if (distanceSquared <= inner * inner || distanceSquared > outer * outer)
        {
          continue;
        }

        // 0 at the top, growing clockwise.
        float turn = fraction(atan2f(static_cast<float>(dx), static_cast<float>(-dy)) / TWO_PI_F);

        if (turn > pose.ringReveal)
        {
          continue;
        }

        float position = fraction(turn - pose.ringRotation) * RING_SEGMENTS;
        int segment = static_cast<int>(position);

        if (position - segment > RING_SEGMENT_FILL)
        {
          continue;
        }

        if (segment == RING_MISSING_A || segment == RING_MISSING_B)
        {
          continue;
        }

        canvas.setPixel(ring.x + dx, ring.y + dy, MascotColor::SIGNAL_LAST - 1);
      }
    }
  }

  // Iris body: fills from the outer edge towards the center.
  if (pose.irisReveal > 0.0f)
  {
    float revealedFrom = IRIS_RADIUS * (1.0f - pose.irisReveal) - 0.5f;

    for (int dy = -IRIS_RADIUS; dy <= IRIS_RADIUS; dy++)
    {
      for (int dx = -IRIS_RADIUS; dx <= IRIS_RADIUS; dx++)
      {
        float distance = sqrtf(static_cast<float>(dx * dx + dy * dy));

        if (distance > IRIS_RADIUS + 0.5f || distance < revealedFrom)
        {
          continue;
        }

        uint8_t color = MascotColor::IRIS_LIGHT;

        if (distance > 8.5f)
        {
          color = MascotColor::IRIS_DARK;
        }
        else if (distance > INNER_RING_RADIUS + 0.5f)
        {
          color = MascotColor::IRIS_MID;
        }
        else if (distance > INNER_RING_RADIUS - 0.5f && pose.irisReveal >= 0.9f)
        {
          color = MascotColor::SIGNAL_LAST;
        }

        canvas.setPixel(iris.x + dx, iris.y + dy, color);
      }
    }
  }

  // Pupil.
  float pupilRadius = PUPIL_RADIUS * pose.pupilScale * pose.pupilReveal;

  if (pupilRadius > 0.3f)
  {
    int reach = static_cast<int>(pupilRadius) + 1;

    for (int dy = -reach; dy <= reach; dy++)
    {
      for (int dx = -reach; dx <= reach; dx++)
      {
        if (dx * dx + dy * dy <= pupilRadius * pupilRadius + pupilRadius)
        {
          canvas.setPixel(pupil.x + dx, pupil.y + dy, MascotColor::BLACK);
        }
      }
    }
  }

  // Highlights: fixed relative to the iris, like a reflection.
  // The 2x2 white spot is what makes the eye look cute.
  if (pose.irisReveal >= 1.0f && pose.pupilReveal >= 0.6f)
  {
    canvas.setPixel(iris.x - 3, iris.y - 3, MascotColor::WHITE);
    canvas.setPixel(iris.x - 2, iris.y - 3, MascotColor::WHITE);
    canvas.setPixel(iris.x - 3, iris.y - 2, MascotColor::WHITE);
    canvas.setPixel(iris.x - 2, iris.y - 2, MascotColor::WHITE);

    canvas.setPixel(iris.x + 2, iris.y + 2, MascotColor::SIGNAL_LAST);
  }
}

// Erases whatever the iris drew outside the eyelids.
void MascotRenderer::applyLids(
  MascotCanvas& canvas,
  Center eye,
  const MascotPose& pose
) const
{
  constexpr int REACH = RING_RADIUS + 6;

  for (int x = -EYE_HALF_WIDTH; x <= EYE_HALF_WIDTH; x++)
  {
    int16_t top = roundToInt(eye.y + lidTop(x, pose.eyeOpen));
    int16_t bottom = roundToInt(eye.y + lidBottom(x, pose.eyeOpen));

    for (int y = eye.y - REACH; y <= eye.y + REACH; y++)
    {
      if (y <= top || y >= bottom)
      {
        canvas.setPixel(eye.x + x, y, MascotColor::BLACK);
      }
    }
  }
}

void MascotRenderer::drawOutline(
  MascotCanvas& canvas,
  Center eye,
  const MascotPose& pose
) const
{
  if (pose.contourReveal <= 0.0f)
  {
    return;
  }

  int16_t previousTop = roundToInt(eye.y + lidTop(-EYE_HALF_WIDTH - 1, pose.eyeOpen));
  int16_t previousBottom = roundToInt(eye.y + lidBottom(-EYE_HALF_WIDTH - 1, pose.eyeOpen));

  for (int x = -EYE_HALF_WIDTH; x <= EYE_HALF_WIDTH; x++)
  {
    int16_t top = roundToInt(eye.y + lidTop(x, pose.eyeOpen));
    int16_t bottom = roundToInt(eye.y + lidBottom(x, pose.eyeOpen));

    if (isColumnRevealed(x, pose.contourReveal))
    {
      // Spanning to the previous column keeps steep parts gap-free.
      canvas.drawVerticalSpan(eye.x + x, previousTop, top, MascotColor::WHITE);
      canvas.drawVerticalSpan(eye.x + x, previousBottom, bottom, MascotColor::WHITE);
    }

    previousTop = top;
    previousBottom = bottom;
  }
}

// The reticle "+" above the eye and the small tick below it.
void MascotRenderer::drawMarks(
  MascotCanvas& canvas,
  Center eye,
  const MascotPose& pose
) const
{
  uint8_t color = MascotColor::signal(pose.marksReveal * 0.75f);

  if (color == MascotColor::BLACK)
  {
    return;
  }

  int16_t plusY = eye.y - EYE_HALF_HEIGHT - 6;

  canvas.setPixel(eye.x, plusY, color);
  canvas.setPixel(eye.x - 1, plusY, color);
  canvas.setPixel(eye.x + 1, plusY, color);
  canvas.setPixel(eye.x, plusY - 1, color);
  canvas.setPixel(eye.x, plusY + 1, color);

  int16_t tickY = eye.y + EYE_HALF_HEIGHT + 5;

  canvas.setPixel(eye.x, tickY, color);
  canvas.setPixel(eye.x, tickY + 1, color);
}

void MascotRenderer::drawWaves(
  MascotCanvas& canvas,
  Center eye,
  const MascotPose& pose
) const
{
  if (pose.waveIntensity <= 0.0f)
  {
    return;
  }

  const float cycle = TWO_PI_F * pose.wavePhase;

  for (int side = -1; side <= 1; side += 2)
  {
    float amplitude = side > 0 ? pose.waveAmpRight : pose.waveAmpLeft;

    if (amplitude <= 0.0f)
    {
      continue;
    }

    // Each side drifts at its own pace: never a perfect mirror.
    float drift = side > 0
      ? 0.6f * sinf(cycle * 0.37f)
      : 0.6f * sinf(cycle * 0.29f + 1.7f);

    for (int wave = 0; wave < WAVES_PER_SIDE; wave++)
    {
      float position = static_cast<float>(wave);
      float strength = 1.0f;

      switch (pose.waveMode)
      {
        case WaveMode::BREATHE:
          position += 0.17f * sinf(cycle - wave * 0.6f);
          strength = (1.0f - 0.22f * wave) * (0.8f + 0.2f * sinf(cycle));
          break;

        case WaveMode::OUTWARD:
          position = fraction(wave / 3.0f + pose.wavePhase) * WAVES_PER_SIDE;
          strength = 1.0f - position / WAVES_PER_SIDE;
          break;

        case WaveMode::INWARD:
          position = fraction(wave / 3.0f - pose.wavePhase) * WAVES_PER_SIDE;
          strength = 1.0f - position / WAVES_PER_SIDE;
          break;
      }

      float radius = pose.waveRadius
        + position * pose.waveSpacing
        + drift
        + (amplitude - 1.0f) * 4.0f;

      drawArc(
        canvas,
        eye,
        side,
        radius,
        pose.waveSpan,
        pose.waveIntensity * strength * amplitude,
        side > 0 ? RIGHT_DASHES[wave] : LEFT_DASHES[wave]
      );
    }
  }
}

// Draws one dashed arc on the left (side = -1) or right (side = 1) of the
// eye. Its ends are dimmer than its middle, which reads as a soft glow.
void MascotRenderer::drawArc(
  MascotCanvas& canvas,
  Center eye,
  int side,
  float radius,
  float span,
  float intensity,
  uint16_t dashPattern
) const
{
  if (radius < 1.0f)
  {
    return;
  }

  uint8_t bodyColor = MascotColor::signal(intensity);
  uint8_t endColor = MascotColor::signal(intensity * 0.55f);

  if (bodyColor == MascotColor::BLACK)
  {
    return;
  }

  const float step = 0.5f / radius;

  for (float angle = -span; angle <= span; angle += step)
  {
    int dash = static_cast<int>((angle + span) * radius / WAVE_DASH_LENGTH) & 0x0F;

    if (((dashPattern >> dash) & 1) == 0)
    {
      continue;
    }

    uint8_t color = fabsf(angle) > span * 0.7f ? endColor : bodyColor;

    if (color == MascotColor::BLACK)
    {
      continue;
    }

    canvas.setPixel(
      roundToInt(eye.x + side * radius * cosf(angle)),
      roundToInt(eye.y + radius * sinf(angle)),
      color
    );
  }
}
