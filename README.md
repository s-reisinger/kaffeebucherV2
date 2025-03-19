# kaffeebucherV2

## Case
![image](https://github.com/user-attachments/assets/a677f507-cfe1-41e2-8de8-da08669bd397)
The Case can be modified here: **[Onshape](https://cad.onshape.com/documents/1f0dee6d78323b198e45b02d/w/3b424e32bea7d99f54bacf88/e/9b85ca8603497c2da58cbfe0?renderMode=0&uiState=67daa8de1124877ad81bda89)**

## Needed Libraries
- Adafruit SSD1306
- MFRC522

## important notes
- There will be a compile error with the MFRC522 library when trying to compile for the first time. Go to the file where the error occurs and replace `if (backData && (backLen > 0))` with `if (backData && backLen != nullptr)`.

## Wiring 
 -  **[OLED Display](https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/)**
 -  **[RFID Scanner](https://www.instructables.com/ESP32-With-RFID-Access-Control/)** Here *Pin 22* got changed to *Pin 13* and *Pin 21* got changed to *Pin 14*.

