#!/usr/bin/env nodejs
var express = require("express");
var app = express();
var mysql = require("mysql");
var mqtt = require("mqtt");
var opt = { oprt:1883, clientId:"server" };
var mqttServer = 'mqtt://ip';

var async = require("async");
app.get("/addDht",function(req, res){
	var user = req.query.user;
	var SID = req.query.sid;
	var dbname = user + "db";
	var temperature = req.query.temperature;
	var humidity = req.query.humidity;
	var index = 0;
	var sql = "SELECT * FROM " + SID + " WHERE id = 1";
	var con = mysql.createConnection({
		host: "localhost",
		user: "root",
		password: "",
		database: dbname
	});
	
	async.waterfall([
		function(next){
			con.connect(function(err){
				//if(err) throw err;
				//console.log("connected");
				if(err)
					res.send("connect fail");
				next(err);
			});
		},
		
		function(next){
			//console.log(sql);
			//res.send("sql login success");
			con.query(sql, function(err, result){
				//if (err) throw err;
				//console.log(result);
				index = result[0].nextID;
				sql = "SELECT id FROM "+ SID + " WHERE id = " + index;
				var client = mqtt.connect(mqttServer, opt);
				var topic = user + '/info/' + SID;
				client.on('connect', function () {
					if(result[0].temperature > temperature)
						client.publish(topic, 'now temperature too low');
					else if(result[0].temperature2 < temperature)
						client.publish(topic, 'now temperature too high');
					if(result[0].humidity > humidity)
						client.publish(topic, 'now humidity too low');
					else if(result[0].humidity2 < humidity)
						client.publish(topic, 'now humidity too high');
					client.end();
				});
				next(err);
			});
		},
		
		function(next){
			//console.log(sql);
			con.query(sql, function(err, result){
				//if(err) throw err;
				//console.log(result);
				try{
					var temp = result[0].id;
					sql = "UPDATE "+SID+" SET temperature="+temperature+", humidity="+humidity+" WHERE id = "+index;
					//console.log(sql);
				}
				catch(err){
					sql = "INSERT INTO " + SID + "(id, temperature, humidity) VALUES " + "(" + index + " ," + temperature + " ," + humidity + ")";
					//console.log(sql);
				}
				next(err);
			});
		},
		function(next){
			con.query(sql, function(err, result){
				//if(err) throw err;
				index += 1;
				if(index > 10010) index = 2;
				sql = "UPDATE "+SID+" SET nextID="+index+" WHERE id = 1";
				//console.log(sql);
				con.query(sql, function(err, result){
					//if(err) throw err;
					if(err)
						res.send("operate fail");
					else
						res.send("operate success");
					con.end();
				});
			});
		}
	]);
});

app.get("/addPot",function(req, res){
	var user = req.query.user;
	var SID = req.query.sid;
	var dbname = user + "db";
	var temperature = req.query.t;
	var humidity = req.query.h;
	var soilHumidity = req.query.sh;
	var index = 0;
	var sql = "SELECT * FROM " + SID + " WHERE id = 1";
	var con = mysql.createConnection({
		host: "localhost",
		user: "root",
		password: "",
		database: dbname
	});
	
	async.waterfall([
		function(next){
			con.connect(function(err){
				//if(err) throw err;
				//console.log("connected");
				if(err)
					res.send("connect fail");
				next(err);
			});
		},
		
		function(next){
			//console.log(sql);
			//res.send("sql login success");
			con.query(sql, function(err, result){
				//if (err) throw err;
				//console.log(result);
				index = result[0].nextID;
				sql = "SELECT id FROM "+ SID + " WHERE id = " + index;
				var client = mqtt.connect(mqttServer, opt);
				var topic = user + '/info/' + SID;
				client.on('connect', function () {
					if(result[0].temperature > temperature)
						client.publish(topic, 'now temperature too low');
					else if(result[0].temperature2 < temperature)
						client.publish(topic, 'now temperature too high');
					if(result[0].humidity > humidity)
						client.publish(topic, 'now humidity too low');
					else if(result[0].humidity2 < humidity)
						client.publish(topic, 'now humidity too high');
					if(result[0].soilHumidity > soilHumidity)
						client.publish(topic, 'now soil humidity too low');
					else if(result[0].soilHumidity2 < soilHumidity)
						client.publish(topic, 'now soil humidity too high');
					client.end();
				});
				next(err);
			});
		},
		
		function(next){
			//console.log(sql);
			con.query(sql, function(err, result){
				//if(err) throw err;
				//console.log(result);
				try{
					var temp = result[0].id;
					sql = "UPDATE "+SID+" SET temperature="+temperature+", humidity="+humidity+", soilHumidity="+soilHumidity+" WHERE id = "+index;
					//console.log(sql);
				}
				catch(err){
					sql = "INSERT INTO " + SID + "(id, temperature, humidity, soilHumidity) VALUES " + "(" + index + " ," + temperature + " ," + humidity + " ," + soilHumidity + ")";
					//console.log(sql);
				}
				next(err);
			});
		},
		function(next){
			con.query(sql, function(err, result){
				//if(err) throw err;
				index += 1;
				if(index > 10010) index = 2;
				sql = "UPDATE "+SID+" SET nextID="+index+" WHERE id = 1";
				//console.log(sql);
				con.query(sql, function(err, result){
					//if(err) throw err;
					if(err)
						res.send("operate fail");
					else
						res.send("operate success");
					con.end();
				});
			});
		}
	]);
});

app.get("/chval",function(req, res){
	var user = req.query.user;
	var SID = req.query.sid;
	var dbname = user + "db";
	var vari = req.query.vari;
	var val = req.query.val;
	var sql = "UPDATE " + SID + " SET " + vari + "=" + val + " WHERE id = 1";
	var con = mysql.createConnection({
		host: "localhost",
		user: "root",
		password: "",
		database: dbname
	});
	
	async.waterfall([
		function(next){
			con.connect(function(err){
				//if(err) throw err;
				//console.log("connected");
				if(err)
					res.send("connect fail");
				next(err);
			});
		},
		
		function(next){
			//console.log(sql);
			//res.send("sql login success");
			con.query(sql, function(err, result){
				if (err)
					res.send("operate fail");
				else
					res.send("operate success");
				var client = mqtt.connect(mqttServer, opt);
				var topic = user + '/info/' + SID;
				client.on('connect', function () {
					var buf = vari + ' = ' + val;
					client.publish(topic, buf);
					client.end();
				});
				con.end();
				next(err);
			});
		}
	]);
});

app.get("/chWater",function(req, res){
	var user = req.query.user;
	var SID = req.query.sid;
	var dbname = user + "db";
	var start = req.query.start;
	var stop = req.query.stop;
	var sql = "UPDATE " + SID + " SET waterStart=" + start + ", waterStop=" + stop + " WHERE id = 1";
	var con = mysql.createConnection({
		host: "localhost",
		user: "root",
		password: "",
		database: dbname
	});
	
	async.waterfall([
		function(next){
			con.connect(function(err){
				//if(err) throw err;
				//console.log("connected");
				if(err)
					res.send("connect fail");
				next(err);
			});
		},
		
		function(next){
			//console.log(sql);
			//res.send("sql login success");
			con.query(sql, function(err, result){
				if(err)
					res.send("operate fail");
				else
					res.send("operate success");
				var client = mqtt.connect(mqttServer, opt);
				var topic = user + '/info/' + SID;
				client.on('connect', function () {
					var buf = 'waterStart=' + start + ',waterStop=' + stop;
					client.publish(topic, buf);
					client.end();
				});
				con.end();
				next(err);
			});
		}
	]);
});

app.get("/addGPS",function(req, res){
	var user = req.query.user;
	var SID = req.query.sid;
	var dbname = user + "db";
	var lat = req.query.lat;
	var lon = req.query.lon;
	var index = 0;
	var sql = "SELECT * FROM " + SID + " WHERE id = 1";
	var con = mysql.createConnection({
		host: "localhost",
		user: "root",
		password: "",
		database: dbname
	});
	
	async.waterfall([
		function(next){
			con.connect(function(err){
				//if(err) throw err;
				//console.log("connected");
				if(err)
					res.send("connect fail");
				next(err);
			});
		},
		
		function(next){
			//console.log(sql);
			//res.send("sql login success");
			con.query(sql, function(err, result){
				//if (err) throw err;
				//console.log(result);
				index = result[0].nextID;
				sql = "SELECT id FROM "+ SID + " WHERE id = " + index;
				next(err);
			});
		},
		
		function(next){
			//console.log(sql);
			con.query(sql, function(err, result){
				//if(err) throw err;
				//console.log(result);
				try{
					var temp = result[0].id;
					sql = "UPDATE "+SID+" SET Latitude="+lat+", Longitude="+lon+" WHERE id = "+index;
					//console.log(sql);
				}
				catch(err){
					sql = "INSERT INTO " + SID + "(id, Latitude, Longitude) VALUES " + "(" + index + " ," + lat + " ," + lon + ")";
					//console.log(sql);
				}
				next(err);
			});
		},
		function(next){
			con.query(sql, function(err, result){
				//if(err) throw err;
				index += 1;
				if(index > 10010) index = 2;
				sql = "UPDATE "+SID+" SET nextID="+index+" WHERE id = 1";
				//console.log(sql);
				con.query(sql, function(err, result){
					//if(err) throw err;
					if(err)
						res.send("operate fail");
					else
						res.send("operate success");
					con.end();
				});
			});
		}
	]);
});

app.get("/registerLgw",function(req, res){
	var user = req.query.user;
	var SID = req.query.sid;
	var dbname = user + "db";
	var gateway = req.query.gw;
	var index = 0;
	var sql = "SELECT * FROM LoRaNode WHERE SID = '" + SID + "'";
	var con = mysql.createConnection({
		host: "localhost",
		user: "root",
		password: "",
		database: dbname
	});
	
	async.waterfall([
		function(next){
			con.connect(function(err){
				if(err) throw err;
				//console.log("connected");
				if(err)
					res.send("connect fail");
				next(err);
			});
		},
		
		function(next){
			//console.log(sql);
			con.query(sql, function(err, result){
				//if (err) throw err;
				//console.log(result);
				try{
					var t = result[0].gateway;
					sql = "UPDATE LoRaNode SET gateway='"+gateway+"' WHERE SID = '"+SID+"'";
					//console.log(sql);
				}
				catch(err){
					sql = "INSERT INTO LoRaNode(SID, gateway) VALUES " + "('" + SID + "', '" + gateway + "')";
					//console.log(sql);
				}
				next(err);
			});
		},
		function(next){
			//console.log(sql);
			con.query(sql, function(err, result){
				//if(err) throw err;
				if(err)
					res.send("operate fail");
				else{
					res.send("operate success");
					var temp = SID + gateway;
					var client = mqtt.connect(mqttServer, opt);
					var topic = user + '/info/' + SID;
					client.on('connect', function () {
						client.publish(topic, temp);
						client.end();
					});
				}
				con.end();
			});
		}
	]);
});

app.get("*", function(req, res){
	res.send('This is general route, develop edition');
});
app.listen(8000, function(req, res){
	console.log("server start in 8000 port");
});
