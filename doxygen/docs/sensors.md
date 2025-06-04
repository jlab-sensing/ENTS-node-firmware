# Sensor Measurements and Uploads

\section sensorbus Sensors Bus

To meet the internal communication requirement, a dedicated I2C bus connects internal peripherals 
including the ADC, FRAM, and ESP32. This bus also features a Qwiic-compatible connector to enable external sensor integration or peripheral expansion without hardware modification. For environmental sensors conforming to industry standards, an SDI-12 interface is provided via dedicated hardware support. 

\section retention Data Retention

To ensure reliable operation in environments with intermittent power and network connectivity, the ENTS platform implements a robust measurement buffering mechanism. This design ensures that data is retained and eventually uploaded even in the presence of failures. All measurements are serialized and stored in a circular buffer implemented on a non-volatile FRAM chip via an I2C interface. This approach modularizes the sensing and transmission pipeline, allowing additional sensor types and communication interfaces to be integrated without impacting core buffering logic.

\subsection serialization Serialization and Storage

Each measurement is serialized into a byte array, which includes a variable-length payload depending on the sensors configured. Before writing the payload, the system stores a 1-byte length prefix, enabling efficient reading during uploads. This fixed overhead allows the device to distinguish and parse each measurement block during retrieval.

When a new measurement is available:

- The system checks that sufficient space exists between the write and clear pointers.
- The length of the serialized packet is written to FRAM at the current write pointer location.
- The serialized data follows immediately after.
- The write pointer is advanced by the total number of bytes written.

This design ensures that only successfully stored measurements advance the write pointer, avoiding partially written or corrupted data blocks.

\subsection uploads "Upload Process and RAM Buffering"

During data transmission:

- The system reads the length field at the read pointer to determine the size of the next measurement.
- The corresponding number of bytes are fetched from FRAM into STM32 RAM.
- The data is transmitted using the selected communication interface.
- A temporary index buffer on the backend records the relative position of each successful upload.

Multiple measurements may be batched into a single transmission if bandwidth and payload size permit. This batching has varying potential benefits depending on the communication interface. When uploading through LoRaWAN measurement batching reduces the number of LoRaWAN MAC layer bytes and total number of transmissions that can extend battery life. With WiFi, the throughput of the sensor measurements can be increased.

\subsection downlinks Downlink Confirmation Protocol

@todo Needs to be implemented

The ENTS node has built-in support for LoRaWAN, which has significant restrictions on the frequency and size of payloads [FIXME Cite LoRaWAN FCC regs]. Additional restrictions are placed on the communication by network servers, such as The Things Network (TTN). We design the system around these constraints as we assume LoRaWAN is the most restrictive communication interface the adopted policies can be applied to other communication methods to reduce power consumption.

To comply with the TTN fair use policy, which allows a maximum of 10 downlinks per device per 24-hour period, ENTS employs an adaptive downlink strategy. The backend computes the downlink frequency based on the current upload interval to remain within policy constraints. Each downlink includes a compact list of indices representing successfully received measurements since the last acknowledgment.

Upon receiving a confirmation downlink that is not empty:

- The device resets the read pointer to the clear pointer.
- Measurements are re-read from FRAM.
- Any measurement whose index is not present in the confirmation message is retransmitted.
- The clear pointer is updated to reflect the last contiguous block of confirmed measurements.

This mechanism ensures that only missing or corrupted measurements are retransmitted, preserving bandwidth and reducing power usage.

\subsection faults Fault Tolerance and Assumptions

The buffering strategy assumes that the majority of uplink transmissions will succeed over time, allowing the buffer to drain naturally. If network conditions deteriorate or power losses prevent regular uploads, the buffer will eventually fill, and data acquisition will pause to prevent overwriting unconfirmed data. The size of the FRAM buffer is therefore a key design parameter, directly tied to deployment longevity and fault resilience. [TODO Write about the tolerance time given size of FRAM].
