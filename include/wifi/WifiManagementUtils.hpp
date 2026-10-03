#pragma once

#include <cstddef>
#include <cstdint>

#include "WifiManagementEvent.hpp"

enum class WifiManagementDirection : uint8_t
{
  UNKNOWN,
  STA_TO_AP,
  AP_TO_STA
};

inline bool macEquals(const uint8_t* first, const uint8_t* second)
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

inline WifiManagementDirection getManagementEventDirection(
  const WifiManagementEvent& event
)
{
  if (!event.valid)
  {
    return WifiManagementDirection::UNKNOWN;
  }

  /*
   * AP transmitted the frame.
  */
  if (macEquals(event.source, event.bssid)
  )
  {
    return WifiManagementDirection::AP_TO_STA;
  }

  /*
   * Station transmitted the frame
   * toward the AP.
  */
  if (macEquals(event.destination, event.bssid))
  {
    return WifiManagementDirection::STA_TO_AP;
  }

  return WifiManagementDirection::UNKNOWN;
}

inline const char* wifiManagementDirectionToString(
  WifiManagementDirection direction
)
{
  switch (direction)
  {
    case WifiManagementDirection::STA_TO_AP:
      return "STA -> AP";

    case WifiManagementDirection::AP_TO_STA:
      return "AP -> STA";

    default:
      return "UNKNOWN";
  }
}

inline const char* wifiManagementEventTypeToString(
  WifiManagementEventType type
)
{
  switch (type)
  {
    case WifiManagementEventType::DEAUTHENTICATION:
      return "DEAUTH";

    case WifiManagementEventType::DISASSOCIATION:
      return "DISASSOC";

    default:
      return "NONE";
  }
}

inline const char* wifiReasonCodeToString(uint16_t reasonCode)
{
  switch (reasonCode)
  {
    case 1:
      return "UNSPECIFIED";

    case 2:
      return "PREV AUTH INVALID";

    case 3:
      return "STA LEAVING";

    case 4:
      return "INACTIVITY";

    case 5:
      return "AP BUSY";

    case 6:
      return "CLASS 2 NONAUTH";

    case 7:
      return "CLASS 3 NONASSOC";

    case 8:
      return "STA LEFT BSS";

    case 9:
      return "ASSOC WITHOUT AUTH";

    case 10:
      return "POWER CAP INVALID";

    case 11:
      return "CHANNEL INVALID";

    case 13:
      return "INVALID IE";

    case 14:
      return "MIC FAILURE";

    case 15:
      return "4-WAY TIMEOUT";

    case 16:
      return "GROUP KEY TIMEOUT";

    case 17:
      return "4-WAY IE MISMATCH";

    case 18:
      return "GROUP CIPHER INVALID";

    case 19:
      return "PAIRWISE CIPHER INVALID";

    case 20:
      return "AKMP INVALID";

    case 21:
      return "RSN VERSION INVALID";

    case 22:
      return "RSN CAP INVALID";

    case 23:
      return "802.1X AUTH FAILED";

    case 24:
      return "CIPHER REJECTED";

    default:
      return "UNKNOWN";
  }
}
