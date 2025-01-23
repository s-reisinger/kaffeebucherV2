#ifndef USER_DATABASE_H
#define USER_DATABASE_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

    // Initializes the SPIFFS file system
     bool initFileSystem();

    // Adds a new user with a unique ID; returns the assigned userId
     bool addUser(const String &username);

    // Maps a cardId to an existing userId
     bool mapCardToUser(const String &cardId, const String &userId);

    // Updates the user's credit by userId with the given delta (positive or negative)
     bool updateCreditByUserId(String userId, int delta);

    // Updates the user's credit using cardId, adds the card to unmapped if not found
     bool updateCreditByCardId(String &cardId, int delta);

    // Retrieves the userId linked to the provided cardId
     bool getUserIdByCardId(String &cardId, String &userId);

    // Retrieves all cards that have no associated user (userId = -1)
     bool getUnmappedCards(String &unmappedCards);

    // Generates the next available userId
     int getNextUserId();

     bool getAllUsers(String &usersList);

#endif // USER_DATABASE_H
