#include "../../../include/ui/screens/MainMenuScreen.hpp"
#include "ui/UiStyle.hpp"

namespace
{
  constexpr MenuItem mainMenuItems[] = {
    {"Wi-Fi",     ScreenId::WIFI_MENU},
    {"Bluetooth", ScreenId::BLUETOOTH_MENU},
    {"System",    ScreenId::SYSTEM_INFO},
    {"About",     ScreenId::ABOUT}
  };

  constexpr int MENU_ITEM_COUNT = sizeof(mainMenuItems) / sizeof(mainMenuItems[0]);

  // The mascot (eye + waves) sits in the top-right corner of the header.
  // The title rule stops at RULE_END_X so it does not run under the mascot.
  constexpr int16_t MASCOT_X = 206;
  constexpr int16_t MASCOT_Y = 0;
  constexpr int16_t RULE_END_X = 200;
}

MainMenuScreen::MainMenuScreen(
  Adafruit_ILI9341* displayInstance,
  NavigationCallback navigationCallback
)
  : display(displayInstance),
    navigationCallback(navigationCallback),
    selectedIndex(0),
    firstVisibleItem(0),
    needsRedraw(true),
    mascot(displayInstance, MASCOT_X, MASCOT_Y),
    mascotFrameDue(false)
{
}

void MainMenuScreen::onEnter()
{
  needsRedraw = true;
  mascot.begin(millis());
}

void MainMenuScreen::handleInput(InputEvent event)
{
  if (event == InputEvent::PREVIOUS)
  {
    moveSelection(-1);
    mascot.glance(-1, millis());
  }
  else if (event == InputEvent::NEXT)
  {
    moveSelection(1);
    mascot.glance(1, millis());
  }
  else if (event == InputEvent::SELECT)
  {
    openSelectedItem();
  }
}

void MainMenuScreen::update()
{
  mascotFrameDue = mascot.update(millis());
}

void MainMenuScreen::render()
{
  bool fullRedraw = needsRedraw;

  if (needsRedraw)
  {
    drawHeader();
    drawMenuItems();

    needsRedraw = false;
  }

  // The mascot is drawn last, so a full redraw never erases it, and on its
  // own it only repaints its small corner (no menu flicker).
  if (fullRedraw || mascotFrameDue)
  {
    mascot.present();
    mascotFrameDue = false;
  }
}

void MainMenuScreen::moveSelection(int direction)
{
  selectedIndex += direction;

  if (selectedIndex < 0)
  {
    selectedIndex = MENU_ITEM_COUNT - 1;
  }

  if (selectedIndex >= MENU_ITEM_COUNT)
  {
    selectedIndex = 0;
  }

  needsRedraw = true;
}

void MainMenuScreen::openSelectedItem()
{
  ScreenId destination = mainMenuItems[selectedIndex].destination;

  if (navigationCallback)
  {
    navigationCallback(destination);
  }
}

void MainMenuScreen::drawHeader()
{
  display->fillScreen(UiColor::BACKGROUND);

  UiStyle::drawTitle(display, "APERIO", RULE_END_X);

  // Version sits next to the title; the top-right corner is the mascot's.
  display->setTextSize(1);
  display->setTextColor(UiColor::TEXT_DIM, UiColor::BACKGROUND);
  display->setCursor(92, 18);
  display->print("v0.1");
}

void MainMenuScreen::drawMenuItems()
{
  constexpr int MENU_START_Y = 58;
  constexpr int ITEM_SPACING = 32;

  for (int i = 0; i < MENU_ITEM_COUNT; i++)
  {
    int y = MENU_START_Y + (i * ITEM_SPACING);

    drawMenuItem(
      i,
      y,
      i == selectedIndex
    );
  }

  UiStyle::drawDivider(display, 199);

  // Footer
  display->setTextSize(1);
  display->setTextColor(UiColor::TEXT_MUTED, UiColor::BACKGROUND);

  display->setCursor(12, 216);
  display->print("< PREV");

  display->setCursor(154, 216);
  display->print("OK");

  display->setCursor(272, 216);
  display->print("NEXT >");
}

void MainMenuScreen::drawMenuItem(int index, int y, bool selected)
{
  constexpr int ITEM_X = 12;
  constexpr int ITEM_WIDTH = 296;
  constexpr int ITEM_HEIGHT = 27;
  constexpr int TEXT_X = 20;

  if (selected)
  {
    UiStyle::drawSelection(
      display,
      ITEM_X,
      y - 5,
      ITEM_WIDTH,
      ITEM_HEIGHT
    );

    display->setTextColor(
      UiColor::TEXT,
      UiColor::SELECTION
    );
  }
  else
  {
    display->setTextColor(
      UiColor::TEXT_MUTED,
      UiColor::BACKGROUND
    );
  }

  display->setTextSize(2);
  display->setCursor(TEXT_X, y);

  display->print(mainMenuItems[index].label);
}
