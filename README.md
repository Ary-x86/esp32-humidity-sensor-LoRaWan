# ESP32 Multi-Cloud IoT Sensor (LoRaWAN & Wi-Fi)

This repository contains the complete firmware and cloud integration code for an ESP32-S3 based environmental sensor. The project is designed to be flexible, allowing data transmission to multiple cloud platforms (Arduino Cloud, Datacake) via two different connectivity methods: LoRaWAN (through KPN Things) and Wi-Fi.

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
* **`firmware_lorawan`**: Uses the **RadioLib** library for LoRaWAN communication and **ThingsML** for efficient payload encoding. Includes robust hardware initialization for the XIAO's specific TCXO and RF antenna switch.
* **`firmware_wifi`**: A standard implementation using the **ArduinoIoTCloud** library for direct Wi-Fi connectivity.

### **Cloud Functions (`/cloud_functions`)**
* **AWS Lambda**: Acts as a bridge between KPN Things and the Arduino Cloud API, handling OAuth2 authentication.
* **Datacake Decoder**: A JavaScript payload decoder used within Datacake to parse incoming SenML data from KPN.

## Setup and Deployment
See the README files within each component's directory for specific deployment instructions. A `platformio.ini` file is provided for easy compilation and library management.
