# Step-by-Step Guide: Registering a Device on The Things Network (TTN)

This guide is based on the [RadioLib LoRaWAN Starter tutorial](https://github.com/jgromes/RadioLib/tree/master/examples/LoRaWAN/LoRaWAN_Starter) and explains how to register your custom device for OTAA activation.

## Prerequisites
- A registered account on [The Things Network](https://www.thethingsnetwork.org/get-started).
- You are logged into your regional [TTN Console](https://console.cloud.thethings.network/).
- It is highly recommended to have a LoRaWAN gateway nearby that is registered to your account. Keep the device at least 5-10 meters (15-30 feet) away from the gateway to avoid overloading the receiver.

---

### Step 1: Create an Application

An application is a container for one or more of your devices.

1.  From the TTN Console, go to **Applications**.
2.  Click **Create application**.
3.  Fill in an **Application ID**. This must be unique across all of TTN, all lowercase, and can contain numbers and dashes (e.g., `your-name-sensor-app`).
4.  The **Name** and **Description** are for your own reference.
5.  Click **Create application**.

---

### Step 2: Register Your End Device

Now, add your physical device to the application you just created.

1.  From your application's main page, click **+ Register end device**.
2.  Under "Enter end device specifics", select the **Manually** tab.
3.  **Frequency plan:** Choose the one that matches your region (e.g., `Europe 863-870 MHz (SF9 for RX2 - recommended)` for EU868).
4.  **LoRaWAN version:** Choose `LoRaWAN Specification 1.0.4`.
5.  **JoinEUI:** You can use all zeros as recommended for development: `0000000000000000`. Click **Confirm**.
6.  **DevEUI & Keys:** Click the **Generate** button next to the `DevEUI`, `AppKey`, and `NwkKey` fields to have TTN create secure, correctly formatted values for you.
7.  **End device ID:** Give your device a memorable ID (e.g., `xiao-s3-moisture-01`).
8.  Click **Register end device**.

---

### Step 3: Enable Frame Counter Resets (Crucial for Development)

For development without persistence, you must allow the frame counters to be reset on the server. Otherwise, your device will only be able to send one message after each join.

1.  On your new device's page, click on the **General settings** tab.
2.  Scroll down to the **Join settings** section and click to expand it.
3.  Check the box for **Resets join nonces** (this may also be called "Resets Frame Counters"). Acknowledge the security warning.

---

### Step 4: Copy Credentials to Your Code

Now, copy the generated credentials from the TTN console into your `secrets.h` or `secrets_abp.h` file.

1.  **EUIs (`DevEUI`, `JoinEUI`):**
    * On the device's **Overview** page, click the copy icon `📋` next to the EUI.
    * Paste it directly into your code, keeping the `0x` prefix.

2.  **Keys (`AppKey`, `NwkKey`):**
    * Click the "show" icon `👁️` next to the key you want to copy.
    * Two new icons will appear. Click the format icon `<>`. This will format the key as a C-style byte array.
    * Click the copy icon `📋`.
    * Paste the entire contents between the `{ }` brackets in your code.

**Example:**
```cpp
// Before
uint8_t appKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// After pasting from TTN
uint8_t appKey[] = { 0xAB, 0x19, 0x88, 0x43, 0x89, 0x11, 0x6A, 0x4D, 0x75, 0x49, 0xAF, 0x4D, 0x9B, 0x16, 0x45, 0x19 };

