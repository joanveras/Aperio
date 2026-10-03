#pragma once

#include <Arduino.h>
#include <esp_wifi.h>

#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>

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

  size_t getManagementEventCount() const;

  bool getManagementEvent(
    size_t index,
    WifiManagementEvent& event
  ) const;

  void clearManagementEvents();

  void clearKnownApChannels();

  bool rememberApChannel(
    const uint8_t* bssid,
    uint8_t channel
  );

  bool rememberApChannel(
    const String& bssid,
    uint8_t channel
  );

private:
  enum class ManagementSubtype : uint8_t
  {
    ProbeRequest     = 0x04,
    ProbeResponse    = 0x05,
    Beacon           = 0x08,
    Disassociation   = 0x0A,
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

  ManagementSubtype classifyManagementSubtype(
    const uint8_t* payload,
    uint16_t length
  ) const;

  void handleManagementFrame(
    ManagementSubtype subtype,
    const uint8_t* payload,
    uint16_t length,
    int8_t rssi,
    uint8_t channel
  );

  void addManagementEvent(
    const WifiManagementEvent& event
  );

  struct KnownApChannel
  {
    uint8_t bssid[6] = {};
    uint8_t channel = 0;
    bool valid = false;
  };

  bool findKnownApChannel(
    const uint8_t* bssid,
    uint8_t& channel
  ) const;

  bool parseMacAddress(
    const String& text,
    uint8_t* mac
  ) const;

  bool macEquals(
    const uint8_t* first,
    const uint8_t* second
  ) const;

  static constexpr size_t KNOWN_AP_CACHE_SIZE = 40;

  KnownApChannel
    knownApChannels[
      KNOWN_AP_CACHE_SIZE
    ];

  size_t knownApCount = 0;

  mutable portMUX_TYPE knownApMux =
    portMUX_INITIALIZER_UNLOCKED;

  bool initialized = false;

  // Guards running/paused, read from the promiscuous callback
  // (handlePacket) and written from the loop/UI side (start, stop,
  // pause, resume, isRunning, isPaused).
  mutable portMUX_TYPE stateMux =
    portMUX_INITIALIZER_UNLOCKED;

  bool running = false;
  bool paused = false;

  uint8_t currentChannel = 6;

  // Guards the frame counters below, which are written from the
  // Wi-Fi driver's promiscuous callback (a different task) and read
  // from getStats()/resetStats() on the loop/UI side.
  mutable portMUX_TYPE statsMux =
    portMUX_INITIALIZER_UNLOCKED;

  volatile uint32_t totalFrames = 0;

  volatile uint32_t managementFrames = 0;
  volatile uint32_t controlFrames = 0;
  volatile uint32_t dataFrames = 0;
  volatile uint32_t miscFrames = 0;

  volatile uint32_t beaconFrames = 0;
  volatile uint32_t probeRequestFrames = 0;
  volatile uint32_t probeResponseFrames = 0;

  volatile uint32_t deauthFrames = 0;
  volatile uint32_t disassociationFrames = 0;

  WifiManagementEvent lastManagementEvent;

  static constexpr size_t MANAGEMENT_EVENT_HISTORY_SIZE = 16;

  WifiManagementEvent
    managementEventHistory[
      MANAGEMENT_EVENT_HISTORY_SIZE
    ];

  size_t managementEventCount = 0;
  size_t nextManagementEventIndex = 0;

  mutable portMUX_TYPE managementEventMux =
    portMUX_INITIALIZER_UNLOCKED;

  uint32_t framesPerSecond = 0;
  uint32_t previousFrameCount = 0;
  uint32_t lastRateUpdate = 0;

  static constexpr uint8_t MIN_CHANNEL = 1;
  static constexpr uint8_t MAX_CHANNEL = 11;
};
