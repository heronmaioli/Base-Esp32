# Base-Esp32
🤖  IOT Project - Smart garden microcontroler firmware


### Description

Connected to the [main server](https://github.com/heronmaioli/node-js-socket-io), this firmware is able to send and receive events through websocket protocol,
using socket io as the manager lib to handle with 



### Missing Features

- Save the status of the pins on the flash memory
- Watchdog protocol
- Wifi configuration via bluetooth
- Future new features


### Features

- Check if the hardware number is already registered in the server
- Connect to NTP server to sync time on clock
- Send and receive data as JSON
- Read and transmit from a DHT sensor
- Change solid relay status through the [app](https://github.com/heronmaioli/expo-NectahGrow) commands
