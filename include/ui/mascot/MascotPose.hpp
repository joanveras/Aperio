#pragma once

#include <cstdint>

// How the waves around the eye move.
//   BREATHE: arcs stay in place and expand/contract slowly.
//   OUTWARD: arcs are born next to the eye and travel outwards.
//   INWARD:  arcs come from far away and are absorbed by the eye.
enum class WaveMode : uint8_t
{
  BREATHE,
  OUTWARD,
  INWARD
};

// Everything the renderer needs to draw one frame of the mascot.
//
// A pose is plain data: it knows nothing about time or states. Whoever
// drives the mascot (the boot timeline today, a state animator later)
// fills a pose every frame and hands it to MascotRenderer.
//
// The default values describe the fully formed eye, looking forward,
// with calm waves.
struct MascotPose
{
  // Eye
  float eyeOpen = 1.0f;       // 0 = closed, 1 = fully open
  float gazeX = 0.0f;         // -1 = left, 1 = right
  float gazeY = 0.0f;         // -1 = up, 1 = down
  float pupilScale = 1.0f;    // 1 = normal size

  // Iris
  float ringRotation = 0.0f;  // segmented ring rotation, in turns

  // Build-up, used by the boot animation (1 = fully formed).
  float coreIntensity = 0.0f; // the "+" spark at the center, before the pupil
  float ringReveal = 1.0f;    // segmented ring, sweeps around clockwise
  float irisReveal = 1.0f;    // iris fills from the outside in
  float pupilReveal = 1.0f;   // pupil grows (values above 1 overshoot)
  float contourReveal = 1.0f; // eye outline, from the corners to the center
  float marksReveal = 1.0f;   // the "+" above and the tick below the eye

  // Waves
  WaveMode waveMode = WaveMode::BREATHE;
  float waveIntensity = 0.5f; // 0..1, brightness of the arcs
  float waveRadius = 38.0f;   // radius of the innermost arc (logical px)
  float waveSpacing = 7.0f;   // distance between arcs (logical px)
  float waveSpan = 0.5f;      // half angle of each arc (radians)
  float waveSpanVar = 0.0f;   // 0 = every arc the same length; >0 shortens
                              // some arcs so sizes vary within a frame
  float waveAmpLeft = 1.0f;   // 0..1 per side, lets one side react alone
  float waveAmpRight = 1.0f;
  float wavePhase = 0.0f;     // grows with time; one unit = one cycle

  // Whole-frame effects, applied when the frame is sent to the display.
  float flash = 0.0f;         // 0..1, pushes every color towards white
  float glitch = 0.0f;        // 0..1, shifts random rows sideways
};
