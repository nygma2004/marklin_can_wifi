# Marklin CAN-Wifi Gateway
This sketch is used to listen in and also inject new messages on the Marklin digital system CAN bus. I used it with a Central Station 2 to monitor the CAN messages and add to control switches (turnouts) on the line.

## Wiring and hardware

The unit requires a ESP32 development module and a SN65HVD230 chip. I purchased the breakout board for the transciever chip from Aliexpress: https://www.aliexpress.com/item/32686393467.html?spm=a2g0s.9042311.0.0.27424c4dMGqLEw. TX is connected to GPIO5 and RX to GPIO4. I used the below example: http://www.iotsharing.com/2017/09/how-to-use-arduino-esp32-can-interface.html

## Program Flow
1. Unit connects to the Wifi network specified (SSID, password defined as constants)
2. Sets up MDNS responder and a local webserver as port 80
3. Sets up the CAN bus connection
4. Sets up the serial output for debug messages baud 112500
5. Starts listening to the CAN messages. Messages are printed on the serial output and also on the webpage (webpage update once a second)
6. CAN messages can also be sent by pressing a button on the UI
