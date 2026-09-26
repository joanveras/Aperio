#pragma once

#include <cstdint>

constexpr uint8_t TFT_CS  = 5;
constexpr uint8_t TFT_DC  = 2;
constexpr uint8_t TFT_RST = 27;

constexpr uint8_t BTN_LEFT   = 25;
constexpr uint8_t BTN_SELECT = 26;
constexpr uint8_t BTN_RIGHT  = 32;

constexpr uint32_t TFT_SPI_FREQUENCY = 8000000;

constexpr const char* APERIO_VERSION = "v0.1";
