const msgpack = require("msgpack-lite");
const nrf24 = require("nrf24");
const ThingSpeakClient = require('thingspeakclient');
const MongoClient = require('mongodb').MongoClient;

var rf24 = new nrf24.nRF24(22, 0);
rf24.begin(print_debug=true);
rf24.config({
	PALevel: nrf24.RF24_PA_LOW,
	DataRate: nrf24.RF24_1MBPS,
	Irq: 27,
}, print_details=true);

var client = new ThingSpeakClient();
var tsChannel = 1762592;
client.attachChannel(tsChannel, { writeKey:'Z9U0WBBEP0HR1SMX', readKey:'UDOBJHDDQ7LDDFFS'});
var dbUrl = "mongodb://localhost:27017/";

var direction1 = "0x65646f4e31";
var direction2 = "0x65646f4e32";
var direction3 = "0x65646f4e33";

var pipe1 = rf24.addReadPipe(direction1, true);
var pipe2 = rf24.addReadPipe(direction2, true);
var pipe3 = rf24.addReadPipe(direction3, true);

var bufferPipe1 = Buffer.alloc(32);
var bufferPipe2 = Buffer.alloc(32);
var bufferPipe3 = Buffer.alloc(32);

var msgPipe1;
  
rf24.read( function (data,items) {
	for(var i=0;i<items;i++) {
		if(data[i].pipe == pipe1) {
			// data[i].data will contain a buffer with the data
			bufferPipe1 = data[i].data;
			console.log(data[i].data);
			console.log('Message from dir:%s', direction1);
			msgPipe1 = msgpack.decode(bufferPipe1)
			console.log(msgPipe1);
			sendToThingSpeak(msgPipe1, tsChannel);
			writeToDB(msgPipe1, "noise")
		} else if (data[i].pipe == pipe2) {
			// rcv from 0xABCD11FF56
			bufferPipe2 = data[i].data
			console.log('Message from dir:%s', direction2);
			msgPipe2 = msgpack.decode(bufferPipe2)
			console.log(msgPipe2)
			sendToThingSpeak(msgPipe2, tsChannel);
			writeToDB(msgPipe2, "noise")
		} else {
			bufferPipe3 = data[i].data;
			console.log('Message from dir:%s', direction3);
			msgPipe3 = msgpack.decode(bufferPipe3)
			console.log(msgPipe3)
			sendToThingSpeak(msgPipe3, tsChannel);
			writeToDB(msgPipe3, "noise")
		}
	}}, function(stop,by_user,err_count) {
		console.log("Error");
	});

function sendToThingSpeak (data, channel) {
	tsData = {
		field1: data.dBA,
	};
	//console.log(tsData);
	client.updateChannel(channel, tsData, function(err, resp) {
		if (!err && resp > 0) {
			console.log('update successfully. Entry number was: ' + resp);
		}
	});
}

function writeToDB (data, collection) {
	
	var now = new Date();
	data._id = now.toISOString();

	MongoClient.connect(dbUrl, function(err, db) {
		if (err) throw err;
		var dbo = db.db("sonometers");
		dbo.collection(collection).insertOne(data, function(err, res) {
			if (err) throw err;
			console.log("1 document inserted on %s in DB", collection);
			db.close();
		});
	});
}
