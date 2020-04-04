#include <ssl_client.h>
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <cert.h>
#include <Adafruit_Sensor.h>
#include "ArduinoJson.h" 
#include "DHT.h"

#define DHTPIN 14    // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11 
// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);
int motorPin = 27;
const int moistPin = 33;
int moisture = 0;
const int waterPin = 32;
int water = 0;

WiFiClientSecure secureClient = WiFiClientSecure();
void callback(char* topic, byte* payload, unsigned int length);
//PubSubClient mqttClient("a3jjkbw4vqw4wh-ats.iot.us-east-1.amazonaws.com",8883,callback,secureClient);
PubSubClient mqttClient(secureClient);



char SSID[] = "WiseFi";
const char *PWD = "*********";
 

//AWS_IOT hornbill;
 
void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, PWD); 
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    //we can even make the ESP32 to sleep
  }
 
  Serial.print("Connected - ");
 
}

void callback(char* topic,byte* payload,unsigned int length1){    
  Serial.print("message arrived[");
  Serial.print(topic);
  Serial.println("]");
  
  for(int i=0;i<length1;i++){
    Serial.println("Payload Value");
    Serial.println(payload[i]); 
    
  }
  if(payload[0]==49) {
    digitalWrite(motorPin,HIGH);    //ASCII VALUE OF '1' IS 49
  }
  else if (payload[0]==50) {
    digitalWrite(motorPin,LOW);//ASCII VALUE OF '2' IS 50
  }
  
  Serial.println();
}

 void connectToAWS() {
  mqttClient.setServer("a3jjkbw4vqw4wh-ats.iot.us-east-1.amazonaws.com", 8883);
  mqttClient.setCallback(callback);
  secureClient.setCACert(AWS_PUBLIC_CERT);
  secureClient.setCertificate(AWS_DEVICE_CERT);
  secureClient.setPrivateKey(AWS_PRIVATE_KEY);
 
  Serial.println("Connecting to MQTT....");
 
  mqttClient.connect("Irrigation");
  // mqttClient.connect("Irrigation","shubham93dhupar@gmail.com","davpublic74824");
  Serial.println(mqttClient.state());
  while (!mqttClient.connected()) {
    Serial.println(mqttClient.state());
    Serial.println("Connecting to MQTT....Retry");
    mqttClient.connect("Irrigation");
    //mqttClient.connect("Irrigation","shubham93dhupar@gmail.com","davpublic74824");
    delay(5000);
  } 
  Serial.println(mqttClient.state());
 
  Serial.println("MQTT Connected");
}

void loop (){
  moisture = analogRead(moistPin);
  //Serial.println(moisture);
  water = analogRead(waterPin);
  //Serial.println(water);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  // Compute the hit Index using celsius
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);
 
  Serial.print("Temperature:");
  Serial.println(temperature);
  Serial.print("Humidity:");
  Serial.println(humidity);
  Serial.print("Heat Index:");
  Serial.println(heatIndex);
  Serial.print("Soil Moisture:");
  Serial.println(moisture);
  Serial.print("Soil water content:");
  Serial.println(water);
  
  StaticJsonDocument<128> jsonDoc;
  JsonObject eventDoc = jsonDoc.createNestedObject("event");
  eventDoc["temp"] = temperature;
  eventDoc["hum"] = humidity;
  //eventDoc["hi"] = heatIndex;
  eventDoc["moist"] = moisture;
  eventDoc["water"] = water;
 
  char jsonBuffer[128];
 
  serializeJson(eventDoc, jsonBuffer);
  mqttClient.publish("Irrigation", jsonBuffer);
  delay(10000);
  if(mqttClient.connect("Irrigation")) {
    mqttClient.subscribe("Result");
    Serial.println(mqttClient.subscribe("Result"));
   // mqttClient.loop();
    delay(10000);
  }
}
  
void setup() {
  pinMode(motorPin, OUTPUT);
  pinMode(moistPin, INPUT);
  pinMode(waterPin, INPUT);
  Serial.begin(115200);
  connectToWiFi();
  connectToAWS();
   
}
