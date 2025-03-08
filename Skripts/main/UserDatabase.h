#ifndef USER_DATABASE_H
#define USER_DATABASE_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

    #pragma once
#include <Arduino.h>

  // -------------------------
  // Initialization & Resets
  // -------------------------
  bool initFileSystem();
  void reset(String password);

  // -------------------------
  // User & Card Management
  // -------------------------
  bool addUser(const String &username);
  bool mapCardToUser(const String &cardId, const String &userId);
  bool updateCreditByUserId(String userId, int delta, String &message);
  bool updateCreditByCardId(String &cardId, int delta, String &message);
  bool getUserIdByCardId(String &cardId, String &userId);

  bool getUnmappedCards(String &unmappedCards);
  bool getAllUsers(String &usersList);
  bool deleteCardById(const String &cardId);

  // -------------------------
  // Product Management
  // -------------------------
  bool loadProducts();
  bool saveProducts();
  bool addProduct(const String &name, int priceCents);
  bool deleteProduct(int index);
  bool getAllProducts(String &productsList); // e.g. "Name;100\nLatte;150\n..."
  int  getProductCount();
  bool getProductInfo(int index, String &name, int &priceCents);

#endif // USER_DATABASE_H
