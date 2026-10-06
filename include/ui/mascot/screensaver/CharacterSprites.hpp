#pragma once

#include "../MascotCanvas.hpp"
#include "../MascotPalette.hpp"

// The little pixel character who stars in the screensaver's Cena 01.
//
// Sprites are kept as a grid of characters so they are easy to read and
// tweak by hand: each character maps to a palette index, and a space is
// transparent (the black background shows through). The character is a
// chibi -- big head, small body, a mop of hair, a hoodie -- holding the
// glowing mini-Aperio, matching the storyboard. He faces the viewer, since
// he walks in and then sits facing us.
namespace CharacterSprites
{
  // One frame of the character, as rows of legend characters.
  struct Sprite
  {
    int16_t width;
    int16_t height;
    const char* const* rows;
  };

  // Legend -> palette index (everything stays in the mascot's cyan/blue
  // neon family).
  //   'h' hair       'f' face        'e' eye
  //   '#' hoodie      '=' shadow      'o' highlight / rim-light
  //   'a' device frame                'A' device screen (bright cyan)
  inline uint8_t colorOf(char c)
  {
    switch (c)
    {
      case 'h': return MascotColor::IRIS_DARK;   // dark-blue hair
      case 'f': return MascotColor::IRIS_LIGHT;  // light face
      case 'e': return MascotColor::SCLERA;      // near-black eyes
      case '#': return MascotColor::IRIS_MID;    // hoodie
      case '=': return MascotColor::SCLERA;      // deep shadow
      case 'o': return MascotColor::WHITE;       // rim-light / highlight
      case 'a': return MascotColor::WHITE;       // device frame
      case 'A': return MascotColor::SIGNAL_LAST; // device screen
      default:  return MascotColor::BLACK;
    }
  }

  // Draws the sprite with its top-left corner at (x, y). Spaces are skipped,
  // and a `dissolve` in 0..1 randomly drops that fraction of the pixels for a
  // "breaking apart" look (0 = fully solid).
  inline void draw(
    MascotCanvas& canvas,
    const Sprite& sprite,
    int16_t x,
    int16_t y,
    float dissolve = 0.0f
  )
  {
    for (int16_t row = 0; row < sprite.height; ++row)
    {
      const char* line = sprite.rows[row];

      for (int16_t col = 0; col < sprite.width && line[col] != '\0'; ++col)
      {
        char c = line[col];
        if (c == ' ')
        {
          continue;
        }

        if (dissolve > 0.0f)
        {
          // Cheap, stable per-pixel hash: the same pixels vanish each frame
          // as the dissolve grows, instead of flickering.
          uint16_t h = static_cast<uint16_t>((x + col) * 73 + (y + row) * 151);
          if ((h & 0x0F) < dissolve * 16.0f)
          {
            continue;
          }
        }

        canvas.setPixel(x + col, y + row, colorOf(c));
      }
    }
  }

  namespace detail
  {
    // Head + torso are shared by every pose; only the legs (last rows) change.
    // 18 wide x 25 tall.
    inline constexpr const char* const STAND_ROWS[] = {
      "      hhhhhh      ",
      "    hhhhhhhhhh    ",
      "   hhhhhhhhhhhh   ",
      "   hhhhhhhhhhhh   ",
      "   hhhffffffhhh   ",
      "   hhffffffffhh   ",
      "   hoffffffffoh   ",
      "   hffeffffeffh   ",
      "   hffffffffffh   ",
      "   hffffeeffffh   ",
      "   hhffffffffhh   ",
      "    hhffffffhh    ",
      "     hh====hh     ",
      "     o######o     ",
      "    o########o    ",
      "   o###    ###o   ",
      "   ###      ###   ",
      "   ###      ###   ",
      "   o###    ###o   ",
      "    o########o    ",
      "    ##########    ",
      "    ###=  =###    ",
      "    ###    ###    ",
      "    ==      ==    ",
      "    oo      oo    "
    };

    inline constexpr const char* const WALK_A_ROWS[] = {
      "      hhhhhh      ",
      "    hhhhhhhhhh    ",
      "   hhhhhhhhhhhh   ",
      "   hhhhhhhhhhhh   ",
      "   hhhffffffhhh   ",
      "   hhffffffffhh   ",
      "   hoffffffffoh   ",
      "   hffeffffeffh   ",
      "   hffffffffffh   ",
      "   hffffeeffffh   ",
      "   hhffffffffhh   ",
      "    hhffffffhh    ",
      "     hh====hh     ",
      "     o######o     ",
      "    o########o    ",
      "   o###    ###o   ",
      "   ###      ###   ",
      "   ###      ###   ",
      "   o###    ###o   ",
      "    o########o    ",
      "    ##########    ",
      "   ###=    =##    ",
      "   ###      ##    ",
      "   ==        =    ",
      "   oo        o    "
    };

    inline constexpr const char* const WALK_B_ROWS[] = {
      "      hhhhhh      ",
      "    hhhhhhhhhh    ",
      "   hhhhhhhhhhhh   ",
      "   hhhhhhhhhhhh   ",
      "   hhhffffffhhh   ",
      "   hhffffffffhh   ",
      "   hoffffffffoh   ",
      "   hffeffffeffh   ",
      "   hffffffffffh   ",
      "   hffffeeffffh   ",
      "   hhffffffffhh   ",
      "    hhffffffhh    ",
      "     hh====hh     ",
      "     o######o     ",
      "    o########o    ",
      "   o###    ###o   ",
      "   ###      ###   ",
      "   ###      ###   ",
      "   o###    ###o   ",
      "    o########o    ",
      "    ##########    ",
      "    ##=    =###   ",
      "    ##      ###   ",
      "    =        ==   ",
      "    o        oo   "
    };

    // The mini-Aperio he holds in front: a bright cyan screen in a white
    // frame. Drawn separately from the body so it can stay on screen later,
    // when the character dissolves (Cena 01, fase 12).
    inline constexpr const char* const DEVICE_ROWS[] = {
      "aaaa",
      "aAAa",
      "aAAa",
      "aAAa",
      "aAAa",
      "aaaa"
    };
  }

  inline constexpr Sprite STAND  { 18, 25, detail::STAND_ROWS };
  inline constexpr Sprite WALK_A { 18, 25, detail::WALK_A_ROWS };
  inline constexpr Sprite WALK_B { 18, 25, detail::WALK_B_ROWS };
  inline constexpr Sprite DEVICE { 4, 6, detail::DEVICE_ROWS };

  // Where the device sits relative to the character's top-left corner
  // (held in front, in both hands, at chest height).
  constexpr int16_t DEVICE_OFFSET_X = 7;
  constexpr int16_t DEVICE_OFFSET_Y = 14;
}
