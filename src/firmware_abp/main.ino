/*
 * ABP LoRaWAN Moisture Sensor for XIAO ESP32S3
 * This version uses ABP activation based on the RadioLib TTN example.
 * It reads the sensor, calculates a percentage, and sends the data.
*/

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <thingsml.h>
#include "config.h"
#include "secrets_abp.h" //Use the new secrets file for ABP

//Sensor config
const int MOIST_PIN = A0;
int DROGE_WAARDE = 3400; //Calibrate this!
int NATTE_WAARDE = 1240; //Calibrate this!
const int NUM_METINGEN = 10;
const unsigned long INDIVIDUAL_MEET_INTERVAL = 50;

//LoRaWAN Hardware and Payload Setup
SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_RST, LORA_BUSY);
SenMLPack data_pack;
SenMLIntRecord moisture_rec(THINGSML_HUMIDITY);
const uint8_t subBand = 0;
LoRaWANNode node(&radio, &EU868, subBand);

//RF Switch config table
static const uint32_t rfswitch_pins[] = {LORA_ANT_SW};
static const Module::RfSwitchMode_t rfswitch_table[] = {
  {Module::MODE_IDLE,  {LOW}},
  {Module::MODE_RX,    {HIGH}},
  {Module::MODE_TX,    {HIGH}},
  END_OF_MODE_TABLE,
};

//Helper function to get moisture percentage
int getMoisturePercentage(int rawValue) {
  if (DROGE_WAARDE == NATTE_WAARDE) { return 0; }
  int percentage = 100 - ((rawValue - NATTE_WAARDE) * 100 / (DROGE_WAARDE - NATTE_WAARDE));
  return constrain(percentage, 0, 100);
}

void setup() {
  Serial.begin(115200);
  delay(2000); 
  Serial.println("\n--- LoRaWAN Moisture Sensor (ABP) ---");

  //Setup sensor pin
  pinMode(MOIST_PIN, INPUT);
  analogReadResolution(12);
  Serial.println("[Sensor] Moisture sensor configured.");

  //Add record to ThingsML packet
  data_pack.add(moisture_rec);

  //--- Radio and LoRaWAN Initialization ---
  Serial.print("[RadioLib] Applying RF switch table... ");
  int16_t state = radio.setRfSwitchTable(rfswitch_pins, rfswitch_table);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("setRfSwitchTable failed, code %d\n", state);
    while (true);
  }
  Serial.println("success!");

  Serial.print("[RadioLib] Initializing radio... ");
  state = radio.begin();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("Radio init failed, code %d\n", state);
    while (true);
  }
  Serial.println("success!");

  Serial.print("[RadioLib] Setting TCXO... ");
  state = radio.setTCXO(1.6);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("setTCXO failed, code %d\n", state);
    while (true);
  }
  Serial.println("success!");
  
  //--- ABP ACTIVATION ---
  //This configures the node locally using the keys from secrets_abp.h
  //No network communication happens here.
  Serial.print("[LoRaWAN] Activating node with ABP credentials... ");
  node.beginABP(devAddr, fNwkSIntKey, sNwkSIntKey, nwkSEncKey, appSKey);
  node.activateABP();
  Serial.println("success! Node is active.");
}

void loop() {
  Serial.println("\n--- Reading Sensor & Sending Uplink ---");

  //Read sensor and average the value for stability
  long totalRawValue = 0;
  for (int i = 0; i < NUM_METINGEN; i++) {
    totalRawValue += analogRead(MOIST_PIN);
    delay(INDIVIDUAL_MEET_INTERVAL);
  }
  int raw_moisture = totalRawValue / NUM_METINGEN;

  //Convert raw value to percentage
  int moisture_percent = getMoisturePercentage(raw_moisture);
  Serial.printf("Raw Value: %d, Moisture: %d%%\n", raw_moisture, moisture_percent);
  
  //Set the value for the ThingsML payload
  moisture_rec.set(moisture_percent);

  //Render the packet into a compact CBOR binary payload
  uint8_t cbor_payload[64];
  int payload_len = data_pack.toCbor((char*)cbor_payload, sizeof(cbor_payload));
  if (payload_len <= 0) {
    Serial.println("[ThingsML] ERROR: Failed to render CBOR payload.");
    delay(60000); //wait before retrying
    return;
  }
  Serial.printf("[ThingsML] Rendered CBOR payload, %d bytes.\n", payload_len);

  //Send the payload over LoRaWAN
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
