#include "ui/boot/BootArt.hpp"
#include "ui/boot/BootPalette.hpp"

#include <math.h>

namespace
{
  constexpr float PI_F = 3.14159265f;

  // ---- Brackets ----------------------------------------------------------

  constexpr int16_t BRACKET_ARM = 15;
  constexpr int16_t BRACKET_THICKNESS = 2;
  constexpr int16_t BRACKET_TICK_GAP = 3;
  constexpr int16_t BRACKET_TICK = 2;

  struct Rect
  {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
  };

  // Start of a run of `length` pixels from `corner` in direction `dir`.
  int16_t runStart(int16_t corner, int16_t offset, int16_t length, int8_t dir)
  {
    return dir > 0 ? corner + offset : corner - offset - length + 1;
  }

  // The two arms of a bracket, plus their ticks once fully grown.
  uint8_t bracketRects(
    Rect* rects,
    int16_t cornerX,
    int16_t cornerY,
    int8_t dirX,
    int8_t dirY,
    float progress
  )
  {
    if (progress > 1.0f)
    {
      progress = 1.0f;
    }

    const int16_t length = static_cast<int16_t>(BRACKET_ARM * progress + 0.5f);

    if (length <= 0)
    {
      return 0;
    }

    // The arms are thick towards the inside of the screen.
    const int16_t rowY = runStart(cornerY, 0, BRACKET_THICKNESS, dirY);
    const int16_t columnX = runStart(cornerX, 0, BRACKET_THICKNESS, dirX);

    uint8_t count = 0;

    rects[count++] = {
      runStart(cornerX, 0, length, dirX), rowY, length, BRACKET_THICKNESS
    };

    rects[count++] = {
      columnX, runStart(cornerY, 0, length, dirY), BRACKET_THICKNESS, length
    };

    if (progress >= 1.0f)
    {
      const int16_t offset = BRACKET_ARM + BRACKET_TICK_GAP;

      rects[count++] = {
        runStart(cornerX, offset, BRACKET_TICK, dirX),
        rowY,
        BRACKET_TICK,
        BRACKET_THICKNESS
      };

      rects[count++] = {
        columnX,
        runStart(cornerY, offset, BRACKET_TICK, dirY),
        BRACKET_THICKNESS,
        BRACKET_TICK
      };
    }

    return count;
  }

  // ---- Logo --------------------------------------------------------------

  constexpr int16_t CELL = 3;
  constexpr int16_t GLYPH_CELLS = 7;
  constexpr int16_t GLYPH_SIZE = GLYPH_CELLS * CELL;
  constexpr int16_t GLYPH_ADVANCE = GLYPH_SIZE + CELL;

  // A P E R I O, 7 x 7 cells each, one byte per row (bit 6 = left).
  constexpr uint8_t LOGO_GLYPHS[BootArt::LOGO_LETTERS][GLYPH_CELLS] = {
    {0x1C, 0x3E, 0x63, 0x63, 0x7F, 0x63, 0x63},
    {0x7E, 0x63, 0x63, 0x7E, 0x60, 0x60, 0x60},
    {0x7F, 0x60, 0x60, 0x7E, 0x60, 0x60, 0x7F},
    {0x7E, 0x63, 0x63, 0x7E, 0x66, 0x63, 0x63},
    {0x3E, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x3E},
    {0x3E, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3E}
  };

  constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b)
  {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }

  constexpr uint16_t LOGO_GLOW = rgb(10, 58, 26);
  constexpr uint16_t LOGO_FACE = rgb(150, 255, 176);
  constexpr uint16_t LOGO_SCANLINE = rgb(58, 178, 96);

  bool cellLit(uint8_t letter, int16_t column, int16_t row)
  {
    return (LOGO_GLYPHS[letter][row] >> (GLYPH_CELLS - 1 - column)) & 1;
  }

  int16_t letterX(int16_t logoX, uint8_t letter)
  {
    return logoX + letter * GLYPH_ADVANCE;
  }

  void clearLetter(Adafruit_ILI9341* display, int16_t x, int16_t y)
  {
    display->fillRect(
      x - 1,
      y - 1,
      GLYPH_SIZE + 2,
      GLYPH_SIZE + 2,
      BootPalette::color(BootTone::BLACK)
    );
  }

  uint32_t hash(uint32_t value)
  {
    value ^= value >> 16;
    value *= 0x7FEB352Du;
    value ^= value >> 15;
    value *= 0x846CA68Bu;
    value ^= value >> 16;
    return value;
  }

  // ---- Tiny font ---------------------------------------------------------

  struct TinyGlyph
  {
    char character;
    uint8_t rows[5]; // 3 bits per row, bit 2 = left
  };

  constexpr TinyGlyph TINY_GLYPHS[] = {
    {'>', {4, 2, 1, 2, 4}},
    {'.', {0, 0, 0, 0, 2}},
    {'-', {0, 0, 7, 0, 0}},
    {'A', {2, 5, 7, 5, 5}},
    {'B', {6, 5, 6, 5, 6}},
    {'D', {6, 5, 5, 5, 6}},
    {'E', {7, 4, 6, 4, 7}},
    {'F', {7, 4, 6, 4, 4}},
    {'G', {3, 4, 5, 5, 3}},
    {'H', {5, 5, 7, 5, 5}},
    {'I', {7, 2, 2, 2, 7}},
    {'K', {5, 5, 6, 5, 5}},
    {'L', {4, 4, 4, 4, 7}},
    {'N', {5, 7, 7, 5, 5}},
    {'O', {2, 5, 5, 5, 2}},
    {'P', {6, 5, 6, 4, 4}},
    {'S', {3, 4, 2, 1, 6}},
    {'T', {7, 2, 2, 2, 2}},
    {'U', {5, 5, 5, 5, 7}},
    {'W', {5, 5, 7, 7, 5}},
    {'Y', {5, 5, 2, 2, 2}}
  };

  const TinyGlyph* findTinyGlyph(char character)
  {
    for (const TinyGlyph& glyph : TINY_GLYPHS)
    {
      if (glyph.character == character)
      {
        return &glyph;
      }
    }

    return nullptr;
  }

  // ---- Emblem ------------------------------------------------------------

  // Plots a ring point with a soft halo: bright centre line, green body,
  // faint glow on both sides.
  void plotRingPoint(
    BootCanvas& canvas,
    int16_t cx,
    int16_t cy,
    float radius,
    float sine,
    float cosine,
    uint8_t coreTone
  )
  {
    for (int8_t offset = -2; offset <= 2; offset++)
    {
      uint8_t tone = BootTone::FAINT;

      if (offset == 0)
      {
        tone = coreTone;
      }
      else if (offset == -1 || offset == 1)
      {
        tone = coreTone > BootTone::GREEN ? BootTone::GREEN : coreTone - 1;
      }

      const float r = radius + offset;

      canvas.plot(
        static_cast<int16_t>(lroundf(cx + r * sine)),
        static_cast<int16_t>(lroundf(cy - r * cosine)),
        tone
      );
    }
  }

  // Shortest distance between two angles, in radians.
  float angleDistance(float a, float b)
  {
    float difference = fmodf(fabsf(a - b), 2.0f * PI_F);
    return difference > PI_F ? 2.0f * PI_F - difference : difference;
  }

  // Draws a ring on both sides of the vertical axis, from `start` to `end`
  // radians measured clockwise from the top (mirrored on the left).
  // `dashFrom` starts a dashed stretch on the right; -1 for none.
  void drawRing(
    BootCanvas& canvas,
    int16_t cx,
    int16_t cy,
    float radius,
    float start,
    float end,
    float reveal,
    float dashFromRight,
    float dashFromLeft,
    float scanAngle,
    uint8_t tone
  )
  {
    if (reveal <= 0.0f)
    {
      return;
    }

    const float stop = start + (end - start) * (reveal > 1.0f ? 1.0f : reveal);
    const float step = 0.6f / radius;

    for (float angle = start; angle <= stop; angle += step)
    {
      const float sine = sinf(angle);
      const float cosine = cosf(angle);

      for (int8_t side = -1; side <= 1; side += 2)
      {
        const float dashFrom = (side > 0) ? dashFromRight : dashFromLeft;

        if (dashFrom >= 0.0f
          && angle > dashFrom
          && fmodf(angle - dashFrom, 0.22f) > 0.14f)
        {
          continue;
        }

        uint8_t core = tone;

        if (scanAngle >= 0.0f
          && angleDistance(side * angle, scanAngle) < 0.32f)
        {
          core = BootTone::WHITE;
        }

        plotRingPoint(canvas, cx, cy, radius, side * sine, cosine, core);
      }
    }
  }

  void drawDot(
    BootCanvas& canvas,
    int16_t cx,
    int16_t cy,
    float radius,
    uint8_t tone
  )
  {
    const int16_t reach = static_cast<int16_t>(radius) + 2;

    for (int16_t y = -reach; y <= reach; y++)
    {
      for (int16_t x = -reach; x <= reach; x++)
      {
        const float distance = sqrtf(static_cast<float>(x * x + y * y));

        if (distance <= radius)
        {
          canvas.plot(cx + x, cy + y, tone);
        }
        else if (distance <= radius + 1.6f)
        {
          canvas.plot(cx + x, cy + y, BootTone::FAINT);
        }
      }
    }
  }

  // Twinkling tone for particle `index` at a given time.
  uint8_t twinkle(uint32_t index, uint32_t timeMs, uint32_t period)
  {
    const uint32_t phase = hash(index * 977u) % period;
    const uint32_t value = hash(index * 131u + (timeMs + phase) / period);
    const uint8_t roll = value % 8;

    if (roll == 0)
    {
      return BootTone::BLACK;
    }

    if (roll < 3)
    {
      return BootTone::DIM;
    }

    if (roll < 6)
    {
      return BootTone::MID;
    }

    return roll == 6 ? BootTone::GREEN : BootTone::BRIGHT;
  }

  // ---- Terrain -----------------------------------------------------------

  constexpr int16_t TERRAIN_ROWS = 10;
  constexpr int16_t CENTER_X = 160;

  float terrainBase(float depth)
  {
    return 12.0f + 42.0f * powf(depth, 1.25f);
  }

  float terrainScale(float depth)
  {
    return 0.45f + 0.75f * depth;
  }

  float terrainAmplitude(float depth)
  {
    return 3.0f + 15.0f * depth;
  }

  float terrainWave(float u, float row, float seconds)
  {
    return 0.65f * sinf(u * 0.022f + row * 0.35f + seconds * 0.8f)
      + 0.35f * sinf(u * 0.009f - seconds * 0.4f + row * 0.50f);
  }

  uint8_t depthTone(float depth)
  {
    if (depth < 0.3f)
    {
      return BootTone::DIM;
    }

    return depth < 0.7f ? BootTone::MID : BootTone::GREEN;
  }

  // Thins out the dots near the left and right edges, like the picture.
  bool edgeKeeps(int16_t x, uint32_t salt)
  {
    const int16_t edge = x < CENTER_X ? x : 319 - x;

    if (edge >= 36)
    {
      return true;
    }

    return static_cast<int16_t>(hash(salt) % 36) < edge;
  }
}

void BootArt::drawBracket(
  Adafruit_ILI9341* display,
  int16_t cornerX,
  int16_t cornerY,
  int8_t directionX,
  int8_t directionY,
  float progress,
  uint16_t color
)
{
  Rect rects[4];
  const uint8_t count = bracketRects(
    rects,
    cornerX,
    cornerY,
    directionX,
    directionY,
    progress
  );

  for (uint8_t i = 0; i < count; i++)
  {
    display->fillRect(rects[i].x, rects[i].y, rects[i].w, rects[i].h, color);
  }
}

void BootArt::drawBracket(
  BootCanvas& canvas,
  int16_t cornerX,
  int16_t cornerY,
  int8_t directionX,
  int8_t directionY,
  float progress,
  uint8_t tone
)
{
  Rect rects[4];
  const uint8_t count = bracketRects(
    rects,
    cornerX,
    cornerY,
    directionX,
    directionY,
    progress
  );

  for (uint8_t i = 0; i < count; i++)
  {
    canvas.fillRect(rects[i].x, rects[i].y, rects[i].w, rects[i].h, tone);
  }
}

int16_t BootArt::logoWidth()
{
  return LOGO_LETTERS * GLYPH_ADVANCE - CELL;
}

void BootArt::drawLogoLetter(
  Adafruit_ILI9341* display,
  int16_t logoX,
  int16_t logoY,
  uint8_t letter
)
{
  if (letter >= LOGO_LETTERS)
  {
    return;
  }

  const int16_t x = letterX(logoX, letter);

  clearLetter(display, x, logoY);

  // Glow first, so the face is drawn over it.
  for (int16_t row = 0; row < GLYPH_CELLS; row++)
  {
    for (int16_t column = 0; column < GLYPH_CELLS; column++)
    {
      if (cellLit(letter, column, row))
      {
        display->fillRect(
          x + column * CELL - 1,
          logoY + row * CELL - 1,
          CELL + 2,
          CELL + 2,
          LOGO_GLOW
        );
      }
    }
  }

  // Each cell is two bright rows and one darker scan line.
  for (int16_t row = 0; row < GLYPH_CELLS; row++)
  {
    for (int16_t column = 0; column < GLYPH_CELLS; column++)
    {
      if (!cellLit(letter, column, row))
      {
        continue;
      }

      const int16_t cellX = x + column * CELL;
      const int16_t cellY = logoY + row * CELL;

      display->fillRect(cellX, cellY, CELL, CELL - 1, LOGO_FACE);
      display->drawFastHLine(cellX, cellY + CELL - 1, CELL, LOGO_SCANLINE);
    }
  }
}

void BootArt::drawLogoNoise(
  Adafruit_ILI9341* display,
  int16_t logoX,
  int16_t logoY,
  uint8_t letter,
  uint32_t seed
)
{
  if (letter >= LOGO_LETTERS)
  {
    return;
  }

  const int16_t x = letterX(logoX, letter);

  clearLetter(display, x, logoY);

  for (int16_t row = 0; row < GLYPH_CELLS; row++)
  {
    for (int16_t column = 0; column < GLYPH_CELLS; column++)
    {
      const uint32_t value = hash(seed * 61u + row * 7u + column + letter * 97u);
      const bool lit = cellLit(letter, column, row);

      // Mostly the right cells, with a few wrong ones flickering around.
      if ((lit && value % 3 != 0) || (!lit && value % 7 == 0))
      {
        display->fillRect(
          x + column * CELL,
          logoY + row * CELL,
          CELL,
          CELL - 1,
          BootPalette::color(lit ? BootTone::MID : BootTone::DIM)
        );
      }
    }
  }
}

void BootArt::drawTinyChar(
  Adafruit_ILI9341* display,
  int16_t x,
  int16_t y,
  char character,
  uint16_t color
)
{
  display->fillRect(x, y, 3, 5, BootPalette::color(BootTone::BLACK));

  const TinyGlyph* glyph = findTinyGlyph(character);

  if (glyph == nullptr)
  {
    return;
  }

  for (int16_t row = 0; row < 5; row++)
  {
    for (int16_t column = 0; column < 3; column++)
    {
      if ((glyph->rows[row] >> (2 - column)) & 1)
      {
        display->drawPixel(x + column, y + row, color);
      }
    }
  }
}

void BootArt::drawEmblem(
  BootCanvas& canvas,
  int16_t cx,
  int16_t cy,
  const EmblemState& state
)
{
  // Particle cloud around and below the rings.
  constexpr uint32_t PARTICLES = 54;

  for (uint32_t i = 0; i < PARTICLES; i++)
  {
    if (state.sparks * PARTICLES <= i)
    {
      break;
    }

    const uint32_t h = hash(i + 11u);
    const float side = (h & 1) ? 1.0f : -1.0f;
    const float angle = 0.95f + ((h >> 1) % 1000) / 1000.0f * 1.75f;
    const float radius = 31.0f + ((h >> 11) % 1000) / 1000.0f * 22.0f;

    const int16_t x = static_cast<int16_t>(cx + side * radius * sinf(angle));
    const int16_t y = static_cast<int16_t>(cy - radius * cosf(angle));
    const uint8_t tone = twinkle(i, state.timeMs, 140);

    if (i % 6 == 0)
    {
      canvas.fillRect(x, y, 2, 2, tone);
    }
    else
    {
      canvas.plot(x, y, tone);
    }
  }

  // Dotted outer arc.
  if (state.sparks > 0.0f)
  {
    uint32_t index = 0;

    for (float angle = 0.75f; angle < 2.25f; angle += 0.13f, index++)
    {
      if (index > state.sparks * 12.0f)
      {
        break;
      }

      for (int8_t side = -1; side <= 1; side += 2)
      {
        const uint8_t tone = twinkle(index * 2 + (side > 0), state.timeMs, 220);

        canvas.fillRect(
          static_cast<int16_t>(cx + side * 36.0f * sinf(angle)),
          static_cast<int16_t>(cy - 36.0f * cosf(angle)),
          2,
          2,
          tone > BootTone::MID ? BootTone::MID : tone
        );
      }
    }
  }

  drawRing(
    canvas,
    cx,
    cy,
    27.0f,
    0.30f,
    2.42f,
    state.outerRing,
    1.60f,
    1.95f,
    state.outerRing >= 1.0f ? state.scanAngle : -1.0f,
    BootTone::BRIGHT
  );

  drawRing(
    canvas,
    cx,
    cy,
    15.0f,
    0.45f,
    PI_F - 0.50f,
    state.innerRing,
    -1.0f,
    -1.0f,
    -1.0f,
    BootTone::BRIGHT
  );

  // Top dot and the stem coming down to the centre.
  if (state.stem > 0.0f)
  {
    drawDot(canvas, cx, cy - 52, 1.6f, BootTone::WHITE);

    const int16_t top = cy - 46;
    const int16_t bottom = cy - 6;
    const int16_t end = top + static_cast<int16_t>((bottom - top) * state.stem);

    for (int16_t y = top; y <= end; y++)
    {
      canvas.plot(cx - 2, y, BootTone::FAINT);
      canvas.plot(cx - 1, y, BootTone::BRIGHT);
      canvas.plot(cx, y, BootTone::WHITE);
      canvas.plot(cx + 1, y, BootTone::FAINT);
    }
  }

  // Centre dot and the short stem below it.
  if (state.core > 0.0f)
  {
    drawDot(
      canvas,
      cx,
      cy,
      (2.0f + 1.4f * state.pulse) * state.core,
      BootTone::BRIGHT
    );

    canvas.fillRect(cx - 1, cy - 1, 2, 2, BootTone::WHITE);

    const int16_t bottom = cy + 12 + static_cast<int16_t>(17 * state.core);

    for (int16_t y = cy + 12; y <= bottom; y++)
    {
      canvas.plot(cx - 2, y, BootTone::FAINT);
      canvas.plot(cx - 1, y, BootTone::GREEN);
      canvas.plot(cx, y, BootTone::BRIGHT);
      canvas.plot(cx + 1, y, BootTone::FAINT);
    }
  }
}

void BootArt::drawTerrain(BootCanvas& canvas, uint32_t timeMs, float reveal)
{
  if (reveal <= 0.0f)
  {
    return;
  }

  const float seconds = timeMs / 1000.0f;
  const float hidden = 1.0f - (reveal > 1.0f ? 1.0f : reveal);

  // Rows of dots running across, from the horizon to the front.
  for (int16_t row = 0; row < TERRAIN_ROWS; row++)
  {
    const float depth = row / static_cast<float>(TERRAIN_ROWS - 1);

    if (depth < hidden)
    {
      continue;
    }

    const float base = terrainBase(depth);
    const float scale = terrainScale(depth);
    const float amplitude = terrainAmplitude(depth);
    const int16_t step = 2 + static_cast<int16_t>(3.0f * depth);
    const uint8_t tone = depthTone(depth);

    for (int16_t x = (row * 2) % step; x < 320; x += step)
    {
      if (!edgeKeeps(x, row * 1000u + x))
      {
        continue;
      }

      const float wave = terrainWave((x - CENTER_X) / scale, row, seconds);
      const int16_t y = static_cast<int16_t>(base - amplitude * wave);

      canvas.plot(x, y, wave > 0.6f ? tone + 1 : tone);
    }
  }

  // Lines running towards the horizon, like a grid in perspective.
  for (int16_t column = -9; column <= 9; column++)
  {
    for (float depth = hidden; depth <= 1.0f; depth += 0.045f)
    {
      const float scale = terrainScale(depth);
      const float u = column * 22.0f;
      const int16_t x = static_cast<int16_t>(CENTER_X + u * scale);

      if (x < 0 || x >= 320 || !edgeKeeps(x, column * 7919u))
      {
        continue;
      }

      const float row = depth * (TERRAIN_ROWS - 1);
      const float wave = terrainWave(u, row, seconds);
      const int16_t y = static_cast<int16_t>(
        terrainBase(depth) - terrainAmplitude(depth) * wave
      );

      canvas.plot(x, y, depth < 0.5f ? BootTone::DIM : BootTone::MID);
    }
  }

  // One ridge, a little in front of the middle, is lit up brightly and
  // crosses the whole screen.
  if (reveal > 0.5f)
  {
    constexpr float CREST_DEPTH = 0.42f;

    const float base = terrainBase(CREST_DEPTH);
    const float scale = terrainScale(CREST_DEPTH);
    const float amplitude = terrainAmplitude(CREST_DEPTH);
    const float row = CREST_DEPTH * (TERRAIN_ROWS - 1);

    for (int16_t x = 0; x < 320; x += 2)
    {
      if (!edgeKeeps(x, x * 31u + 5u))
      {
        continue;
      }

      const float wave = terrainWave((x - CENTER_X) / scale, row, seconds);
      const int16_t y = static_cast<int16_t>(base - amplitude * wave);
      const bool sparkle = hash(x * 13u + timeMs / 90u) % 23 == 0;

      canvas.plot(x, y, sparkle ? BootTone::WHITE : BootTone::BRIGHT);
      canvas.plot(x, y + 1, BootTone::FAINT);
    }
  }

  // Particles floating up above the terrain.
  constexpr uint32_t PARTICLES = 36;
  constexpr int16_t PARTICLE_SPAN = 46;

  for (uint32_t i = 0; i < PARTICLES; i++)
  {
    const uint32_t h = hash(i * 7u + 3u);
    const int16_t x = h % 320;
    const float speed = 3.0f + (h >> 9) % 6;
    const float start = static_cast<float>((h >> 13) % PARTICLE_SPAN);
    const float y = fmodf(start - seconds * speed + PARTICLE_SPAN * 8, PARTICLE_SPAN);

    const uint8_t tone = twinkle(i + 300, timeMs, 160);

    if (i % 7 == 0)
    {
      canvas.fillRect(x, static_cast<int16_t>(y), 2, 2, tone);
    }
    else
    {
      canvas.plot(x, static_cast<int16_t>(y), tone);
    }
  }
}
