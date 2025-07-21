#include <SPI.h>
#include <WiFi.h>
#include <MFRC522.h>
//#include <TinyGPS++.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include <HardwareSerial.h>

#define SS_PIN 5    // ESP32 pin GPIO5
#define RST_PIN 21  // ESP32 pin GPIO21

MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "Dadir";
const char* password = "09 hxmza";

//HardwareSerial gpsSerial(2);
WiFiClient client;

const char* serverURL = "https://abdikadirstudenttracker.com";

//#define GPS_RX_PIN 16
//#define GPS_TX_PIN 17

int Buzzer = 13;
int tracker_1 = 0;
int tracker_2 = 0;
int tracker_3 = 0;

//TinyGPSPlus gps;

uint32_t tagID = 0;

float latitude = 0.0;   // Declare latitude as a global variable
float longitude = 0.0;  // Declare longitude as a global variable

enum State {
  IDLE,
  SCAN_TAG,
  RESET
};

State currentState = IDLE;

HardwareSerial GSM(1);

const int rs = 32, en = 33, d4 = 25, d5 = 26, d6 = 27, d7 = 14;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  Serial.begin(9600);
  delay(10000);
  GSM.begin(9600, SERIAL_8N1, 16, 17);
  delay(1000);

  pinMode(Buzzer, OUTPUT);
  WiFi.begin(ssid, password);
  SPI.begin();      // init SPI bus
  rfid.PCD_Init();  // init MFRC522
  //gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  digitalWrite(Buzzer, LOW);
  lcd.begin(20, 4);

  lcd.clear();
  lcd.setCursor(7, 1);
  lcd.print("SYSTEM");
  lcd.setCursor(3, 2);
  lcd.print("INITIALIZATION");
  delay(2000);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi: Connecting");
    lcd.setCursor(0, 1);
    lcd.print("IP: Loading...");
    delay(3000);
  }
  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print("WiFi: Connected");
  lcd.setCursor(1, 2);
  lcd.print("IP: " + WiFi.localIP().toString());
  delay(3000);

  GSM.println("AT");  // Send a basic AT command to test communication
  delay(1000);
  if (GSM.available()) {
    String response = GSM.readString();
    Serial.println("GSM Response: " + response);
  }
}

void loop() {
  switch (currentState) {
    case IDLE:
      idle();
      break;
    case SCAN_TAG:
      scan_tag();
      break;
    case RESET:
      reset();
      break;
  }
}
void idle() {
  scan_tag();
  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print("SCAN TAG");
  delay(1000);
  digitalWrite(Buzzer, LOW);
}
void scan_tag() {
  if (rfid.PICC_IsNewCardPresent()) {  // new tag is available
    if (rfid.PICC_ReadCardSerial()) {  // NUID has been read
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));

      // print UID in Serial Monitor in the hex format
      Serial.print("UID:");
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);

        // Shift the existing bits in `tag` by 8 positions to make room for the new byte
        tagID = (tagID << 8) | rfid.uid.uidByte[i];
      }
      Serial.println();

      // Now, `tag` contains the UID as an integer
      Serial.print("Tag as Integer: ");
      Serial.println(tagID);
      if (tagID == 1139462418 && tracker_1 == 0) {
        Serial.println("Abdikadir Board the Bus");
        digitalWrite(Buzzer, HIGH);
        delay(500);
        digitalWrite(Buzzer, LOW);
        float longitude = 37.09;
        float latitude = -1.05;
        tracker_1 = 1;
        lcd.clear();
        lcd.setCursor(3, 2);
        lcd.print("PLEASE WAIT...");
        delay(500);
        sendToServer("/verify.php", String(tagID), latitude, longitude);
      } else if (tagID == 2477708826 && tracker_1 == 1) {
        Serial.println("Abdikadir Alight the Bus Undefined Location");
        digitalWrite(Buzzer, HIGH);
        delay(500);
        digitalWrite(Buzzer, LOW);
        float longitude = generateRandomLongitude();
        float latitude = generateRandomLatitude();
        tagID = 1139462418;
        tracker_1 = 3;
        lcd.clear();
        lcd.setCursor(3, 2);
        lcd.print("PLEASE WAIT...");
        delay(500);
        sendToServer("/verify.php", String(tagID), latitude, longitude);
        sendSMS("+254720277892", "Hello Abdikadir's Parent we are notifying you of your child alighting to uknown location. Kindly click the link to see his current location https://abdikadirstudenttracker.com/Parents/parent1.php");
      } else if (tagID == 1139462418 && tracker_1 == 1) {
        Serial.println("Abdikadir Alight the Bus");
        digitalWrite(Buzzer, HIGH);
        delay(500);
        digitalWrite(Buzzer, LOW);
        float longitude = 37.08;
        float latitude = -1.03;
        tracker_1 = 2;
        lcd.clear();
        lcd.setCursor(3, 2);
        lcd.print("PLEASE WAIT...");
        delay(500);
        sendToServer("/verify.php", String(tagID), latitude, longitude);
      } else if (tagID == 3822428685 && tracker_2 == 0) {
        Serial.println("Mohammed Board the Bus");
        digitalWrite(Buzzer, HIGH);
        delay(500);
        digitalWrite(Buzzer, LOW);
        float longitude = 4.67043689;
        float latitude = 6.89323241;
        lcd.clear();
        lcd.setCursor(3, 2);
        lcd.print("PLEASE WAIT...");
        delay(500);
        sendToServer("/verify_1.php", String(tagID), latitude, longitude);
        tracker_2 = 1;
      } else if (tagID == 601040632 && tracker_2 == 1) {
        Serial.println("Mohammed Alight the Bus Undefined Location");
        digitalWrite(Buzzer, HIGH);
        delay(500);
        digitalWrite(Buzzer, LOW);
        float longitude = generateRandomLongitude();
        float latitude = generateRandomLatitude();
        tracker_2 = 0;
        lcd.clear();
        lcd.setCursor(3, 2);
        lcd.print("PLEASE WAIT...");
        delay(500);
        sendToServer("/verify_1.php", String(tagID), latitude, longitude);
      } else if (tagID == 2477708826 && tracker_2 == 1) {
        Serial.println("Mohammed Alight the Bus");
        digitalWrite(Buzzer, HIGH);
        delay(500);
        digitalWrite(Buzzer, LOW);
        float longitude = 8.67043689;
        float latitude = 12.89323241;
        tracker_2 = 0;
        lcd.clear();
        lcd.setCursor(3, 2);
        lcd.print("PLEASE WAIT...");
        delay(500);
        sendToServer("/verify_1.php", String(tagID), latitude, longitude);
      } else if (tagID == 2206945955 && tracker_3 == 0) {
        Serial.println("Abuu Board the Bus");
        digitalWrite(Buzzer, HIGH);
        delay(500);
        digitalWrite(Buzzer, LOW);
        float longitude = 37.09;
        float latitude = -1.05;
        tracker_3 = 1;
        lcd.clear();
        lcd.setCursor(3, 2);
        lcd.print("PLEASE WAIT...");
        delay(500);
        sendToServer("/verify_2.php", String(tagID), latitude, longitude);
      } else if (tagID == 2363502666 && tracker_3 == 1) {
        Serial.println("Abuu Alight the Bus Undefined Location");
        digitalWrite(Buzzer, HIGH);
        delay(500);
        digitalWrite(Buzzer, LOW);
        float longitude = generateRandomLongitude();
        float latitude = generateRandomLatitude();
        tagID = 2206945955;
        tracker_3 = 3;
        lcd.clear();
        lcd.setCursor(3, 2);
        lcd.print("PLEASE WAIT...");
        delay(500);
        sendToServer("/verify_2.php", String(tagID), latitude, longitude);
        sendSMS("+254740915820", "Hello Abuu's Parent we are notifying you of your child alighting to uknown location. Kindly click the link to see his current location https://abdikadirstudenttracker.com/Parents/parent2.php");
      } else if (tagID == 2206945955 && tracker_3 == 1) {
        Serial.println("Abuu Alight the Bus");
        digitalWrite(Buzzer, HIGH);
        delay(500);
        digitalWrite(Buzzer, LOW);
        float longitude = 37.08;
        float latitude = -1.03;
        tracker_3 = 2;
        lcd.clear();
        lcd.setCursor(3, 2);
        lcd.print("PLEASE WAIT...");
        delay(500);
        sendToServer("/verify_2.php", String(tagID), latitude, longitude);
      }
      rfid.PICC_HaltA();       // halt PICC
      rfid.PCD_StopCrypto1();  // stop encryption on PCD
    }
  }
}
void sendToServer(String endpoint, String tagID, float latitude, float longitude) {
  HTTPClient http;

  // Construct URL with query parameters
  String url = serverURL + endpoint + "?tagID=" + tagID + "&longitude=" + String(longitude) + "&latitude=" + String(latitude);

  // Print the URL
  Serial.println("Sending GET request to: " + url);

  // Make HTTP GET request
  http.begin(url);

  // Make HTTP GET request
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("HTTP Response: " + response);

    // Parse JSON response
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.println("Failed to parse JSON");
      return;
    }

    const char* status = doc["status"];
    const char* firstName = doc["first_name"];
    const char* lastName = doc["last_name"];

    // Check status
    if (strcmp(status, "success") == 0) {
      // Tag ID found, print first name and last name
      Serial.print("First Name: ");
      Serial.println(firstName);
      Serial.print("Last Name: ");
      Serial.println(lastName);
      if (strcmp(firstName, "Abdikadir") == 0 && tracker_1 == 1) {
        Serial.println("Hello Abdikadir boarding");
        lcd.clear();
        lcd.setCursor(3, 2);
        lcd.print(firstName);
        lcd.print(" ");
        lcd.print(lastName);
        lcd.setCursor(6, 3);
        lcd.print("BOARDING");
        delay(3000);
      } else if (strcmp(firstName, "Abdikadir") == 0 && tracker_1 == 2) {
        Serial.println("Hello Abdikadir alighting");
        lcd.clear();
        lcd.setCursor(3, 2);
        lcd.print(firstName);
        lcd.print(" ");
        lcd.print(lastName);
        lcd.setCursor(5, 3);
        lcd.print("ALIGHTING");
        delay(3000);
        tracker_1 = 0;
        currentState = IDLE;
      } else if (strcmp(firstName, "Abdikadir") == 0 && tracker_1 == 3) {
        Serial.println("Hello Abdikadir Undefined Location");
        lcd.clear();
        lcd.setCursor(3, 2);
        lcd.print(firstName);
        lcd.print(" ");
        lcd.print(lastName);
        lcd.setCursor(1, 3);
        lcd.print("UNDEFINED LOCATION");
        delay(3000);
        currentState = RESET;
      }
      if (strcmp(firstName, "Abuu") == 0 && tracker_3 == 1) {
        Serial.println("Hello Abuu boarding");
        lcd.clear();
        lcd.setCursor(5, 2);
        lcd.print(firstName);
        lcd.print(" ");
        lcd.print(lastName);
        lcd.setCursor(6, 3);
        lcd.print("BOARDING");
        delay(3000);
      } else if (strcmp(firstName, "Abuu") == 0 && tracker_3 == 2) {
        Serial.println("Hello Abuu alighting");
        lcd.clear();
        lcd.setCursor(5, 2);
        lcd.print(firstName);
        lcd.print(" ");
        lcd.print(lastName);
        lcd.setCursor(5, 3);
        lcd.print("ALIGHTING");
        delay(3000);
        tracker_3 = 0;
        currentState = IDLE;
      } else if (strcmp(firstName, "Abuu") == 0 && tracker_3 == 3) {
        Serial.println("Hello Abuu Undefined");
        lcd.clear();
        lcd.setCursor(5, 2);
        lcd.print(firstName);
        lcd.print(" ");
        lcd.print(lastName);
        lcd.setCursor(1, 3);
        lcd.print("UNDEFINED LOCATION");
        delay(3000);
        currentState = RESET;
      }

    } else {
      // Tag ID not found or error
      Serial.println("Tag ID not found or error occurred");
    }
  } else {
    Serial.println("HTTP Request failed");
  }

  http.end();
}
void sendSMS(const char* phoneNumber, const char* message) {
  Serial.println("Sending SMS...");

  // Set GSM module to Text Mode
  GSM.println("AT+CMGF=1");
  delay(1000);
  if (GSM.available()) {
    String response = GSM.readString();
    Serial.println("Set Text Mode Response: " + response);
  }

  // Start SMS command
  GSM.print("AT+CMGS=\"");
  GSM.print(phoneNumber);  // Add the recipient phone number
  GSM.println("\"");
  delay(1000);
  if (GSM.available()) {
    String response = GSM.readString();
    Serial.println("Phone Number Response: " + response);
  }

  // Add the SMS text message
  GSM.print(message);
  delay(1000);

  // End SMS with CTRL+Z (ASCII code 26)
  GSM.write(26);
  delay(5000);  // Wait for the SMS to be sent

  // Check GSM module response
  if (GSM.available()) {
    String response = GSM.readString();
    Serial.println("GSM Response: " + response);
  }

  Serial.println("SMS sent!");
}
void reset(){
  if(tracker_3 == 3){
    tracker_3 = 0;
    currentState = IDLE;
  }else if (tracker_1 == 3){
    tracker_1 = 0;
    currentState = IDLE;
  }
}
float generateRandomLatitude() {
  return random(3600000, 3784308) / 100000.0;
}

float generateRandomLongitude() {
  return random(-1033611, -1093611) / 1000000.0;
}
