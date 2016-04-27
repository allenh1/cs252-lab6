/*port = process.argv[1] || 8888;

function submitCompile() {
    var text = document.getElementById("textareaCode").value;
    var date = new Data();
    var name = "code" + date.getMonth() + "" + date.getHours() + "" +
	date.getMinutes() + "" + date.getFullYear() + ".bc";
    fs.write(name, text, function(err) {
	console.log("Saving:\n"+text);
    });
}
var io = require('socket.io').listen(80);

io.sockets.on('connection', function (socket) {
    socket.on('myClick', function (data) {
	socket.broadcast.emit('myClick', data);
    });
});*/

/*var net = require('net');

var HOST = '127.0.0.1';
PORT = process.argv[2] || 8888;

// Create a server instance, and chain the listen function to it
// The function passed to net.createServer() becomes the event handler for the 'connection' event
// The sock object the callback function receives UNIQUE for each connection
net.createServer(function(sock) {

    // We have a connection - a socket object is assigned to the connection automatically
    console.log('CONNECTED: ' + sock.remoteAddress +':'+ sock.remotePort);

    // Add a 'data' event handler to this instance of socket
    sock.on('data', function(data) {

	console.log('DATA ' + sock.remoteAddress + ': ' + data);
	// Write the data back to the socket, the client will receive it as data from the server
	sock.write('You said "' + data + '"');

    });

    // Add a 'close' event handler to this instance of socket
    sock.on('close', function(data) {
	console.log('CLOSED: ' + sock.remoteAddress +' '+ sock.remotePort);
    });

}).listen(PORT, HOST);

console.log('Server listening on ' + HOST +':'+ PORT);*/
var imported = document.createElement('script');
imported.src = 'require.js';
document.head.appendChild(imported);

var express = require('express');
var http = require('http');
var io = require('socket.io');

var connect = require('connect');
var serveStatic = require('serve-static');
connect().use(serveStatic(__dirname)).listen(2015, function(){
    console.log('Server running on 2015...');
});


var app = express();
app.use(express.static('./'));
//Specifying the public folder of the server to make the html accesible using the static middleware

var server =http.createServer(app).listen(2015);
//Server listens on the port 2015
io = io.listen(server);
/*initializing the websockets communication , server instance has to be sent as the argument */

io.sockets.on("connection",function(socket){
        /*Associating the callback function to be executed when client visits the page and 
	  websocket connection is made */

    var message_to_client = {
	data:"Connection with the server established"
    }
    socket.send(JSON.stringify(message_to_client));
    /*sending data to the client , this triggers a message event at the client side */
    console.log('Socket.io Connection with the client established');
    socket.on("textareaCode",function(data){
	/*This event is triggered at the server side when client sends the data using socket.send() method */
	data = JSON.parse(data);

	console.log(data);
	/*Printing the data */
	var ack_to_client = {
	    data:"Server Received the message"
	}
	socket.send(JSON.stringify(ack_to_client));
	/*Sending the Acknowledgement back to the client , this will trigger "message" event on the clients side*/
    });

});
