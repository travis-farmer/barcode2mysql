#include <Arduino.h>
#include <SPI.h>
#include <WiFi101.h>
#include "Wire.h"
#include "Adafruit_LiquidCrystal.h"


Adafruit_LiquidCrystal lcd(0);

byte mac_addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

char server[] = "www.tjfhome.net";

// WiFi card example
char ssid[] = "XXXXXXXX";    // your SSID
char pass[] = "XXXXXXXX";       // your SSID Password



WiFiClient client;

// Globals
String rxBarCode = "";
String rxQty = "";
byte inByte = 0x00;
int flagPos = 0;
int curMode = 0;
String curLoc = "";
String curBarCode = "";
int curQty = 1;
bool globalActive = false;



void add(String inBarCode, int inQty, String inLoc) {
  char query[128];
  char INSERT_NEW[] = "GET /inventory.php?mode=add&Barcode=%s&Quantity=%d&Location=%s  HTTP/1.1";
  sprintf(query, INSERT_NEW, inBarCode, inQty, inLoc);
  if (client.connect(server, 80)) {
    // Make a HTTP request:
    client.println(query);
    client.println("Host: www.tjfhome.net");
    client.println("Connection: close");
    client.println();
  }
}

void take(String inBarCode, int inQty, String inLoc) {
  char query[128];
  char INSERT_NEW[] = "GET /inventory.php?mode=take&Barcode=%s&Quantity=%d&Location=%s  HTTP/1.1";
  sprintf(query, INSERT_NEW, inBarCode, inQty, inLoc);
  if (client.connect(server, 80)) {
    // Make a HTTP request:
    client.println(query);
    client.println("Host: www.tjfhome.net");
    client.println("Connection: close");
    client.println();
  }
}

bool updateMove(String inBarCode, String inLoc) {
  char query[128];
  char UPDATE_MOV[] = "GET /inventory.php?mode=move&Barcode=%s&Location=%s  HTTP/1.1";
  sprintf(query, UPDATE_MOV, inBarCode, inLoc);
  if (client.connect(server, 80)) {
    // Make a HTTP request:
    client.println(query);
    client.println("Host: www.tjfhome.net");
    client.println("Connection: close");
    client.println();
  }
}

void dispLoc() {
  lcd.setCursor(0,1);
  lcd.print("L:");
  lcd.print(curLoc);
}

void dispMode() {
  lcd.setCursor(0,2);
  lcd.print("M:");
  if (curMode == 0) { lcd.print("                  "); }
  else if (curMode == 1) { lcd.print("Add               "); }
  else if (curMode == 2) { lcd.print("Take              "); }
  else if (curMode == 3) { lcd.print("Remove            "); }
  else if (curMode == 4) { lcd.print("Change            "); }
}

void dispBarCode() {
  lcd.setCursor(0,3);
  lcd.print("B:");
  lcd.print(curBarCode);
}

void modeActive() {
    
  // Begin WiFi section
  int status = WiFi.begin(ssid, pass);
  if ( status != WL_CONNECTED) {
    lcd.setBacklight(LOW);
    lcd.print("W:X");
    globalActive = false;
    while(true);
  } else {
    lcd.setBacklight(HIGH);
    lcd.print("W:OK");
    globalActive = true;
  }
  // End WiFi section
  
  curMode = 0;
  curLoc = "";
  curBarCode = "";
  dispMode();
  dispLoc();
  dispBarCode();
  
}

void procTask() {
  // check if ready
  if (curMode > 0 && curLoc != "" && curBarCode != "") {
    if (curMode == 1) { // Add
      add(curBarCode,curQty, curLoc); // insert
    } else if (curMode == 2) { // Take
      take(curBarCode,curQty,curLoc);
    } else if (curMode == 3) { // Change
      updateMove(curBarCode, curLoc);
    }
  }
}

void procBarCode(String inBarCode, String inQty) {
  if (inBarCode.startsWith("loc") == true) {
    curLoc = inBarCode;
  } else if (inBarCode.startsWith("mod") == true) {
    if (inBarCode.endsWith("add") == true) {
      curMode = 1;
    } else if (inBarCode.endsWith("take") == true) {
      curMode = 2;
    } else if (inBarCode.endsWith("remove") == true) {
      curMode = 3;
    } else if (inBarCode.endsWith("change") == true) {
      curMode = 4;
    }
    dispMode();
  } else {
    curBarCode = inBarCode;
    dispBarCode();
    if (inQty != "") {
      curQty = inQty.toInt();
    } else {
      curQty = 1;
    }
    procTask();
  }
}

void setup() {
  //Serial.begin(115200);
  Serial1.begin(115200);
  lcd.begin(20, 4);
  lcd.setBacklight(LOW);
  lcd.setCursor(0,0);
  //Configure pins for Adafruit ATWINC1500 Breakout
  WiFi.setPins(53,49,48);
  modeActive();
}

void loop() {
  if (Serial1.available() > 0) {
    // get incoming byte:
    inByte = Serial1.read();
    if (inByte == 0x0d && rxBarCode.length() <= 0) { // [Enter] without data
      // start data
      rxBarCode = "";
      rxQty = "";
      flagPos = 0;
    } else if (inByte == 0x0d && rxBarCode.length() > 0) { // [Enter] with data
      procBarCode(rxBarCode,rxQty);
      rxBarCode = "";
      rxQty = "";
      flagPos = 0;
    } else if (inByte == 0x09) { // [TAB]
      ++flagPos;
    } else {
      if (flagPos == 0) {
        rxBarCode = rxBarCode + char(inByte);
      } else {
        rxQty = rxQty + char(inByte);
      }
    }
  }

}
