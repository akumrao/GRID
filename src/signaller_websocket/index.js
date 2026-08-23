'use strict';

var os = require('os');
const fs = require('fs');
var nodeStatic = require('node-static');
var http = require('https');
const { WebSocketServer, WebSocket } = require('ws');
const config = require('./config');
const express = require('express');
const { v4 : uuidv4 } = require('uuid');

let webServer;
let expressApp;

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
    const { sslKey, sslCrt } = config;
    if (!fs.existsSync(sslKey) || !fs.existsSync(sslCrt)) {
        console.error('SSL files are not found. check your config.js file');
        process.exit(0);
    }
    const tls = {
        cert: fs.readFileSync(sslCrt),
        key: fs.readFileSync(sslKey),
    };
        
    webServer = http.createServer(tls, expressApp);
    webServer.on('error', (err) => {
        console.error('starting web server failed:', err.message);
    });

    await new Promise((resolve) => {
        const { listenIp, listenPort } = config;
        webServer.listen(listenPort, listenIp, () => {
            console.log('server is running');
            console.log(`open https://127.0.0.1:${listenPort} in your web browser`);
            resolve();
        });
    });
}

const wsState = new Map();
const rooms = new Map();

async function runSocketServer() {
    console.error('runSocketServer');
        
    const wss = new WebSocketServer({ server: webServer });

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
            //console.log(`[EMIT] To Socket: ${targetSocketId} | Event: ${eventName}`);
            clientState.ws.send(JSON.stringify({ event: eventName, payload: data }));
        } else {
            console.log(`[EMIT FAILED] Socket ${targetSocketId} unavailable or closed.`);
        }
    }

    wss.on('connection', function(ws, req) {
        const socketId = Math.random().toString(36).substring(2, 15);
        console.log(`[CONNECTION] New socket connected: ${socketId}`);

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

        ws.on('close', function() {
            const roleTag = socketContext.isclient ? '[CLIENT_TO_SERVER_FLOW]' : '[SERVER_TO_CLIENT_FLOW]';
            console.log(`${roleTag} Disconnect: ${socketContext.id}`);
            
            const targetRoom = socketContext.room;
            if (!targetRoom) {
                wsState.delete(socketContext.id);
                return;
            }

            leaveRoom(targetRoom, socketContext.id);
            const clientsInRoom = getClientsInRoom(targetRoom);

            if (!socketContext.isclient) {
                console.log(`[SERVER_TO_CLIENT_FLOW] Host server ${socketContext.id} left room ${targetRoom}. Disconnecting remaining clients.`);
                clientsInRoom.forEach(function(scid) {
                    let sc = wsState.get(scid);
                    if (sc && sc.isclient) {
                        console.log(`[SERVER_TO_CLIENT_FLOW] Closing peer client: ${sc.id}`);
                        emitToSocket(sc.id, 'leave', [targetRoom, -1, -1]);
                        sc.ws.close();
                    }
                });
            } else {
                console.log(`[CLIENT_TO_SERVER_FLOW] Client ${socketContext.id} disconnected from room ${targetRoom}`);
                clientsInRoom.forEach(function(scid) {
                    let sc = wsState.get(scid);
                    if (sc && !sc.isclient) {
                        console.log(`[CLIENT_TO_SERVER_FLOW] Notifying Host server ${sc.id} about client disconnection`);
                        emitToSocket(sc.id, 'disconnectClient', socketContext.id);
                    }
                });
            }

            wsState.delete(socketContext.id);
        });

        ws.on('message', function(rawData) {
            try {
                const packet = JSON.parse(rawData);
                const event = packet.event;
                const data = packet.payload;

                if (event === 'createorjoin') {
                    const roomId = data.roomId;

                    if (socketContext.room) {
                        leaveRoom(socketContext.room, socketContext.id);
                    }

                    socketContext.room = roomId;
                    joinRoom(roomId, socketContext.id);

                    log('Received request to createorjoin room ' + roomId);

                    const clientsInRoom = getClientsInRoom(socketContext.room);
                    const numClients = clientsInRoom.size;

                    log('Room ' + roomId + ' now has ' + numClients + ' client(s)');

                    if (numClients === 1) {
                        socketContext.isclient = false;
                        console.log(`[SERVER_TO_CLIENT_FLOW] Socket ${socketContext.id} created room ${roomId} as Host Server.`);
                        emitToSocket(socketContext.id, 'join', [roomId, socketContext.id, numClients]);
                    } else if (numClients > 1) {
                        socketContext.isclient = true;
                        //console.log(`[CLIENT_TO_SERVER_FLOW] Socket ${socketContext.id} joined room ${roomId} as Client.`);

                        clientsInRoom.forEach(function(scid) {
                            let sc = wsState.get(scid);
                            if (sc && !sc.isclient) {
                                var message = {
                                    type: "joined",
                                    room: roomId,
                                    socketId: socketContext.id
                                };
                                console.log(`[CLIENT_TO_SERVER_FLOW] Forwarding message ${message.type} from Client ${socketContext.id} to Host server ${sc.id} `);
                                emitToSocket(sc.id, 'joined', message);
                            }
                        });

                        emitToSocket(socketContext.id, 'join', [roomId, socketContext.id, numClients]);
                    }
                }

                else if (event === 'message') {
                    let message = data;
                    message.from = socketContext.id;
                    const flowTag = socketContext.isclient ? '[CLIENT_TO_SERVER_FLOW]' : '[SERVER_TO_CLIENT_FLOW]';

                    if (message.to && message.to.length !== 0) {
                        console.log(`${flowTag} Direct message ${message.type}  from ${socketContext.id} to ${message.to}`);
                        emitToSocket(message.to, 'message', message);
                    } 
                    else {
                        const clientsInRoom = getClientsInRoom(message.room);
                        if (clientsInRoom.size === 0) return;

                        //console.log(`${flowTag} Broadcast message from ${socketContext.id} in room ${message.room}`);

                        clientsInRoom.forEach(function(scid) {
                            let sc = wsState.get(scid);
                            if (!sc) return;

                            if (socketContext.isclient) {
                                if (!sc.isclient) {
                                    console.log(`[CLIENT_TO_SERVER_FLOW] Forwarding message ${message.type}  from ${socketContext.id} to Host server ${sc.id}`);
                                    emitToSocket(sc.id, 'message', message);
                                }
                            } else {
                                if (sc.isclient) {
                                    console.log(`[SERVER_TO_CLIENT_FLOW] Forwarding message ${message.type} from ${socketContext.id} to Client ${sc.id}`);
                                    emitToSocket(sc.id, 'message', message);
                                }
                            }
                        });
                    }
                }

            } catch (err) {
                console.error("Failed handling websocket frame message:", err);
            }
        });
    });
}
