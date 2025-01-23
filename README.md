# kaffeebucherV2

## Neede Libraries
- Adafruit SSD1306
- MFRC522

## important notes
- When trying to compile for the first time their is going to be a compile error with the MFRC522 library. Go to the file where the error acours and replace `if (backData && (backLen > 0))` with `if (backData && backLen != nullptr)`
