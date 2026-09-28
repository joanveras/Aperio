#pragma once

#include <Arduino.h>
#include <esp_wifi.h>

#include "WifiMonitorStats.hpp"

class WifiMonitor
{
public:
  bool begin();

  bool start(uint8_t channel = 6);
  void stop();

  void pause();
  void resume();

  bool isRunning() const;
  bool isPaused() const;

  bool setChannel(uint8_t channel);
  uint8_t getChannel() const;

  void update();

  WifiMonitorStats getStats() const;
  void resetStats();

  bool nextChannel();
  bool previousChannel();

private:
  enum class ManagementSubtype : uint8_t
  {
    ProbeRequest = 0x04,
    ProbeResponse = 0x05,
    Beacon = 0x08,
    Deauthentication = 0x0C
  };

  static WifiMonitor* activeInstance;

  static void promiscuousCallback(
    void* buffer,
    wifi_promiscuous_pkt_type_t type
  );

  void handlePacket(
    void* buffer,
    wifi_promiscuous_pkt_type_t type
  );

  void handleManagementFrame(
    const uint8_t* payload,
    uint16_t length
  );

  bool initialized = false;
  bool running = false;
  bool paused = false;

  uint8_t currentChannel = 6;

  volatile uint32_t totalFrames = 0;

  volatile uint32_t managementFrames = 0;
  volatile uint32_t controlFrames = 0;
  volatile uint32_t dataFrames = 0;
  volatile uint32_t miscFrames = 0;

  volatile uint32_t beaconFrames = 0;
  volatile uint32_t probeRequestFrames = 0;
  volatile uint32_t probeResponseFrames = 0;
  volatile uint32_t deauthFrames = 0;

  uint32_t framesPerSecond = 0;
  uint32_t previousFrameCount = 0;
  uint32_t lastRateUpdate = 0;

  static constexpr uint8_t MIN_CHANNEL = 1;
  static constexpr uint8_t MAX_CHANNEL = 11;
};
