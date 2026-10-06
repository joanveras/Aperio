#pragma once

#include <Arduino.h>
#include <Adafruit_ILI9341.h>

#include "../Screen.hpp"
#include "../../wifi/WifiScanner.hpp"
#include "../../wifi/WifiTransmitter.hpp"

enum class TxPayloadType : uint8_t
{
  PROBE_REQUEST,
  DEAUTH_TEST
};

enum class TxLabMode : uint8_t
{
  PAYLOAD_SELECT,
  CONFIGURE,
  READY
};

struct WifiReasonCode
{
  uint16_t code;
  const char* label;
};

class TxLabScreen : public Screen
{
public:
  TxLabScreen(
    Adafruit_ILI9341* displayInstance,
    WifiTransmitter* transmitterInstance
  );

  void setTarget(const WifiNetwork& network);

  void onEnter() override;

  void handleInput(InputEvent event) override;

  void update() override;
  void render() override;

  // Never sleep while the TX Lab is set up to transmit.
  bool allowsIdle() const override { return false; }

private:
  static constexpr size_t PAYLOAD_COUNT = 2;

  static constexpr WifiReasonCode REASON_CODES[] = {
    {1,  "UNSPECIFIED"},
    {2,  "PREV AUTH INVALID"},
    {3,  "STA LEAVING"},
    {4,  "INACTIVITY"},
    {5,  "AP BUSY"},
    {6,  "CLASS 2 NONAUTH"},
    {7,  "CLASS 3 NONASSOC"},
    {8,  "STA LEFT BSS"},
    {9,  "ASSOC WITHOUT AUTH"},
    {10, "POWER CAP INVALID"},
    {11, "CHANNEL INVALID"},
    {13, "INVALID IE"},
    {14, "MIC FAILURE"},
    {15, "4-WAY TIMEOUT"},
    {16, "GROUP KEY TIMEOUT"},
    {17, "4-WAY IE MISMATCH"},
    {18, "GROUP CIPHER INVALID"},
    {19, "PAIRWISE CIPHER INVALID"},
    {20, "AKMP INVALID"},
    {21, "RSN VERSION INVALID"},
    {22, "RSN CAP INVALID"},
    {23, "802.1X AUTH FAILED"},
    {24, "CIPHER REJECTED"}
  };

  static constexpr size_t REASON_CODE_COUNT =
    sizeof(REASON_CODES) / sizeof(REASON_CODES[0]);

  Adafruit_ILI9341* display;
  WifiTransmitter* transmitter;

  WifiNetwork targetNetwork;
  WifiTxStats stats;

  TxPayloadType selectedPayload = TxPayloadType::PROBE_REQUEST;

  TxLabMode mode = TxLabMode::PAYLOAD_SELECT;

  /*
   * Index 2 =
   * Reason Code 3 - STA LEAVING
  */
  size_t selectedReasonIndex = 2;

  bool hasTarget = false;

  bool transmitterStartFailed = false;
  bool invalidTarget = false;

  bool hasSendResult = false;
  bool lastSendSucceeded = false;

  bool needsRedraw = true;

  void handlePrevious();
  void handleNext();
  void handleSelect();

  void movePayload(int direction);

  void moveReasonCode(int direction);

  void enterConfiguration();
  void confirmConfiguration();

  void executeSelectedPayload();

  void executeProbeRequest();
  void executeDeauthTest();

  void refreshStats();

  bool isTargetValid() const;

  bool payloadRequiresConfiguration() const;

  const char* getPayloadName() const;

  const WifiReasonCode& getSelectedReasonCode() const;

  void drawScreen();
  void drawHeader();
  void drawContent();
  void drawFooter();
  void drawError();

  void drawReasonCode(
    int16_t y,
    bool selected
  );

  uint16_t statusColor() const;

  void drawCentered(
    const char* text,
    int16_t y,
    uint8_t textSize,
    uint16_t color
  );
};
