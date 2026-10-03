#pragma once

#include <Arduino.h>

#include "WifiManagementEvent.hpp"

struct WifiMonitorStats
{
  uint32_t totalFrames = 0;
  uint32_t framesPerSecond = 0;

  uint32_t managementFrames = 0;
  uint32_t controlFrames = 0;
  uint32_t dataFrames = 0;
  uint32_t miscFrames = 0;

  uint32_t beaconFrames = 0;
  uint32_t probeRequestFrames = 0;
  uint32_t probeResponseFrames = 0;

  uint32_t deauthFrames = 0;
  uint32_t disassociationFrames = 0;

  WifiManagementEvent lastManagementEvent;

  uint8_t channel = 1;
};
