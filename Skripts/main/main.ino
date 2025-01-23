#include <Arduino.h>
#include "UserDatabase.h"
#include "DatabaseWebServer.h"

// Replace with your actual WiFi SSID and password
const char* ssid     = "hurensohn";
const char* password = "12345678";

void setup() {
    Serial.begin(9600);
        delay(1000);
    Serial.println("starting initFileSystem");
    // 1. Init SPIFFS for our "database"
    if (!initFileSystem()) {
        Serial.println("Failed to init SPIFFS. Stopping.");
        while (true) { delay(1000); }
    }
    // 2. Connect to WiFi
    if (!initWiFi(ssid, password)) {
        Serial.println("WiFi connection failed, stopping.");
        while (true) { delay(1000); }
    }

    String cardId = "3kl35j3k4j32";
    updateCreditByCardId(cardId, 12);

    // 3. Setup Web Server
    setupWebServer();
}

void loop() {
    // 4. Continuously handle incoming HTTP clients
    handleWebServer();
}
