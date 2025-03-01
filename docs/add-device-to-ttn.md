# Adding device to The Things Network {#add-device-to-ttn}

The following tutorial covers the end to end procedure for setting up the device with [The Things Network](https://www.thethingsnetwork.org/) (TTN). The process covers (1) creating an account to get access to TTN resouces, (2) Adding an end device to the `soil-power-sensor` application, and (3) verifying uplinks and downlinks from the serial link.

## Register for an account

1. Open https://www.thethingsnetwork.org/ in a web browser.
2. Click sign-up and select "Join The Things Network". This creates an account on the community (free) version of TTN.
3. Login to your account.
4. Navigate to the console and select "North America 1". This registers an account under the `nam1` region.
5. Contact an admin to gain access to soil power sensor (SPS) TTN resources.
6. Verify access to the `soil-power-sensor` TTN application.

## Add the end device

1. Flash the firmware for the stm32 module.
2. Open a serial connection. And press the `RST` button. You should see a printout of keys as follows. The keys are in hex and can be directly copied to TTN.

```
###### AppKey:      XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX
###### NwkKey:      XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX
###### AppSKey:     XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX
###### NwkSKey:     XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX
###### DevEUI:      XX:XX:XX:XX:XX:XX:XX:XX
###### AppEUI:      XX:XX:XX:XX:XX:XX:XX:XX
###### DevAddr:     XX:XX:XX:XX
```

3. Navigate to *Applications -> End devices -> Register end device*.
4. Select *Enter end device specifics manually* under *Input method*.
5. Select *United States 902-928 MHz, FSB 2 (used by TTN)* for *Frequency plan*.
6. Select *LoRaWAN Specification 1.0.3* for *LoRaWAN Version*.
7. Enter *Provisioning* information collected from the serial connection. Input `AppUEI` for *JoinUEI* in TTN.
8. Leave the generated *End device ID* as default.
9. Click *Register end device*. The following message shows the device has been registered using OTAA. Verify the device has joined on the TTN portal.

```
###### = JOINED = OTAA =====================
###### MCRootKey:   XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX
###### MCKEKey:     XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX
###### AppSKey:     XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX
###### NwkSKey:     XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX
###### DBIntKey:    XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX
###### DevEUI:      XX:XX:XX:XX:XX:XX:XX:XX
###### AppEUI:      XX:XX:XX:XX:XX:XX:XX:XX
###### DevAddr:     XX:XX:XX:XX
20s042:TX on freq 904700000 Hz at DR 0
20s044:SEND REQUEST
20s333:MAC txDone
25s316:RX_1 on freq 925700000 Hz at DR 10
25s418:MAC rxDone
```

## Verify uplink/downlinks

**Uplinks:** By default the device should upload on a periodic interval. The following is an example log message showing a successful uplink. Verify within TTN a packet was received.

```
###### ========== MCPS-Confirm =============
120s042:TX on freq 904500000 Hz at DR 3
120s044:SEND REQUEST
120s091:MAC txDone
125s071:RX_1 on freq 925100000 Hz at DR 13
125s114:IRQ_RX_TX_TIMEOUT
125s114:MAC rxTimeOut
126s085:RX_2 on freq 923300000 Hz at DR 8
126s153:IRQ_RX_TX_TIMEOUT
126s153:MAC rxTimeOut
```

**Downlink:** The TTN console can simulate downlink messages and verification can be performed as follows:

1. Navigate to *Applications* -> *End devices* -> *Select your device* -> *Messaging* -> *Downlink*.
2. Select *Replace downlink queue* under *Insert Mode*.
3. Set *FPort* to `1`.
4. Set *Payload* to `FF` (Can be any payload of your choosing).
5. Click *Schedule downlink*.
6. Verify messages was received in the serial monitor. The following is an example log message of a successful downlink. Notice there are no `rxTimeOut` messages. See LoRaWAN specifications for a class C device for information on the downlink protocol.

```
###### ========== MCPS-Confirm =============
340s042:TX on freq 904500000 Hz at DR 3
340s044:SEND REQUEST
340s091:MAC txDone
345s071:RX_1 on freq 925100000 Hz at DR 13
345s103:MAC rxDone
```
