#pragma once

#include <Arduino.h>
#include <WiFi.h>

struct WifiNetwork
{
  String ssid;
  String bssid;

  int32_t rssi;
  uint8_t channel;

  wifi_auth_mode_t security;
};
