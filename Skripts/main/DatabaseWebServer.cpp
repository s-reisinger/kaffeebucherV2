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
    html += F("h1, h2 { color: #333; }");
    html += F("table { width: 100%; border-collapse: collapse; margin-top: 10px; cursor: pointer; }");
    html += F("th, td { padding: 10px; border: 1px solid #ddd; text-align: left; }");
    html += F("th { background: #007BFF; color: white; }");
    html += F("tr:nth-child(even) { background: #f9f9f9; }");
    html += F("tr:hover { background: #d3e3fc; }");
    html += F("form { margin-top: 20px; padding: 15px; background: #f8f9fa; border-radius: 5px; }");
    html += F("input[type='text'], input[type='number'] { width: 80%; padding: 8px; margin: 5px 0; border: 1px solid #ccc; border-radius: 4px; }");
    html += F("input[type='submit'] { background: #007BFF; color: white; border: none; padding: 10px; cursor: pointer; border-radius: 5px; width: 85%; }");
    html += F("input[type='submit']:hover { background: #0056b3; }");
    html += F("hr { margin: 20px 0; border: 1px solid #ddd; }");
    html += F("</style>");
    
    // JavaScript for Clickable Rows
    html += F("<script>");
    html += F("function fillUserFields(userId) {");
    html += F("document.getElementById('userId-map').value = userId;");
    html += F("document.getElementById('userId-credit').value = userId;");
    html += F("}");
    
    html += F("function fillCardField(cardId) {");
    html += F("document.getElementById('cardId').value = cardId;");
    html += F("}");
    html += F("</script>");

    html += F("</head><body>");
    
    html += F("<div class='container'>");
    html += F("<h1>ESP32 User Database</h1>");

    // Show all users
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

            int firstComma = line.indexOf(',');
            int secondComma = line.indexOf(',', firstComma + 1);

            if (firstComma == -1 || secondComma == -1) continue;

            String userId = line.substring(0, firstComma);
            String username = line.substring(firstComma + 1, secondComma);
            String credit = line.substring(secondComma + 1);

            html += "<tr onclick='fillUserFields(\"" + userId + "\")'>";
            html += "<td>" + userId + "</td><td>" + username + "</td><td>" + credit + "</td></tr>";
        }
        html += F("</table>");
    } else {
        html += F("<p>Could not read users from database.</p>");
    }
    html += F("<hr>");

    // Show unmapped cards
    String unmappedCards;
    if (getUnmappedCards(unmappedCards)) {
        html += F("<h2>Unmapped Cards</h2>");
        html += F("<table><tr><th>Card ID</th></tr>");

        int startIndex = 0;
        while (true) {
            int lineEnd = unmappedCards.indexOf('\n', startIndex);
            if (lineEnd == -1) break;

            String cardId = unmappedCards.substring(startIndex, lineEnd);
            startIndex = lineEnd + 1;

            html += "<tr onclick='fillCardField(\"" + cardId + "\")'>";
            html += "<td>" + cardId + "</td></tr>";
        }
        html += F("</table>");
    } else {
        html += F("<p>Could not read unmapped cards from database.</p>");
    }
    html += F("<hr>");

    // Form to add a user
    html += F("<h2>Add a New User</h2>");
    html += F("<form action='/addUser' method='POST'>");
    html += F("Username: <input type='text' name='username' required><br>");
    html += F("<input type='submit' value='Add User'>");
    html += F("</form>");
    html += F("<hr>");

    // Form to map a card to a user
    html += F("<h2>Map a Card to a User</h2>");
    html += F("<form action='/mapCard' method='POST'>");
    html += F("Card ID: <input type='text' id='cardId' name='cardId' required><br>");
    html += F("User ID: <input type='text' id='userId-map' name='userId' required><br>");
    html += F("<input type='submit' value='Map Card'>");
    html += F("</form>");
    html += F("<hr>");

    // Form to update credit
    html += F("<h2>Update Credit</h2>");
    html += F("<form action='/updateCredit' method='POST'>");
    html += F("User ID: <input type='text' id='userId-credit' name='userId' required><br>");
    html += F("Delta: <input type='number' name='delta' required><br>");
    html += F("<input type='submit' value='Update Credit'>");
    html += F("</form>");

    html += F("</div></body></html>");

    // Send the HTML to the client
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

static void handleReset() {
  if(server.method() == HTTP_DELETE) {
    if(server.hasArg("password")) {
      String password = server.arg("password");
      reset(password);
      server.sendHeader("Location", "/");
      server.send(303); // 303: "See Other"
       return;
    }
  }
}


// --------------------------------------------------------------------
// 3. Setup and Loop for WebServer
// --------------------------------------------------------------------
void setupWebServer()
{
    // Define our routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/addUser", HTTP_POST, handleAddUser);
    server.on("/mapCard", HTTP_POST, handleMapCard);
    server.on("/updateCredit", HTTP_POST, handleUpdateCredit);
    server.on("/reset", HTTP_DELETE, handleReset);

    // Start server
    server.begin();
    Serial.println("HTTP server started on port 80");
}

void handleWebServer()
{
    server.handleClient();
}
