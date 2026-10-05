#pragma once

#include <Arduino.h>
#include <WiFi.h>

enum class WifiPmfMode : uint8_t
{
  PMF_UNKNOWN = 0,
  PMF_DISABLED,
  PMF_OPTIONAL,
  PMF_REQUIRED
};

struct WifiNetwork
{
  String ssid;
  String bssid;

  int32_t rssi;
  uint8_t channel;

  wifi_auth_mode_t security;

  WifiPmfMode pmf =
    WifiPmfMode::PMF_UNKNOWN;
};
