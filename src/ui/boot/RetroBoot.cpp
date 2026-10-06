#include "ui/boot/RetroBoot.hpp"
#include "ui/boot/BootArt.hpp"
#include "ui/boot/BootPalette.hpp"
#include "Config.hpp"

#include <Arduino.h>
#include <cstring>
#include <math.h>

namespace
{
  // ---- Layout (320 x 240) ------------------------------------------------

  constexpr int16_t SCREEN_WIDTH = 320;
  constexpr int16_t CENTER_X = SCREEN_WIDTH / 2;

  constexpr int16_t BRACKET_LEFT = 8;
  constexpr int16_t BRACKET_RIGHT = 311;
  constexpr int16_t BRACKET_TOP = 10;
  constexpr int16_t BRACKET_BOTTOM = 229;

  constexpr int16_t VERSION_X = 280;
  constexpr int16_t VERSION_Y = 20;

  constexpr int16_t EMBLEM_X = 104;
  constexpr int16_t EMBLEM_Y = 4;
  constexpr int16_t EMBLEM_WIDTH = 112;
  constexpr int16_t EMBLEM_HEIGHT = 88;
  constexpr int16_t EMBLEM_CX = 56;
  constexpr int16_t EMBLEM_CY = 58;

  constexpr int16_t LOGO_Y = 95;

  constexpr const char* SUBTITLE = "QUOD LATET";
  constexpr int16_t SUBTITLE_Y = 122;
  constexpr int16_t SUBTITLE_ADVANCE = 9; // wide letter spacing
  constexpr int16_t DASH_LENGTH = 12;
  constexpr int16_t DASH_GAP = 9;

  constexpr int16_t LOG_X = 5;
  constexpr int16_t LOG_Y = 124;
  constexpr int16_t LOG_SPACING = 9;
  constexpr int16_t LOG_OK_COLUMN = 12;
  constexpr int16_t CHAR_WIDTH = 6;

  constexpr const char* LOG_TEXT[] = {
    "> DISPLAY..",
    "> INPUT....",
    "> WI-FI....",
    "> BLUETOOTH",
    "> LOADING"
  };

  constexpr int16_t BAR_X = 117;
  constexpr int16_t BAR_Y = 141;
  constexpr int16_t BAR_WIDTH = 86;
  constexpr int16_t BAR_HEIGHT = 16;
  constexpr int16_t BAR_BORDER = 2;
  constexpr int16_t BAR_PADDING = 2;
  constexpr uint8_t BAR_SEGMENTS = 10;
  constexpr int16_t SEGMENT_WIDTH = 6;
  constexpr int16_t SEGMENT_PITCH = 8;

  constexpr int16_t TERRAIN_Y = 170;
  constexpr int16_t TERRAIN_HEIGHT = 66;

  // ---- Timeline (ms from power on) ---------------------------------------

  constexpr uint32_t POWER_LINE_END = 150;
  constexpr uint32_t POWER_ON_END = 300;

  constexpr uint32_t BRACKETS_START = 300;
  constexpr uint32_t BRACKETS_END = 650;
  constexpr uint32_t VERSION_START = 650;
  constexpr uint32_t VERSION_CHAR_MS = 40;

  constexpr uint32_t STEM_START = 500;
  constexpr uint32_t STEM_END = 1000;
  constexpr uint32_t CORE_START = 900;
  constexpr uint32_t CORE_END = 1150;
  constexpr uint32_t INNER_START = 1000;
  constexpr uint32_t INNER_END = 1500;
  constexpr uint32_t OUTER_START = 1200;
  constexpr uint32_t OUTER_END = 1900;
  constexpr uint32_t SPARKS_START = 1500;
  constexpr uint32_t SPARKS_END = 2500;

  constexpr uint32_t TERRAIN_START = 700;
  constexpr uint32_t TERRAIN_END = 1800;

  constexpr uint32_t LOGO_START = 1700;
  constexpr uint32_t LOGO_STAGGER = 110;
  constexpr uint32_t LOGO_NOISE_MS = 160;

  constexpr uint32_t DASHES_START = 2350;
  constexpr uint32_t DASHES_END = 2600;
  constexpr uint32_t SUBTITLE_START = 2400;
  constexpr uint32_t SUBTITLE_CHAR_MS = 30;

  constexpr uint32_t BAR_FRAME_START = 2700;

  constexpr uint32_t LOG_START = 2800;
  constexpr uint32_t LOG_LINE_MS = 650;
  constexpr uint32_t LOG_CHAR_MS = 22;
  constexpr uint32_t LOG_OK_DELAY = 420;
  constexpr uint32_t LOADING_DOT_MS = 220;

  constexpr uint32_t PROGRESS_START = 2800;
  constexpr uint32_t PROGRESS_END = 6100;
  constexpr uint32_t FLASH_END = 6300;

  constexpr uint32_t BOOT_END = 6800;

  // 0..1 across [start, end].
  float progressBetween(uint32_t elapsed, uint32_t start, uint32_t end)
  {
    if (elapsed <= start)
    {
      return 0.0f;
    }

    if (elapsed >= end)
    {
      return 1.0f;
    }

    return static_cast<float>(elapsed - start) / (end - start);
  }

  float easeOut(float value)
  {
    return 1.0f - (1.0f - value) * (1.0f - value);
  }

  uint8_t typedChars(uint32_t elapsed, uint32_t start, uint32_t charMs, uint8_t length)
  {
    if (elapsed < start)
    {
      return 0;
    }

    const uint32_t chars = (elapsed - start) / charMs + 1;
    return chars > length ? length : static_cast<uint8_t>(chars);
  }
}

RetroBoot::RetroBoot(Adafruit_ILI9341* displayInstance)
  : display(displayInstance),
    emblemCanvas(EMBLEM_WIDTH, EMBLEM_HEIGHT),
    terrainCanvas(SCREEN_WIDTH, TERRAIN_HEIGHT),
    startTime(0),
    lastFrameTime(0),
    finished(false)
{
}

void RetroBoot::begin(uint32_t now)
{
  display->fillScreen(BootPalette::color(BootTone::BLACK));

  startTime = now;
  lastFrameTime = now - FRAME_INTERVAL_MS;
  finished = false;

  screenCleared = false;
  bracketsShown = 0.0f;
  versionChars = 0;
  memset(logoStates, 0, sizeof(logoStates));
  dashesShown = 0.0f;
  subtitleChars = 0;
  barFrameShown = false;
  memset(logChars, 0, sizeof(logChars));
  memset(logOkShown, 0, sizeof(logOkShown));
  loadingDots = 0;
  segmentsShown = 0;
  segmentsTone = BootTone::GREEN;

  frameCount = 0;
  totalFrameMicros = 0;

  if (!emblemCanvas.begin() || !terrainCanvas.begin())
  {
    Serial.println("[boot] not enough memory for the boot screen, skipping");
    finish();
  }
}

void RetroBoot::update(uint32_t now)
{
  if (finished || now - lastFrameTime < FRAME_INTERVAL_MS)
  {
    return;
  }

  lastFrameTime = now;

  const uint32_t elapsed = now - startTime;

  if (elapsed >= BOOT_END)
  {
    finish();
    return;
  }

  const uint32_t frameStart = micros();

  if (elapsed < POWER_ON_END)
  {
    drawPowerOn(elapsed);
  }
  else
  {
    drawFrame(elapsed);
  }

  frameCount++;
  totalFrameMicros += micros() - frameStart;
}

void RetroBoot::skip()
{
  if (!finished)
  {
    finish();
  }
}

bool RetroBoot::isFinished() const
{
  return finished;
}

// A CRT warming up: a thin line opens across the middle of the screen,
// then blooms and fades away.
void RetroBoot::drawPowerOn(uint32_t elapsed)
{
  constexpr int16_t LINE_Y = 119;

  if (elapsed < POWER_LINE_END)
  {
    const float grow = easeOut(progressBetween(elapsed, 0, POWER_LINE_END));
    const int16_t width = static_cast<int16_t>(SCREEN_WIDTH * grow);

    display->fillRect(
      CENTER_X - width / 2,
      LINE_Y,
      width,
      2,
      BootPalette::color(BootTone::WHITE)
    );

    return;
  }

  const float bloom = progressBetween(elapsed, POWER_LINE_END, POWER_ON_END);
  const int16_t height = 2 + static_cast<int16_t>(40 * bloom);
  const uint8_t tone = bloom < 0.35f
    ? BootTone::BRIGHT
    : (bloom < 0.7f ? BootTone::MID : BootTone::FAINT);

  display->fillRect(
    0,
    LINE_Y + 1 - height / 2,
    SCREEN_WIDTH,
    height,
    BootPalette::color(tone)
  );
}

void RetroBoot::drawFrame(uint32_t elapsed)
{
  if (!screenCleared)
  {
    display->fillScreen(BootPalette::color(BootTone::BLACK));
    screenCleared = true;
  }

  drawBrackets(elapsed);
  drawVersion(elapsed);
  drawEmblem(elapsed);
  drawLogo(elapsed);
  drawSubtitle(elapsed);
  drawLog(elapsed);
  drawProgress(elapsed);
  drawTerrain(elapsed);
}

void RetroBoot::drawBrackets(uint32_t elapsed)
{
  const float progress = easeOut(
    progressBetween(elapsed, BRACKETS_START, BRACKETS_END)
  );

  if (progress <= bracketsShown)
  {
    return;
  }

  const uint16_t color = BootPalette::color(BootTone::GREEN);

  BootArt::drawBracket(display, BRACKET_LEFT, BRACKET_TOP, 1, 1, progress, color);
  BootArt::drawBracket(display, BRACKET_RIGHT, BRACKET_TOP, -1, 1, progress, color);

  bracketsShown = progress;
}

void RetroBoot::drawVersion(uint32_t elapsed)
{
  const uint8_t length = strlen(APERIO_VERSION);
  const uint8_t visible = typedChars(elapsed, VERSION_START, VERSION_CHAR_MS, length);

  while (versionChars < visible)
  {
    display->drawChar(
      VERSION_X + versionChars * CHAR_WIDTH,
      VERSION_Y,
      APERIO_VERSION[versionChars],
      BootPalette::color(BootTone::GREEN),
      BootPalette::color(BootTone::BLACK),
      1
    );

    versionChars++;
  }
}

// Each letter flickers as noise for a moment, then locks into place.
void RetroBoot::drawLogo(uint32_t elapsed)
{
  const int16_t logoX = CENTER_X - BootArt::logoWidth() / 2;

  for (uint8_t letter = 0; letter < BootArt::LOGO_LETTERS; letter++)
  {
    const uint32_t start = LOGO_START + letter * LOGO_STAGGER;

    if (elapsed < start || logoStates[letter] == 2)
    {
      continue;
    }

    if (elapsed < start + LOGO_NOISE_MS)
    {
      BootArt::drawLogoNoise(display, logoX, LOGO_Y, letter, elapsed / 50);
      logoStates[letter] = 1;
      continue;
    }

    BootArt::drawLogoLetter(display, logoX, LOGO_Y, letter);
    logoStates[letter] = 2;
  }
}

void RetroBoot::drawSubtitle(uint32_t elapsed)
{
  const uint8_t length = strlen(SUBTITLE);
  const int16_t width = length * SUBTITLE_ADVANCE - (SUBTITLE_ADVANCE - 5);
  const int16_t left = CENTER_X - width / 2;

  // The dashes on each side grow outwards from the text.
  const float dashes = easeOut(progressBetween(elapsed, DASHES_START, DASHES_END));

  if (dashes > dashesShown)
  {
    const int16_t dash = static_cast<int16_t>(DASH_LENGTH * dashes);
    const int16_t dashY = SUBTITLE_Y + 3;
    const uint16_t color = BootPalette::color(BootTone::GREEN);

    display->fillRect(left - DASH_GAP - dash, dashY, dash, 2, color);
    display->fillRect(left + width + DASH_GAP, dashY, dash, 2, color);

    dashesShown = dashes;
  }

  const uint8_t visible = typedChars(elapsed, SUBTITLE_START, SUBTITLE_CHAR_MS, length);

  while (subtitleChars < visible)
  {
    display->drawChar(
      left + subtitleChars * SUBTITLE_ADVANCE,
      SUBTITLE_Y,
      SUBTITLE[subtitleChars],
      BootPalette::color(BootTone::BRIGHT),
      BootPalette::color(BootTone::BLACK),
      1
    );

    subtitleChars++;
  }
}

// One init line at a time: typed out, then "OK" after a short pause. The
// last line keeps cycling its dots until the boot ends.
void RetroBoot::drawLog(uint32_t elapsed)
{
  const uint16_t textColor = BootPalette::color(BootTone::MID);
  const uint16_t okColor = BootPalette::color(BootTone::BRIGHT);
  const uint16_t black = BootPalette::color(BootTone::BLACK);

  for (uint8_t line = 0; line < LOG_LINES; line++)
  {
    const uint32_t start = LOG_START + line * LOG_LINE_MS;
    const int16_t y = LOG_Y + line * LOG_SPACING;
    const char* text = LOG_TEXT[line];
    const uint8_t length = strlen(text);
    const uint8_t visible = typedChars(elapsed, start, LOG_CHAR_MS, length);

    while (logChars[line] < visible)
    {
      display->drawChar(
        LOG_X + logChars[line] * CHAR_WIDTH,
        y,
        text[logChars[line]],
        textColor,
        black,
        1
      );

      logChars[line]++;
    }

    const bool lastLine = line == LOG_LINES - 1;

    if (lastLine || logOkShown[line] || elapsed < start + LOG_OK_DELAY)
    {
      continue;
    }

    display->drawChar(LOG_X + LOG_OK_COLUMN * CHAR_WIDTH, y, 'O', okColor, black, 1);
    display->drawChar(LOG_X + (LOG_OK_COLUMN + 1) * CHAR_WIDTH, y, 'K', okColor, black, 1);
    logOkShown[line] = true;
  }

  // "> LOADING" + 0..3 dots.
  const uint8_t last = LOG_LINES - 1;
  const uint8_t lastLength = strlen(LOG_TEXT[last]);

  if (logChars[last] < lastLength)
  {
    return;
  }

  const uint32_t typedAt = LOG_START + last * LOG_LINE_MS + lastLength * LOG_CHAR_MS;
  const uint8_t dots = 1 + ((elapsed - typedAt) / LOADING_DOT_MS) % 3;

  if (dots == loadingDots)
  {
    return;
  }

  for (uint8_t dot = 0; dot < 3; dot++)
  {
    display->drawChar(
      LOG_X + (lastLength + dot) * CHAR_WIDTH,
      LOG_Y + last * LOG_SPACING,
      dot < dots ? '.' : ' ',
      textColor,
      black,
      1
    );
  }

  loadingDots = dots;
}

void RetroBoot::drawProgress(uint32_t elapsed)
{
  if (elapsed < BAR_FRAME_START)
  {
    return;
  }

  if (!barFrameShown)
  {
    const uint16_t color = BootPalette::color(BootTone::GREEN);

    for (int16_t border = 0; border < BAR_BORDER; border++)
    {
      display->drawRect(
        BAR_X + border,
        BAR_Y + border,
        BAR_WIDTH - 2 * border,
        BAR_HEIGHT - 2 * border,
        color
      );
    }

    barFrameShown = true;
  }

  const float progress = progressBetween(elapsed, PROGRESS_START, PROGRESS_END);
  const uint8_t segments = static_cast<uint8_t>(progress * BAR_SEGMENTS + 0.5f);

  // A short white flash when the bar is full, then back to green.
  uint8_t tone = BootTone::GREEN;

  if (elapsed >= PROGRESS_END && elapsed < FLASH_END)
  {
    tone = BootTone::WHITE;
  }

  const int16_t segmentX = BAR_X + BAR_BORDER + BAR_PADDING;
  const int16_t segmentY = BAR_Y + BAR_BORDER + BAR_PADDING;
  const int16_t segmentHeight = BAR_HEIGHT - 2 * (BAR_BORDER + BAR_PADDING);

  const uint8_t firstToDraw = (tone == segmentsTone) ? segmentsShown : 0;

  for (uint8_t segment = firstToDraw; segment < segments; segment++)
  {
    display->fillRect(
      segmentX + segment * SEGMENT_PITCH,
      segmentY,
      SEGMENT_WIDTH,
      segmentHeight,
      BootPalette::color(tone)
    );
  }

  segmentsShown = segments;
  segmentsTone = tone;
}

void RetroBoot::drawEmblem(uint32_t elapsed)
{
  if (elapsed < STEM_START)
  {
    return;
  }

  BootArt::EmblemState state;
  state.stem = easeOut(progressBetween(elapsed, STEM_START, STEM_END));
  state.core = easeOut(progressBetween(elapsed, CORE_START, CORE_END));
  state.innerRing = easeOut(progressBetween(elapsed, INNER_START, INNER_END));
  state.outerRing = easeOut(progressBetween(elapsed, OUTER_START, OUTER_END));
  state.sparks = progressBetween(elapsed, SPARKS_START, SPARKS_END);
  state.scanAngle = fmodf((elapsed - OUTER_END) * 0.0042f, 2.0f * 3.14159265f)
    - 3.14159265f;
  state.pulse = 0.5f + 0.5f * sinf(elapsed * 0.006f);
  state.timeMs = elapsed;

  emblemCanvas.clear();
  BootArt::drawEmblem(emblemCanvas, EMBLEM_CX, EMBLEM_CY, state);

  uint16_t palette[BootTone::COUNT];
  BootPalette::build(palette);

  emblemCanvas.present(display, EMBLEM_X, EMBLEM_Y, palette);
}

void RetroBoot::drawTerrain(uint32_t elapsed)
{
  const float reveal = progressBetween(elapsed, TERRAIN_START, TERRAIN_END);
  const float brackets = easeOut(
    progressBetween(elapsed, BRACKETS_START, BRACKETS_END)
  );

  terrainCanvas.clear();

  BootArt::drawTerrain(terrainCanvas, elapsed, easeOut(reveal));

  const int16_t bottom = BRACKET_BOTTOM - TERRAIN_Y;

  BootArt::drawBracket(terrainCanvas, BRACKET_LEFT, bottom, 1, -1, brackets, BootTone::GREEN);
  BootArt::drawBracket(terrainCanvas, BRACKET_RIGHT, bottom, -1, -1, brackets, BootTone::GREEN);

  uint16_t palette[BootTone::COUNT];
  BootPalette::build(palette, 0.35f + 0.65f * reveal);

  // The brackets keep their full color while the terrain fades in.
  palette[BootTone::GREEN] = BootPalette::color(BootTone::GREEN);

  terrainCanvas.present(display, 0, TERRAIN_Y, palette);
}

void RetroBoot::finish()
{
  finished = true;

  emblemCanvas.end();
  terrainCanvas.end();

  if (frameCount == 0)
  {
    return;
  }

  Serial.printf(
    "[boot] %lu frames, avg frame %lu us\n",
    static_cast<unsigned long>(frameCount),
    static_cast<unsigned long>(totalFrameMicros / frameCount)
  );
}
