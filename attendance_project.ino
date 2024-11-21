#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <SPI.h>
#include <MFRC522.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#define WIFI_SSID "R-907"
#define WIFI_PASSWORD "helloworld@907"
#define API_KEY "AIzaSyAqUuCm9cIFrxJxf15fZkc-PrUS_MLGmZk"
#define FIREBASE_PROJECT_ID "studentattendance-ee8ab"
#define USER_EMAIL "roy35-909@diu.edu.bd"
#define USER_PASSWORD "123456789"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
constexpr uint8_t RST_PIN = D2; 
constexpr uint8_t SS_PIN = D4; 
MFRC522 mfrc522(SS_PIN, RST_PIN);
String classcode; 
// For Time 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"asia.pool.ntp.org",21600,6000);

void fcsUploadCallback(CFS_UploadStatusInfo info)
{
    if (info.status == fb_esp_cfs_upload_status_init)
    {
        Serial.printf("\nUploading data (%d)...\n", info.size);
    }
    else if (info.status == fb_esp_cfs_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
    }
    else if (info.status == fb_esp_cfs_upload_status_complete)
    {
        Serial.println("Upload completed ");
    }
    else if (info.status == fb_esp_cfs_upload_status_process_response)
    {
        Serial.print("Processing the response... ");
    }
    else if (info.status == fb_esp_cfs_upload_status_error)
    {
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}


String day[]={" Sunday"," Monday"," Tuesday"," Wednesday"," Thursday", " Friday", " Saturday"};

void setup()
{

    Serial.begin(115200);
     pinMode(D3,OUTPUT);
     digitalWrite(D3,LOW);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card
 
  Serial.println(F("Read personal data on a MIFARE PICC:")); 



}



 std::vector<String> students;   
void loop()
{
        

        // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
          MFRC522::MIFARE_Key key;
          for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

          //some variables we need
          byte block;
          byte len;
          MFRC522::StatusCode status;

          //-------------------------------------------

          // Look for new cards
          if ( ! mfrc522.PICC_IsNewCardPresent()) {
            return;
          }

          // Select one of the cards
          if ( ! mfrc522.PICC_ReadCardSerial()) {
            return;
          }

          Serial.println(F("**Card Detected:**"));

  //-------------------------------------------

   char str[32] = "";
   array_to_string( mfrc522.uid.uidByte, 4, str); //Insert (byte array, length, char array for output)
   Serial.println(str);
   students.push_back(str);
   Serial.println(F("\n**End Reading**\n"));
        String documentPath ; 
        Serial.print("Get a document... ");
        documentPath = "projects/R-612/";




   

  // Create the JSON structure for Firestore
  FirebaseJson json;
  FirebaseJson arrayField;
  FirebaseJsonArray jsonArray;

  // Add values to the array
  FirebaseJson value1;

    for (size_t i = 0; i < students.size(); i++) {
    Serial.println(students[i]);
    value1.set("stringValue", students[i]);
    jsonArray.add(value1);
  }
  


  


  // Wrap the array inside the Firestore structure
  arrayField.set("arrayValue/values", jsonArray);
  json.set("fields/arrayField", arrayField);

  // Upload JSON to Firestore
  if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath, json.raw(), "")) {
    Serial.println("Array uploaded successfully!");
  } else {
    Serial.println("Error: " + fbdo.errorReason());
  }
 

         
  delay(200);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
    
}


void array_to_string(byte array[], unsigned int len, char buffer[])
{
   for (unsigned int i = 0; i < len; i++)
   {
      byte nib1 = (array[i] >> 4) & 0x0F;
      byte nib2 = (array[i] >> 0) & 0x0F;
      buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
      buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
   }
   buffer[len*2] = '\0';
}