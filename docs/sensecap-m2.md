# SenseCAP M2 LoRaWAN Gateway
The SenseCAP M2 LoRaWAN gateway is available in [4G cellular (nano SIM)](https://www.seeedstudio.com/SenseCAP-Multi-Platform-LoRaWAN-Indoor-Gateway-SX1302-4G-US915-p-5600.html) and [non-cellular models](https://www.seeedstudio.com/SenseCAP-Multi-Platform-LoRaWAN-Indoor-Gateway-SX1302-US915-p-5472.html).

## Instructions
- Follow the [Quick Start](https://wiki.seeedstudio.com/quick_start_with_M2_MP/) guide to gain access to the configuration console and set up internet connectivity.
    - If using a (data only) SIM card, try activating the SIM card on a phone first and confirming that it provides data before putting it into the gateway.
- Then, follow the [Setup Guide (LoRa Basics Station)](https://wiki.seeedstudio.com/Network/SenseCAP_Network/SenseCAP_M2_Multi_Platform/Tutorial/Connect-M2-Multi-Platform-Gateway-to-The-Things-Network/#connecting-via-basic-station) while referring to the [Modifications to Setup Guide Procedures](#modifications-to-setup-guide-procedures).

## Modifications to Setup Guide Procedures
- Note: These modifications assume you are setting up the gateway in North America.
- TTN Configuration
    - Step 1: After entering and checking the EUI, set the frequency plan to FSB 2.
    - Step 3: Check both boxes to generate API keys for CUPS and LNS.
    - After registering the gateway, enter in the location and any other miscellaneous information on TTN.
- Gateway Configuration
    - `System --> Backup / Flash Firmware`: Upgrade OS.
    - `System --> System`: Set timezone. Save and Apply.
        - Optionally set up the [Low Data Consumption Mode](https://wiki.seeedstudio.com/traffic_saving_config/). White List Config is on the `LoRa --> LoRa Network --> Packet Filter` tab. Ping parameters are located on the `Network --> Multi WAN` page within the WAN you are using for upstream internet connectivity (WAN for ethernet, WWAN for wifi).
    - Step 3:
        - Server: `CUPS Server`
        - CUPS URL: `https://nam1.cloud.thethings.network:443`
        - (unused) LNS URL: `wss://nam1.cloud.thethings.network:8887`
    - Gateway Configuration Step 6: Make sure that there is a space between the colon and "Bearer" like so `Authorization: Bearer <Your_CUPS_Key>`

## Resources and References

- [Quick Start](https://wiki.seeedstudio.com/quick_start_with_M2_MP/)
- [Setup Guide (LoRa Basics Station)](https://wiki.seeedstudio.com/Network/SenseCAP_Network/SenseCAP_M2_Multi_Platform/Tutorial/Connect-M2-Multi-Platform-Gateway-to-The-Things-Network/#connecting-via-basic-station)
    - NOTE: See [Modifications to Setup Guide Procedures](#modifications-to-setup-guide-procedures)
- [Product Page](https://wiki.seeedstudio.com/Network/SenseCAP_Network/SenseCAP_M2_Multi_Platform/SenseCAP_M2_Multi_Platform_Overview/)
- [Low Data Consumption Mode](https://wiki.seeedstudio.com/traffic_saving_config/)