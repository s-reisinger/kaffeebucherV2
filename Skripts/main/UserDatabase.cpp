#include "UserDatabase.h"

static const char *USER_FILE_PATH = "/users.txt";
static const char *CARD_FILE_PATH = "/cards.txt";
static const char *PRODUCT_FILE_PATH= "/products.txt";

// curl -X DELETE "http://192.168.4.1/reset?password=AigI3e2GKT83s5i"
#define RESET_PASSWORD "AigI3e2GKT83s5i" // Change this to your desired password

struct Product {
    String name;
    int priceCents;
};

// We keep products in a static global list in this .cpp
static std::vector<Product> g_products;

//--------------------------------------------
// SPIFFS init
//--------------------------------------------
bool initFileSystem()
{
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed!");
        return false;
    }
    Serial.println("SPIFFS Mount OK!");
    return true;
}


//--------------------------------------------
// Reset (clear all files)
//--------------------------------------------
void reset(String password) {
    if (password != RESET_PASSWORD) {
        Serial.println("Invalid password! Reset aborted.");
        return;
    }

    SPIFFS.remove(USER_FILE_PATH);
    SPIFFS.remove(CARD_FILE_PATH);
    SPIFFS.remove(PRODUCT_FILE_PATH);

    Serial.println("All data files cleared successfully.");
}

int getNextUserId()
{
    File file = SPIFFS.open(USER_FILE_PATH, FILE_READ);
    if (!file)
    {
        return 1; // Start with ID 1 if file doesn't exist
    }

    int maxId = 0;
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
            continue;

        int firstSep = line.indexOf(';');
        if (firstSep == -1)
            continue;

        int userId = line.substring(0, firstSep).toInt();
        maxId = max(maxId, userId);
    }
    file.close();
    return maxId + 1;
}

bool addUser(const String &username)
{
    int userId = getNextUserId();
    File file = SPIFFS.open(USER_FILE_PATH, FILE_APPEND);
    if (!file)
    {
        Serial.println("Failed to open user file for appending");
        return false;
    }

    String line = String(userId) + ";" + username + ";0\n";
    file.print(line);
    file.close();
    return true;
}

bool mapCardToUser(const String &cardId, const String &userId)
{
    File readFile = SPIFFS.open(CARD_FILE_PATH, FILE_READ);
    if (!readFile)
    {
        Serial.println("Failed to open card file for reading");
        return false;
    }

    File tempFile = SPIFFS.open("/temp_card_data.txt", FILE_WRITE);
    if (!tempFile)
    {
        Serial.println("Failed to open temp file for writing");
        readFile.close();
        return false;
    }

    bool cardUpdated = false;

    while (readFile.available())
    {
        String line = readFile.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
            continue;

        int sepIndex = line.indexOf(';');
        if (sepIndex == -1)
        {
            tempFile.println(line);
            continue;
        }

        String existingCardId = line.substring(0, sepIndex);
        String existingUserId = line.substring(sepIndex + 1);

        if (existingCardId == cardId)
        {
            cardUpdated = true;
            tempFile.println(cardId + ";" + String(userId));
        }
        else
        {
            tempFile.println(line);
        }
    }

    if (!cardUpdated)
    {
        tempFile.println(cardId + ";" + String(userId));
    }

    readFile.close();
    tempFile.close();

    SPIFFS.remove(CARD_FILE_PATH);
    SPIFFS.rename("/temp_card_data.txt", CARD_FILE_PATH);
    return true;
}

bool updateCreditByUserId(String userId, int delta,  String &message)
{
    File readFile = SPIFFS.open(USER_FILE_PATH, FILE_READ);
    if (!readFile)
    {
        Serial.println("Failed to open user file for reading");
        return false;
    }

    File tempFile = SPIFFS.open("/temp_user_data.txt", FILE_WRITE);
    if (!tempFile)
    {
        Serial.println("Failed to open temp file for writing");
        readFile.close();
        return false;
    }

    bool userFound = false;
    while (readFile.available())
    {
        String line = readFile.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
            continue;

        int firstSep = line.indexOf(';');
        int secondSep = line.indexOf(';', firstSep + 1);
        if (firstSep == -1 || secondSep == -1)
        {
            tempFile.println(line); // Malformed line
            continue;
        }

        String existingUserId = line.substring(0, firstSep);
        String username = line.substring(firstSep + 1, secondSep);
        int existingCredit = line.substring(secondSep + 1).toInt();

        if (existingUserId == userId)
        {
            int newCredit = existingCredit + delta;
            if(newCredit < 0) {
              message = "No Credit \n left!";
            } else {
              userFound = true;
              message = username + ":\n" + String(newCredit / 100.0, 2) + " EUR";
              tempFile.println(String(existingUserId) + ";" + username + ";" + String(newCredit));
            }
        }
        else
        {
          if(userId == "-1") {
            message = "Card not \n mapped!";
          }
            tempFile.println(line);
        }
    }

    readFile.close();
    tempFile.close();

    if (userFound)
    {
        SPIFFS.remove(USER_FILE_PATH);
        SPIFFS.rename("/temp_user_data.txt", USER_FILE_PATH);
    }
    else
    {
        SPIFFS.remove("/temp_user_data.txt");
    }

    return userFound;
}

bool updateCreditByCardId(String &cardId, int delta, String &message)
{
    String userId;
    if (getUserIdByCardId(cardId, userId))
    {
        return updateCreditByUserId(userId, delta, message);
    }
    message = "Unknown \n Card!";
    // Add card with no user if not found
    return mapCardToUser(cardId, "-1");
}

bool getUserIdByCardId(String &cardId, String &userId)
{
    File file = SPIFFS.open(CARD_FILE_PATH, FILE_READ);
    if (!file)
    {
        Serial.println("Failed to open card file for reading");
        return false;
    }

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
            continue;

        int sepIndex = line.indexOf(';');
        if (sepIndex == -1)
            continue;

        String existingCardId = line.substring(0, sepIndex);
        String existingUserId = line.substring(sepIndex + 1);

        if (existingCardId == cardId)
        {
            userId = existingUserId;
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

bool getUnmappedCards(String &unmappedCards)
{
    unmappedCards = ""; // Clear output

    File file = SPIFFS.open(CARD_FILE_PATH, FILE_READ);
    if (!file)
    {
        Serial.println("Failed to open card file for reading");
        return false;
    }

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
            continue;

        int sepIndex = line.indexOf(';');
        if (sepIndex == -1)
            continue;

        String cardId = line.substring(0, sepIndex);
        int userId = line.substring(sepIndex + 1).toInt();

        if (userId == -1)
        {
            unmappedCards += cardId + "\n";
        }
    }

    file.close();
    return true;
}

bool getAllUsers(String &usersList)
{
    usersList = ""; // Clear output string

    File file = SPIFFS.open(USER_FILE_PATH, FILE_READ);
    if (!file)
    {
        Serial.println("Failed to open user file for reading (getAllUsers)");
        return false;
    }

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
            continue; // Skip empty lines

        // Format: userId;username;credit
        int firstSep = line.indexOf(';');
        int secondSep = line.indexOf(';', firstSep + 1);
        if (firstSep == -1 || secondSep == -1)
        {
            // Malformed line, skip it
            continue;
        }

        String userId = line.substring(0, firstSep);
        String username = line.substring(firstSep + 1, secondSep);
        String credit = line.substring(secondSep + 1);

        // Append "userId,username,credit" to usersList (or any desired format)
        usersList += userId + "," + username + "," + credit + "\n";
    }

    file.close();
    return true;
}

bool deleteCardById(const String &cardId)
{
    File readFile = SPIFFS.open(CARD_FILE_PATH, FILE_READ);
    if (!readFile)
    {
        Serial.println("Failed to open card file for reading");
        return false;
    }

    File tempFile = SPIFFS.open("/temp_card_data.txt", FILE_WRITE);
    if (!tempFile)
    {
        Serial.println("Failed to open temp file for writing");
        readFile.close();
        return false;
    }

    bool cardFound = false;

    while (readFile.available())
    {
        String line = readFile.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) {
            continue; // Skip empty lines
        }

        int sepIndex = line.indexOf(';');
        if (sepIndex == -1)
        {
            // Malformed line: write back as-is
            tempFile.println(line);
            continue;
        }

        String existingCardId = line.substring(0, sepIndex);

        if (existingCardId == cardId)
        {
            // We've found the card; skip writing this line to effectively "delete" it
            cardFound = true;
        }
        else
        {
            // Keep all other cards
            tempFile.println(line);
        }
    }

    readFile.close();
    tempFile.close();

    // If card was found and removed, finalize changes
    if (cardFound)
    {
        SPIFFS.remove(CARD_FILE_PATH);
        SPIFFS.rename("/temp_card_data.txt", CARD_FILE_PATH);
    }
    else
    {
        // Card not found -> discard temp
        SPIFFS.remove("/temp_card_data.txt");
    }

    return cardFound;
}

//=============================================================================
//                              PRODUCT Management
//=============================================================================
bool loadProducts()
{
    g_products.clear();
    File file = SPIFFS.open(PRODUCT_FILE_PATH, FILE_READ);
    if (!file) {
        Serial.println("No products file found; starting empty product list.");
        return false;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.isEmpty()) continue;

        int sepIndex = line.indexOf(';');
        if (sepIndex == -1) {
            // Malformed line
            continue;
        }

        String name  = line.substring(0, sepIndex);
        int price    = line.substring(sepIndex + 1).toInt();

        Product p;
        p.name       = name;
        p.priceCents = price;
        g_products.push_back(p);
    }

    file.close();
    Serial.println("Products loaded.");
    return true;
}

bool saveProducts()
{
    File file = SPIFFS.open(PRODUCT_FILE_PATH, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open products file for writing.");
        return false;
    }

    for (auto &prod : g_products) {
        file.printf("%s;%d\n", prod.name.c_str(), prod.priceCents);
    }

    file.close();
    Serial.println("Products saved.");
    return true;
}

bool addProduct(const String &name, int priceCents)
{
    // Just add to vector
    Product p;
    p.name = name;
    p.priceCents = priceCents;
    g_products.push_back(p);

    // Persist
    return saveProducts();
}

bool deleteProduct(int index)
{
    if (index < 0 || index >= (int)g_products.size()) {
        return false;
    }
    g_products.erase(g_products.begin() + index);
    return saveProducts();
}

bool getAllProducts(String &productsList)
{
    productsList = "";
    for (auto &p : g_products) {
        // Format each line as "Name;Price\n"
        productsList += p.name + ";" + String(p.priceCents) + "\n";
    }
    return true;
}

int getProductCount()
{
    return (int)g_products.size();
}

bool getProductInfo(int index, String &name, int &priceCents)
{
    if (index < 0 || index >= (int)g_products.size()) {
        return false;
    }
    name       = g_products[index].name;
    priceCents = g_products[index].priceCents;
    return true;
}

