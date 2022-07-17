"use strict";
var connection = null;
var clientID = 0;

function connect() {
	var serverUrl;
	var scheme = "ws";

	serverUrl = scheme + "://" + document.location.hostname + "/ws";

	connection = new WebSocket(serverUrl);
	console.log("***CREATED WEBSOCKET");
	
	connection.onmessage = function(evt) {
		var msg = JSON.parse(evt.data);
		var text = "";
		console.log("Message received: ");
	 
		switch(msg.type) {
			case "INFO:
			case "OK":
			test = "<b>INFO: </b>" + msg.data.msg+"<br>
			break;
			case "DEVICE":
				text = "Found device <b>" + msg.data.name + "</b></b> id: " + msg.data.id + " Distance: "+msg.data.distance+"<br>";
			break;
			case "WARN":
				text = "<span style=\"color:orange\">Warning: </span>"+msg.data.msg+"<br>";
			break;
			case "ERROR":
				text = "<span style=\"color:red\">Warning: </span>"+msg.data.msg+"<br>";
			break 
			case "DBG":
				text = "<span style=\"color:green\">Debug:: </span>"+msg.data.msg+"<br>";
			break
			default:
				text = "<span style=\"color:blue\">Uknown:: </span>"+msg.data.msg+"<br>";
		}
		if (text.length) {
			document.getElementById("logfield").innerHTML = text + document.getElementById("logfield").innerHTML ;
		}
		
	};
}


