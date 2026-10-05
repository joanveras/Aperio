#pragma once

#include <cstdint>

#include "MascotPose.hpp"

// Everything the boot animation shows at a given moment.
struct BootFrame
{
  MascotPose pose;
  float titleProgress;    // 0..1, how much of "APERIO" is visible
  float subtitleProgress; // 0..1, how much of "Quod Latet" is visible
  bool finished;
};

// The boot story as a pure function of time:
//
//   spark -> faint pulses -> growing waves -> energy converges
//   -> iris rings form (outside in) -> pupil -> outline -> strong pulse
//   -> waves settle -> blink -> APERIO / Quod Latet
//
// No state and no drawing here: given the elapsed time it returns the
// pose and text progress, so the whole sequence can be read top to
// bottom in BootTimeline.cpp and tuned by editing the numbers there.
namespace BootTimeline
{
  constexpr uint32_t DURATION_MS = 6200;

  BootFrame at(uint32_t elapsedMs);
}
