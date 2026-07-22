// (c) 2023 Adappt.  All Rights reserved.
// https://stackoverflow.com/questions/62370962/how-to-create-join-chat-room-using-ws-websocket-package-in-node-js

'use strict';

var os = require('os');
const fs = require('fs');
var nodeStatic = require('node-static');
var http = require('https');
//const WebSocket = require('ws');
const { WebSocketServer, WebSocket } = require('ws');
const config = require('./config');
const express = require('express');

const {
    v4 : uuidv4
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

async function runWebServer() 
{


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
            console.log(`open https://127.0.0.1:${listenPort} in your web browser`);
            resolve();
        });
    });
}


// Global maps to handle standard room management and state tracking
// wsState helps attach metadata (id, room, isclient) directly to the socket object abstraction
const wsState = new Map(); 
const rooms = new Map(); // Map format: roomId -> Set of socket ids

async function runSocketServer() {
    console.error('runSocketServer');
    
    // Initialize the native WebSocket server attached to your HTTP webServer
    const wss = new WebSocketServer({ server: webServer });

    // Helper functions replacing Socket.io room abstractions
    function joinRoom(roomId, socketId) {
        if (!rooms.has(roomId)) {
            rooms.set(roomId, new Set());
        }
        rooms.get(roomId).add(socketId);
    }

    function leaveRoom(roomId, socketId) {
        if (rooms.has(roomId)) {
            rooms.get(roomId).delete(socketId);
            if (rooms.get(roomId).size === 0) {
                rooms.delete(roomId);
            }
        }
    }

    function getClientsInRoom(roomId) {
        return rooms.get(roomId) || new Set();
    }

    function emitToSocket(targetSocketId, eventName, data) {
        const clientState = wsState.get(targetSocketId);
        if (clientState && clientState.ws.readyState === WebSocket.OPEN) {
            clientState.ws.send(JSON.stringify({ event: eventName, payload: data }));
        }
    }

    wss.on('connection', function(ws, req) {

        console.log('received');

        // Generate a unique ID for this client mimicking socket.id
        const socketId = Math.random().toString(36).substring(2, 15);
        
        // Initialize state tracker for the newly connected socket
        const socketContext = {
            id: socketId,
            room: null,
            isclient: false,
            ws: ws
        };
        wsState.set(socketId, socketContext);

        function log() {
            var array = ['log:'];
            array.push.apply(array, arguments);
            console.log(array);
        }

        // Handle client disconnection
        ws.on('close', function() {
            console.log("disconnect " + socketContext.id);
            if (!socketContext.room) return;

            const clientsInRoom = getClientsInRoom(socketContext.room);

            if (clientsInRoom.size === 0) {
                console.log("error at disconnect, no client in room");
                return;
            }

            if (!socketContext.isclient) {
                // Main / Host WebRTC peer went down -> Kick out and alert all clients in the room
                clientsInRoom.forEach(function(scid) {
                    let sc = wsState.get(scid);
                    if (sc && sc.isclient) {
                        console.log(" webrtc is down, so client leaved " + sc.id);
                        emitToSocket(sc.id, 'leave', [socketContext.room, -1, -1]);
                        sc.ws.close();
                    }
                });
            } else {
                // Client peer went down -> Notify the non-client (host/server) inside the room
                clientsInRoom.forEach(function(scid) {
                    let sc = wsState.get(scid);
                    if (sc && !sc.isclient) {
                        console.log("clinet is closed " + socketContext.id);
                        emitToSocket(sc.id, 'disconnectClient', socketContext.id);
                    }
                });
            }

            // Cleanup memory architectures
            leaveRoom(socketContext.room, socketContext.id);
            wsState.delete(socketContext.id);
        });

        // Main event router for Native WebSockets
        ws.on('message', function(rawData) {
            try {
                // Native WebSockets only receive string/buffer. Parse out the Custom Event wrapper.
                const packet = JSON.parse(rawData);
                const event = packet.event;
                const data = packet.payload;

                // ROUTE: createorjoin
                if (event === 'createorjoin') {
                    const roomId = data.roomId;

                    if (socketContext.room) {
                        leaveRoom(socketContext.room, socketContext.id);
                    }

                    socketContext.room = roomId;
                    joinRoom(roomId, socketContext.id);

                    log('Received request to createorjoin room ' + roomId + " isclient " + socketContext.isclient);

                    const clientsInRoom = getClientsInRoom(socketContext.room);
                    const numClients = clientsInRoom.size;

                    log('Room ' + roomId + ' now has ' + numClients + ' client(s)');

                    if (numClients === 1) {
                        socketContext.isclient = false;
                        log('Client ID ' + socketContext.id + ' created room ' + roomId);
                        emitToSocket(socketContext.id, 'join', [roomId, socketContext.id, numClients]);
                    } else if (numClients > 1) {
                        
                        socketContext.isclient = true;
                        log('Client ID ' + socketContext.id + ' joined room ' + roomId + ' isclinet '+ socketContext.isclient);

                        clientsInRoom.forEach(function(scid) {
                            let sc = wsState.get(scid);
                            if (sc) {
                                console.log('joined room: %o %o', sc.isclient, scid);
                                if (!sc.isclient) {
                                    console.log('joined roomid: ', roomId);
                                    var message = {
                                        type: "joined",
                                        room: roomId
                                    };
                                    // Send "joined" broadcast event payload to the host peer
                                    emitToSocket(sc.id, 'joined', message);
                                }
                            }
                        });

                        emitToSocket(socketContext.id, 'join', [roomId, socketContext.id, numClients]);
                    }
                }

                // ROUTE: message
                else if (event === 'message') {
                    let message = data;
                    message.from = socketContext.id;

                    if (socketContext.isclient)
                        console.log('Second participant: ', message);
                    else
                        console.log('First participant: ', message);

                    if (message.to && message.to.length !== 0) {
                        // Direct targeted event delivery
                        emitToSocket(message.to, 'message', message);
                    } else {
                        const clientsInRoom = getClientsInRoom(message.room);
                        if (clientsInRoom.size === 0) return;

                        clientsInRoom.forEach(function(scid) {
                            let sc = wsState.get(scid);
                            if (sc && sc.isclient !== socketContext.isclient && socketContext.id !== sc.id) {
                                emitToSocket(sc.id, 'message', message);
                            }
                        });
                    }
                }

                // ROUTE: postAppMessage
                else if (event === 'postAppMessage') {
                    let message = data;
                    if (message.type === "user") {
                        message.user = message.desc;
                    }

                    console.log('notification ' + JSON.stringify(message, null, 4));
                    message.from = socketContext.id;

                    if (socketContext.user)
                        message.user = socketContext.user;

                    if (message.type === "chat") {
                        if ('room' in message) {
                            const clientsInRoom = getClientsInRoom(message.room);
                            clientsInRoom.forEach(function(scid) {
                                emitToSocket(scid, 'message', message);
                            });
                        }
                    } else {
                        emitToSocket(message.to, 'message', message);
                    }
                }

                // ROUTE: bye
                else if (event === 'bye') {
                    console.log('received bye');
                }

            } catch (err) {
                console.error("Failed handling websocket frame message:", err);
            }
        });
    });
}




// async function runSocketServer() {

//     const rooms = {};

//     console.error('runSocketServer');
//     const wss = new WebSocket.Server({server: webServer});  
//     wss.on('connection', function connection(ws) {
//     //console.log('received');
//     ws.id = uuidv4();

//     ws.on('message', function incoming(data)
//     {
//        console.log('received: %s', data);
//        let msg;
 
//        try {
//           msg = JSON.parse(data);
//         } catch (e) {
//         return console.error(e); // error in the above string (in this case, yes)!
// 		 }

//        if( !msg.messageType)
//        {
//           console.log("websock data error %o", msg);
// 	  return;
//        }


//        switch (msg.messageType) {
//         case "createorjoin":
//         {
//            // console.log('first: %o', rooms);

//             console.log("createorjoin " + msg.room );

//             if(msg.server)
//             {
//                 if(rooms[msg.room])
//                  {
//                     rooms[msg.room].forEach((client) => {
//                     if ( client.readyState === WebSocket.OPEN)
//                     {
//                          console.log('close: %o %o %o', client.server, client.room,  client.id);
//                         rooms[client.room] = rooms[client.room].filter((cl) => cl !== ws);
//                     }
//                     });

//                     if(rooms[msg.room])
//                     delete rooms[msg.room];
//                     console.log('delete: %o',  rooms[msg.room]);
//                     rooms[msg.room] = [];

//                  }

//             }    

//             ws["room"] = msg.room;
//              if(! rooms[msg.room])
//               rooms[msg.room] = [];
            
//            if (rooms[msg.room].indexOf(ws) < 0) {


//                 rooms[msg.room].push(ws);
//             } 
//             else
//             {
//                 console.log("websocket connection exist");
//             }
            

//             var numClients = rooms[msg.room].length; 

//             if(numClients == 1)
//             {  ws.server = true;
//                ws.send( JSON.stringify({"messageType": "join", "room": msg.room}));
//             }
//             else if (numClients > 1)
//             {
//                 ws.server = false;
//                ws.send( JSON.stringify({"messageType": "joined","room": msg.room})); 
//             }

           

//             break;
//         }
//         case "ICE_CANDIDATE":
//         case "SDP_OFFER":
//         case "SDP_ANSWER":
//         {
//             rooms[ws.room].forEach((client) => {
//             if (client !== ws && client.readyState === WebSocket.OPEN)
//             {   
//                 msg.senderClientId = ws.id;

//                 if((  ws.server == true &&  client.server == false) ||  (  ws.server == false &&  client.server == true))
//                 {
//                     console.log('RecipientClientId= %o client.id= %o',  msg.RecipientClientId,  client.id );
//                   if( !msg.RecipientClientId  ||  (msg.RecipientClientId == client.id ))
//                   { 
//                     //console.log('client.server: %o', client.server);
//                     //console.log('ws.server: %o', ws.server);
//                     msg.room = ws.room;
//                     if(ws.server  &&  !client.server)
//                     {
//                          console.log('camera sending: %s', JSON.stringify(msg));
//                     }
//                     else if(!ws.server  &&  client.server)
//                     {
//                          console.log('Particpant sending: %s', JSON.stringify(msg));
//                     }
//                     else
//                     {
//                         console.error('not possbile state');
//                     }
                   
//                     client.send(JSON.stringify(msg));
//                   }
//                 }
//             }
             
//             });

//             break;
//         }
//         case "bye":
        
//         break;

//         default:
//         {
//           console.log("WARNING: Ignoring unknown msg of messageType '" + msg.messageType + "'");
//           break;
//         }


//         };




//     });


//     ws.on('error',e=>console.log(e));
//     ws.on('close',(e)=>
//     {
       
//         console.log('websocket closed'+e);

//         if(  ws.server == true && ws.room)
//         {
//             rooms[ws.room].forEach((client) => {
//                 if (client !== ws && client.readyState === WebSocket.OPEN)
//                 {
//                      console.log('close: %o %o %o', client.server, client.room,  client.id);
//                     rooms[client.room] = rooms[client.room].filter((cl) => cl !== ws);
//                 }
//             });
//         }
//         else
//         {
//             rooms[ws.room].forEach((client) => {
//             if (client !== ws && client.readyState === WebSocket.OPEN)
//             {   
//                // msg.senderClientId = ws.id;

//                 if(  ws.server == false &&  client.server == true)
//                 {
//                     client.send(JSON.stringify({"messageType": "disconnectClient", "senderClientId":ws.id})); 
//                 }
//             }
             
//             });
//         }

//         if(ws.room )
//         {
//           console.log('close:  %o %o %o', ws.server, ws.room,  ws.id);
//           rooms[ws.room] = rooms[ws.room].filter((client) => client !== ws);
//         }
//         //   console.log('delete: %o',  rooms[ws.room]);


//         //delete rooms[ws.room];
       

       

//     });



//     });

// }



