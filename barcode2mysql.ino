#include <SPI.h>
#include <WiFi101.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "Wire.h"
#include "Adafruit_LiquidCrystal.h"


Adafruit_LiquidCrystal lcd(0);

byte mac_addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress server_addr(192,168,1,171);  // IP of the MySQL *server* here
char user[] = "arduino";              // MySQL user login username
char password[] = "XXXXXXXX";        // MySQL user login password

// WiFi card example
char ssid[] = "XXXXXXXX";    // your SSID
char pass[] = "XXXXXXXX";       // your SSID Password



WiFiClient client;
MySQL_Connection conn((Client *)&client);

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

int isNewBC(String inBarCode) {
  const char QUERY_BC[] = "SELECT Barcode FROM PartsDB.Parts WHERE Barcode = '%s';";
  char query[128];
  if (conn.connect(server_addr, 3306, user, password)) {
    delay(1000);
  
    // Initiate the query class instance
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    // Supply the parameter for the query
    // Here we use the QUERY_POP as the format string and query as the
    // destination. This uses twice the memory so another option would be
    // to allocate one buffer for all formatted queries or allocate the
    // memory as needed (just make sure you allocate enough memory and
    // free it when you're done!).
    sprintf(query, QUERY_BC, inBarCode);
    // Execute the query
    cur_mem->execute(query);
    
    // Read the rows and print them
    row_values *row = NULL;
    row = cur_mem->get_next_row();
      if (row != NULL) {
        return(1); // found the barcode
      } else {
        return(2); // did not find the barcode
      }
    delete cur_mem;
  }
  else
    return(0); // error
}

int getQty(String inBarCode) {
  const char QUERY_QTY[] = "SELECT Quantity FROM PartsDB.Parts WHERE Barcode = '%s';";
  char query[128];
  if (conn.connect(server_addr, 3306, user, password)) {
    delay(1000);
  
    // Initiate the query class instance
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    // Supply the parameter for the query
    // Here we use the QUERY_POP as the format string and query as the
    // destination. This uses twice the memory so another option would be
    // to allocate one buffer for all formatted queries or allocate the
    // memory as needed (just make sure you allocate enough memory and
    // free it when you're done!).
    sprintf(query, QUERY_QTY, inBarCode);
    // Execute the query
    cur_mem->execute(query);
    
    // Read the rows and print them
    row_values *row = NULL;
    row = cur_mem->get_next_row();
    if (row != NULL) {
      return(row->values[0]);
    }
    delete cur_mem;
  }
  else
    return(0); // error
}

void insertNew(String inBarCode, int inQty, String inLoc) {
  char query[128];
  char INSERT_NEW[] = "INSERT INTO PartsDB.Parts (Barcode, Quantity, Location) VALUES ('%s',%d ,'%s');";

  if (conn.connect(server_addr, 3306, user, password)) {
    delay(1000);
    // Initiate the query class instance
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    // Save
    sprintf(query, INSERT_NEW, inBarCode, inQty, inLoc);
    // Execute the query
    cur_mem->execute(query);
    // Note: since there are no results, we do not need to read any data
    // Deleting the cursor also frees up memory used
    delete cur_mem;
  }
  conn.close();
}

bool updateExisting(String inBarCode, int inQty) {
  char query[128];
  char UPDATE_EX[] = "UPDATE PartsDB.Parts SET Quantity=%d WHERE Barcode = '%s';";

  if (conn.connect(server_addr, 3306, user, password)) {
    delay(1000);
    // Initiate the query class instance
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    // Save
    sprintf(query, UPDATE_EX, inQty, inBarCode);
    // Execute the query
    cur_mem->execute(query);
    // Note: since there are no results, we do not need to read any data
    // Deleting the cursor also frees up memory used
    delete cur_mem;
    return(true); // everything ok
  }
  else
    return(false); // ERROR
  conn.close();
}

bool updateMove(String inBarCode, String inLoc) {
  char query[128];
  char UPDATE_MOV[] = "UPDATE PartsDB.Parts SET Location='%s' WHERE Barcode = '%s';";

  if (conn.connect(server_addr, 3306, user, password)) {
    delay(1000);
    // Initiate the query class instance
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    // Save
    sprintf(query, UPDATE_MOV, inLoc, inBarCode);
    // Execute the query
    cur_mem->execute(query);
    // Note: since there are no results, we do not need to read any data
    // Deleting the cursor also frees up memory used
    delete cur_mem;
    return(true); // everything ok
  }
  else
    return(false); // ERROR
  conn.close();
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
      if (isNewBC(curBarCode) == 2 || isNewBC(curBarCode) == 0) { // none stored, or error
        insertNew(curBarCode,curQty, curLoc); // insert
      } else { // one, or more stored
        int storedQtyAdd = getQty(curBarCode);
        updateExisting(curBarCode,(curQty + storedQtyAdd)); // update DB
      }
    } else if (curMode == 2) { // Take
      if (isNewBC(curBarCode) == 1) { // must exist to take
        int storedQtyTake = getQty(curBarCode);
        if (storedQtyTake >= curQty) { // must have more or equal to take
          updateExisting(curBarCode,(storedQtyTake - curQty));
        }
      }
    } else if (curMode == 3) { // Change
      if (isNewBC(curBarCode) == 1) { // must be there to move
        updateMove(curBarCode, curLoc);
      }
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
