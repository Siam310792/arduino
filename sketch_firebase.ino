#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>

#define FIREBASE_HOST "humiditeplantes.firebaseio.com/"
#define FIREBASE_AUTH "hwSjDnso1SLi4pJel1t9obEHkJLubKXst9cSUywi"

#define WIFI_SSID "H20"
#define WIFI_PASSWORD "qqqq4444"

#define WIFI_LED_CONNECT 12
#define LED_OUT 14
#define LED_STATUS_DRY 12
#define LED_STATUS_NORMAL 13
#define LED_STATUS_WET 15

#define CAV_PIN0 16
#define CAV_PIN1 4

#define SENSOR_MOISTURE A0


int idZone;
int connectedWifi;
int connectedFirebase;

void setup() {
  Serial.begin(9600);
  
  //define pinmodes
  pinMode(WIFI_LED_CONNECT,OUTPUT);   
  pinMode(LED_OUT, OUTPUT);
  pinMode(LED_STATUS_DRY, OUTPUT);
  pinMode(LED_STATUS_NORMAL, OUTPUT);  
  pinMode(LED_STATUS_WET,OUTPUT);      

  pinMode(CAV_PIN0, INPUT);
  pinMode(CAV_PIN1, INPUT);                    


  
  Serial.print("Connection to ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  
  //connect to wifi
  long timeout = millis();
  
  
  while (WiFi.status() != WL_CONNECTED && ((millis() - timeout) <= 20000)) {           //wait till connected to WiFi or timeout
    delay(100);  
    digitalWrite(WIFI_LED_CONNECT,LOW);             //Blink the light till connected to WiFi
    delay(100);
    digitalWrite(WIFI_LED_CONNECT,HIGH);
    Serial.print(".");
    Serial.println(millis() - timeout);
  }
  if(millis() - timeout >= 20000) {
    connectedWifi = 0;
    connectedFirebase = 0;
    Serial.println("");
    Serial.println("WiFi connection timed out.");
  } else {
    connectedWifi = 1;
    Serial.println("");
    Serial.println("WiFi connected");
    digitalWrite(WIFI_LED_CONNECT,LOW);  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);          //connect to Database
    if(Firebase.success()) {
      connectedFirebase = 1;
    } else {
      connectedFirebase = 0;
    }    
  }
  
  Serial.println("connectedWifi : " + String(connectedWifi));
  Serial.println("connectedFirebase : " + String(connectedFirebase));
  
  delay(1000);
}

void loop() {
  
  int pin0 = digitalRead(CAV_PIN0);
  int pin1 = digitalRead(CAV_PIN1);
  idZone = getId(pin0, pin1);
  long lastUpdate = 0;

  // Reade from Firebase

  if(millis() - lastUpdate >= 30000) {
    lastUpdate = millis();
    int autoMode = 1;
    if(connectedWifi && connectedFirebase) {
      autoMode = firebaseGet("/" + String(idZone) + "/autoValue");
    }
    Serial.println("modeAuto : " + String(autoMode));
    if(autoMode == 1) {
          
      //Reset Leds
      digitalWrite(LED_STATUS_DRY, LOW);
      digitalWrite(LED_STATUS_NORMAL, LOW);
      digitalWrite(LED_STATUS_WET, LOW);
      
      //Light Leds
      if(analogRead(SENSOR_MOISTURE) < 300) {
        Serial.println("I am thirsty, please give me water");
        digitalWrite(LED_STATUS_DRY, HIGH);
        digitalWrite(LED_OUT,HIGH);  
      }
      if(analogRead(SENSOR_MOISTURE) > 300 && analogRead(SENSOR_MOISTURE) < 800) {
        Serial.println("I feel so comfortable");
        digitalWrite(LED_STATUS_NORMAL, HIGH);        
        digitalWrite(LED_OUT,LOW);  
      }  
      if(analogRead(SENSOR_MOISTURE) > 800) {
        Serial.println("Too much water, I might get hurt");
        digitalWrite(LED_STATUS_WET, HIGH); 
        digitalWrite(LED_OUT,LOW); 
      }
      
      // Write to Firebase
      firebaseSet("/" + String(idZone) + "/inValue", analogRead(SENSOR_MOISTURE));
      delay(100);
      
    } else {
  
      //Reset Leds
      digitalWrite(LED_STATUS_DRY, LOW);
      digitalWrite(LED_STATUS_NORMAL, LOW);
      digitalWrite(LED_STATUS_WET, LOW);
        
      //Light Leds
      if(analogRead(SENSOR_MOISTURE) < 300) {
        Serial.println("I am thirsty, please give me water");
        digitalWrite(LED_STATUS_DRY, HIGH);
      }
      if(analogRead(SENSOR_MOISTURE) > 300 && analogRead(SENSOR_MOISTURE) < 800) {
        Serial.println("I feel so comfortable");
        digitalWrite(LED_STATUS_NORMAL, HIGH); 
      }  
      if(analogRead(SENSOR_MOISTURE) > 800) {
        Serial.println("Too much water, I might get hurt");
        digitalWrite(LED_STATUS_WET, HIGH); 
      }
      
      // Write to Firebase
      firebaseSet("/" + String(idZone) + "/inValue", analogRead(SENSOR_MOISTURE));
      delay(100);
    }
    if(connectedWifi && connectedFirebase) {
      int firebaseResult = firebaseGet("/" + String(idZone) + "/outValue");
      //Serial.println(firebaseResult);
      delay(100);
      
      if (Firebase.success() && firebaseResult == 0) {
        digitalWrite(LED_OUT,LOW);  
      } else if(Firebase.success() && firebaseResult == 1) {
        digitalWrite(LED_OUT,HIGH);  
      } 
    }    
  }  
} 

int firebaseGet(String s){
  int x = Firebase.getInt(s);
  yield();
  delay(100);
  return x;
  delay(100);
}

void firebaseSet(String s, int value) {
  Firebase.setInt(s, value);  
  yield();
  delay(100);
}

int getId(int pin0, int pin1) {
  int id = pin0 * 1 + pin1 *2;
  //Serial.println("idZone : " + String(id));
  return id;
}
