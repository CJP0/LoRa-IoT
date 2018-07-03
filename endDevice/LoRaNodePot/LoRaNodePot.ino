#include <SPI.h>              
#include <LoRa.h>
#include <dht11.h>
#include<SoftwareSerial.h>
#include <EEPROM.h>
#include "Arduino.h"

SoftwareSerial BT(4, 5);
const byte dhtPin = A0, soilSensor = A1, sw1Pin = 3, userRomAddress = 0, autoRomAddress = 20, startWaterAdd = 22, stopWaterAdd = 24;
String text, serialBuf;             
byte msgCount = 1, sendf = 0, autoWater = 1, incomingMsgId, sw1 = 0, startWater = 10, stopWater = 50;            
String sid = "pot001";     
String confirmCode = "cjp";
String broadcast = "000000";      
unsigned long lastSendTime = 0;       
int interval = 5000, currentSnr, currentRssi;          
String temp = "";
String sender = "";
String destination = "";
char packetType;
dht11 DHT11;
float temperature = 0, humidity = 0, pTemperature = 0, pHumidity = 0, soilHumidity = 0, pSoilHumidity = 0;
String serialData = "";
String user = "";
String host = "";
String port = "8080";
String route = "";
String chValRouter = "";
String chWaterRouter = "";
String routeReg = "";
String gateway = "";

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
	int i=0;
	while(gateway.length() == 0){
		sendMessage("searchGateway", broadcast, 'S', 0);
		waitSearchAck();
		++i;
		if(i > 2) break;
	}
	if(i < 3)
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
			tempn = LoRa.packetRssi();
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
	delay(5);
	while((millis()-start) < 1500){ 
		packetSize = LoRa.parsePacket();
		if (packetSize){
			wReceive = "";
			for(int i=0;i<16;i++)
				wReceive += (char)LoRa.read();
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

void sendSensorData(){
	char t[5], h[5], sh[5];
	text = host + port + route + "user=" + user + "&sid=" + sid + "&t=";
	dtostrf(temperature, 3, 2, t);
	text += t;
	dtostrf(humidity, 3, 2, h);
	text += "&h=";
	text += h;
	dtostrf(soilHumidity, 3, 2, sh);
	text += "&sh=";
	text += sh;
	sendMessage(text, gateway, 'D', 0);
}

void readSoil(){
	soilHumidity = analogRead(soilSensor);
	soilHumidity = map(soilHumidity, 2500, 0, 0, 100);
}

void handOver(){
	text = host + port + routeReg + "user=" + user + "&sid=" + sid + "&gw=" + gateway;
	sendMessage(text, gateway, 'D', 0);
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
void setCallBack(String str){
	byte len = str.length() - 1;
    if(str[len] == '\r')
	    str[len] = '\0';
	String buf = str.substring(0, 4);
	String buf2 = "";
	if(buf == "USER"){
		user = str.substring(4);
		EEPROM_write(user, userRomAddress);
	}
	else if(buf == "SW01"){
		buf2 = str.substring(4);
		sw1 = (byte)buf2.toInt();
		if(sw1)
			digitalWrite(sw1Pin, HIGH);
		else
			digitalWrite(sw1Pin, LOW);
		updateStatus("sw1", sw1);
	}
	else if(buf == "AUTO"){
		buf2 = str.substring(4);
		autoWater = (byte)buf2.toInt();
		updateStatus("autoWater", autoWater);
		EEPROM.write(autoRomAddress, autoWater);
	}
	else if(buf == "STWT"){
		buf2 = str.substring(4, 6);
		startWater = buf2.toInt();
		buf2 = str.substring(7);
		stopWater = buf2.toInt();
		updateWater();
		EEPROM.write(startWaterAdd, startWater);
		EEPROM.write(stopWaterAdd, stopWater);
	}
}

void loadSet(){
	autoWater = EEPROM.read(autoRomAddress);
	startWater = EEPROM.read(startWaterAdd);
	stopWater = EEPROM.read(stopWaterAdd);
}

void updateStatus(String vari, byte val){
	text = host + port + chValRouter + "user=" + user + "&sid=" + sid + "&vari=" + vari + "&val=" + val;
	sendMessage(text, gateway, 'D', 0);
}

void updateWater(){
	text = host + port + chWaterRouter + "user=" + user + "&sid=" + sid + "&start=" + startWater + "&stop=" + stopWater;
	sendMessage(text, gateway, 'D', 0);
}

void setup() {
	Serial.begin(9600);
	pinMode(sw1Pin, OUTPUT);
	while (!Serial);

	if (!LoRa.begin(866E6)) {
		Serial.println("LoRa init failed.");
		while (true);
	}
	LoRa.enableCrc();
	BT.begin(9600);
	//BT.print("AT+NAME" + sid);
	if(EEPROM.read(userRomAddress))
		EEPROM_read(user, userRomAddress);
	else
		setUser();
	
	/*EEPROM.write(startWaterAdd, startWater);
	EEPROM.write(stopWaterAdd, stopWater);
	EEPROM.write(autoRomAddress, autoWater);*/
	loadSet();
	Serial.print("startWater: ");
	Serial.println(startWater);
	Serial.print("stopWater: ");
	Serial.println(stopWater);
	Serial.print("autoWater: ");
	Serial.println(autoWater);
	
	while(gateway.length() == 0){
		searchGateway();
	}
	Serial.println("LoRa init succeeded.");
}

void loop() {
	if (millis() - lastSendTime > interval) {
		lastSendTime = millis();   
		readDht();
		readSoil();
		if((pTemperature != temperature) || (pHumidity != humidity) || (pSoilHumidity != soilHumidity)){
			sendSensorData();
			if(autoWater){
				if((soilHumidity <= startWater) && (sw1 == 0)){
					digitalWrite(sw1Pin, HIGH);
                    sw1 = 1;
					updateStatus("sw1", 1);
				}
				else if((soilHumidity >= stopWater) && sw1){
					digitalWrite(sw1Pin, LOW);
                    sw1 = 0;
					updateStatus("sw1", 0);
				}
			}
			pTemperature = temperature;
			pHumidity = humidity;
			pSoilHumidity = soilHumidity;
		}
	}
	if(BT.available()){
		serialBuf = BT.readStringUntil('\r\n');
		if(serialBuf.substring(0, 4) == "SET+")
			setCallBack(serialBuf.substring(4));
	}
    onReceive(LoRa.parsePacket());
}

void sendMessage(String outgoing, String sDestination, char type, byte MID) {
	if(MID == 0) MID = msgCount;
	Serial.println("sendMessage to " + sDestination + " type: " + type + " MID: " + MID);
	Serial.println("outgoing: " + outgoing);
	int c = 2;
	while(c--){
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
		if(incoming.substring(0, 4) == "SET+")
			setCallBack(incoming.substring(4));
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

