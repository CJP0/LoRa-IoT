var SerialPort = require('serialport');
var Readline = SerialPort.parsers.Readline;
var request = require('request');
var mqtt = require('mqtt');
var LoRaCardPort = process.argv[2];
var mqttServer = 'mqtt://' + process.argv[3];
var mqttAccount = process.argv[4];
var checkCode = "000";
var initCode = "111";
var gatewayID = "";

var opt = {
	port:1883,
	clientId:mqttAccount
};

var client = mqtt.connect(mqttServer, opt);
client.on('connect', function(){
	console.log('connect to mqtt server');
});

var serialPort = new SerialPort(LoRaCardPort, {
	baudRate: 9600
});

client.on('message', function(topic, msg){
	console.log('receive topic: ' + topic + ' msg: ' + msg);
	serialPort.write(msg);
});

var parser = new Readline();
serialPort.pipe(parser);
parser.on('data', function (data) {
	console.log('data received: ' + data);
	if(checkCode == data.slice(0, 3)){
		var url = data.slice(3);
		console.log(url);
		request(url, function(err, response, body){
			console.log('error:', err); 
			console.log('statusCode:', response && response.statusCode); 
			console.log('body:', body); 
		});
	}
	else if(initCode == data.slice(0, 3)){
		gatewayID = data.slice(3, 9);
		console.log('gatewayID: ' + gatewayID);
		client.subscribe(gatewayID);
		console.log('subscribe success topic: ' + gatewayID);
	}
});

serialPort.on('open', function () {
	console.log('Communication is on!');
});