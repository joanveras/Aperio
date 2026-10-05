#include "../../include/wifi/WifiMonitor.hpp"
#include "../../include/wifi/WifiManagementUtils.hpp"
#include "../../include/wifi/WifiPmfParser.hpp"

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

  portENTER_CRITICAL(&stateMux);
  paused = false;
  running = true;
  portEXIT_CRITICAL(&stateMux);

  esp_err_t result = esp_wifi_set_promiscuous(true);

  if (result != ESP_OK)
  {
    portENTER_CRITICAL(&stateMux);
    running = false;
    portEXIT_CRITICAL(&stateMux);

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

  portENTER_CRITICAL(&stateMux);
  running = false;
  paused = false;
  portEXIT_CRITICAL(&stateMux);

  framesPerSecond = 0;
}

void WifiMonitor::pause()
{
  portENTER_CRITICAL(&stateMux);

  if (!running)
  {
    portEXIT_CRITICAL(&stateMux);
    return;
  }

  paused = true;

  portEXIT_CRITICAL(&stateMux);

  framesPerSecond = 0;
}

void WifiMonitor::resume()
{
  portENTER_CRITICAL(&stateMux);

  if (!running)
  {
    portEXIT_CRITICAL(&stateMux);
    return;
  }

  paused = false;

  portEXIT_CRITICAL(&stateMux);

  portENTER_CRITICAL(&statsMux);
  previousFrameCount = totalFrames;
  portEXIT_CRITICAL(&statsMux);

  lastRateUpdate = millis();
  framesPerSecond = 0;
}

bool WifiMonitor::isRunning() const
{
  portENTER_CRITICAL(&stateMux);
  bool value = running;
  portEXIT_CRITICAL(&stateMux);

  return value;
}

bool WifiMonitor::isPaused() const
{
  portENTER_CRITICAL(&stateMux);
  bool value = paused;
  portEXIT_CRITICAL(&stateMux);

  return value;
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

  portENTER_CRITICAL(&statsMux);
  previousFrameCount = totalFrames;
  portEXIT_CRITICAL(&statsMux);

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

  portENTER_CRITICAL(&statsMux);
  uint32_t currentFrameCount = totalFrames;
  portEXIT_CRITICAL(&statsMux);

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

  portENTER_CRITICAL(&statsMux);

  stats.totalFrames = totalFrames;
  stats.managementFrames = managementFrames;
  stats.controlFrames = controlFrames;
  stats.dataFrames = dataFrames;
  stats.miscFrames = miscFrames;

  stats.beaconFrames = beaconFrames;
  stats.probeRequestFrames = probeRequestFrames;
  stats.probeResponseFrames = probeResponseFrames;

  stats.deauthFrames = deauthFrames;
  stats.disassociationFrames = disassociationFrames;

  portEXIT_CRITICAL(&statsMux);

  stats.framesPerSecond = framesPerSecond;

  portENTER_CRITICAL(&managementEventMux);
  stats.lastManagementEvent = lastManagementEvent;
  portEXIT_CRITICAL(&managementEventMux);

  stats.channel = currentChannel;

  return stats;
}

void WifiMonitor::resetStats()
{
  portENTER_CRITICAL(&statsMux);

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

  portEXIT_CRITICAL(&statsMux);

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
  if (buffer == nullptr)
  {
    return;
  }

  portENTER_CRITICAL(&stateMux);
  bool isActive = running && !paused;
  portEXIT_CRITICAL(&stateMux);

  if (!isActive)
  {
    return;
  }

  const wifi_promiscuous_pkt_t* packet =
    static_cast<const wifi_promiscuous_pkt_t*>(buffer);

  if (type == WIFI_PKT_MGMT)
  {
    // Classify before taking statsMux: this only reads one byte of
    // the (already-captured, immutable) frame, no shared state.
    ManagementSubtype subtype =
      classifyManagementSubtype(
        packet->payload,
        packet->rx_ctrl.sig_len
      );

    // Single critical section for every counter tied to this one
    // packet, so a concurrent getStats() can never observe total
    // out of sync with management/control/data/misc (or with the
    // management subtype counters).
    portENTER_CRITICAL(&statsMux);

    totalFrames++;
    managementFrames++;

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
        disassociationFrames++;
        break;

      case ManagementSubtype::Deauthentication:
        deauthFrames++;
        break;

      default:
        break;
    }

    portEXIT_CRITICAL(&statsMux);

    // Parsing, the known-AP lookup, and addManagementEvent() all
    // happen outside statsMux.
    handleManagementFrame(
      subtype,
      packet->payload,
      packet->rx_ctrl.sig_len,
      packet->rx_ctrl.rssi,
      packet->rx_ctrl.channel
    );

    return;
  }

  portENTER_CRITICAL(&statsMux);

  totalFrames++;

  switch (type)
  {
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

  portEXIT_CRITICAL(&statsMux);
}

WifiMonitor::ManagementSubtype WifiMonitor::classifyManagementSubtype(
  const uint8_t* payload,
  uint16_t length
) const
{
  constexpr uint8_t UNKNOWN_SUBTYPE = 0xFF;

  if (payload == nullptr || length < 2)
  {
    return static_cast<ManagementSubtype>(UNKNOWN_SUBTYPE);
  }

  return static_cast<ManagementSubtype>((payload[0] >> 4) & 0x0F);
}

void WifiMonitor::handleManagementFrame(
  ManagementSubtype subtype,
  const uint8_t* payload,
  uint16_t length,
  int8_t rssi,
  uint8_t channel
)
{
  /*
   * Beacon and Probe Response carry
   * the Information Elements used
   * to discover the PMF state.
  */
  if (
    subtype == ManagementSubtype::Beacon ||
    subtype == ManagementSubtype::ProbeResponse
  )
  {
    if (payload == nullptr || length < 24)
    {
      return;
    }

    WifiPmfMode pmf =
      WifiPmfParser::parseManagementFrame(payload, length);

    if (pmf != WifiPmfMode::PMF_UNKNOWN)
    {
      /*
       * Address 3 of Beacon/Probe Response
       * is the BSSID.
      */
      rememberApPmf(payload + 16, pmf);
    }

    return;
  }

  WifiManagementEventType eventType;

  switch (subtype)
  {
    case ManagementSubtype::Disassociation:
      eventType = WifiManagementEventType::DISASSOCIATION;
      break;

    case ManagementSubtype::Deauthentication:
      eventType = WifiManagementEventType::DEAUTHENTICATION;
      break;

    default:
      return;
  }

  if (payload == nullptr || length < 26)
  {
    return;
  }

  WifiManagementEvent event;

  event.type = eventType;

  event.reasonCode =
    static_cast<uint16_t>(
      payload[24]
    ) |
    (
      static_cast<uint16_t>(
        payload[25]
      ) << 8
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

  if (findKnownApChannel(event.bssid, knownChannel))
  {
    event.networkChannel = knownChannel;

    event.networkChannelKnown = true;
  }

  event.valid = true;

  addManagementEvent(event);
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
  return ::macEquals(first, second);
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

bool WifiMonitor::rememberApPmf(const uint8_t* bssid, WifiPmfMode pmf)
{
  if (bssid == nullptr || pmf == WifiPmfMode::PMF_UNKNOWN)
  {
    return false;
  }

  portENTER_CRITICAL(
    &knownApMux
  );

  /*
   * Updates existing entry.
  */
  for (size_t i = 0; i < KNOWN_AP_CACHE_SIZE; i++)
  {
    if (
      knownApChannels[i].valid &&
      macEquals(knownApChannels[i].bssid, bssid)
    )
    {
      knownApChannels[i].pmf = pmf;

      portEXIT_CRITICAL(
        &knownApMux
      );

      return true;
    }
  }

  /*
   * The Beacon may have been captured
   * before a scan placed the BSSID
   * in the cache.
   *
   * In that case we create an entry
   * only with PMF; channel stays 0.
  */
  for (size_t i = 0; i < KNOWN_AP_CACHE_SIZE; i++)
  {
    if (!knownApChannels[i].valid)
    {
      for (size_t j = 0; j < 6; j++)
      {
        knownApChannels[i].bssid[j] = bssid[j];
      }

      knownApChannels[i].pmf = pmf;

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

bool WifiMonitor::getKnownApPmf(
  const uint8_t* bssid, WifiPmfMode& pmf
) const
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
      knownApChannels[i].valid &&
      macEquals(knownApChannels[i].bssid, bssid)
    )
    {
      if (knownApChannels[i].pmf == WifiPmfMode::PMF_UNKNOWN)
      {
        portEXIT_CRITICAL(
          &knownApMux
        );

        return false;
      }

      pmf = knownApChannels[i].pmf;

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

bool WifiMonitor::getKnownApPmf(
  const String& bssid, WifiPmfMode& pmf
) const
{
  uint8_t mac[6];

  if (!parseMacAddress(bssid, mac))
  {
    return false;
  }

  return getKnownApPmf(mac, pmf);
}

bool WifiMonitor::forgetKnownApPmf(const String& bssid)
{
  uint8_t mac[6];

  if (!parseMacAddress(bssid, mac))
  {
    return false;
  }

  portENTER_CRITICAL(
    &knownApMux
  );

  for (size_t i = 0; i < KNOWN_AP_CACHE_SIZE; i++)
  {
    if (
      knownApChannels[i].valid &&
      macEquals(knownApChannels[i].bssid, mac)
    )
    {
      knownApChannels[i].pmf = WifiPmfMode::PMF_UNKNOWN;

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
