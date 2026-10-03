#include "../../include/wifi/WifiMonitor.hpp"

#include <WiFi.h>
#include <cstring>

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

  if (!initialized)
  {
    if (!begin())
    {
      return false;
    }
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

  stats.disassociationFrames = disassociationFrames;

  portENTER_CRITICAL(&managementEventMux);
  stats.lastManagementEvent = lastManagementEvent;
  portEXIT_CRITICAL(&managementEventMux);

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
  disassociationFrames = 0;

  clearManagementEvents();

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

      handleManagementFrame(
        packet->payload,
        packet->rx_ctrl.sig_len,
        packet->rx_ctrl.rssi,
        packet->rx_ctrl.channel
      );

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
  uint16_t length,
  int8_t rssi,
  uint8_t channel
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

    case ManagementSubtype::Disassociation:
    {
      disassociationFrames++;

      if (length < 26)
      {
        break;
      }

      WifiManagementEvent event;

      event.type =
        WifiManagementEventType::DISASSOCIATION;

      event.reasonCode =
        static_cast<uint16_t>(payload[24]) |
        (
          static_cast<uint16_t>(payload[25])
          << 8
        );

      event.rssi = rssi;
      event.receivedChannel = channel;

      std::memcpy(
        event.destination,
        payload + 4,
        6
      );

      std::memcpy(
        event.source,
        payload + 10,
        6
      );

      std::memcpy(
        event.bssid,
        payload + 16,
        6
      );

      uint8_t knownChannel = 0;

      if (
        findKnownApChannel(
          event.bssid,
          knownChannel
        )
      )
      {
        event.networkChannel =
          knownChannel;

        event.networkChannelKnown =
          true;
      }

      event.valid = true;

      addManagementEvent(event);

      break;
    }

    case ManagementSubtype::Deauthentication:
    {
      deauthFrames++;

      if (length < 26)
      {
        break;
      }

      WifiManagementEvent event;

      event.type =
        WifiManagementEventType::DEAUTHENTICATION;

      event.reasonCode =
        static_cast<uint16_t>(payload[24]) |
        (
          static_cast<uint16_t>(payload[25])
          << 8
        );

      event.rssi = rssi;
      event.receivedChannel = channel;

      std::memcpy(
        event.destination,
        payload + 4,
        6
      );

      std::memcpy(
        event.source,
        payload + 10,
        6
      );

      std::memcpy(
        event.bssid,
        payload + 16,
        6
      );

      uint8_t knownChannel = 0;

      if (
        findKnownApChannel(
          event.bssid,
          knownChannel
        )
      )
      {
        event.networkChannel =
          knownChannel;

        event.networkChannelKnown =
          true;
      }

      event.valid = true;

      addManagementEvent(event);

      break;
    }

    default:
      break;
  }
}

void WifiMonitor::addManagementEvent(const WifiManagementEvent& event)
{
  if (!event.valid)
  {
    return;
  }

  portENTER_CRITICAL(
    &managementEventMux
  );

  managementEventHistory[nextManagementEventIndex] = event;

  nextManagementEventIndex =
    (nextManagementEventIndex + 1) % MANAGEMENT_EVENT_HISTORY_SIZE;

  if (managementEventCount < MANAGEMENT_EVENT_HISTORY_SIZE)
  {
    managementEventCount++;
  }

  lastManagementEvent = event;

  portEXIT_CRITICAL(
    &managementEventMux
  );
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

size_t WifiMonitor::getManagementEventCount() const
{
  portENTER_CRITICAL(
    &managementEventMux
  );

  size_t count = managementEventCount;

  portEXIT_CRITICAL(
    &managementEventMux
  );

  return count;
}

bool WifiMonitor::getManagementEvent(
  size_t index,
  WifiManagementEvent& event
) const
{
  portENTER_CRITICAL(
    &managementEventMux
  );

  if (index >= managementEventCount)
  {
    portEXIT_CRITICAL(
      &managementEventMux
    );

    return false;
  }

  size_t historyIndex =
    (
      nextManagementEventIndex +
      MANAGEMENT_EVENT_HISTORY_SIZE -
      1 -
      index
    ) %
    MANAGEMENT_EVENT_HISTORY_SIZE;

  event = managementEventHistory[historyIndex];

  portEXIT_CRITICAL(
    &managementEventMux
  );

  return true;
}

void WifiMonitor::clearManagementEvents()
{
  portENTER_CRITICAL(
    &managementEventMux
  );

  managementEventCount = 0;
  nextManagementEventIndex = 0;

  lastManagementEvent = WifiManagementEvent{};

  portEXIT_CRITICAL(
    &managementEventMux
  );
}

bool WifiMonitor::macEquals(const uint8_t* first, const uint8_t* second) const
{
  if (first == nullptr || second == nullptr)
  {
    return false;
  }

  for (size_t i = 0; i < 6; i++)
  {
    if (first[i] != second[i])
    {
      return false;
    }
  }

  return true;
}

bool WifiMonitor::rememberApChannel(const uint8_t* bssid, uint8_t channel)
{
  if (
    bssid == nullptr || channel < MIN_CHANNEL || channel > MAX_CHANNEL
  )
  {
    return false;
  }

  portENTER_CRITICAL(
    &knownApMux
  );

  for (size_t i = 0; i < KNOWN_AP_CACHE_SIZE; i++)
  {
    if (
      knownApChannels[i].valid && macEquals(knownApChannels[i].bssid, bssid)
    )
    {
      knownApChannels[i].channel = channel;

      portEXIT_CRITICAL(
        &knownApMux
      );

      return true;
    }
  }

  for (
    size_t i = 0;
    i < KNOWN_AP_CACHE_SIZE;
    i++
  )
  {
    if (!knownApChannels[i].valid)
    {
      for (size_t j = 0; j < 6; j++)
      {
        knownApChannels[i].bssid[j] = bssid[j];
      }

      knownApChannels[i].channel = channel;

      knownApChannels[i].valid = true;

      if (knownApCount < KNOWN_AP_CACHE_SIZE)
      {
        knownApCount++;
      }

      portEXIT_CRITICAL(
        &knownApMux
      );

      return true;
    }
  }

  portEXIT_CRITICAL(
    &knownApMux
  );

  return false;
}

bool WifiMonitor::rememberApChannel(const String& bssid, uint8_t channel)
{
  uint8_t mac[6];

  if (!parseMacAddress(bssid, mac))
  {
    return false;
  }

  return rememberApChannel(mac, channel);
}

bool WifiMonitor::findKnownApChannel(const uint8_t* bssid, uint8_t& channel) const
{
  if (bssid == nullptr)
  {
    return false;
  }

  portENTER_CRITICAL(
    &knownApMux
  );

  for (size_t i = 0; i < KNOWN_AP_CACHE_SIZE; i++)
  {
    if (
      knownApChannels[i].valid && macEquals(knownApChannels[i].bssid, bssid)
    )
    {
      channel = knownApChannels[i].channel;

      portEXIT_CRITICAL(
        &knownApMux
      );

      return true;
    }
  }

  portEXIT_CRITICAL(
    &knownApMux
  );

  return false;
}

bool WifiMonitor::parseMacAddress(const String& text, uint8_t* mac) const
{
  if (mac == nullptr)
  {
    return false;
  }

  unsigned int values[6];

  int parsed = sscanf(
    text.c_str(),
    "%x:%x:%x:%x:%x:%x",
    &values[0],
    &values[1],
    &values[2],
    &values[3],
    &values[4],
    &values[5]
  );

  if (parsed != 6)
  {
    return false;
  }

  for (size_t i = 0; i < 6; i++)
  {
    if (values[i] > 0xFF)
    {
      return false;
    }

    mac[i] = static_cast<uint8_t>(values[i]);
  }

  return true;
}

void WifiMonitor::clearKnownApChannels()
{
  portENTER_CRITICAL(
    &knownApMux
  );

  for (size_t i = 0; i < KNOWN_AP_CACHE_SIZE; i++)
  {
    knownApChannels[i] = KnownApChannel{};
  }

  knownApCount = 0;

  portEXIT_CRITICAL(
    &knownApMux
  );
}
