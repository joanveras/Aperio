#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include "WifiNetwork.hpp"

class WifiScanner
{
  public:
    void begin();
    void scan();
    size_t getNetworkCount() const;
    const WifiNetwork& getNetwork(size_t index) const;

  private:
    static constexpr size_t MAX_NETWORKS = 40;
    WifiNetwork networks[MAX_NETWORKS] = {};
    size_t networkCount = 0;
};
