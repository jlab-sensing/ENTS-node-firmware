# LoRaWAN Fport Usage via TTN


---

## Overview

TTN (The Things Network) enables communications in ranges longer than what WIFI would allow for with the help of LoRAWAN. Downlinks through TTN in particular allow the user to send a message to an end device, allowing for remote operations. To easily distinguish the functionality of an intended message, Fports from 3-223 (excluding 200,201, and 202) are able to freely be used as tags. For example, solenoid operations use a Fport of 3. Within each message is a payload of bytes which can be interpreted depending on how implementation for payload handling was executed.

## Fport Map

| Component                     | Fport  | Meaning for each byte | Length(Bytes) |
|-------------------------------|--------|-----------------------|---------------|
| Solenoid                      |    3   | On/Off (1/0), Pin     | 2             |
