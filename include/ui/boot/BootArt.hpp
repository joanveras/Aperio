#pragma once

#include <Adafruit_ILI9341.h>
#include <cstdint>

#include "BootCanvas.hpp"

// The pictures of the retro boot screen. Every function draws one element;
// RetroBoot decides when and how far along each one is.
namespace BootArt
{
  // ---- Drawn straight on the display (static once shown) --------------

  // Corner bracket with its little tick, like a camera viewfinder.
  // `progress` (0..1) grows the arms from the corner.
  void drawBracket(
    Adafruit_ILI9341* display,
    int16_t cornerX,
    int16_t cornerY,
    int8_t directionX,
    int8_t directionY,
    float progress,
    uint16_t color
  );

  // The APERIO logo: chunky pixel letters with a soft glow and CRT scan
  // lines. Letters are drawn one by one so they can be revealed in turn.
  constexpr uint8_t LOGO_LETTERS = 6;

  int16_t logoWidth();

  void drawLogoLetter(
    Adafruit_ILI9341* display,
    int16_t logoX,
    int16_t logoY,
    uint8_t letter
  );

  // The same letter as flickering noise, before it "decodes".
  void drawLogoNoise(
    Adafruit_ILI9341* display,
    int16_t logoX,
    int16_t logoY,
    uint8_t letter,
    uint32_t seed
  );

  // A tiny 3 x 5 pixel font for the init log, smaller than the built-in
  // one. Only the characters the log uses exist (A-Z subset, '>', '.',
  // '-', space); anything else draws as a blank cell.
  constexpr int16_t TINY_ADVANCE = 4;

  void drawTinyChar(
    Adafruit_ILI9341* display,
    int16_t x,
    int16_t y,
    char character,
    uint16_t color
  );

  // ---- Drawn into canvases every frame ---------------------------------

  // Same bracket, drawn into a canvas (the bottom ones share their space
  // with the terrain).
  void drawBracket(
    BootCanvas& canvas,
    int16_t cornerX,
    int16_t cornerY,
    int8_t directionX,
    int8_t directionY,
    float progress,
    uint8_t tone
  );

  // How far each part of the emblem has been drawn (0..1).
  struct EmblemState
  {
    float stem;       // top dot and the line coming down
    float core;       // the centre dot
    float innerRing;
    float outerRing;
    float sparks;     // the dotted outer arc and the particle cloud
    float scanAngle;  // radians; a bright sweep travelling the outer ring
    float pulse;      // 0..1, breathing of the centre dot
    uint32_t timeMs;  // for twinkling
  };

  // The signal eye: a dot on a stem, a centre point inside two broken rings
  // and a cloud of pixels around it. Centred on (cx, cy) of the canvas.
  void drawEmblem(
    BootCanvas& canvas,
    int16_t cx,
    int16_t cy,
    const EmblemState& state
  );

  // Dotted wave terrain in perspective, with floating particles.
  // `reveal` (0..1) lets it rise from the bottom.
  void drawTerrain(BootCanvas& canvas, uint32_t timeMs, float reveal);
}
