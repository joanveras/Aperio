#include "ui/CarouselMenu.hpp"
#include "ui/UiStyle.hpp"
#include "Config.hpp"

#include <math.h>
#include <string.h>

namespace
{
  constexpr float PI_F = 3.14159265f;

  // ---- Layout (320 x 240) ------------------------------------------------

  constexpr int16_t TITLE_X = 14;
  constexpr int16_t TITLE_Y = 8;
  constexpr int16_t SUBTITLE_Y = 26;
  constexpr int16_t UNDERLINE_Y = 27;
  constexpr int16_t VERSION_Y = 10;
  constexpr int16_t MARGIN = 10;

  constexpr int16_t CONTENT_TOP = 36;
  constexpr int16_t CONTENT_BOTTOM = 198;

  constexpr int16_t CENTER_X = 160;
  constexpr int16_t CENTER_Y = 110;

  constexpr int16_t ARC_RADIUS = 104;
  constexpr float ARC_SPAN = 20.0f;       // degrees, each side of horizontal
  constexpr int16_t DOT_ARC_RADIUS = 95;
  constexpr float DOT_ARC_SPAN = 44.0f;

  constexpr int16_t SPOKE_START = 20;     // distance from the centre
  constexpr int16_t SPOKE_END = 90;

  // The selected name sits on the centre line, its neighbours above and
  // below, each separated by a small arrow.
  constexpr int16_t SELECTED_Y = CENTER_Y - 7;
  constexpr int16_t UP_ARROW_Y = 89;
  constexpr int16_t DOWN_ARROW_Y = 128;
  constexpr int16_t PREVIOUS_Y = 64;
  constexpr int16_t NEXT_Y = 142;

  constexpr uint8_t LABEL_SIZE = 2;
  constexpr int16_t LABEL_HEIGHT = 7 * LABEL_SIZE;

  // ---- Colors --------------------------------------------------------------

  constexpr uint16_t RING = UiColor::ACCENT;
  constexpr uint16_t RING_SOFT = UiColor::rgb(40, 150, 76);
  constexpr uint16_t DECOR_DIM = UiColor::rgb(30, 110, 56);
  constexpr uint16_t NEIGHBOR = UiColor::rgb(70, 124, 84);
  constexpr uint16_t NEIGHBOR_BLUR = UiColor::rgb(14, 44, 24);

  // ---- Helpers ---------------------------------------------------------------

  struct Box
  {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
  };

  // The labels' boxes (with a margin); decoration never draws inside them.
  Box labelBoxes[3];

  int16_t textWidth(Adafruit_ILI9341* display, const char* text, uint8_t size)
  {
    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;

    display->setTextSize(size);
    display->getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

    return width;
  }

  bool covered(int16_t x, int16_t y)
  {
    for (const Box& box : labelBoxes)
    {
      if (x >= box.x && x < box.x + box.w && y >= box.y && y < box.y + box.h)
      {
        return true;
      }
    }

    return false;
  }

  void plot(Adafruit_ILI9341* display, int16_t x, int16_t y, uint16_t color)
  {
    if (y < CONTENT_TOP || y > CONTENT_BOTTOM || covered(x, y))
    {
      return;
    }

    display->drawPixel(x, y, color);
  }

  // Angles in degrees, measured clockwise from the top.
  void polar(float radius, float degrees, int16_t& x, int16_t& y)
  {
    const float radians = degrees * PI_F / 180.0f;

    x = static_cast<int16_t>(lroundf(CENTER_X + radius * sinf(radians)));
    y = static_cast<int16_t>(lroundf(CENTER_Y - radius * cosf(radians)));
  }

  void arc(
    Adafruit_ILI9341* display,
    float radius,
    float from,
    float to,
    uint8_t thickness,
    uint16_t color
  )
  {
    const float step = 40.0f / radius; // ~0.7 px along the arc

    for (float angle = from; angle <= to; angle += step)
    {
      for (uint8_t layer = 0; layer < thickness; layer++)
      {
        int16_t x;
        int16_t y;
        polar(radius - layer, angle, x, y);
        plot(display, x, y, color);
      }
    }
  }

  void dottedArc(
    Adafruit_ILI9341* display,
    float radius,
    float from,
    float to,
    float spacing,
    uint16_t color
  )
  {
    for (float angle = from; angle <= to; angle += spacing)
    {
      int16_t x;
      int16_t y;
      polar(radius, angle, x, y);
      plot(display, x, y, color);
    }
  }

  void drawDecoration(Adafruit_ILI9341* display)
  {
    // Spokes out to the side arcs; they stop short of the selected name.
    for (int16_t offset = SPOKE_START; offset < SPOKE_END; offset++)
    {
      plot(display, CENTER_X - offset, CENTER_Y, RING_SOFT);
      plot(display, CENTER_X + offset, CENTER_Y, RING_SOFT);
    }

    // Big side arcs, solid and dotted.
    arc(display, ARC_RADIUS, 90.0f - ARC_SPAN, 90.0f + ARC_SPAN, 2, RING);
    arc(display, ARC_RADIUS, -90.0f - ARC_SPAN, -90.0f + ARC_SPAN, 2, RING);
    dottedArc(display, DOT_ARC_RADIUS, 90.0f - DOT_ARC_SPAN, 90.0f + DOT_ARC_SPAN, 3.5f, DECOR_DIM);
    dottedArc(display, DOT_ARC_RADIUS, -90.0f - DOT_ARC_SPAN, -90.0f + DOT_ARC_SPAN, 3.5f, DECOR_DIM);
  }

  // Small chevron pointing up (direction -1) or down (+1).
  void drawArrow(Adafruit_ILI9341* display, int16_t tipY, int8_t direction)
  {
    for (int16_t row = 0; row < 4; row++)
    {
      const int16_t y = tipY + (direction < 0 ? row : -row);
      display->drawFastHLine(CENTER_X - row - 1, y, 2, RING);
      display->drawFastHLine(CENTER_X + row - 1, y, 2, RING);
    }
  }

  Box labelBox(Adafruit_ILI9341* display, const char* label, int16_t y)
  {
    const int16_t width = textWidth(display, label, LABEL_SIZE) + 1;
    return {
      static_cast<int16_t>(CENTER_X - width / 2 - 6),
      static_cast<int16_t>(y - 3),
      static_cast<int16_t>(width + 12),
      static_cast<int16_t>(LABEL_HEIGHT + 6)
    };
  }

  // A dimmed label with a soft halo, so it reads as out of focus.
  void drawNeighbor(Adafruit_ILI9341* display, const char* label, int16_t y)
  {
    const int16_t x = CENTER_X - textWidth(display, label, LABEL_SIZE) / 2;

    display->setTextSize(LABEL_SIZE);
    display->setTextColor(NEIGHBOR_BLUR);

    const int8_t offsets[][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    for (const auto& offset : offsets)
    {
      display->setCursor(x + offset[0], y + offset[1]);
      display->print(label);
    }

    display->setTextColor(NEIGHBOR);
    display->setCursor(x, y);
    display->print(label);
  }

  // The selected label: bright and bold (drawn twice, 1 px apart).
  void drawSelected(Adafruit_ILI9341* display, const char* label, int16_t y)
  {
    const int16_t x = CENTER_X - textWidth(display, label, LABEL_SIZE) / 2;

    display->setTextSize(LABEL_SIZE);
    display->setTextColor(UiColor::TEXT);

    display->setCursor(x, y);
    display->print(label);
    display->setCursor(x + 1, y);
    display->print(label);
  }
}

void CarouselMenu::drawFrame(
  Adafruit_ILI9341* display,
  const char* title,
  const char* subtitle
)
{
  display->fillScreen(UiColor::BACKGROUND);
  display->setTextWrap(false);

  // Title, bold.
  display->setTextSize(2);
  display->setTextColor(UiColor::TEXT);
  display->setCursor(TITLE_X, TITLE_Y);
  display->print(title);
  display->setCursor(TITLE_X + 1, TITLE_Y);
  display->print(title);

  const int16_t titleWidth = textWidth(display, title, 2) + 1;

  if (subtitle != nullptr)
  {
    // Centred under the title.
    const int16_t subtitleWidth = textWidth(display, subtitle, 1);

    display->setTextColor(UiColor::ACCENT_SOFT);
    display->setCursor(TITLE_X + (titleWidth - subtitleWidth) / 2, SUBTITLE_Y);
    display->print(subtitle);
  }
  else
  {
    display->drawFastHLine(TITLE_X, UNDERLINE_Y, titleWidth, UiColor::ACCENT_SOFT);
  }

  display->setTextSize(1);
  display->setTextColor(UiColor::ACCENT_SOFT);
  display->setCursor(
    display->width() - MARGIN - textWidth(display, APERIO_VERSION, 1),
    VERSION_Y
  );
  display->print(APERIO_VERSION);

  UiStyle::drawMenuFooter(display);
}

void CarouselMenu::drawItems(
  Adafruit_ILI9341* display,
  const MenuItem* items,
  int count,
  int selected
)
{
  if (count <= 0)
  {
    return;
  }

  display->fillRect(
    0,
    CONTENT_TOP,
    display->width(),
    CONTENT_BOTTOM - CONTENT_TOP + 1,
    UiColor::BACKGROUND
  );

  const char* previous = items[(selected + count - 1) % count].label;
  const char* current = items[selected].label;
  const char* next = items[(selected + 1) % count].label;

  labelBoxes[0] = labelBox(display, previous, PREVIOUS_Y);
  labelBoxes[1] = labelBox(display, current, SELECTED_Y);
  labelBoxes[2] = labelBox(display, next, NEXT_Y);

  drawDecoration(display);

  drawArrow(display, UP_ARROW_Y, -1);
  drawArrow(display, DOWN_ARROW_Y + 3, 1);

  drawNeighbor(display, previous, PREVIOUS_Y);
  drawSelected(display, current, SELECTED_Y);
  drawNeighbor(display, next, NEXT_Y);
}
