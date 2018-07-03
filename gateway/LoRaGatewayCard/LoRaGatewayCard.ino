#include <SPI.h>              // include libraries
#include <LoRa.h>
             
byte msgCount = 1;            
String sid = "Lgw001";     
String confirmCode = "cjp";
String broadcast = "000000";      
unsigned long lastSendTime = 0, publishInterval = 300000;       
int interval = 2000;         
String temp = "";
String sender = "";
String destination = "";
char packetType, charTemp;
String serialData = "";
byte sendf = 0, incomingMsgId;
String serialInput = "";

void listenChannel(){
	unsigned long start = millis();
	int packetSize;
	long listenTime = random(500, 700);
	while((millis()-start) < listenTime){ 
		if (onReceive(LoRa.parsePacket()))
			start = millis();
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
            Serial.println("wRecevice: " + wReceive);
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

void setup() {
	Serial.begin(9600); 
	while (!Serial);

	if (!LoRa.begin(866E6)) {
		Serial.println("LoRa init failed. Check your connections.");
		while (true); 
	}
	LoRa.enableCrc();
	Serial.println("LoRa init succeeded.");
    Serial.println("111"+sid);
}

void loop() {
    if(Serial.available()){
        serialInput = Serial.readString();
        Serial.println("========================");
        Serial.println(serialInput);
		sendMessage(serialInput.substring(6), serialInput.substring(0, 6), 'T', 0);
    }
	if((millis() - lastSendTime) > publishInterval){
		lastSendTime = millis();
		sendMessage("inform", broadcast, 'I', 0);
        Serial.println("*********send inform");
	}
    onReceive(LoRa.parsePacket());
}

void sendMessage(String outgoing, String sDestination, char type, byte MID) {
	if(MID == 0) MID = msgCount;
	Serial.println("sendMessage to " + sDestination + " type: " + type + " MID: " + MID);
	Serial.println("outgoing: " + outgoing);
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
		if((type == 'A') || (type == 'I') || waitAck(sDestination, MID)) break;
	}
	if(i>2) Serial.println("send packer fail");
	else Serial.println("send packet success");
    Serial.println();
	if(msgCount < 254)
		msgCount++;                          
	else
		msgCount = 1;
}

byte onReceive(int packetSize) {
	if (packetSize == 0) return 0;          
    //Serial.println("receive packer=================");
	temp = "";
	for(int i=0;i<3;i++)
		temp += (char)LoRa.read();
	if(!(confirmCode.equals(temp))) return 1;
	//Serial.println("temp : " + temp);
	packetType = (char)LoRa.read();

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

	Serial.println("Received from: " + sender);
	Serial.println("Sent to: " + destination);
	Serial.print("Packet Type: "); 
	Serial.write(packetType);
	Serial.println();
	Serial.println("Message ID: " + String(incomingMsgId));
	Serial.println("Message length: " + String(incomingLength));
	Serial.println("Message: " + incoming);
	Serial.println("RSSI: " + String(LoRa.packetRssi()));
	Serial.println("Snr: " + String(LoRa.packetSnr()));
	Serial.println();
	if(packetType == 'D'){
		serialData = "000" + incoming;
		Serial.println(serialData);
        sendMessage("ack", sender, 'A', incomingMsgId);
	}
	else if(packetType == 'S'){
        sendMessage("ack", sender, 'A', incomingMsgId);
	}
   return 1;
}

