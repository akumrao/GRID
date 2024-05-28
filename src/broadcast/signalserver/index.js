// (c) 2023 Adappt.  All Rights reserved.
// https://stackoverflow.com/questions/62370962/how-to-create-join-chat-room-using-ws-websocket-package-in-node-js

'use strict';

var os = require('os');
const fs = require('fs');
var nodeStatic = require('node-static');
var http = require('https');
var socketIO = require('socket.io');
const config = require('./config');
const express = require('express');

const {
    v4: uuidV4
} = require('uuid');

let webServer;
let socketServer;
let expressApp;
let io;

(async () => {
    try {
        await runExpressApp();
        await runWebServer();
        await runSocketServer();
    } catch (err) {
        console.error(err);
    }
})();

console.log("To browse https://localhost/ or https://ip/");

let serverSocketid = null;

var fileServer = new(nodeStatic.Server)();

async function runExpressApp() {
    expressApp = express();
    expressApp.use(express.json());
    expressApp.use(express.static(__dirname));

    expressApp.use((error, req, res, next) => {
        if (error) {
            console.log('Express app error,', error.message);

            error.status = error.status || (error.name === 'TypeError' ? 400 : 500);
            res.statusMessage = error.message;
            res.status(error.status).send(String(error));
        } else {
            next();
        }
    });
}

async function runWebServer() {
    console.error('runWebServer');

    const {
        sslKey,
        sslCrt
    } = config;
    if (!fs.existsSync(sslKey) || !fs.existsSync(sslCrt)) {
        console.error('SSL files are not found. check your config.js file');
        process.exit(0);
    }
    const tls = {
        cert: fs.readFileSync(sslCrt),
        key: fs.readFileSync(sslKey),
    };
    
    webServer = http.createServer(tls, expressApp);
    //webServer = http.createServer(expressApp); // for http
    webServer.on('error', (err) => {
        console.error('starting web server failed:', err.message);
    });

    await new Promise((resolve) => {
        const {
            listenIp,
            listenPort
        } = config;
        webServer.listen(listenPort, listenIp, () => {
            console.log('server is running');
            console.log(`open http://127.0.0.1:${listenPort} in your web browser`);
            resolve();
        });
    });
}

async function runSocketServer() {
    console.error('runSocketServer');
    io = socketIO.listen(webServer);

    io.sockets.on('connection', function(socket) {
        // convenience function to log server messages on the client
        function log() {
            var array = ['log:'];
            array.push.apply(array, arguments);
	    // Uncomment to see server side messages on client side JavaScript console.
            // socket.emit('log', array);
            console.log(array);
        }

        socket.on('disconnect', function() {


            console.log( "disconnect "+ socket.id);
            var clientsInRoom = io.sockets.adapter.rooms[socket.room];

            if ( !clientsInRoom)
            {
                 console.log( "error at disconnect, not client in room");
                 return 
            }

            

            if ( !socket.isclient) {

                // console.log("server down " + serverSocketid);
                // serverSocketid = null;

                // for (let soc in io.sockets.connected) {
                //     io.sockets.connected[soc].emit('leave', socket.room, -1, -1);
                //     io.sockets.connected[soc].disconnect();
                // }

                    //var numClients = clientsInRoom.length; 
                    //console.log( "disconnect" , numClients);

                    Object.keys(clientsInRoom.sockets).forEach(function(scid){
                     
                     let sc = io.sockets.connected[scid];

                     if(sc.isclient)
                     {
                        console.log( " webrtc is down, so client leaved " + sc.id);
                        sc.emit('leave', socket.room, -1, -1);
                        sc.disconnect();
                     }


                    });
                }


            else  
            {

               // var numClients = clientsInRoom.length; 
                //console.log( "disconnect" , numClients);


                
                Object.keys(clientsInRoom.sockets).forEach(function(scid){
                     
                let sc = io.sockets.connected[scid];
                 
                 if(!sc.isclient)
                 {
                    console.log( "clinet is closed " + socket.id);

                    sc.emit('disconnectClient', socket.id);
                    
                 }


                });

            }
        });

        // socket.on('WebrtcSocket', function() {
        //     log('Received request to create WebrtcSocket');

        //     if (serverSocketid !== null && io.sockets.connected[serverSocketid]) {
        //         io.sockets.connected[serverSocketid].disconnect();
        //         serverSocketid = null;
        //     }

        //     log('Webrtc ID ', socket.id);
        //     serverSocketid = socket.id;
        // });

        socket.on('createorjoin', function(roomId, client) {
            
        
            if (socket.room)
                socket.leave(socket.room);

            socket.room = roomId;
            socket.join(roomId);

            if(!client)
             socket.isclient= false;
            else 
            socket.isclient = true;


            log('Received request to createorjoin room ' + roomId + " isclient " +  socket.isclient ) ;


           
            var clientsInRoom = io.sockets.adapter.rooms[socket.room];
  
            var numClients = clientsInRoom.length; //For socket.io versions >= 1.4:

            log('Room ' + roomId + ' now has ' + numClients + ' client(s)');

            if (numClients === 1) {
                log('Client ID ' + socket.id + ' created room ' + roomId);
                //when first time client connection then socket is created, store socket id and then emit  Join event
               // socket.emit('created', roomId, socket.id);
                socket.emit('join', roomId, socket.id, numClients);
            } else if (numClients > 1) {
                //log('Client ID ' + socket.id + ' joined room ' + roomId);
                //if already client connections are there then we send event Joined event.
                
                // when all the users in room need get joining event

                
                Object.keys(clientsInRoom.sockets).forEach(function(scid){
                 
                 let sc = io.sockets.connected[scid];

                 console.log('joined room: %o %o', sc.isclient , scid);
                 
                 if(!sc.isclient)
                 {
                    console.log('joined room: ', roomId);

                    io.sockets.in(roomId).emit('joined', roomId, socket.id, numClients);
                 }

                });

                socket.emit('join', roomId, socket.id, numClients);

            }
        });

        socket.on('message', function(message) {
            console.log('webrtc server message: ', message.type);

            message.from = socket.id;

            socket.to(message.to).emit('message', message);
        });

        socket.on('messageToWebrtc', function(message) {
            message.from = socket.id;


            console.log('app message: ', message.type);

            console.log( message.room );

            var clientsInRoom = io.sockets.adapter.rooms[message.room];
            if(!clientsInRoom)
                return;

            //console.log( clientsInRoom );

         //   var numClients = clientsInRoom.length; //For socket.io versions >= 1.4:
            Object.keys(clientsInRoom.sockets).forEach(function(scid){
             
             let sc = io.sockets.connected[scid];
                 
             if(!sc.isclient)
             {
                sc.emit('message', message);
             }


            });


        });

        /* To broadcast to all in a roomm including sender: socket.in(room).emit();
	   To exclude sender: socket.to(); */
        socket.on('postAppMessage', function(message) {
            if (message.type === "user") {
                message.user = message.desc;
            }

            console.log('notification ' + JSON.stringify(message, null, 4));
            message.from = socket.id;

            if (socket.user)
                message.user = socket.user;

            if (message.type === "chat") {
                if ('room' in message) {
                    io.in(message.room).emit('message', message);
                }
            } else {
                //if ('room' in message) {
                socket.to(message.to).emit('message', message);
                //}
            }
        });

        socket.on('bye', function() {
            console.log('received bye');
        });

    });
}



