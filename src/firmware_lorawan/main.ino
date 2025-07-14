// This version does NOT use persistence / ESP32 Deep Sleep

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <thingsml.h>
#include "config.h"
#include "secrets.h"

// RadioBoards can also be used for the XIAO ESP32S3 + WIO-SX1262, this code uses the manual constructor


// Moisture Sensor SETUP
const int MOIST_PIN = A0; // analog pin for moisture sensor

// IMPORTANT: These values should be calibrated accordingly
int DROGE_WAARDE = 3400; // Raw value that should be completely dry (slider all the way to the left in DataCake)
int NATTE_WAARDE = 1240; // Raw analog value when sensor is fully wet (all the way to the right in datacake)

const int NUM_METINGEN = 10; // Number of readings to average for a stable result
const unsigned long INDIVIDUAL_MEET_INTERVAL = 50; // ms delay between readings


//Use the manual constructor from the board's confirmed pinout in config.h
SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_RST, LORA_BUSY);

// ThingsML Payload Setup
SenMLPack data_pack;
SenMLDoubleRecord moisture_rec(THINGSML_HUMIDITY);

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

// Helper: converts the raw sensor value into a percentage
int getMoisturePercentage(int rawValue) {
  if (DROGE_WAARDE == NATTE_WAARDE) { return 0; }
  int percentage = 100 - ((rawValue - NATTE_WAARDE) * 100 / (DROGE_WAARDE - NATTE_WAARDE));
  return constrain(percentage, 0, 100);
}

void setup() {
  Serial.begin(115200);
  delay(2000); 
  Serial.println("\n--- LoRaWAN Sensor Node ---");

  // --- Sensor Setup ---
  pinMode(MOIST_PIN, INPUT);
  analogReadResolution(12); // Set ADC to 12-bit resolution (0-4095)
  Serial.println("[Sensor] Moisture sensor configured.");

  // --- ThingsML Packet Setup ---
  data_pack.add(moisture_rec);

  //Apply RF switch configuration BEFORE radio.begin()
  Serial.print("[RadioLib] Applying RF switch table... ");
  int16_t state = radio.setRfSwitchTable(rfswitch_pins, rfswitch_table);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("setRfSwitchTable failed, code %d\n", state);
    while (true);
  }
  Serial.println("success!");

  // radio.begin - the default parameters should be correct
  Serial.print("[RadioLib] Initializing radio... ");
  state = radio.begin();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("Radio init failed, code %d\n", state);
    while (true);
  }
  Serial.println("success!");

  // configure the TCXO AFTER begin() - might be redundant, should be enabled by default
  Serial.print("[RadioLib] Setting TCXO... ");
  state = radio.setTCXO(1.6);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("setTCXO failed, code %d\n", state);
    while (true);
  }
  Serial.println("success!");
  
  // Setup for LoRaWAN
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
  Serial.println("\n--- Reading Sensor & Sending Uplink ---");

  // Read the raw sensor value with averaging for stability
  long totalRawValue = 0;
  for (int i = 0; i < NUM_METINGEN; i++) {
    totalRawValue += analogRead(MOIST_PIN);
    delay(INDIVIDUAL_MEET_INTERVAL);
  }
  int raw_moisture = totalRawValue / NUM_METINGEN;

  //Convert the raw value to a moisture percentage
  int moisture_percent = getMoisturePercentage(raw_moisture);
  Serial.printf("Raw Value: %d, Moisture: %d%%\n", raw_moisture, moisture_percent);

  // Set the value in the ThingsML record
  moisture_rec.set(moisture_percent);

  //Render the packet to a compact CBOR binary payload
  uint8_t cbor_payload[64];
  int payload_len = data_pack.toCbor((char*)cbor_payload, sizeof(cbor_payload));
  if (payload_len <= 0) {
    Serial.println("[ThingsML] ERROR: Failed to render CBOR payload.");
    delay(60000); // Wait before retrying
    return;
  }
  Serial.printf("[ThingsML] Rendered CBOR payload, %d bytes.\n", payload_len);

  // Send the payload over LoRaWAN
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
