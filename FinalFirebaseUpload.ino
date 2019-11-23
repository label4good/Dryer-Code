#include <WiFi.h>
#include "FirebaseESP32.h"
#include "DHT.h"

#define DHTPIN 14     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22

FirebaseData firebaseData;
const char* ssid = "SSID"; //Use the wifi name as ssid
const char* password =  NULL; //If your wifi has a password put it here. Must be a string
int setTemp = 60; //Just the temperature that you want 

DHT dht(DHTPIN, DHTTYPE); //Init the DHT 
void setup() {
  
  Serial.begin(115200); //Using 115200 bud
  Serial.println(F("DHTxx test!"));
  delay(10);
  
  pinMode(32, OUTPUT); //This is for the relay for the dryer. Pin 32 is defined as an output.
  
  dht.begin(); //start the dht module
  Serial.println('\n');
  WiFi.begin(ssid, password); 
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); //Checking to see if the wifi is connected
  }
  Serial.println(".......");
  Serial.println("WiFi Connected....IP Address:");
  Serial.println(WiFi.localIP());
  Firebase.begin("project.firebaseio.com", "firebase secret");
  Firebase.reconnectWiFi(true);
}

void updateFB() {
  // set
  delay(1000); //Delay 1 second to make sure everything is done setting up
  float h = dht.readHumidity(); //define the humidity as h 
  float t = dht.readTemperature(); //define the temperature as t
  if (isnan(h) || isnan(t)) { 
    Serial.println(F("Failed to read from DHT sensor!")); //alert us if the sensor isn't working
    return;
  }

  //if it is working then do the following.
  if (Firebase.setTimestamp(firebaseData, "/data/0/ts")) { //get the timestamp as ts for firebase
    Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData.dataPath()); //print the path of the db  
      int ts = firebaseData.intData(); //assign the int of the timestamp as ts for later use. 
      Serial.println("VALUE:  " + String(ts));
      Serial.println("------------------------------------");
      Serial.println();

      String ts_str = String(ts); //convert the int of ts to a string to be used a path

      if(Firebase.set(firebaseData, "/data/0/" + ts_str + "/humidity", h)) {
        Serial.println("PATH: " + firebaseData.dataPath()); //Set the humidity to h and print the path if successful 
      } else {
            Serial.println("FAILED"); //if it's not then alert us the reason why
          Serial.println("REASON: " + firebaseData.errorReason());
          Serial.println("------------------------------------");
          Serial.println();
      }
      if(Firebase.set(firebaseData, "/data/0/" + ts_str + "/temperature", t)){ //try to set the temperature to the temp 
        Serial.println("PATH: " + firebaseData.dataPath()); //if it can then print the path 
        } else {
            Serial.println("FAILED"); //if it can not then print the reason why it failed. 
          Serial.println("REASON: " + firebaseData.errorReason());
          Serial.println("------------------------------------");
          Serial.println();
      }
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
  //just for us to see that we are sending the right data to firebase.Print the humidity and temperature 
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
  //Delay again just to slow down date upload. You can change this but remember it will be +1 second as we have a delay at the beginning. 
  delay(1000);
}
void loop() {
    //define the temp again to be used in the if statement
    float t = dht.readTemperature();
    //If the temp is less than our set temp then turn the dryer on. 
    if (t < setTemp) {
      digitalWrite(32, HIGH); //We do that by setting it to high.
    }
    else if(t > setTemp) { //if it is more than our set temp then turn the dryer off.
      digitalWrite(32, LOW);
    }
    //Finally, run the update firebase function to update firebase. This is ran every 2.5 seconds. You can change it by changing the delay under this function call or in the function itself. 
    updateFB();
    
    delay(500);
}
