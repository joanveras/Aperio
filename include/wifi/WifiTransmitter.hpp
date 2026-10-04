#pragma once

#include <Arduino.h>

struct WifiTxStats
{
  uint32_t attemptedFrames = 0;
  uint32_t sentFrames = 0;
  uint32_t failedFrames = 0;

  uint8_t channel = 6;
};

class WifiTransmitter
{
public:
  bool begin();

  bool isReady() const;

  bool setChannel(
    uint8_t channel
  );

  uint8_t getChannel() const;

  bool sendProbeRequest(
    const char* ssid = "Luiz Gonzaga"
  );

  bool sendDeauth(
    const uint8_t* targetBssid,
    uint16_t reasonCode
  );

  static bool parseMac(
    const String& mac,
    uint8_t out[6]
  );

  WifiTxStats getStats() const;

  void resetStats();

private:
  static constexpr uint8_t MIN_CHANNEL = 1;
  static constexpr uint8_t MAX_CHANNEL = 11;

  static constexpr uint8_t DEFAULT_CHANNEL = 6;

  static constexpr size_t MAX_FRAME_SIZE = 128;

  bool initialized = false;

  uint8_t currentChannel = DEFAULT_CHANNEL;

  uint16_t sequenceNumber = 0;

  WifiTxStats stats;

  bool sendRawFrame(
    const uint8_t* frame,
    size_t length
  );

  size_t buildProbeRequest(
    uint8_t* buffer,
    size_t bufferSize,
    const char* ssid
  );

  size_t buildDeauth(
    uint8_t* buffer,
    size_t bufferSize,
    const uint8_t* targetBssid,
    uint16_t reasonCode
  );

  bool getSourceMac(
    uint8_t* mac
  ) const;

  bool isValidChannel(
    uint8_t channel
  ) const;

  uint16_t nextSequenceNumber();
};
