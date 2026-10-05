#pragma once

#include "MascotCanvas.hpp"
#include "MascotPose.hpp"

// Turns a MascotPose into pixels on a MascotCanvas.
//
// The renderer has no state and no notion of time: the same pose always
// produces the same image. The eye is centered on the canvas.
//
// Layers, from back to front:
//   sclera -> core spark -> iris (segmented ring, iris body, inner ring,
//   pupil, highlights) -> eyelids -> outline -> marks -> waves
class MascotRenderer
{
public:
  // Eye geometry, in logical pixels.
  static constexpr int EYE_HALF_WIDTH = 34;
  static constexpr int EYE_HALF_HEIGHT = 17;
  static constexpr int RING_RADIUS = 14;
  static constexpr int IRIS_RADIUS = 11;
  static constexpr int INNER_RING_RADIUS = 6;
  static constexpr int PUPIL_RADIUS = 4;

  void draw(MascotCanvas& canvas, const MascotPose& pose) const;

private:
  struct Center
  {
    int16_t x;
    int16_t y;
  };

  float lidTop(float x, float eyeOpen) const;
  float lidBottom(float x, float eyeOpen) const;
  bool isColumnRevealed(int x, float contourReveal) const;

  void drawSclera(MascotCanvas& canvas, Center eye, const MascotPose& pose) const;
  void drawCore(MascotCanvas& canvas, Center eye, const MascotPose& pose) const;
  void drawIris(MascotCanvas& canvas, Center eye, const MascotPose& pose) const;
  void applyLids(MascotCanvas& canvas, Center eye, const MascotPose& pose) const;
  void drawOutline(MascotCanvas& canvas, Center eye, const MascotPose& pose) const;
  void drawMarks(MascotCanvas& canvas, Center eye, const MascotPose& pose) const;
  void drawWaves(MascotCanvas& canvas, Center eye, const MascotPose& pose) const;

  void drawArc(
    MascotCanvas& canvas,
    Center eye,
    int side,
    float radius,
    float span,
    float intensity,
    uint16_t dashPattern
  ) const;
};
