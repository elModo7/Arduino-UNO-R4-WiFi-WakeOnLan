# Arduino UNO R4 Wi-Fi WakeOnLan

Basic example of an Arduino Uno R4 WiFi WoL sender on startup.
You can use this with a beautiful web UI.

> [!NOTE] 
> Remember to change this parts of the script to adapt it to your needs!

```c++
const uint8_t TARGET_MAC[6] = { 0x00, 0x14, 0x93, 0xEA, 0x23, 0x46 }; // Replace this with your device's MAC address
const char* SSID = "<Your wifi name>";
const char* PASS = "<Your wifi password>";
```