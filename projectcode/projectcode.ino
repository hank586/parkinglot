#include <Wire.h> // Include Wire library for I2C
#include <LiquidCrystal_I2C.h> // Include the LiquidCrystal I2C library
#include <Arduino.h>
#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2SNoDAC.h"
#include "viola.h"// file âm thanh
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h> 
//NodeMCU--------------------------
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
WiFiClient wifiClient;
//************************************************************************
#define SS_PIN  D2  //D2
#define RST_PIN D1  //D1
#define BUZZER_PIN D4

MFRC522 mfrc522(SS_PIN, RST_PIN); 
//-------------------------
/* Set these to your desired credentials. */
const char *ssid = "vietlot";
const char *password = "0995363571";
const char* device_token  = "69c19b1f363cce81";
//-------------
String URL = "http://192.168.100.10/parkinglot/getdata.php"; //computer IP 
String URL1 = "http://192.168.100.10/parkinglot/registercheck.php";
String getData, Link;
String OldCardID = "";
unsigned long previousMillis = 0;
//-----------
LiquidCrystal_I2C lcd(0x27,16,2);

Servo myservo; // Create a servo object
int pos = 0;   // Variable to store the servo position
//----------
AudioGeneratorWAV *wav;
AudioFileSourcePROGMEM *file;
AudioOutputI2SNoDAC *out;
void setup() {
  delay(1000);
  Serial.begin(115200);
  SPI.begin();  // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  //---------------------------------------------
  connectToWiFi();
  //---------------------------------------------
  myservo.attach(D3); // Attach the servo to pin D3
  pinMode(BUZZER_PIN, OUTPUT);
  audioLogger = &Serial;
  file = new AudioFileSourcePROGMEM(viola, sizeof(viola));
  out = new AudioOutputI2SNoDAC();
  wav = new AudioGeneratorWAV();
  wav->begin(file, out);
  Wire.begin(2,0); //set chân cho SDA và SCL của LCD
  lcd.clear();
  lcd.init();
  lcd.backlight();

}
//************************************************************************
void loop() {
  //check if there's a connection to Wi-Fi or not
  if(!WiFi.isConnected()){
    connectToWiFi();    //Retry to connect to Wi-Fi
  }
  //---------------------------------------------
  if (millis() - previousMillis >= 15000) {
    previousMillis = millis();
    OldCardID="";
  }
  delay(50);
  //---------------------------------------------
  //look for new card
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;//got to start of loop if there is no card present
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;//if read card serial(0) returns 1, the uid struct contians the ID of the read card.
  }
  String CardID ="";
  for (byte i = 0; i < mfrc522.uid.size; i++) {  // THÊM THÔNG TIN UID VÀO biến CardID
    CardID += mfrc522.uid.uidByte[i];
  }
  //---------------------------------------------
  if( CardID == OldCardID ){
    return;
  }
  else{
    OldCardID = CardID;
  }
  //---------------------------------------------
//  Serial.println(CardID);
playBuzzer();
  SendCardID(CardID);
  if(checkRegisteredCard(CardID)) { // Check if the card is registered
    moveServo(); // Move the servo if the card is registered
    lcd.setCursor(0,0);
    lcd.print("MOI XE QUA");
    playSound();// chạy âm thanh
    delay(3000);
    lcd.clear();
 
  }
  delay(1000);
  if (wav->isRunning()) {
    if (!wav->loop()) {
      wav->stop();
    }
  }
}

//************send the Card UID to the website*************
void SendCardID( String Card_uid ){
  Serial.println("Sending the Card ID");
  if(WiFi.isConnected()){
    HTTPClient http;    //Declare object of class HTTPClient
    //GET Data
    getData = "?card_uid=" + String(Card_uid) + "&device_token=" + String(device_token); // Add the Card ID to the GET array in order to send it
    //GET methode
    Link = URL + getData;
    http.begin(wifiClient,Link); //initiate HTTP request   //Specify content-type header
    
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload

//    Serial.println(Link);   //Print HTTP return code
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(Card_uid);     //Print Card ID
    Serial.println(payload);    //Print request response payload

    if (httpCode == 200) {
      if (payload.substring(0, 5) == "login") {
        String user_name = payload.substring(5);
    //  Serial.println(user_name);

      }
      else if (payload.substring(0, 6) == "logout") {
        String user_name = payload.substring(6);
    //  Serial.println(user_name);
        
      }
      else if (payload == "succesful") {

      }
      else if (payload == "available") {

      }
      delay(100);
      http.end();  //Close connection
    }
  }
}

//************Check if the Card is registered*************
bool checkRegisteredCard(String Card_uid) {
  if(WiFi.isConnected()) {
    HTTPClient http;    // Declare object of class HTTPClient
    // Construct the URL with the card UID as a parameter
    String check = "?card_uid=" + String(Card_uid) + "&device_token=" + String(device_token);
    String requestURL= URL1+check;
    http.begin(wifiClient, requestURL); // Initiate HTTP request

    int httpCode = http.GET();   // Send the request
    if (httpCode == 200) {
      String payload = http.getString(); // Get the response payload
      if (payload == "registered") {
        Serial.println("Card is registered");
        return true;
      } else if (payload == "unregistered") {
        Serial.println("Card is not registered");
        return false;
      } else {
        Serial.println("Unexpected response from server");
        return false;
      }
    } else {
      Serial.print("Error in HTTP request. HTTP Code: ");
      Serial.println(httpCode);
      return false;
    }

    http.end();  // Close connection
  } else {
    Serial.println("WiFi not connected");
    return false;
  }
}

//************Move the Servo*************
void moveServo() {
  for (pos = 0; pos <= 150; pos += 1) { 
    myservo.write(pos);              
  }
  delay(2000); 
  for (pos = 150; pos >= 0; pos -= 1) { 
    myservo.write(pos);               
  }
}
void playBuzzer() {
  tone(BUZZER_PIN, 1000); 
  delay(500);             
  noTone(BUZZER_PIN);     
}

void playSound() {
  file = new AudioFileSourcePROGMEM(viola, sizeof(viola));
  wav->begin(file, out);
}
//********************connect to the WiFi******************
void connectToWiFi(){
    WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
    delay(1000);
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("Connected");
  
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP
    
    delay(1000);
}