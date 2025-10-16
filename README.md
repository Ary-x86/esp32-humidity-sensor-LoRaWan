# ESP32 Multi-Cloud IoT Sensor (LoRaWAN & Wi-Fi)

!! This Repository is out of date. The up to date code is not released yet but finished. The documentation for the new code is available at: https://ary-x86.github.io/Think2Day-Vochtsensor/02-hardware.html

This repository contains the complete firmware and cloud integration code for an ESP32-S3 based environmental sensor. The project is designed to be flexible, allowing data transmission to multiple cloud platforms (Arduino Cloud, Datacake) via two different connectivity methods: LoRaWAN (through KPN Things) and Wi-Fi.


## ⚠️ Current Project Status - Prototype

This project is currently in a prototype stage.

The primary goal is to use LoRaWAN 1.0.x with **KPN Things** via the **OTAA (Over-the-Air Activation)** method. At present, the device firmware is theoretically correct but is unable to receive the `Join Accept` downlink from the KPN network. We are in contact with KPN to resolve this hardware/protocol compatibility issue.

Because of this, the current status is:
* **KPN Things Cloud Testing:** The data flow from **KPN Things -> AWS -> Arduino Cloud** and **KPN -> Datacake** can be fully tested using the **KPN Device Simulator**. This allows you to verify the entire cloud backend is working correctly without a physical device.
* **Testing on TTN with LoRaWan 1.0.x:** To test the physical device sending data all the way to a cloud platform, the current working method for LoRaWaN 1.0.x (Also KPN Things LW Spec)  is to use **ABP (Activation by Personalization) with The Things Network (TTN)**.
* **Testing on TTN with LoRaWan 1.1:** To test the physical device sending data all the way to a cloud platform, the current working method for LoRaWan 1.1 is to use **OTAA with The Things Network (TTN)**. OTAA works with LoRaWAN 1.1, but unfortunately KPN doesn't (yet) support LoRaWan 1.1.

The current goal is to get OTAA working with KPN's LoRaWan 1.0.4. The next major update will be the **"OTAA Support Update"** once the downlink issue with KPN Things is resolved.

<img width="859" height="382" alt="image" src="https://github.com/user-attachments/assets/2491575f-fcd7-4113-aa8d-45dbd7d6e5f0" />



## System Architecture

This project has two primary data paths:

1.  **LoRaWAN Path:** The device sends a highly efficient binary payload over LoRaWAN.
    ```
    ESP32 -> LoRaWAN -> KPN Things -> AWS Lambda -> Arduino Cloud API
                                   └-> Datacake HTTP Endpoint
    ```
2.  **Wi-Fi Path:** The device connects directly to a Wi-Fi network and uses the standard Arduino IoT Cloud library.
    ```
    ESP32 -> Wi-Fi -> Arduino IoT Cloud
    ```

## Components

### **Hardware**
* **Microcontroller:** Seeed Studio XIAO ESP32S3
* **LoRa Module:** Seeed Studio WIO-SX1262 (B2B Connector Version)

### **Firmware (`/src`)**
* **`firmware_lorawan_1.1_TTN & firmware_lorawan_1.0.x_OTAA_KPN`**: Uses the **RadioLib** library for LoRaWAN communication (in src/firmware_lorawan_1.0.x_OTAA_KPN: **ThingsML** for efficient payload encoding). Includes hardware initialization for the XIAO's specific TCXO and RF antenna switch and the correct pinout.
* **`firmware_wifi`**: A standard implementation using the **ArduinoIoTCloud** library for direct Wi-Fi connectivity.

### **Cloud Functions (`/cloud_functions`)**
* **AWS Lambda**: Acts as a bridge between KPN Things and the Arduino Cloud API, handling OAuth2 authentication.
* **Datacake Decoder**: A JavaScript payload decoder used within Datacake to parse incoming SenML data from KPN.

## Setup and Deployment
See the README files within each component's directory for specific deployment instructions. A `platformio.ini` file is provided for easy compilation and library management.




## Setup and Deployment Guide

### KPN Things Setup (for Cloud Testing)
This path allows you to test the entire cloud data flow using a simulated device.

1.  **Create an Account:** Register for an account on the KPN Things portal.
2.  **Follow KPN Instructions:** Follow the official KPN Things documentation to set up your cloud components:
    * Create **Destinations** pointing to your deployed AWS Lambda and Datacake endpoints.
    * Create a **Flow** to direct data.
3.  **Use the Device Simulator:**
    * In the KPN Things portal, you can use the **Device Simulator** to send test payloads.
    * This is extremely useful for verifying that your destinations and cloud functions are working correctly *before* getting the physical device to connect. You can send simulated SenML JSON payloads to test the entire data flow.

### The Things Network (TTN) Setup (for Device Testing with ABP) - found in .src/firmware_lorawan_1.1_TTN/old/* 
This is the current working method for testing with the less secure ABP.

1.  **Register on TTN:** Create an account and an application on The Things Network console.
2.  **Add Device with ABP:** Register your device, making sure to select **ABP (Activation by Personalization)** as the activation mode.
3.  **Enable Frame Counter Resets:** In your device's settings on TTN, go to "General settings" -> "Join settings" and enable **"Resets Frame Counters"**. This is critical for development without persistence.
4.  **Copy Credentials:** Copy the **Device Address**, **Network Session Key**, and **App Session Key** into the `/src/firmware_abp/secrets_abp.h` file.
5.  **Compile and Upload:** Use PlatformIO to build and upload the `abp` environment to your device.

### The Things Network (TTN) Setup (for Device Testing with OTAA) - found in .src/firmware_lorawan_1.1_TTN/* 
This is the current working method for testing the physical LoRaWAN device end-to-end. 
See README.md in src/firmware_lorawan_1.1_TTN/ for instructions.

