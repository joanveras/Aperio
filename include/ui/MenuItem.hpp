#pragma once

enum class ScreenId {
  MAIN_MENU,
  WIFI_MENU,
  BLUETOOTH_MENU,
  SYSTEM_INFO,
  ABOUT
};

struct MenuItem {
  const char* label;
  ScreenId destination;
};
