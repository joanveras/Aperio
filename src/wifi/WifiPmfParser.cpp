#include "../../include/wifi/WifiPmfParser.hpp"

namespace
{
  constexpr uint8_t MANAGEMENT_TYPE = 0;

  constexpr uint8_t BEACON_SUBTYPE = 0x08;
  constexpr uint8_t PROBE_RESPONSE_SUBTYPE = 0x05;

  constexpr uint8_t RSN_ELEMENT_ID = 0x30;

  constexpr size_t MANAGEMENT_HEADER_SIZE = 24;

  /*
   * Beacon / Probe Response fixed parameters:
   *
   * Timestamp        8 bytes
   * Beacon Interval  2 bytes
   * Capability Info  2 bytes
  */
  constexpr size_t FIXED_PARAMETERS_SIZE = 12;

  constexpr size_t INFORMATION_ELEMENTS_OFFSET =
    MANAGEMENT_HEADER_SIZE + FIXED_PARAMETERS_SIZE;

  /*
   * RSN Capabilities:
   *
   * Bit 6 = MFPR
   * Bit 7 = MFPC
  */
  constexpr uint16_t RSN_CAP_MFPR = 0x0040;
  constexpr uint16_t RSN_CAP_MFPC = 0x0080;
}

WifiPmfMode WifiPmfParser::parseManagementFrame(
  const uint8_t* payload, size_t length
)
{
  if (payload == nullptr || length < INFORMATION_ELEMENTS_OFFSET)
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  /*
   * Frame Control byte 0:
   *
   * bits 2..3 = Type
   * bits 4..7 = Subtype
  */
  uint8_t frameType = (payload[0] >> 2) & 0x03;

  uint8_t frameSubtype = (payload[0] >> 4) & 0x0F;

  if (frameType != MANAGEMENT_TYPE)
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  if (
    frameSubtype != BEACON_SUBTYPE &&
    frameSubtype != PROBE_RESPONSE_SUBTYPE
  )
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  const uint8_t* informationElements =
    payload + INFORMATION_ELEMENTS_OFFSET;

  size_t informationElementsLength =
    length - INFORMATION_ELEMENTS_OFFSET;

  return parseInformationElements(
    informationElements,
    informationElementsLength
  );
}

WifiPmfMode WifiPmfParser::parseInformationElements(
  const uint8_t* data, size_t length
)
{
  if (data == nullptr)
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  size_t offset = 0;

  while (offset + 2 <= length)
  {
    uint8_t elementId = data[offset];

    uint8_t elementLength = data[offset + 1];

    offset += 2;

    /*
     * Incomplete/truncated element.
     *
     * We stop parsing instead of accessing
     * memory outside the frame.
    */
    if (offset + static_cast<size_t>(elementLength) > length)
    {
      break;
    }

    if (elementId == RSN_ELEMENT_ID)
    {
      return parseRsnElement(data + offset, elementLength);
    }

    offset += elementLength;
  }

  /*
   * Valid Beacon/Probe Response, but
   * no RSN IE was advertised.
   *
   * Therefore PMF is not being advertised.
  */
  return WifiPmfMode::PMF_DISABLED;
}

WifiPmfMode WifiPmfParser::parseRsnElement(
  const uint8_t* data, size_t length
)
{
  if (data == nullptr)
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  size_t offset = 0;

  /*
   * RSN Version
  */
  if (offset + 2 > length)
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  uint16_t version = readLe16(data + offset);

  offset += 2;

  if (version != 1)
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  /*
   * Group Data Cipher Suite
   *
   * 4 bytes
  */
  if (offset + 4 > length)
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  offset += 4;

  /*
   * Pairwise Cipher Suite Count
  */
  if (offset + 2 > length)
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  uint16_t pairwiseCipherCount = readLe16(data + offset);

  offset += 2;

  size_t pairwiseCipherBytes =
    static_cast<size_t>(pairwiseCipherCount) * 4;

  if (offset + pairwiseCipherBytes > length)
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  offset += pairwiseCipherBytes;

  /*
   * AKM Suite Count
  */
  if (offset + 2 > length)
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  uint16_t akmSuiteCount = readLe16(data + offset);

  offset += 2;

  size_t akmSuiteBytes = static_cast<size_t>(akmSuiteCount) * 4;

  if (offset + akmSuiteBytes > length)
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  offset += akmSuiteBytes;

  /*
   * RSN Capabilities is optional.
   *
   * If it is not present, MFPC/MFPR
   * are effectively not advertised.
  */
  if (offset == length)
  {
    return WifiPmfMode::PMF_DISABLED;
  }

  /*
   * If only one byte is left,
   * the RSN IE is truncated.
  */
  if (offset + 2 > length)
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  uint16_t capabilities = readLe16(data + offset);

  bool mfpr = (capabilities & RSN_CAP_MFPR) != 0;

  bool mfpc = (capabilities & RSN_CAP_MFPC) != 0;

  /*
   * MFPR without MFPC is an inconsistent
   * combination.
  */
  if (mfpr && !mfpc)
  {
    return WifiPmfMode::PMF_UNKNOWN;
  }

  if (mfpc && mfpr)
  {
    return WifiPmfMode::PMF_REQUIRED;
  }

  if (mfpc)
  {
    return WifiPmfMode::PMF_OPTIONAL;
  }

  return WifiPmfMode::PMF_DISABLED;
}

uint16_t WifiPmfParser::readLe16(const uint8_t* data)
{
  return (
    static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8)
  );
}
