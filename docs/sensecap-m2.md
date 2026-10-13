# SenseCAP M2 LoRaWAN Gateway
The SenseCAP M2 LoRaWAN gateway is available in [4G cellular (nano SIM)](https://www.seeedstudio.com/SenseCAP-Multi-Platform-LoRaWAN-Indoor-Gateway-SX1302-4G-US915-p-5600.html) and [non-cellular models](https://www.seeedstudio.com/SenseCAP-Multi-Platform-LoRaWAN-Indoor-Gateway-SX1302-US915-p-5472.html). Follow the manufacturer's instructions in the Setup Guide to configure the gateway to upload as a LoRa Basics Station, with 

## Resources

- [Setup Guide (LoRa Basics Station)](https://wiki.seeedstudio.com/Network/SenseCAP_Network/SenseCAP_M2_Multi_Platform/Tutorial/Connect-M2-Multi-Platform-Gateway-to-The-Things-Network/#connecting-via-basic-station)
    - NOTE: See [Modifications to Setup Guide Procedures](#modifications-to-setup-guide-procedures)
- [Product Page](https://wiki.seeedstudio.com/Network/SenseCAP_Network/SenseCAP_M2_Multi_Platform/SenseCAP_M2_Multi_Platform_Overview/)
- [Low Data Consumption Mode](https://wiki.seeedstudio.com/traffic_saving_config/)

## Notes
- If using a (data only) SIM card, try activating the SIM card on a phone first before putting it into the gateway.

## Modifications to Setup Guide Procedures
- TTN Configuration Step 2: Do not check "Require authenticated connection".
- TTN Configuration Step 3: Skipped (due to step 2 modification).
    - Instead, manually create the LNS and CUPS API keys:
        - LNS: Select "link as Gateway to a Gateway Server for traffic exchange, i.e. write uplink and read downlink"
            - Enter the LNS key into the TTN gateway's General settings -> LoRa Basics Station LNS Authentication Key.
        - CUPS: Select "View gateway information" AND "Retrieve secrets associated with a gateway" AND "Edit basic gateway settings"
- Gateway Configuration Step 3:
    - Server: `CUPS Server`
    - URL: `https://nam1.cloud.thethings.network:443`
- Gateway Configuration Step 6: Make sure that there is a space between the colon and "Bearer" like so `Authorization: Bearer <Your_CUPS_Key>`
