# ESP32 Multi-Cloud IoT Sensor (LoRaWAN & Wi-Fi)

This repository contains the firmware and cloud integration code for a Seeed Studio XIAO ESP32-S3 based environmental sensor. The goal of this project is to provide a reliable, low-power solution for monitoring soil moisture in locations without Wi-Fi access, sending data to cloud dashboards like Arduino Cloud and Datacake.

The project is designed to be flexible, with firmware versions for both LoRaWAN and standard Wi-Fi connectivity.

## System Architecture

The primary data path for this project uses LoRaWAN to achieve long-range communication.

ESP32 -> LoRaWAN -> The Things Network -> AWS Lambda -> Arduino Cloud API
└-> Datacake HTTP Endpoint

A secondary Wi-Fi firmware is included for direct connection to Arduino Cloud when a Wi-Fi network is available.

---

## ⚠️ Current Project Status - Prototype

This project is currently in the prototype and testing phase.

* **KPN Things (OTAA):** The primary goal is to use the KPN LoRaWAN network. Currently, we are facing an issue where the device can send `join request` uplinks, but fails to receive the `join accept` downlink. We are in contact with KPN to resolve this potential hardware/network compatibility issue.

* **The Things Network (TTN):** End-to-end testing of the physical hardware is **fully working** using **OTAA on The Things Network (TTN)**, which supports LoRaWAN 1.1. This folder is the current stable reference.

* **Next Steps:** Once the downlink issue with KPN is resolved, the LoRaWAN firmware will be finalized. This will be released as the "OTAA Support Update".

---

## Setup Guide: LoRaWAN with The Things Network (TTN)

This guide explains how to get the physical device working end-to-end using the recommended TTN setup.

### Part 1: Registering Your Device on TTN

Before flashing the device, you need to register it on The Things Network to get the required credentials.

1.  **Create a TTN Account:** If you don't have one, register at [thethingsnetwork.org](https://www.thethingsnetwork.org/get-started). Log in to the regional console (e.g., [eu1.cloud.thethings.network](https://eu1.cloud.thethings.network/)).

2.  **Create an Application:** In the console, go to **Applications** and create a new one. The `Application ID` must be unique across all of TTN (e.g., `your-client-moisture-sensors`).

3.  **Register the End Device:**
    * Inside your application, click **+ Register end device**.
    * Select the **Manually** tab.
    * **Frequency plan:** Choose the one for your region (e.g., `Europe 863-870 MHz (SF9 for RX2 - recommended)`).
    * **LoRaWAN version:** Select **`LoRaWAN Specification 1.1.0`**. This is critical.
    * **JoinEUI:** You can fill this in yourself or leave this as all zeros (`0000000000000000`).
    * **DevEUI & Keys:** Click the **Generate** button next to `DevEUI`, `AppKey`, and `NwkKey` to have TTN create them for you.
    * Give the device a memorable ID and click **Register end device**.

4.  **Enable Frame Counter Resets (For Development ONLY):**
    * On your new device's page, go to the **General settings** tab.
    * Expand the **Join settings** section.
    * Check the box for **Resets join nonces** (or "Resets Frame Counters"). This is necessary to allow your device to rejoin repeatedly during testing without being blocked. Once the first full OTAA support update released, this will not be necessary.

### Part 2: Configuring and Flashing the Firmware

1.  **Copy Credentials:**
    * From the TTN device page, copy the `DevEUI`, `JoinEUI`, `AppKey`, and `NwkKey`.
    * Use the `👁️` and `<>` icons on the TTN console to format the keys as a C-style byte array.
    * Paste these values into the **`src/firmware_ttn_otaa/secrets_ttn.h`** file in this repository.

2.  **Hardware Placement:** Remember that LoRa is long-range. Keep your device at least **5-10 meters (15-30 feet)** away from your gateway, preferably with a wall in between, to prevent the gateway's powerful signal from overwhelming your device's receiver.

3.  **Compile and Upload:**
    * It is recommended to use [PlatformIO](https://platformio.org/) to build this project. Arduino IDE is possible too.
    * Currently, it's an Arduino IDE (.ino) file. To use in PlatformIO, change the extension to .cpp
    * Open this project folder in VS Code with the PlatformIO extension.
    * Select the `ttn_otaa` environment from the PlatformIO toolbar.
    * Click **Upload**.

### Part 3: Setting up the Payload Formatter

The firmware sends a very compact binary payload. You need to tell TTN how to decode it into human-readable data.

1.  In the TTN console, go to your device's page.
2.  Click on **Payload formatters** in the left menu, then **Uplink**.
3.  Select **Custom Javascript formatter**.
4.  Delete the default code and paste the entire contents of the **`src/firmware_lorawan_1.1_TTN/TTN_Payload_Formatter.js`** file from this repository.
5.  Save the changes.

After these steps, your device will join the network, and you will see decoded moisture readings in the "Live data" tab on the TTN console. From there, you can use TTN's integrations to forward the data to Datacake. This can be done bt simply following the steps on Datacake's platform.
