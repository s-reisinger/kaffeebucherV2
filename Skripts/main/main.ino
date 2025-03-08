#include <Arduino.h>
#include "UserDatabase.h"
#include "DatabaseWebServer.h"
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ----------------------------
// OLED Settings
// ----------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ----------------------------
// MFRC522 / RFID
// ----------------------------
#define SS_PIN  14
#define RST_PIN 13
MFRC522 rfid(SS_PIN, RST_PIN); 
MFRC522::MIFARE_Key key;  

// ----------------------------
// Rotary Encoder on GPIO8, 9
// ----------------------------
#define ENCODER_CLK  32
#define ENCODER_DT   33

// We'll poll these in loop
int lastClkState = HIGH; // or whatever the idle state is
int currentClkState;

static int selectedProductIndex = 0;


const char* ssid = "kaffeev2";
const char* password = "12345678";

String lastCardId                = "";
bool showingTemporaryMessage     = false;
unsigned long lastMessageTime    = 0;    // When we displayed a temporary message


// Forward declarations
void handleReadCard();
void displayText(const String &text);
String getCardId(byte *buffer, byte bufferSize);

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // 1) SPIFFS + load DB
    if (!initFileSystem()) {
        Serial.println("Failed to init SPIFFS. Stopping.");
        while (true) { delay(1000); }
    }
    loadProducts(); // read existing products from /products.txt

    // 2) WiFi AP
    if (!initWiFi(ssid, password)) {
        Serial.println("WiFi AP failed, stopping.");
        while (true) { delay(1000); }
    }

    // 3) RFID
    SPI.begin();
    rfid.PCD_Init();
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    // 4) OLED
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        while(true);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    // 5) Web server
    setupWebServer();

    // 6) Rotary Encoder pins
    pinMode(ENCODER_CLK, INPUT);
    pinMode(ENCODER_DT, INPUT);
    lastClkState = digitalRead(ENCODER_CLK);

    // 7) Buzzer pins
    pinMode(17, OUTPUT);
    digitalWrite(17, LOW);

    // Simple beep sequence to signal startup
    for (int i = 0; i < 3; i++) {
        digitalWrite(17, HIGH);
        delay(150);
        digitalWrite(17, LOW);
        delay(150);
    }

    // Initially show the current product
    displayCurrentProduct();
}

void loop()
{
    // Handle web requests
    handleWebServer();

   currentClkState = digitalRead(ENCODER_CLK);
    // Detect the transition from HIGH -> LOW or LOW -> HIGH
    if (currentClkState != lastClkState && currentClkState == HIGH) {
      // If CLK changed, check DT for direction
      if (digitalRead(ENCODER_DT) == currentClkState) {
          selectedProductIndex++;
      } else {
          selectedProductIndex--;
      }

      // Clamp
      if (selectedProductIndex < 0) selectedProductIndex = 0;
      if (selectedProductIndex >= 3) {
          selectedProductIndex = 3 - 1;
      }
      Serial.println(selectedProductIndex);
    } 
    lastClkState = currentClkState;
    displayCurrentProduct();

    // Check for RFID card
    handleReadCard();

    // If a temporary message was displayed, revert to the product after 3s
    if (showingTemporaryMessage && (millis() - lastMessageTime >= 3000)) {
        showingTemporaryMessage = false;
        displayCurrentProduct();
    }
}

// ----------------------------------------------------
// handleReadCard: Deduct price of selected product
// and show a temporary message (only the message).
// ----------------------------------------------------
void handleReadCard()
{
    if (!rfid.PICC_IsNewCardPresent()) {
        lastCardId = "";
        return;
    }
    if (!rfid.PICC_ReadCardSerial()) {
        return;
    }

    String cardId = getCardId(rfid.uid.uidByte, rfid.uid.size);
    // Avoid re-processing the same card
    if (cardId == lastCardId) {
        return;
    }

    // 1) Figure out the current product’s price
    int priceToDeduct = 1; // default if no products
    if (getProductCount() > 0) {
        String name;
        int cost = 0;
        if (getProductInfo(selectedProductIndex, name, cost)) {
            priceToDeduct = cost;
        }
    }

    // 2) Deduct that product price from user's credit
    String message;
    updateCreditByCardId(cardId, -priceToDeduct, message);

    // 3) Show ONLY the message for 3 seconds
    displayMessageOnly(message);
    showingTemporaryMessage = true;
    lastMessageTime         = millis();
    lastCardId              = cardId;

    // Example beep if message includes "Kaffee"
    if (message.indexOf("Kaffee") != -1) {
        digitalWrite(17, HIGH);
        delay(200);
        digitalWrite(17, LOW);
    }

    // Halt PICC
    rfid.PICC_HaltA();
}

// ----------------------------------------------------
// displayCurrentProduct: Show ONLY the product
// ----------------------------------------------------
void displayCurrentProduct()
{
    display.clearDisplay();
    display.setCursor(0, 0);

    int pCount = getProductCount();
    if (pCount == 0) {
        display.println("No product");
    } else {
        String pName;
        int pPrice;
        if (getProductInfo(selectedProductIndex, pName, pPrice)) {
            // e.g. "Coffee (100 ct)" or just the name, your choice
            String line = pName + " (" + String(pPrice) + " ct)";
            display.println(line);
        } else {
            display.println("No product");
        }
    }

    display.display();
}

// ----------------------------------------------------
// displayMessageOnly: Clear display and show ONLY 'msg'
// ----------------------------------------------------
void displayMessageOnly(const String &msg)
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(msg);
    display.display();
}

// ----------------------------------------------------
// getCardId: Convert RFID UID bytes to a String
// ----------------------------------------------------
String getCardId(byte *buffer, byte bufferSize)
{
    String cardId;
    for (byte i = 0; i < bufferSize; i++) {
        cardId += String(buffer[i]);
    }
    return cardId;
}