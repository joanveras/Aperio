#include "../../include/wifi/WifiScanner.hpp"

void WifiScanner::begin()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
}

void WifiScanner::scan()
{
  networkCount = 0;

  int found = WiFi.scanNetworks(false, true);

  if (found == 0)
  {
    Serial.print("No networks were found");
    WiFi.scanDelete();
    return;
  } else if (found < 0)
  {
    Serial.print("Error trying to search for networks");
    WiFi.scanDelete();
    return;
  }

  size_t storedCount = static_cast<size_t>(found) > MAX_NETWORKS ?
    MAX_NETWORKS : static_cast<size_t>(found);

  for (size_t i = 0; i < storedCount; i++)
  {
    networks[i].ssid = WiFi.SSID(i);
    networks[i].bssid = WiFi.BSSIDstr(i);
    networks[i].rssi = WiFi.RSSI(i);
    networks[i].channel = WiFi.channel(i);
    networks[i].security = WiFi.encryptionType(i);

    networks[i].pmf =WifiPmfMode::PMF_UNKNOWN;
  }

  networkCount = storedCount;

  WiFi.scanDelete();
}

size_t WifiScanner::getNetworkCount() const
{
  return networkCount;
}

const WifiNetwork& WifiScanner::getNetwork(size_t index) const
{
  return networks[index];
}
