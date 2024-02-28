#include <SPI.h>
#include <WiFi.h>
#include <MFRC522.h>
#include <TinyGPS++.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>

#define SS_PIN 5    // ESP32 pin GPIO5
#define RST_PIN 22  // ESP32 pin GPIO27

MFRC522 rfid(SS_PIN, RST_PIN);

const char *ssid = "Mwarandus Lab";
const char *password = "30010231";

HardwareSerial gpsSerial(2);

#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

int Buzzer = 13;

TinyGPSPlus gps;

uint32_t tagID = 0;

float latitude = 0.0;   // Declare latitude as a global variable
float longitude = 0.0;  // Declare longitude as a global variable

const int rs = 32, en = 33, d4 = 25, d5 = 26, d6 = 27, d7 = 14;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  Serial.begin(9600);
  pinMode(Buzzer, OUTPUT);
  WiFi.begin(ssid, password);
  SPI.begin();      // init SPI bus
  rfid.PCD_Init();  // init MFRC522
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

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
}

void loop() {
  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print("SCAN TAG");
  delay(1000);
  digitalWrite(Buzzer, LOW);

  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

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

      rfid.PICC_HaltA();       // halt PICC
      rfid.PCD_StopCrypto1();  // stop encryption on PCD
      
      // Check if a valid GPS fix is available
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();

        // Print GPS data for debugging
        // Serial.print("Latitude: ");
        // Serial.println(latitude, 6);
        // Serial.print("Longitude: ");
        // Serial.println(longitude, 6);
        // delay(1000);

        // Call the sendTagToServer function here to send the tag and GPS data
        sendTagToServer(tagID, latitude, longitude);
      }
    }
  }
}
void sendTagToServer(uint32_t tagID, float latitude, float longitude) {
  // Check if latitude and longitude are valid and non-zero
  if (latitude == 0.0 || longitude == 0.0) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    String serverURL = "http://moscoinnovators/test.php";
    http.begin(client, serverURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // Prepare data to send to the server
    String postData = "tag=" + String(tagID) + "&latitude=" + String(latitude, 6) + "&longitude=" + String(longitude, 6);

    int httpResponseCode = http.POST(postData);
    if (httpResponseCode > 0) {
      digitalWrite(Buzzer, LOW);
      Serial.printf("Tag sent to server. HTTP Response code: %d\n", httpResponseCode);
      Serial.println("Tag Send Successfully");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("TAG: ");
      lcd.setCursor(5, 0);
      lcd.print(tagID);
      lcd.setCursor(0, 1);
      lcd.print("SENDING...");
      delay(2000);

      // Wait for the server response
      while (!client.available()) {
        delay(10);
      }

      // Parse the JSON response
      DynamicJsonDocument jsonBuffer(256);
      String response = client.readString();
      DeserializationError error = deserializeJson(jsonBuffer, response);

      if (error) {
        Serial.println("JSON parsing error");
      }

      // Check if the tagID is available in the database
      bool tagAvailable = jsonBuffer["tag_available"];

      if (tagAvailable) {
        digitalWrite(Buzzer, LOW);
        Serial.println("Tag Found!");
        lcd.clear();
        lcd.setCursor(4, 0);
        lcd.print("TAG FOUND");
        lcd.setCursor(0, 1);
        lcd.print("Name: ");
        lcd.print(jsonBuffer["student_lastname"].as<String>());
        delay(1000);  // Display the name for 5 seconds
      } else {
        digitalWrite(Buzzer, HIGH);
        Serial.println("Tag Not Found");
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("TAG NOT FOUND");
        delay(500);
      }
    } else {
      digitalWrite(Buzzer, HIGH);
      Serial.println("Error sending tag to server.");
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("ERROR");
      lcd.setCursor(3, 1);
      lcd.print("SENDING DATA");
      delay(500);
    }
    http.end();
  } else {
    digitalWrite(Buzzer, LOW);
    Serial.println("Invalid or zero latitude/longitude, data not sent to server.");
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("INVALID");
    lcd.setCursor(2, 1);
    lcd.print("LOCATION INFO");
    delay(500);
  }
}
