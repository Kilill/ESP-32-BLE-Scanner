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
      case "device":
        text = "Found device <b>" + msg.data.name + "</b></b> id: " + msg.data.id + " Distance: "+msg.data.distance+"<br>";
        break;
      case "warn":
        text = "<span style=\"color:orange\">Warning: </span>"+msg.data.msg+"<br>";
        break;
    }
    if (text.length) {
      document.getElementById("logfield").innerHTML = text + document.getElementById("logfield").innerHTML ;
    }
    
  };
}


