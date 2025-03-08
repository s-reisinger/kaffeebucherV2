#include "DatabaseWebServer.h"
#include <WiFi.h>         // For ESP32 WiFi
#include <WebServer.h>    // For the built-in ESP32 WebServer class
#include "UserDatabase.h" // Our file-based DB

// Create a single WebServer instance on port 80
static WebServer server(80);

// --------------------------------------------------------------------
// 1. WiFi Initialization
// --------------------------------------------------------------------
bool initWiFi(const char* ssid, const char* password)
{
    Serial.println("Starting Wi-Fi in AP mode...");

    // Set up the access point (returns true on success)
    bool apStarted = WiFi.softAP(ssid, password);
    if (!apStarted) {
        Serial.println("Failed to start Soft AP.");
        return false;
    }

    Serial.println("Soft AP started successfully!");

    // Print out the IP address of the AP
    IPAddress AP_IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(AP_IP);

    // Optionally: limit the number of connected clients, 
    // configure channel, etc. For example:
    // WiFi.softAPConfig(local_IP, gateway, subnet);

    // Return success
    return true;
}


// --------------------------------------------------------------------
// 2. HTTP Handlers
// --------------------------------------------------------------------

// Home page: displays list of users + forms for Add User and Set CardKey
static void handleRoot()
{
    String html = F("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
    html += F("<title>ESP32 User Database</title>");
    html += F("<style>");
    html += F("body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 20px; text-align: center; }");
    html += F(".container { width: 90%; max-width: 800px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1); }");
    html += F("h1, h2, h3 { color: #333; }");
    html += F("table { width: 100%; border-collapse: collapse; margin-top: 10px; }");
    html += F("th, td { padding: 10px; border: 1px solid #ddd; text-align: left; }");
    html += F("th { background: #007BFF; color: white; }");
    html += F("tr:nth-child(even) { background: #f9f9f9; }");
    html += F("form { margin-top: 20px; padding: 15px; background: #f8f9fa; border-radius: 5px; }");
    html += F("input[type='text'], input[type='number'] { width: 80%; padding: 8px; margin: 5px 0; border: 1px solid #ccc; border-radius: 4px; }");
    html += F("input[type='submit'] { background: #007BFF; color: white; border: none; padding: 10px; cursor: pointer; border-radius: 5px; width: 85%; }");
    html += F("input[type='submit']:hover { background: #0056b3; }");
    html += F("</style>");
    html += F("</head><body><div class='container'>");
    html += F("<h1>ESP32 User Database</h1>");

    // ---------------------------
    // 1) Show all users
    // ---------------------------
    {
        String allUsers;
        if (getAllUsers(allUsers)) {
            html += F("<h2>All Users</h2>");
            html += F("<table><tr><th>User ID</th><th>Username</th><th>Credit</th></tr>");

            int startIndex = 0;
            while (true) {
                int lineEnd = allUsers.indexOf('\n', startIndex);
                if (lineEnd == -1) break;

                String line = allUsers.substring(startIndex, lineEnd);
                startIndex = lineEnd + 1;

                // line = "userId,username,credit"
                int firstComma  = line.indexOf(',');
                int secondComma = line.indexOf(',', firstComma+1);
                if (firstComma == -1 || secondComma == -1) continue;

                String userId   = line.substring(0, firstComma);
                String username = line.substring(firstComma+1, secondComma);
                String credit   = line.substring(secondComma+1);

                html += "<tr><td>" + userId + "</td><td>" + username + "</td><td>" + credit + "</td></tr>";
            }
            html += F("</table>");
        } else {
            html += F("<p>Could not read users from database.</p>");
        }
    }
    html += F("<hr>");

    // ---------------------------
    // 2) Show unmapped cards
    // ---------------------------
    {
        String unmappedCards;
        if (getUnmappedCards(unmappedCards)) {
            html += F("<h2>Unmapped Cards</h2>");
            html += F("<table><tr><th>Card ID</th><th>Action</th></tr>");

            int startIndex = 0;
            while (true) {
                int lineEnd = unmappedCards.indexOf('\n', startIndex);
                if (lineEnd == -1) break;

                String cardId = unmappedCards.substring(startIndex, lineEnd);
                startIndex = lineEnd + 1;

                html += "<tr>";
                html += "<td>" + cardId + "</td>";
                // Delete button for unmapped card
                html += "<td>";
                html += "<form action='/deleteUnmappedCard' method='POST'>";
                html += "<input type='hidden' name='cardId' value='" + cardId + "'>";
                html += "<input type='submit' value='Delete'>";
                html += "</form>";
                html += "</td>";
                html += "</tr>";
            }
            html += F("</table>");
        } else {
            html += F("<p>Could not read unmapped cards from database.</p>");
        }
    }
    html += F("<hr>");

    // ---------------------------
    // 3) Add a new user form
    // ---------------------------
    html += F("<h2>Add a New User</h2>");
    html += F("<form action='/addUser' method='POST'>");
    html += F("Username: <input type='text' name='username' required><br>");
    html += F("<input type='submit' value='Add User'>");
    html += F("</form>");
    html += F("<hr>");

    // ---------------------------
    // 4) Map card to user
    // ---------------------------
    html += F("<h2>Map a Card to a User</h2>");
    html += F("<form action='/mapCard' method='POST'>");
    html += F("Card ID: <input type='text' name='cardId' required><br>");
    html += F("User ID: <input type='text' name='userId' required><br>");
    html += F("<input type='submit' value='Map Card'>");
    html += F("</form>");
    html += F("<hr>");

    // ---------------------------
    // 5) Update credit
    // ---------------------------
    html += F("<h2>Update Credit</h2>");
    html += F("<form action='/updateCredit' method='POST'>");
    html += F("User ID: <input type='text' name='userId' required><br>");
    html += F("Delta: <input type='number' name='delta' required><br>");
    html += F("<input type='submit' value='Update Credit'>");
    html += F("</form>");

    html += F("<hr>");

    // ---------------------------
    // 6) Products Section
    // ---------------------------
    {
        html += F("<h2>Products</h2>");
        html += F("<table><tr><th>Name</th><th>Price (cents)</th><th>Action</th></tr>");

        String productsList;
        if (getAllProducts(productsList)) {
            int startIdx = 0;
            while (true) {
                int lineEnd = productsList.indexOf('\n', startIdx);
                if (lineEnd == -1) break;

                String line = productsList.substring(startIdx, lineEnd);
                startIdx = lineEnd + 1;

                // "Name;Price"
                int sep = line.indexOf(';');
                if (sep == -1) continue;

                static int productIndexCounter = 0; // not persistent, just for the loop
                String productName = line.substring(0, sep);
                String priceStr    = line.substring(sep + 1);

                html += "<tr>";
                html += "<td>" + productName + "</td>";
                html += "<td>" + priceStr + "</td>";
                // Delete button
                html += "<td>";
                html += "<form action='/deleteProduct' method='POST'>";
                // We'll find an index by scanning. Or we can pass the index by building in a second pass.
                // Simpler approach: We store an index in separate function getProductInfo?
                // For demonstration, we do a naive approach:
                // We'll do an extra pass to find the index. For large lists, you'd do this differently.
                // But let's do it inline for clarity.

                // We can do simpler: we let the web handler parse the line to find index. 
                // We'll cheat by including the name and searching in the handler. 
                // Or we store a hidden "index" parameter if we create the table from the actual function. 
                // For now, let's do a quick approach: We'll do a separate pass for generating indexes.

                // We'll just do a second function to produce the table with indexes. 
                // For demonstration, let's assume each line is in the same order as g_products, 
                // so we track them with a static or global. 
                // We'll do an external integer we increment each row:

                html += "<input type='hidden' name='index' value='" + String(productIndexCounter) + "'>";
                html += "<input type='submit' value='Delete'>";
                html += "</form>";
                html += "</td>";
                html += "</tr>";

                productIndexCounter++;
            }
        }
        html += F("</table>");
    }

    html += F("<hr>");
    // Form to add a new product
    html += F("<h3>Add New Product</h3>");
    html += F("<form action='/addProduct' method='POST'>");
    html += F("Name: <input type='text' name='name' required><br>");
    html += F("Price (cents): <input type='number' name='price' required><br>");
    html += F("<input type='submit' value='Add Product'>");
    html += F("</form>");

    html += F("</div></body></html>");
    server.send(200, "text/html", html);
}


// POST /addUser
static void handleAddUser()
{
    if (server.method() == HTTP_POST) {
        if (server.hasArg("username")) {
            String newUsername = server.arg("username");
            if (addUser(newUsername)) {
                // Redirect back to root
                server.sendHeader("Location", "/");
                server.send(303);
            } else {
                server.send(200, "text/plain", 
                            "Failed to add user (already exists or DB error).");
            }
            return;
        }
    }
    // If not POST or missing arg
    server.send(400, "text/plain", "Bad Request.");
}

// POST /setCard
static void handleMapCard()
{
    if (server.method() == HTTP_POST) {
        if (server.hasArg("cardId") && server.hasArg("userId")) {
            String cardId = server.arg("cardId");
            String userId = server.arg("userId");
            if (mapCardToUser(cardId, userId)) {
                server.sendHeader("Location", "/");
                server.send(303); // Redirect back to root
            } else {
                server.send(200, "text/plain", "Failed to map card.");
            }
            return;
        }
    }
    server.send(400, "text/plain", "Bad Request.");
}


static void handleUpdateCredit()
{
    if (server.method() == HTTP_POST) {
        // Check if form arguments exist
        if (server.hasArg("userId") && server.hasArg("delta")) {
            String userId = server.arg("userId");
            int delta = server.arg("delta").toInt(); // Convert to int
            String message = "";
            if (updateCreditByUserId(userId, delta, message)) {
                // Success => redirect back to home
                server.sendHeader("Location", "/");
                server.send(303); // 303: "See Other"
                return;
            } else {
                // User not found or DB error
                server.send(200, "text/plain", 
                            "Failed to update credit (user not found?). Go back and try again.");
                return;
            }
        }
    }
    // If we get here, request is malformed or not POST
    server.send(400, "text/plain", "Bad Request");
}

// Optional reset route if you want it
static void handleReset() {
    if (server.method() == HTTP_DELETE && server.hasArg("password")) {
        String pw = server.arg("password");
        reset(pw);
        server.sendHeader("Location", "/");
        server.send(303);
    } else {
        server.send(400, "text/plain", "Bad Request (need password).");
    }
}

// POST /deleteUnmappedCard
static void handleDeleteUnmappedCard()
{
    // Only proceed if it's a POST request and has 'cardId'
    if (server.method() == HTTP_POST && server.hasArg("cardId"))
    {
        String cardId = server.arg("cardId");

        // Directly try deleting it (no need to check userId, 
        // because we only got this from the Unmapped table)
        if (deleteCardById(cardId))
        {
            // Card removed -> redirect back to home
            server.sendHeader("Location", "/");
            server.send(303); 
        }
        else
        {
            // Card not found or SPIFFS error
            server.send(200, "text/plain",
                        "Failed to delete card (not found?).");
        }
    }
    else
    {
        server.send(400, "text/plain", "Bad Request.");
    }
}

// Product handlers
static void handleAddProduct()
{
    if (server.method() == HTTP_POST && server.hasArg("name") && server.hasArg("price")) {
        String name  = server.arg("name");
        int price    = server.arg("price").toInt();
        if (addProduct(name, price)) {
            server.sendHeader("Location", "/");
            server.send(303);
        } else {
            server.send(200, "text/plain", "Failed to add product.");
        }
        return;
    }
    server.send(400, "text/plain", "Bad Request.");
}

static void handleDeleteProduct()
{
    if (server.method() == HTTP_POST && server.hasArg("index")) {
        int idx = server.arg("index").toInt();
        if (deleteProduct(idx)) {
            server.sendHeader("Location", "/");
            server.send(303);
        } else {
            server.send(200, "text/plain", "Invalid product index or DB error.");
        }
        return;
    }
    server.send(400, "text/plain", "Bad Request.");
}


// --------------------------------------------------------------------
// 3. Setup and Loop for WebServer
// --------------------------------------------------------------------
void setupWebServer()
{
    server.on("/", HTTP_GET, handleRoot);

    server.on("/addUser",          HTTP_POST, handleAddUser);
    server.on("/mapCard",          HTTP_POST, handleMapCard);
    server.on("/updateCredit",     HTTP_POST, handleUpdateCredit);
    server.on("/deleteUnmappedCard", HTTP_POST, handleDeleteUnmappedCard);

    // Product endpoints
    server.on("/addProduct",    HTTP_POST, handleAddProduct);
    server.on("/deleteProduct", HTTP_POST, handleDeleteProduct);

    // Optional reset
    server.on("/reset", HTTP_DELETE, handleReset);

    server.begin();
    Serial.println("HTTP server started on port 80");
}

void handleWebServer()
{
    server.handleClient();
}
