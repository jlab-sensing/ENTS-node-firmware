# Module Handler

Provides interface for registering code that runs on external Arduino based hardware and communicates with the stm32 through i2c interface. Modules are associated with a protobuf message type that directs the flow of messages. Each module is written as a state machine where messages from the stm32 change the current state.

## Main flow

1. Create package manager object
2. Register classes based on message type
3. Run all initialization code
4. Run infinite loop waiting for i2c interrupt

## Template Class

- Initialization
- onReceive
- onRequest
- Deinitialization
- Reset