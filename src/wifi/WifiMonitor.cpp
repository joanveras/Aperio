#include "../../include/wifi/WifiMonitor.hpp"

#include <WiFi.h>

WifiMonitor* WifiMonitor::activeInstance = nullptr;

bool WifiMonitor::begin()
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

  activeInstance = this;

  esp_err_t result =
    esp_wifi_set_promiscuous_rx_cb(
      &WifiMonitor::promiscuousCallback
    );

  if (result != ESP_OK)
  {
    activeInstance = nullptr;
    return false;
  }

  wifi_promiscuous_filter_t filter = {};

  filter.filter_mask =
    WIFI_PROMIS_FILTER_MASK_MGMT |
    WIFI_PROMIS_FILTER_MASK_DATA |
    WIFI_PROMIS_FILTER_MASK_CTRL;

  result =
    esp_wifi_set_promiscuous_filter(
      &filter
    );

  if (result != ESP_OK)
  {
    activeInstance = nullptr;
    return false;
  }

  wifi_promiscuous_filter_t ctrlFilter = {};

  ctrlFilter.filter_mask =
    WIFI_PROMIS_CTRL_FILTER_MASK_ALL;

  result =
    esp_wifi_set_promiscuous_ctrl_filter(
      &ctrlFilter
    );

  if (result != ESP_OK)
  {
    activeInstance = nullptr;
    return false;
  }

  initialized = true;

  return true;
}

bool WifiMonitor::start(uint8_t channel)
{
  if (!initialized && !begin())
  {
    return false;
  }

  if (!setChannel(channel))
  {
    return false;
  }

  resetStats();

  activeInstance = this;

  paused = false;
  running = true;

  esp_err_t result = esp_wifi_set_promiscuous(true);

  if (result != ESP_OK)
  {
    running = false;
    return false;
  }

  lastRateUpdate = millis();

  return true;
}

void WifiMonitor::stop()
{
  if (!initialized)
  {
    return;
  }

  esp_wifi_set_promiscuous(false);

  running = false;
  paused = false;

  framesPerSecond = 0;
}

void WifiMonitor::pause()
{
  if (!running)
  {
    return;
  }

  paused = true;
  framesPerSecond = 0;
}

void WifiMonitor::resume()
{
  if (!running)
  {
    return;
  }

  paused = false;

  previousFrameCount = totalFrames;
  lastRateUpdate = millis();
  framesPerSecond = 0;
}

bool WifiMonitor::isRunning() const
{
  return running;
}

bool WifiMonitor::isPaused() const
{
  return paused;
}

bool WifiMonitor::setChannel(uint8_t channel)
{
  if (channel < MIN_CHANNEL || channel > MAX_CHANNEL)
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

  framesPerSecond = 0;
  previousFrameCount = totalFrames;
  lastRateUpdate = millis();

  return true;
}

uint8_t WifiMonitor::getChannel() const
{
  return currentChannel;
}

void WifiMonitor::update()
{
  if (!running || paused)
  {
    return;
  }

  uint32_t now = millis();
  uint32_t elapsed = now - lastRateUpdate;

  if (elapsed < 1000)
  {
    return;
  }

  uint32_t currentFrameCount = totalFrames;

  uint32_t framesSinceLastUpdate =
    currentFrameCount - previousFrameCount;

  framesPerSecond =
    (framesSinceLastUpdate * 1000UL) / elapsed;

  previousFrameCount = currentFrameCount;
  lastRateUpdate = now;
}

WifiMonitorStats WifiMonitor::getStats() const
{
  WifiMonitorStats stats;

  stats.totalFrames = totalFrames;
  stats.framesPerSecond = framesPerSecond;

  stats.managementFrames = managementFrames;
  stats.controlFrames = controlFrames;
  stats.dataFrames = dataFrames;
  stats.miscFrames = miscFrames;

  stats.beaconFrames = beaconFrames;
  stats.probeRequestFrames = probeRequestFrames;
  stats.probeResponseFrames = probeResponseFrames;
  stats.deauthFrames = deauthFrames;

  stats.channel = currentChannel;

  return stats;
}

void WifiMonitor::resetStats()
{
  totalFrames = 0;

  managementFrames = 0;
  controlFrames = 0;
  dataFrames = 0;
  miscFrames = 0;

  beaconFrames = 0;
  probeRequestFrames = 0;
  probeResponseFrames = 0;
  deauthFrames = 0;

  framesPerSecond = 0;
  previousFrameCount = 0;
  lastRateUpdate = millis();
}

void WifiMonitor::promiscuousCallback(
  void* buffer,
  wifi_promiscuous_pkt_type_t type
)
{
  if (activeInstance == nullptr)
  {
    return;
  }

  activeInstance->handlePacket(buffer, type);
}

void WifiMonitor::handlePacket(
  void* buffer,
  wifi_promiscuous_pkt_type_t type
)
{
  if (!running || paused || buffer == nullptr)
  {
    return;
  }

  totalFrames++;

  switch (type)
  {
    case WIFI_PKT_MGMT:
    {
      managementFrames++;

      const wifi_promiscuous_pkt_t* packet =
        static_cast<const wifi_promiscuous_pkt_t*>(buffer);

      const uint8_t* payload = packet->payload;

      uint16_t length =
        packet->rx_ctrl.sig_len;

      handleManagementFrame(payload, length);

      break;
    }

    case WIFI_PKT_CTRL:
      controlFrames++;
      break;

    case WIFI_PKT_DATA:
      dataFrames++;
      break;

    case WIFI_PKT_MISC:
      miscFrames++;
      break;

    default:
      break;
  }
}

void WifiMonitor::handleManagementFrame(
  const uint8_t* payload,
  uint16_t length
)
{
  if (payload == nullptr || length < 2)
  {
    return;
  }

  uint8_t frameControl = payload[0];

  ManagementSubtype subtype =
    static_cast<ManagementSubtype>((frameControl >> 4) & 0x0F);

  switch (subtype)
  {
    case ManagementSubtype::ProbeRequest:
      probeRequestFrames++;
      break;

    case ManagementSubtype::ProbeResponse:
      probeResponseFrames++;
      break;

    case ManagementSubtype::Beacon:
      beaconFrames++;
      break;

    case ManagementSubtype::Deauthentication:
      deauthFrames++;
      break;

    default:
      break;
  }
}

bool WifiMonitor::nextChannel()
{
  uint8_t next =
    currentChannel >= MAX_CHANNEL ?
      MIN_CHANNEL : currentChannel + 1;

  return setChannel(next);
}

bool WifiMonitor::previousChannel()
{
  uint8_t previous =
    currentChannel <= MIN_CHANNEL ?
      MAX_CHANNEL : currentChannel - 1;

  return setChannel(previous);
}
