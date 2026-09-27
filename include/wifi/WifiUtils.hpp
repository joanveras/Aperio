#pragma once

#include <WiFi.h>

inline const char* wifiAuthModeToString(wifi_auth_mode_t mode)
{
  switch (mode)
  {
    case WIFI_AUTH_OPEN:
      return "OPEN";

    case WIFI_AUTH_WEP:
      return "WEP";

    case WIFI_AUTH_WPA_PSK:
      return "WPA";

    case WIFI_AUTH_WPA2_PSK:
      return "WPA2";

    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA/WPA2";

    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2-ENT";

    case WIFI_AUTH_WPA3_PSK:
      return "WPA3";

    case WIFI_AUTH_WPA2_WPA3_PSK:
      return "WPA2/WPA3";

    default:
      return "UNKNOWN";
  }
}
