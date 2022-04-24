# Base-Esp32-with-Node
🤖 The ESP32 firmware to Nectah Grow app.


### Description

Connected to the [main server](https://github.com/heronmaioli/node-js-socket-io), this firmware are able to send and receive events through websocket protocol,
using socket io as the manager lib to handle with.


### Features

- Check if the hardware number is already registered in the server
- Connect to NTP server to sync time on clock
- Send and receive data as JSON
- Read and transmit from a DHT sensor
- Change solid relay status through the app commands
