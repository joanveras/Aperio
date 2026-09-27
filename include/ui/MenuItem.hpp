#pragma once

enum class ScreenId {
  MAIN_MENU,

  WIFI_MENU,
  WIFI_SCAN,
  WIFI_NETWORKS,
  WIFI_CHANNELS,
  WIFI_MONITOR,
  WIFI_NETWORK_DETAILS,

  BLUETOOTH_MENU,
  SYSTEM_INFO,
  ABOUT
};

struct MenuItem {
  const char* label;
  ScreenId destination;
};
