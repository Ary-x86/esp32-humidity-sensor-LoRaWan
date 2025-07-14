/*
 * LoRaWAN OTAA Moisture Sensor for The Things Network (TTN)
 *
 * This sketch uses the definitive hardware initialization for the XIAO ESP32S3 + WIO-SX1262
 * and connects to TTN using LoRaWAN 1.1. It sends a simple 1-byte binary payload.
*/

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include "config.h"
#include "secrets_ttn.h"

#define RADIO_BOARD_AUTO
#include <RadioBoards.h>

Radio radio = new RadioModule();

//-- Sensor Configuration
const int MOIST_PIN = A0;
int DROGE_WAARDE = 3400; //Calibrate this!
int NATTE_WAARDE = 1240; //Calibrate this!
const int NUM_METINGEN = 10;
const unsigned long INDIVIDUAL_MEET_INTERVAL = 50;

//-- LoRaWAN Hardware Setup
// SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_RST, LORA_BUSY);
const uint8_t subBand = 0;
LoRaWANNode node(&radio, &EU868, subBand);

//-- RF Switch configuration table
// static const uint32_t rfswitch_pins[] = {LORA_ANT_SW};
// static const Module::RfSwitchMode_t rfswitch_table[] = {
//   {Module::MODE_IDLE,  {LOW}},
//   {Module::MODE_RX,    {HIGH}},
//   {Module::MODE_TX,    {HIGH}},
//   END_OF_MODE_TABLE,
// };

//-- Helper Function
int getMoisturePercentage(int rawValue) {
  if (DROGE_WAARDE == NATTE_WAARDE) { return 0; }
  int percentage = 100 - ((rawValue - NATTE_WAARDE) * 100 / (DROGE_WAARDE - NATTE_WAARDE));
  return constrain(percentage, 0, 100);
}

void setup() {
  Serial.begin(115200);
  delay(2000); 
  Serial.println("\n--- TTN OTAA Moisture Sensor ---");

  //-- Sensor Setup
  pinMode(MOIST_PIN, INPUT);
  analogReadResolution(12);
  Serial.println("[Sensor] Moisture sensor configured.");

  //--- Radio and LoRaWAN Initialization ---
  // Serial.print("[RadioLib] Applying RF switch table... ");
  // int16_t state = radio.setRfSwitchTable(rfswitch_pins, rfswitch_table);
  // if (state != RADIOLIB_ERR_NONE) {
  //   Serial.printf("setRfSwitchTable failed, code %d\n", state);
  //   while (true);
  // }
  // Serial.println("success!");

  Serial.print("[RadioLib] Initializing radio... ");
  state = radio.begin();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("Radio init failed, code %d\n", state);
    while (true);
  }
  Serial.println("success!");

//   Serial.print("[RadioLib] Setting TCXO... ");
//   state = radio.setTCXO(1.6);
//   if (state != RADIOLIB_ERR_NONE) {
//     Serial.printf("setTCXO failed, code %d\n", state);
//     while (true);
//   }
//   Serial.println("success!");
//
  //--- OTAA Activation for LoRaWAN 1.1 ---
  Serial.print("[LoRaWAN] Configuring node for TTN... ");
  node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);
  Serial.println("success!");

  Serial.print("[LoRaWAN] Attempting to join network... ");
  state = node.activateOTAA();
  if (state != RADIOLIB_LORAWAN_NEW_SESSION && state != RADIOLIB_LORAWAN_SESSION_RESTORED) {
    Serial.printf("Join failed, code %d\n", state);
    while (true);
  }
  Serial.println("JOIN SUCCESS!");
}

void loop() {
  Serial.println("\n--- Reading Sensor & Sending Uplink ---");

  //Read sensor value
  long totalRawValue = 0;
  for (int i = 0; i < NUM_METINGEN; i++) {
    totalRawValue += analogRead(MOIST_PIN);
    delay(INDIVIDUAL_MEET_INTERVAL);
  }
  int raw_moisture = totalRawValue / NUM_METINGEN;
  int moisture_percent = getMoisturePercentage(raw_moisture);
  Serial.printf("Raw Value: %d, Moisture: %d%%\n", raw_moisture, moisture_percent);
  
  //Create a simple 1-byte payload for the moisture percentage
  uint8_t uplinkPayload[1];
  uplinkPayload[0] = (uint8_t)moisture_percent;
  Serial.printf("[LoRaWAN] Preparing 1-byte payload: 0x%02X\n", uplinkPayload[0]);

  //Send the payload
  Serial.print("[LoRaWAN] Sending uplink... ");
  int16_t state = node.sendReceive(uplinkPayload, sizeof(uplinkPayload));

  if (state >= RADIOLIB_ERR_NONE) {
    Serial.println("success!");
  } else {
    Serial.printf("failed, code %d\n", state);
  }

  Serial.println("Uplink complete. Sleeping for 5 minutes.");
  delay(5 * 60 * 1000);
}
