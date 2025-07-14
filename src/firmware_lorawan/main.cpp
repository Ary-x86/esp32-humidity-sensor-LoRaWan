/*
 * FINAL LORAWAN SKETCH for XIAO ESP32S3 with WIO-SX1262
 * This version uses the definitive, correct initialization sequence.
 */

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <thingsml.h>
#include "config.h"
#include "secrets.h"

// Use the manual constructor from the board's confirmed pinout
SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_RST, LORA_BUSY);

// ThingsML Payload Setup
SenMLPack data_pack;
SenMLDoubleRecord temperature_rec(THINGSML_TEMPERATURE);

// LoRaWAN Node Setup
const uint8_t subBand = 0; // Use 0 for EU868
LoRaWANNode node(&radio, &EU868, subBand);

// RF Switch configuration table
static const uint32_t rfswitch_pins[] = {LORA_ANT_SW};
static const Module::RfSwitchMode_t rfswitch_table[] = {
  {Module::MODE_IDLE,  {LOW}},
  {Module::MODE_RX,    {HIGH}},
  {Module::MODE_TX,    {HIGH}},
  END_OF_MODE_TABLE,
};

void setup() {
  Serial.begin(115200);
  delay(2000); 
  Serial.println("\n--- LoRaWAN Sensor Node ---");

  data_pack.add(temperature_rec);

  // STEP 1: Apply RF switch configuration BEFORE begin()
  Serial.print("[RadioLib] Applying RF switch table... ");
  int16_t state = radio.setRfSwitchTable(rfswitch_pins, rfswitch_table);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("setRfSwitchTable failed, code %d\n", state);
    while (true);
  }
  Serial.println("success!");

  // STEP 2: Initialize radio with parameter-less begin()
  Serial.print("[RadioLib] Initializing radio... ");
  state = radio.begin();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("Radio init failed, code %d\n", state);
    while (true);
  }
  Serial.println("success!");

  // STEP 3: Configure the TCXO AFTER begin()
  Serial.print("[RadioLib] Setting TCXO... ");
  state = radio.setTCXO(1.6);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("setTCXO failed, code %d\n", state);
    while (true);
  }
  Serial.println("success!");
  
  // STEP 4: Configure and join LoRaWAN
  Serial.print("[LoRaWAN] Configuring node... ");
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
  Serial.println("\n--- Sending Uplink ---");

  float current_temp = 20.0 + (radio.random(100) / 10.0);
  temperature_rec.set(current_temp);
  Serial.printf("Set temperature value to: %.2f\n", current_temp);

  uint8_t cbor_payload[64];
  int payload_len = data_pack.toCbor((char*)cbor_payload, sizeof(cbor_payload));
  if (payload_len <= 0) {
    Serial.println("[ThingsML] ERROR: Failed to render CBOR payload.");
    delay(60000);
    return;
  }
  Serial.printf("[ThingsML] Rendered CBOR payload, %d bytes.\n", payload_len);

  Serial.print("[LoRaWAN] Sending uplink... ");
  int16_t state = node.sendReceive(cbor_payload, payload_len);

  if (state >= RADIOLIB_ERR_NONE) {
    Serial.println("success!");
  } else {
    Serial.printf("failed, code %d\n", state);
  }

  Serial.println("Uplink complete. Sleeping for 5 minutes.");
  delay(5 * 60 * 1000);
}
