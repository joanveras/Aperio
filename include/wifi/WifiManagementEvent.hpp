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

  // Channel the radio was tuned to when this frame was captured.
  uint8_t receivedChannel = 0;

  // The BSSID's real channel from the known-AP cache, which can
  // differ from receivedChannel if captured off-channel.
  uint8_t networkChannel = 0;

  bool networkChannelKnown = false;

  uint8_t source[6] = {};
  uint8_t destination[6] = {};
  uint8_t bssid[6] = {};

  bool valid = false;
};
