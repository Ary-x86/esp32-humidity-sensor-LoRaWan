#pragma once

//LoRaWAN v1.1 Credentials for The Things Network (TTN) - OTAA
//IMPORTANT: Replace placeholders with your actual device credentials from the TTN console.
//Follow the TTN setup guide to generate these.

//JoinEUI (or AppEUI) - provided by TTN
uint64_t joinEUI = 0x0000000000000000;

//Device EUI - provided by TTN
uint64_t devEUI  = 0x0000000000000000;

//Application Key (AppKey) - provided by TTN
uint8_t appKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

//Network Key (NwkKey) - provided by TTN for LoRaWAN 1.1
uint8_t nwkKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
