#pragma once

#include <cstddef>
#include <cstdint>

#include "WifiNetwork.hpp"

class WifiPmfParser
{
public:
  static WifiPmfMode parseManagementFrame(
    const uint8_t* payload,
    size_t length
  );

private:
  static WifiPmfMode parseInformationElements(
    const uint8_t* data,
    size_t length
  );

  static WifiPmfMode parseRsnElement(
    const uint8_t* data,
    size_t length
  );

  static uint16_t readLe16(
    const uint8_t* data
  );
};
