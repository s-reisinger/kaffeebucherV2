#ifndef DATABASEWEBSERVER_H
#define DATABASEWEBSERVER_H

#include <Arduino.h>

/**
 * Initializes Wi-Fi with provided SSID and Password.
 * Returns true if connected, false otherwise.
 */
bool initWiFi(const char* ssid, const char* password);

/**
 * Sets up the WebServer routes and starts listening on port 80.
 */
void setupWebServer();

/**
 * Call this regularly (e.g., in loop()) to handle incoming web requests.
 */
void handleWebServer();

#endif // DATABASEWEBSERVER_H
