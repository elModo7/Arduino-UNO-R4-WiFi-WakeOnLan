// UNO R4 WiFi — Wake-on-LAN sender
// Requires: WiFiS3, WiFiUdp
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

WiFiUDP Udp;
const uint16_t WOL_PORT1 = 9; // I added both ports for reliability
const uint16_t WOL_PORT2 = 7;
const uint16_t RETRIES = 5; // Send magick packet 5 times for reliability
ArduinoLEDMatrix matrix; // Display in matrix

// >>> Put the target machine's MAC address here <<<
const uint8_t TARGET_MAC[6] = { 0x00, 0x14, 0x93, 0xEA, 0x23, 0x46 }; // Replace this with your device's MAC address

// (Optional) If you want the board to connect itself, fill these:
const char* SSID = "<Your wifi name>";
const char* PASS = "<Your wifi password>";

// Build the UDP WOL payload: 6 bytes of 0xFF + 16 repetitions of the MAC.
void buildMagicPacket(uint8_t* buf, const uint8_t mac[6]) {
  // 6×0xFF
  for (int i = 0; i < 6; i++) buf[i] = 0xFF;
  // 16×MAC
  for (int i = 0; i < 16; i++) {
    memcpy(buf + 6 + i*6, mac, 6);
  }
}

// Compute LAN broadcast address from local IP + subnet mask.
IPAddress broadcastAddress(IPAddress ip, IPAddress mask) {
  IPAddress b;
  for (int i = 0; i < 4; i++) {
    b[i] = (ip[i] & mask[i]) | (~mask[i] & 0xFF);
  }
  return b;
}

void sendWOL(const uint8_t mac[6], IPAddress bcast, uint16_t port) {
  uint8_t pkt[6 + 16*6]; // 102 bytes
  buildMagicPacket(pkt, mac);

  // A local UDP port to bind from (any high port is fine)
  Udp.begin(40000);

  // Send to broadcast:port
  Udp.beginPacket(bcast, port);
  Udp.write(pkt, sizeof(pkt));
  Udp.endPacket();

  // Not strictly required to stop, but keeps things tidy
  Udp.stop();
}

void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  if (SSID && SSID[0]) {
    Serial.print("Connecting to "); Serial.println(SSID);
    WiFi.begin(SSID, PASS);
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
      delay(250);
      Serial.print('.');
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { /* wait for USB */ }

  ensureWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi OK. IP: "); Serial.println(WiFi.localIP());
    IPAddress bcast = broadcastAddress(WiFi.localIP(), WiFi.subnetMask());
    Serial.print("Broadcast: "); Serial.println(bcast);

    // Send immediately on boot:
    matrix.begin();
    matrix.beginDraw();

    matrix.stroke(0xFFFFFFFF);
    matrix.textScrollSpeed(50);

    const char text[] = "   WoL!";
    matrix.textFont(Font_5x7);
    matrix.beginText(0, 1, 0xFFFFFF);
    matrix.println(text);
    matrix.endText(SCROLL_LEFT);
    matrix.endDraw();
	for (int i = 0; i < RETRIES; ++i) {
		sendWOL(TARGET_MAC, bcast, WOL_PORT1);
		sendWOL(TARGET_MAC, bcast, WOL_PORT2);
	}
    Serial.println("WOL packets sent.");

    Serial.println("\nType 's' + Enter in Serial Monitor to send again.");
  } else {
    Serial.println("Not connected to WiFi. Fill SSID/PASS or ensure pre-connected.");
  }
}

void loop() {
  // Allow manual re-send from Serial Monitor
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 's' || c == 'S') {
      IPAddress bcast = broadcastAddress(WiFi.localIP(), WiFi.subnetMask());
	  for (int i = 0; i < RETRIES; ++i) {
	    sendWOL(TARGET_MAC, bcast, WOL_PORT1);
	    sendWOL(TARGET_MAC, bcast, WOL_PORT2);
	  }
      Serial.println("WOL packets sent.");
    }
  }
}
