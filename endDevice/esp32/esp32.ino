#include <dht11.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <BluetoothSerial.h>
#include <EEPROM.h>
#define EEPROM_SIZE 64
const byte dhtPin = 15, sw01Pin = 2;
bool sw01 = false;
dht11 DHT11;
BluetoothSerial BT;
char ssidC[20] = "";
char passwordC[20] = "";
String ssid = "";
String password = "";
String user = "";
char userC[20], subscribeTopic[20];
const String sid = "DHT001";
const char* sidC = "DHT001";
const char* host = "serverIP";
//const String port = ":8080";
const int port = 8080;
const char* mqttServer = "serverIP";
const int mqttPort = 1883;
unsigned long prevMillis = 0, timeout;
const unsigned long interval = 10000;
float oldTemp=0,temperature=0,oldHum=0,humidity=0;
byte chk;
WiFiClient espClient;
PubSubClient client(espClient);
const byte wifiSsidRomAddress = 0, wifiPasswdRomAddress = 20, userRomAddress = 30;
bool wifiState = 0;
String serialBuf, text;

int EEPROM_write(String str, int address) {
	char buf[20];
	int len = str.length();
	str.toCharArray(buf, sizeof(buf));
	EEPROM.write(address++, len);
	for (int i=0; i<len; i++) {
		EEPROM.write(address++, buf[i]);
	}
	EEPROM.commit();
	return len;
}
void EEPROM_read(String &out, int address){
	byte len = EEPROM.read(address++);
	out = "";
	for (int i=0;i<len; i++) {
		out += (char)EEPROM.read(address++);
	}
}

void setUser(){
	Serial.println("waiting setUser");
	while(user.length() == 0){
		if(BT.available()){
			serialBuf = BT.readStringUntil('\r\n');
			if(serialBuf.substring(0, 8) == "SET+USER"){
				byte len = serialBuf.length() - 1;
				user = serialBuf.substring(8, len);
				EEPROM_write(user, userRomAddress);
			}
		}
	}
	Serial.println("+++++++user: " + user);
}

void sendData(){
	if (!espClient.connect(host, port)) {
        Serial.println("connection failed");
        return;
    }
	String url = "/addDht?user=" + user + "&sid=" + sid + "&temperature=" + temperature + "&humidity=" + humidity;
	espClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
	Serial.println(url);
	oldTemp=temperature;
	oldHum=humidity;

	timeout = millis();
    while (espClient.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            espClient.stop();
            return;
        }
    }

    while(espClient.available()) {
        String line = espClient.readStringUntil('\r');
        Serial.print(line);
    }
	Serial.println();
}
void callback(char* topic, byte* payload, unsigned int len){
	Serial.print("receive mqtt topic : ");
	Serial.println(topic);
	text = "";
	for(int i=0;i<len;i++){
		Serial.print((char)payload[i]);
		text += (char)payload[i];
	}
	Serial.println();
	Serial.println(text);
	Serial.println("-------------------");
	if(text.substring(0, 4) == "SET+"){
		setCallBack(text.substring(4));
	}
}
void mqttConnect(){
	while(!client.connected()){
		if(client.connect(sidC))
			Serial.println("mqtt connect");
		else{
			Serial.println("fail connest mqtt");
			Serial.println(client.state());
			delay(2000);
		}
	}
	client.subscribe(subscribeTopic);
}

void readDht(){
	int chk = DHT11.read(dhtPin);
	if(chk){
		temperature = DHT11.temperature;
		humidity = DHT11.humidity;
		Serial.print("temperature : ");
		Serial.print(temperature);
		Serial.print(", humidity : ");
		Serial.println(humidity);
	}
	else
		Serial.println("read dht11 fail");
}

bool BTwait(){
	if(BT.available()){
		serialBuf = BT.readStringUntil('\r\n');
		if(serialBuf.substring(0, 4) == "SET+"){
			setCallBack(serialBuf.substring(4));
			return true;
		}
	}
	return false;
}

void setCallBack(String str){
	byte len = str.length() - 1;
    if(str[len] == '\r')
	    str[len] = '\0';
	String buf = str.substring(0, 4);
	String buf2 = "";
	if(buf == "USER"){
		user = str.substring(4);
		EEPROM_write(user, userRomAddress);
		user.toCharArray(userC, 20);
	}
	else if(buf == "SSID"){
		ssid = str.substring(4);
		EEPROM_write(ssid, wifiSsidRomAddress);
		ssid.toCharArray(ssidC, 20);
		Serial.println(ssidC);
	}
	else if(buf == "PSWD"){
		password = str.substring(4);
		EEPROM_write(password, wifiPasswdRomAddress);
		password.toCharArray(passwordC, 20);
		Serial.println(passwordC);
	}
	else if(buf == "SW01"){
		buf2 = str.substring(4);
		sw01 = (byte)buf2.toInt();
		if(sw01)
			digitalWrite(sw01Pin, HIGH);
		else
			digitalWrite(sw01Pin, LOW);
		updateStatus("SW01", sw01);
	}
	else if(buf == "CLER"){
		EEPROM.write(wifiSsidRomAddress, 0);
		EEPROM.write(wifiPasswdRomAddress, 0);
		EEPROM.write(userRomAddress, 0);
		EEPROM.commit();
		ESP.restart();
	}
}

void updateStatus(String str, bool s){
	byte s0;
	s0= s ? 1 : 0;
	if (!espClient.connect(host, port)) {
        Serial.println("connection failed");
        return;
    }
	String url = "/chval?user=" + user + "&sid=" + sid + "&vari=" + str + "&val=" + s0;
	espClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
	Serial.println(url);
	timeout = millis();
    while (espClient.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            espClient.stop();
            return;
        }
    }

    while(espClient.available()) {
        String line = espClient.readStringUntil('\r');
        Serial.print(line);
    }
	Serial.println();
}

/*void updateStatusMqtt(String str, bool s){
	char buf0[30];
	for(int i=0;i<30;i++)
		buf0[i]='\0';
	String buf = user + "/state/" + sid + '/' + str;
	buf.toCharArray(buf0,30);
	if(s)
		client.publish(buf0, "1", true);
	else
		client.publish(buf0, "0", true);
	for(int i=0;i<30;i++)
		Serial.print(buf0[i]);
	Serial.println();
}*/

bool loadWifiInfo(){
	if(EEPROM.read(wifiSsidRomAddress)){
		EEPROM_read(ssid, wifiSsidRomAddress);
		EEPROM_read(password, wifiPasswdRomAddress);
		ssid.toCharArray(ssidC, 20);
		password.toCharArray(passwordC, 20);
		return true;
	}
	return false;
}

void connectWifi(){
	loadWifiInfo();
	Serial.print("connecting to ");
	Serial.println(ssidC);
	WiFi.begin(ssidC, passwordC);
	while(WiFi.status() != WL_CONNECTED){
		delay(500);
		Serial.print(".");
		if(BTwait()) return;
    }
	Serial.println();
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	client.setServer(mqttServer, mqttPort);
	client.setCallback(callback);
	mqttConnect();
	for(int i = 0 ; i < 20;i++)
		subscribeTopic[i]='\0';
    String temp = user;
    temp += "/ctrl/";
    temp += sid;
    temp.toCharArray(subscribeTopic,20);
    client.subscribe(subscribeTopic);
    for(int i=0;i<20;i++)
		Serial.print(subscribeTopic[i]);
    Serial.println();
	wifiState = 1;
}

void setup() {
	Serial.begin(9600);
	delay(10);
	pinMode (sw01Pin, OUTPUT);
	Serial.println();
	Serial.println();
	BT.begin(sidC);
	Serial.println("Bluetooth Device is Ready to Pair");
	if (!EEPROM.begin(EEPROM_SIZE)){
		Serial.println("failed to initialise EEPROM"); delay(1000000);
	}
	if(EEPROM.read(userRomAddress))
		EEPROM_read(user, userRomAddress);
	else
		setUser();
	user.toCharArray(userC, 20);
	Serial.println("user complete");
	while(!wifiState){
		BTwait();
		if(loadWifiInfo()){
			connectWifi();
		}
	}
	digitalWrite(sw01Pin, LOW);
	updateStatus("SW01", false);
}
 
void loop() {
	if(millis() - prevMillis > interval){
		prevMillis = millis();
		readDht();
		if((oldTemp!=temperature || oldHum!=humidity) && !isnan(temperature) && !isnan(humidity)){
			sendData();
		}
	}
	if(BT.available()){
		serialBuf = BT.readStringUntil('\r\n');
		if(serialBuf.substring(0, 4) == "SET+")
			setCallBack(serialBuf.substring(4));
	}
	if(!client.connected())
		mqttConnect();
	client.loop();
}
