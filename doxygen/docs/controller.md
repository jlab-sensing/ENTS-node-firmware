# Controller Interface

@note See @ref controller for the implementation of the controller interface.

The `controller` library runs on the I2C *controller* devices to add functionality from externally connection microcontroller. Communication is performed over the I2C bus with a protobuf protocol to format data. For the ents system, we use library to add support for WiFi communication and additional storage through a micro SD card. Thus, the `controller` on the stm32 is written ontop STM Hardware Abstraction Layer (HAL) libraries that interface with the I2C peripherals.

The `module_handler` library runs on the external devices that provides the interface with the host. The device is configured as an I2C *target* devices and accepts protobuf encoded commands. The library is implemented using the [Arduino Wire library](https://docs.arduino.cc/language-reference/en/functions/communication/wire/).

@image html controller_layer_diagram.png "Layer Diagram" width=20%

The communication interface is based on a software layer transaction between the *controller* (stm32) and *target* (esp32). The *controller* controls the flow of communication by transmitting a command over the I2C interface and subsequently waiting to receive data back from the *target*. *Every transmit is followed by a receive!* This ensures that *target* devices with multiple functionality, ie. WiFi and SD card, aren't competing with the I2C to transmit data. The *target* looks at the protobuf serialized message to "forward" the data to the respective module. Since the size of messages exceeds the Arduino I2C buffer (32 for the Uno, 128 for esp32 implementations) the data is chunked with a single byte flag at the start of the serialized message used to indicate if data still needs to be communicated.

@image html esp32controller2.png "Esp32 Controller" width=40%


## Sensor Modules

Sensor modules are implemented on target devices and follow a non-blocking, staged measurement protocol:

- Measurement Request: The controller initiates a measurement transaction. The target stores the ongoing measurement in its internal buffer.
- Polling for Completion: The controller periodically polls the target. If the measurement is incomplete, the target responds with a “wait” status.
- Data Retrieval: Once the measurement is ready, the controller requests the serialized measurement and timestamp. The target transmits the data without decoding it. This serialization abstracts the data format from the controller.
- Data Validation: The controller checks that the size of the serialized message fits within the allowable transmission buffer before forwarding it to the backend.

This approach is particularly important for sensors with long measurement latencies, such as those involving mechanical components or multi-sensor fusion.

We currently don't have any sensors modules implemented this way. All sensors are currently measured onboard the stm32 itself.

## Upload Modules

@note See @ref controllerWiFi for an example implementation of the WiFi upload module.

Communication modules on target devices must implement the following core interface functions:

- Connect: Initiates a connection to a network (e.g., WiFi). The controller sends necessary configuration parameters (e.g., SSID, password), and periodically polls for connection status.
- Check: Validates full bidirectional communication by requesting a backend echo or acknowledgment. This acts as a special case of the send function.
- Time Sync: Synchronizes the STM32’s RTC by initiating a network time request from the target. The controller polls the target until the timestamp is available, then updates its RTC.
- Status: Requests the current network or device state to determine connectivity and readiness.
- Send: Transmits serialized measurement data from the controller to the backend. The target device acts purely as a forwarding layer and does not decode or modify the measurement payload.

This modular design provides a flexible architecture that allows new sensor types or communication methods to be added with minimal firmware changes to the controller. It also ensures that core system responsibilities—such as timekeeping, data integrity, and transmission efficiency—are maintained independently of the specific external modules used.
