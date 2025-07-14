#pragma once

// LoRaWAN Credentials for KPN Things - OTAA
// IMPORTANT: Replace these placeholders with your actual device credentials

// LoRaWAN App/Join EUI (big-endian)
uint64_t joinEUI = 0x0059AC0000010DA8;

// LoRaWAN Device EUI (big-endian)
uint64_t devEUI  = 0x0059AC00001B3448;

// LoRaWAN App Key (msb)
uint8_t appKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// For LoRaWAN 1.0.x, nwkKey is not used for OTAA
uint8_t *nwkKey = nullptr;
