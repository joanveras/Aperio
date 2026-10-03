#pragma once

#include <cstdint>

enum class WifiManagementEventType : uint8_t
{
  NONE,
  DEAUTHENTICATION,
  DISASSOCIATION
};

struct WifiManagementEvent
{
  WifiManagementEventType type =
    WifiManagementEventType::NONE;

  uint16_t reasonCode = 0;

  int8_t rssi = 0;

  uint8_t receivedChannel = 0;

  uint8_t networkChannel = 0;

  bool networkChannelKnown = false;

  uint8_t source[6] = {};
  uint8_t destination[6] = {};
  uint8_t bssid[6] = {};

  bool valid = false;
};
