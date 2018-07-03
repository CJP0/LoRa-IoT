#include "DHT.h"        // including the library of DHT11 temperature and humidity sensor
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>
#define DHTTYPE DHT11   
#define dht_dpin 2

char* user = "";            
char* passwd = "";      
const char* host = "";
const char* sid = "dht001";
WiFiClient client;
const int httpPort = 8080;
char table[] = "test";
const char* ssid     = "";
const char* password = "";

float h, t, before_h=0, before_t=0;
DHT dht(dht_dpin, DHTTYPE); 
void dhtDisplay(){
    //float h = dht.readHumidity();
    //float t = dht.readTemperature();         
    Serial.print("Current humidity = ");
    Serial.print(h);
    Serial.print("%  ");
    Serial.print("temperature = ");
    Serial.print(t); 
    Serial.println("C  ");
 }
void sendData1(){
  String url = "/addDht?user=";
  url += user;
  //url += "&passwd=";
  //url += passwd;
  url += "&sid=";
  url += sid;
  url += "&temperature=";
  url += t;
  url += "&humidity=";
  url += h;

  // This will send the request to the server
  Serial.print("Connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpPort))
    Serial.println("Connection failed!");
  Serial.print("Requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      //return;
    }
  }
  }
void setup(void)
{ 
  dht.begin();
  Serial.begin(9600);
  delay(5000);
  //wifi connect
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Connecting to ");
  Serial.println(host);
  // Use WiFiClient class to create TCP connections
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed!");
    return;
  }
  delay(700);
}
void loop() {
  h = dht.readHumidity();
  t = dht.readTemperature();
  if((h != before_h || t != before_t) && !(isnan(h) || isnan(t))){
      sendData1();
      dhtDisplay();
      before_h=h;
      before_t=t;
    }
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
  delay(5000);
}

