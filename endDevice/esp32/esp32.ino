#include <dht11.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
byte dhtPin=15;
dht11 DHT11;
char* ssid     = "";
char* password = "";
char* user = "";
const String sid = "dht002";
const String host = "";
const String port = ":80";
const char* mqttServer = "";
const int mqttPort = 1883;
unsigned long prevMillis = 0;
const long interval = 10000;
float oldTemp=0,temperature=0,oldHum=0,humidity=0;
byte chk;
WiFiClient espClient;
PubSubClient client(espClient);
void sendData(){
	HTTPClient http;
	String url = "http://" + host + port + "/addDht?user=" + user + "&sid=" + sid + "&temperature=" + temperature + "&humidity=" + humidity;
	http.begin(url);
	http.GET();
	http.end();
	Serial.println(url);
	Serial.print("humidity: ");
	Serial.print(humidity);
	Serial.print("  temperature: ");
	Serial.println(temperature);
	oldTemp=temperature;
	oldHum=humidity;
  }
void callback(char* topic, byte* payload, unsigned int len){
	Serial.print("receive mqtt topic : ");
	Serial.println(topic);
	for(int i=0;i<len;i++)
		Serial.print((char)payload[i]);
	Serial.println();
	Serial.println("-------------------");
}
void mqttConnect(){
	while(!client.connected()){
		if(client.connect(user))
			Serial.println("mqtt connect");
		else{
			Serial.println("fail connest mqtt");
			Serial.println(client.state());
			delay(2000);
		}
    
	}
}

void readDht(){
	int chk = DHT11.read(dhtPin);
	if(chk){
		temperature = DHT11.temperature;
		humidity = DHT11.humidity;
	}
}

void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    }
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  mqttConnect();
  char buf[20];
    String temp = user;
    temp += "/ctrl/";
    temp += sid;
    temp.toCharArray(buf,20);
    client.subscribe(buf);
    for(int i=0;i<20;i++)
      Serial.print(buf[i]);
    Serial.println();
}
 
void loop() {
	if(millis() - prevMillis > interval){
		prevMillis = millis();
		readDht();
		if((oldTemp!=temperature || oldHum!=humidity) && !isnan(temperature) && !isnan(humidity))
			sendData();
    }
	if(!client.connected())
		mqttConnect();
	client.loop();
}
