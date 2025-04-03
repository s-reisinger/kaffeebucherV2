# kaffeebucherV2
This Project uses an ESP32 to make a booking System for different Products. As it is used for Coffee here, it is called **Kaffeebucher**. This is the second version, with the first one using a Raspberry PI.

## Hardware
Here you find everything you need to build it.

### Components
 - **[ESP32](https://de.aliexpress.com/item/1005006336964908.html?spm=a2g0o.productlist.main.4.57481b32Pr6LjD&aem_p4p_detail=202503190428471195379678556000002160445&algo_pvid=cfc3ee26-adf2-4dd7-9dca-7bde46350526&algo_exp_id=cfc3ee26-adf2-4dd7-9dca-7bde46350526-3&pdp_ext_f=%7B%22order%22%3A%2224171%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21EUR%217.98%212.67%21%21%2161.64%2120.63%21%40210388c917423837274915284e54bc%2112000036806447870%21sea%21AT%214052680276%21X&curPageLogUid=1CEvHKn0o6MO&utparam-url=scene%3Asearch%7Cquery_from%3A&search_p4p_id=202503190428471195379678556000002160445_1)**
 - **[OLED-Display](https://de.aliexpress.com/item/1005006117094887.html?spm=a2g0o.productlist.main.5.7dfb7600m4JhWt&algo_pvid=d6de378b-8002-469d-a86a-dc739a51d447&algo_exp_id=d6de378b-8002-469d-a86a-dc739a51d447-4&pdp_ext_f=%7B%22order%22%3A%22346%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21EUR%213.16%211.90%21%21%2124.40%2114.64%21%402103892f17423838623757920e3034%2112000035826763351%21sea%21AT%214052680276%21X&curPageLogUid=muTsAkR4glKQ&utparam-url=scene%3Asearch%7Cquery_from%3A)**
 - **[RFID-RC522](https://de.aliexpress.com/item/1005007038131464.html?spm=a2g0o.productlist.main.3.837469cajh0PQo&algo_pvid=9eabcaa0-b890-422b-8ba1-f11500df8ed9&algo_exp_id=9eabcaa0-b890-422b-8ba1-f11500df8ed9-2&pdp_ext_f=%7B%22order%22%3A%2210643%22%2C%22eval%22%3A%221%22%2C%22orig_sl_item_id%22%3A%221005007038131464%22%2C%22orig_item_id%22%3A%221005006894076021%22%7D&pdp_npi=4%40dis%21EUR%213.75%211.50%21%21%2128.92%2111.57%21%402103847817423838968168535e3711%2112000039179323989%21sea%21AT%214052680276%21X&curPageLogUid=uiwE4rtg3dv3&utparam-url=scene%3Asearch%7Cquery_from%3A)**
 - **[Rotary Encoder](https://de.aliexpress.com/item/1005006459289182.html?spm=a2g0o.productlist.main.16.58a4sZNUsZNUB2&algo_pvid=d2c15ac2-acfe-4715-8172-b26baeb5ea6b&algo_exp_id=d2c15ac2-acfe-4715-8172-b26baeb5ea6b-15&pdp_ext_f=%7B%22order%22%3A%22295%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21EUR%214.16%211.33%21%21%2132.14%2110.28%21%40211b613917423732448715925e1076%2112000037273588086%21sea%21AT%214052680276%21X&curPageLogUid=fEC39mkGTLN4&utparam-url=scene%3Asearch%7Cquery_from%3A)**
 - **[Buzzer](https://de.aliexpress.com/item/1005002576043967.html?spm=a2g0o.productlist.main.4.977a6e66Y7o2Y4&aem_p4p_detail=202503190434151538553962886960002012538&algo_pvid=0ba262aa-6b30-4807-8dbd-d9d98bdae037&algo_exp_id=0ba262aa-6b30-4807-8dbd-d9d98bdae037-3&pdp_ext_f=%7B%22order%22%3A%2246%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21EUR%211.73%211.51%21%21%211.85%211.61%21%4021038e6617423840550788870e1859%2112000021227023571%21sea%21AT%214052680276%21X&curPageLogUid=OnWukqnw2UYt&utparam-url=scene%3Asearch%7Cquery_from%3A&search_p4p_id=202503190434151538553962886960002012538_1)**
 - **[18650 shield](https://de.aliexpress.com/item/4001251319520.html?spm=a2g0o.productlist.main.2.1d3515cbmkrBJT&algo_pvid=b87e4da4-a3e9-4deb-a5a2-491527c90a82&algo_exp_id=b87e4da4-a3e9-4deb-a5a2-491527c90a82-1&pdp_ext_f=%7B%22order%22%3A%2213%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21EUR%211.84%211.31%21%21%211.97%211.40%21%4021038df617423840921186410e4130%2110000015469373028%21sea%21AT%214052680276%21X&curPageLogUid=xA1n3U4GZWUG&utparam-url=scene%3Asearch%7Cquery_from%3A)**
 - **[18650 Cell](https://www.akkuteile.de/en/eve-inr18650/26v-with-2550mah-max-7-5a-3-6v-3-7v-lithium-ion-battery_100853_3125)**
 - **[USB-C Breakout board](https://de.aliexpress.com/item/1005007038016920.html?spm=a2g0o.productlist.main.4.2b923073DMgd0S&aem_p4p_detail=202503190439001993094405293200002143647&algo_pvid=af440eae-5092-40dd-821f-feb6c10d7084&algo_exp_id=af440eae-5092-40dd-821f-feb6c10d7084-3&pdp_ext_f=%7B%22order%22%3A%2245%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21EUR%211.92%211.61%21%21%2114.82%2112.45%21%40210388c917423843400513635e54c1%2112000039179955237%21sea%21AT%214052680276%21X&curPageLogUid=u9wT0uriIdee&utparam-url=scene%3Asearch%7Cquery_from%3A&search_p4p_id=202503190439001993094405293200002143647_1)**
### Wiring 
![image](https://github.com/user-attachments/assets/1bc05d6b-3b50-41a5-b08b-484b70d81df5)

### Case
![image](https://github.com/user-attachments/assets/a677f507-cfe1-41e2-8de8-da08669bd397)

The Case can be downloaded as a **.3mf** File here: **[Github](https://github.com/s-reisinger/kaffeebucherV2/tree/main/hardware)**

The Case can be modified here: **[Onshape](https://cad.onshape.com/documents/1f0dee6d78323b198e45b02d/w/3b424e32bea7d99f54bacf88/e/9b85ca8603497c2da58cbfe0?renderMode=0&uiState=67daa8de1124877ad81bda89)**

## Software
Here you find everything you need to make the software run.
### Needed Libraries
- Adafruit SSD1306
- MFRC522

### important notes
- There will be a compile error with the MFRC522 library when trying to compile for the first time. Go to the file where the error occurs and replace `if (backData && (backLen > 0))` with `if (backData && backLen != nullptr)`.
