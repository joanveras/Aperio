#pragma once

#include <Arduino.h>

struct WifiChannelStats
{
  uint8_t channel = 1;

  uint16_t networkCount = 0;

  int32_t strongestRssi = -127;
};
