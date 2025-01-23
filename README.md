# kaffeebucherV2

## Needed Libraries
- Adafruit SSD1306
- MFRC522

## important notes
- There will be a compile error with the MFRC522 library when trying to compile for the first time. Go to the file where the error occurs and replace `if (backData && (backLen > 0))` with `if (backData && backLen != nullptr)`.

## Wiring 
 -  **[OLED Display](https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/)**
 -  **[RFID Scanner](https://www.instructables.com/ESP32-With-RFID-Access-Control/)** Here *Pin 22* got changed to *Pin 13* and *Pin 21* got changed to *Pin 14*.

