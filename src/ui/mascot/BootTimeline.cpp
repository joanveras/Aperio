#include "ui/mascot/BootTimeline.hpp"

#include <cmath>

namespace
{
  constexpr float PI_F = 3.14159265f;

  // 0 before `start`, 1 after `end`, linear in between.
  float progress(float time, float start, float end)
  {
    if (time <= start)
    {
      return 0.0f;
    }

    if (time >= end)
    {
      return 1.0f;
    }

    return (time - start) / (end - start);
  }

  float lerp(float from, float to, float amount)
  {
    return from + (to - from) * amount;
  }

  float easeOut(float x)
  {
    float inverse = 1.0f - x;

    return 1.0f - inverse * inverse * inverse;
  }

  float easeInOut(float x)
  {
    if (x < 0.5f)
    {
      return 4.0f * x * x * x;
    }

    float inverse = -2.0f * x + 2.0f;

    return 1.0f - inverse * inverse * inverse / 2.0f;
  }

  // Goes slightly past 1 before settling: a small "pop".
  float easeOutBack(float x)
  {
    constexpr float OVERSHOOT = 1.70158f;

    float shifted = x - 1.0f;

    return 1.0f
      + (OVERSHOOT + 1.0f) * shifted * shifted * shifted
      + OVERSHOOT * shifted * shifted;
  }

  // 0 -> 1 -> 0
  float bump(float x)
  {
    return sinf(x * PI_F);
  }

  // Wave phase reached at the end of the "growing pulses" stage. The
  // converging stage starts from it so the arcs do not jump.
  constexpr float GROW_PHASE_END = (2400.0f - 400.0f) / 900.0f;
}

BootFrame BootTimeline::at(uint32_t elapsedMs)
{
  const float t = static_cast<float>(elapsedMs);

  BootFrame frame;
  MascotPose& pose = frame.pose;

  // ------------------------------------------------------------------
  // Waves, one stage at a time.
  // ------------------------------------------------------------------

  if (t < 2400.0f)
  {
    // 0.4 - 2.4 s: faint pulses leave the center and grow stronger.
    float grow = progress(t, 400.0f, 2400.0f);

    pose.waveMode = WaveMode::OUTWARD;
    pose.waveIntensity = t < 400.0f ? 0.0f : lerp(0.15f, 1.0f, grow * grow);
    pose.waveRadius = lerp(5.0f, 14.0f, easeInOut(grow));
    pose.waveSpacing = lerp(5.0f, 9.0f, grow);
    pose.waveSpan = lerp(0.9f, 0.6f, grow);
    pose.wavePhase = (t - 400.0f) / 900.0f;
    pose.waveAmpLeft = 0.85f + 0.15f * sinf(t / 310.0f);
    pose.waveAmpRight = 0.85f + 0.15f * sinf(t / 270.0f + 1.0f);
  }
  else if (t < 3800.0f)
  {
    // 2.4 - 3.3 s: the energy turns around and concentrates in the center.
    pose.waveMode = WaveMode::INWARD;
    pose.waveIntensity = 1.0f - progress(t, 2900.0f, 3300.0f);
    pose.waveRadius = 14.0f;
    pose.waveSpacing = 9.0f;
    pose.waveSpan = 0.6f;
    pose.wavePhase = -GROW_PHASE_END + (t - 2400.0f) / 600.0f;
  }
  else if (t < 4300.0f)
  {
    // 3.8 - 4.3 s: the finished eye releases one strong pulse.
    pose.waveMode = WaveMode::OUTWARD;
    pose.waveIntensity = lerp(1.0f, 0.6f, progress(t, 3800.0f, 4300.0f));
    pose.waveRadius = 38.0f;
    pose.waveSpacing = 7.0f;
    pose.waveSpan = 0.5f;
    pose.wavePhase = (t - 3800.0f) / 500.0f;
  }
  else
  {
    // 4.3 s onwards: the waves settle into a slow breathing.
    pose.waveMode = WaveMode::BREATHE;
    pose.waveIntensity = lerp(0.6f, 0.5f, progress(t, 4300.0f, 4800.0f));
    pose.waveRadius = 38.0f;
    pose.waveSpacing = 7.0f;
    pose.waveSpan = 0.5f;
    pose.wavePhase = (t - 4300.0f) / 2800.0f;
  }

  // ------------------------------------------------------------------
  // The eye forms: spark -> ring -> iris -> pupil -> outline -> marks.
  // ------------------------------------------------------------------

  // 0.1 - 0.5 s: a spark flickers on at the center, like a signal locking
  // in. It fades when the pupil takes its place.
  bool flickerLow = t < 500.0f && (elapsedMs / 70) % 2 == 0;

  pose.coreIntensity = easeOut(progress(t, 100.0f, 500.0f))
    * (flickerLow ? 0.4f : 0.85f)
    * (1.0f - progress(t, 3050.0f, 3250.0f));

  // 2.4 - 3.1 s: the segmented ring sweeps in while turning into place,
  // then the iris fills from the outside in.
  pose.ringReveal = easeInOut(progress(t, 2400.0f, 2900.0f));
  pose.ringRotation = 0.25f * (1.0f - easeOut(progress(t, 2400.0f, 3800.0f)));
  pose.irisReveal = easeOut(progress(t, 2700.0f, 3100.0f));

  // 3.05 - 3.35 s: the pupil pops in.
  pose.pupilReveal = easeOutBack(progress(t, 3050.0f, 3350.0f));

  // 3.3 - 3.8 s: the outline materializes, with two short glitches.
  pose.contourReveal = easeInOut(progress(t, 3300.0f, 3800.0f));

  bool glitching = (t >= 3380.0f && t < 3460.0f) || (t >= 3620.0f && t < 3680.0f);
  pose.glitch = glitching ? 0.7f : 0.0f;

  // 3.8 - 4.3 s: the pulse. A white flash, the pupil contracts briefly
  // and the reticle marks appear.
  float flash = 1.0f - progress(t, 3800.0f, 4150.0f);
  pose.flash = t >= 3800.0f ? 0.85f * flash * flash : 0.0f;
  pose.pupilScale = 1.0f - 0.25f * bump(progress(t, 3800.0f, 4300.0f));
  pose.marksReveal = easeOut(progress(t, 3800.0f, 4100.0f));

  // ------------------------------------------------------------------
  // First signs of life: a blink, then the eye reads the title.
  // ------------------------------------------------------------------

  // 4.5 - 4.72 s: closes fast, opens slower. The waves dim with it.
  if (t >= 4500.0f && t < 4570.0f)
  {
    pose.eyeOpen = 1.0f - progress(t, 4500.0f, 4570.0f);
  }
  else if (t >= 4570.0f && t < 4720.0f)
  {
    pose.eyeOpen = easeOut(progress(t, 4570.0f, 4720.0f));
  }

  pose.waveIntensity *= 0.8f + 0.2f * pose.eyeOpen;

  // 4.75 - 5.6 s: the title types itself and the eye follows the letters.
  frame.titleProgress = progress(t, 4750.0f, 5250.0f);
  frame.subtitleProgress = progress(t, 5200.0f, 5600.0f);

  float reading = easeInOut(progress(t, 4650.0f, 4850.0f))
    * (1.0f - easeInOut(progress(t, 5650.0f, 5900.0f)));

  pose.gazeY = 0.8f * reading;
  pose.gazeX = lerp(-0.6f, 0.6f, progress(t, 4750.0f, 5600.0f)) * reading;

  frame.finished = elapsedMs >= DURATION_MS;

  return frame;
}
