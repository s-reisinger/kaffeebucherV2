#include <Arduino.h>
#include "UserDatabase.h"
#include "DatabaseWebServer.h"
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SS_PIN 14
#define RST_PIN 13
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];

// Replace with your actual WiFi SSID and password
const char* ssid     = "hurensohn";
const char* password = "12345678";

unsigned long lastUpdateTime = 0; // To track the last time the display was updated
bool resetMessageScheduled = false; // To track if "Ready..." is scheduled
String lastCardId = "";// Variable to store the last read card ID

void setup() {
    Serial.begin(9600);
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

    SPI.begin(); // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522 

    for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
    }

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
      Serial.println(F("SSD1306 allocation failed"));
      for(;;);
    }
    delay(2000);
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    displayText("Ready...");
    // 3. Setup Web Server
    setupWebServer();
}

void loop() {
    handleWebServer();
    handleReadCard();

    //to reset display
    if (resetMessageScheduled && millis() - lastUpdateTime >= 3000) {
      displayText("Ready...");
      resetMessageScheduled = false; // Reset the flag
  }
}

void handleReadCard() {
  if (!rfid.PICC_IsNewCardPresent()) {
    // Reset lastCardId if no card is present
    lastCardId = ""; 
    return;
  }

  if(resetMessageScheduled){
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  String cardId = getCardId(rfid.uid.uidByte, rfid.uid.size);

  if (cardId == lastCardId) {
    return;
  }
  
  String message = "noMessage";
  updateCreditByCardId(cardId, -1, message);
  Serial.println(cardId);
  displayText(message);

  // Schedule the "Ready..." message update after 3 seconds
  lastUpdateTime = millis();
  resetMessageScheduled = true;

  lastCardId = cardId;

  rfid.PICC_HaltA();
}

String getCardId(byte *buffer, byte bufferSize) {
  String cardId = "";
  for (byte i = 0; i < bufferSize; i++) {
    cardId = cardId + buffer[i];
  }
  return cardId;
}

void displayText(String text) {
      display.clearDisplay();
      display.setCursor(0, 10);
    display.println(text);
    display.display();
}
