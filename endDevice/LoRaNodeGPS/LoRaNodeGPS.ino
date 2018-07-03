#include <SPI.h>              
#include <LoRa.h>
#include <dht11.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <TinyGPS++.h>
#include "Arduino.h"


SoftwareSerial ss(A4, A5);
SoftwareSerial BT(4, 5);
const byte dhtPin = A0, userRomAddress = 0;
String text, serialBuf;             
byte msgCount = 1, sendf = 0, incomingMsgId;            
String sid = "GPS001";     
String confirmCode = "cjp";
String broadcast = "000000";      
unsigned long lastSendTime = 0;       
int interval = 30000, currentSnr, currentRssi;          
String temp = "";
String sender = "";
String destination = "";
char packetType;
dht11 DHT11;
TinyGPSPlus gps;
float temperature = 0, humidity = 0, pTemperature = 0, pHumidity = 0;
String serialData = "";
String user = "";
String host = "serverIP";
String port = "8080";
String route = "";
String routeReg = "";
String gateway = "";
double Latitude, Longitude;

int EEPROM_write(String str, int address) {
	char buf[20];
	int len = str.length();
	str.toCharArray(buf, sizeof(buf));
	EEPROM.write(address++, len);
	for (int i=0; i<len; i++) {
		EEPROM.write(address++, buf[i]);
	}
	return len;
}
void EEPROM_read(String &out, int address){
	byte len = EEPROM.read(address++);
	out = "";
	for (int i=0;i<len; i++) {
		out += (char)EEPROM.read(address++);
	}
}

void searchGateway(){
	gateway = "";
	while(gateway.length() == 0){
		sendMessage("searchGateway", broadcast, 'S', 0);
		waitSearchAck();
	}
	handOver();
}

void waitSearchAck(){
	unsigned long start = millis();
	int packetSize, f = 0, rssi = -200, tempn;
	String buf = "cjpA" + sid;
	String wReceive = "", tempg = "";
	while((millis()-start) < 2000){ 
		packetSize = LoRa.parsePacket();
		if (packetSize){
			wReceive = "";
			tempg = "";
			for(int i=0;i<10;i++)
				wReceive += (char)LoRa.read();
			for(int i=0;i<6;i++)
				tempg += (char)LoRa.read();
            //Serial.println("wRecevice: " + wReceive);
			tempn = LoRa.packetSnr();
			if((buf.equals(wReceive)) && (tempn > rssi)){
				rssi = tempn;
				gateway = tempg;
			}
			while (LoRa.available())
				LoRa.read();
		} 
	}
	currentRssi = rssi;
}

void listenChannel(){
	unsigned long start = millis();
	long listenTime = random(500, 700);
	while((millis()-start) < listenTime){ 
		if (onReceive(LoRa.parsePacket())){
			start = millis();
		}
	}
}

int waitAck(String wSender, byte MID){
	unsigned long start = millis();
	int packetSize, f = 0;
	String buf = "cjpA" + sid + wSender;
	String wReceive = "";
	delay(10);
	while((millis()-start) < 1500){ 
		packetSize = LoRa.parsePacket();
		if (packetSize){
			wReceive = "";
			for(int i=0;i<16;i++)
				wReceive += (char)LoRa.read();
            //Serial.println("wRecevice: " + wReceive);
			if((LoRa.read() == MID) && (buf.equals(wReceive))){
				f = 1;
			}
			while (LoRa.available())
				LoRa.read();
			if(f) break;
		} 
	}
	return f;
}

void readDht(){
	int chk = DHT11.read(dhtPin);
	if(chk){
		temperature = DHT11.temperature;
		humidity = DHT11.humidity;
	}
}

void sendDht(){
	char t[5], h[5];
	text = host + port + route + "user=" + user + "&sid=" + sid + "&temperature=";
	dtostrf(temperature, 3, 2, t);
	text += t;
	dtostrf(humidity, 3, 2, h);
	text += "&humidity=";
	text += h;
	sendMessage(text, gateway, 'D', 0);
	Serial.println("text: " + text);
}

void sendGPS(){
	char lati[10], lon[10];
	text = host + port + route + "user=" + user + "&sid=" + sid + "&lat=";
	dtostrf(Latitude, 3, 6, lati);
	text += lati;
	dtostrf(Longitude, 3, 6, lon);
	text += "&lon=";
	text += lon;
	sendMessage(text, gateway, 'D', 0);
}

void handOver(){
	text = host + port + routeReg + "user=" + user + "&sid=" + sid + "&gw=" + gateway;
	sendMessage(text, gateway, 'D', 0);
	Serial.println("text: " + text);
}

void setUser(){
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
void BTCallBack(String str){
	byte len = str.length() - 1;
	str[len] = '\0';
	if(str.substring(0, 4) == "USER"){
		user = str.substring(4);
		EEPROM_write(user, userRomAddress);
	}
}

void setup() {
	Serial.begin(9600);
	while (!Serial);

	if (!LoRa.begin(866E6)) {
		Serial.println("LoRa init failed. Check your connections.");
		while (true);
	}
	LoRa.enableCrc();
	BT.begin(9600);
	//BT.print("AT+NAME" + sid);
	if(EEPROM.read(userRomAddress))
		EEPROM_read(user, userRomAddress);
	else
		setUser();
	
	gateway = "";
	while(gateway.length() == 0){
		searchGateway();
	}
	ss.begin(9600);
	Serial.println("LoRa init succeeded.");
}

void loop() {
	while(millis() - lastSendTime > interval){
        ss.listen();
		if (ss.available() > 0){
			gps.encode(ss.read());
			if (gps.location.isUpdated()){
				Latitude = gps.location.lat();
				Longitude = gps.location.lng();
				Serial.print("Latitude= "); 
				Serial.print(Latitude);
				Serial.print(" Longitude= "); 
				Serial.println(Longitude);
                sendGPS();
                lastSendTime = millis();
			}
		}
	}
    BT.listen();
	if(BT.available()){
		serialBuf = BT.readStringUntil('\r\n');
        //Serial.println("BT: " + serialBuf);
		if(serialBuf.substring(0, 4) == "SET+")
			BTCallBack(serialBuf.substring(4));
	}
    onReceive(LoRa.parsePacket());
}

void sendMessage(String outgoing, String sDestination, char type, byte MID) {
	if(MID == 0) MID = msgCount;
	Serial.println("sendMessage to " + sDestination + " type: " + type + " MID: " + MID);
	Serial.println("outgoing: " + outgoing);
	while(1){
		int i;
		for(i=0;i<3;i++){
			if(type != 'A')
				listenChannel();
            else
                while(onReceive(LoRa.parsePacket())>0);
			LoRa.beginPacket();                   
			LoRa.print(confirmCode);
			LoRa.print(type);
			LoRa.print(sDestination);
			LoRa.print(sid);            
			LoRa.write(MID);                 
			LoRa.write(outgoing.length());        
			LoRa.print(outgoing);                 
			LoRa.endPacket(); 
			if(((type == 'A') || (type == 'S')) || waitAck(sDestination, MID)) break;
		}
		if(i>2){
			Serial.println("send packer fail");
			searchGateway();
		}
		else{
			Serial.println("send packet success");
			break;
		}
	}
	Serial.println();
	if(msgCount < 254)
		msgCount++;                          
	else
		msgCount = 1;
}

byte onReceive(int packetSize) {
	if (packetSize == 0) return 0;          

	temp = "";
	for(int i=0;i<3;i++)
		temp += (char)LoRa.read();
	if(!(confirmCode.equals(temp))) return 1;
	//Serial.println("temp : " + temp);
	packetType = (char)LoRa.read();
	
	if(packetType == 'S')
		return 1;

	destination = "";
	for(int i=0;i<6;i++)
		destination += (char)LoRa.read();
	if(!(sid.equals(destination)) && !(broadcast.equals(destination))){
		Serial.println("not my packet");
		return 1;
	}
	sender = "";
	for(int i=0;i<6;i++)
		sender += (char)LoRa.read();

	incomingMsgId = LoRa.read();
	byte incomingLength = LoRa.read();

	String incoming = "";

	while (LoRa.available()) {
		incoming += (char)LoRa.read();
	}

	if (incomingLength != incoming.length()) {
		Serial.println("error: message length does not match length");
		return 1;
	}
	//int tempSnr = LoRa.packetSnr();
	int tempRssi = LoRa.packetRssi();
	
	Serial.println("Received from: " + sender);
	Serial.println("Sent to: " + destination);
	Serial.print("Packet Type: "); 
	Serial.write(packetType);
	Serial.println();
	Serial.println("Message ID: " + String(incomingMsgId));
	Serial.println("Message length: " + String(incomingLength));
	Serial.println("Message: " + incoming);
	Serial.println("RSSI: " + String(tempRssi));
	Serial.println("Snr: " + String(LoRa.packetSnr()));
	Serial.println();
	if(packetType == 'T'){
		sendMessage("ack", sender, 'A', incomingMsgId);
	}
	else if((packetType == 'I') && (sender != gateway) && (tempRssi > currentRssi)){
		gateway = sender;
		currentRssi = tempRssi;
		handOver();
	}
	
	if(sender == gateway)
		currentRssi = tempRssi;
    return 1;
}

