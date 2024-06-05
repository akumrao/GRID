'use strict';

var isChannelReady = false;
var isInitiator = false;
var isStarted = false;
var localStream;
var pc;
var turnReady;

let channelSnd;
let starttime;
let inboundStream;

// Set up audio and video regardless of what devices are present.
var sdpConstraints = {
  'mandatory': {
    'OfferToReceiveAudio': true,
    'OfferToReceiveVideo': true
  }
};

var browserName = (function(agent) {
    switch (true) {
        case agent.indexOf("edge") > -1:
            return "Edge";
        case agent.indexOf("edg/") > -1:
            return "Edge ( chromium based)";
        case agent.indexOf("opr") > -1 && !!window.opr:
            return "Opera";
        case agent.indexOf("chrome") > -1 && !!window.chrome:
            return "Chrome";
        case agent.indexOf("trident") > -1:
            return "MS IE";
        case agent.indexOf("firefox") > -1:
            return "Firefox";
        case agent.indexOf("safari") > -1:
            return "Safari";
        default:
            return "other";
    }
})(window.navigator.userAgent.toLowerCase());

// Could prompt for room name:
var room = prompt('Enter camera name:', '65f570720af337cec5335a70ee88cbfb7df32b5ee33ed0b4a896a0');

if (room === '') {
  room = '65f570720af337cec5335a70ee88cbfb7df32b5ee33ed0b4a896a0';
}

var socket = io.connect();
socket.on('created', function(room) {
    console.log('Created room ' + room);
    isInitiator = true;
});

socket.on('join', function(room, id, numClients) {
    console.log('New peer, room: ' + room + ', ' + " client id: " + id);
    isChannelReady = true;
});



socket.emit('createorjoin', room , true);
console.log('Attempted to createorjoin room', room);

socket.on('full', function(room) {
  console.log('Room ' + room + ' is full');
});

// socket.on('join', function (room){
//   console.log('Another peer made a request to join room ' + room);
//   console.log('This peer is the initiator of room ' + room + '!');
//   isChannelReady = true;
// });

socket.on('joined', function(room) {
  console.log('joined: ' + room);
  isChannelReady = true;
  isInitiator = true;
  maybeStart();
});

socket.on('log', function(array) {
  console.log.apply(console, array);
});



function sendMessage(message) {
    console.log('Client sending message: ', message);
    //log('Client sending message: ', message);
    socket.emit('messageToWebrtc', message);
}

// This client receives a message
socket.on('message', function(message) {
  console.log('Client received message:', message);
  //log('Client received message:', message);


  if (message === 'got user media') {
    maybeStart();
  } else if (message.type === 'offer') {
    if (!isInitiator && !isStarted) {
      maybeStart();
    }
   // remotePeerID=message.from;
   // log('got offfer from remotePeerID: ' + remotePeerID);

    pc.setRemoteDescription(new RTCSessionDescription(message.desc));
    doAnswer();
  } else if (message.type === 'answer' && isStarted) {
    pc.setRemoteDescription(new RTCSessionDescription(message.desc));
  } else if (message.type === 'candidate' && isStarted) {
    var candidate = new RTCIceCandidate({
      sdpMLineIndex: message.candidate.sdpMLineIndex,
      sdpMid: message.candidate.sdpMid,
      candidate: message.candidate.candidate
    });
    pc.addIceCandidate(candidate);
  } else if (message.type === 'bye' && isStarted) {

    console.log('Camera state', message.desc);
    //log('Camera state:', message.desc);

    handleRemoteHangup();
  }
  else if(message.type === 'error') {
   
    console.log('Camera state', message.desc);
    //log('Camera state:', message.desc);
    hangup();
  }

});

////////////////////////////////////////////////////

var localVideo = document.querySelector('#localVideo');
var remoteVideo = document.querySelector('#remoteVideo');

// navigator.mediaDevices.getUserMedia({
//   audio: true,
//   video: true
// })
// .then(gotStream)
// .catch(function(e) {
//   alert('getUserMedia() error: ' + e.name);
// });

// function gotStream(stream) {
//   console.log('Adding local stream.');
//   if ('srcObject' in localVideo) {
//     localVideo.srcObject = stream;
//   } else {
//     // deprecated
//     localVideo.src = window.URL.createObjectURL(stream);
//   }
//   localStream = stream;
//   //sendMessage('got user media');
//   if (isInitiator) {
//    // maybeStart();
//   }
// }

// var constraints = {
//   video: true
// };

//console.log('Getting user media with constraints', constraints);


async function maybeStart() {
  console.log('>>>>>>> maybeStart() ', isStarted, localStream, isChannelReady);
  if (!isStarted  && isChannelReady) {
    console.log('>>>>>> creating peer connection');
    createPeerConnection();


    const stream = await navigator.mediaDevices.getUserMedia({audio: true})

    localVideo.srcObject = stream;

    stream.getTracks().forEach(track => pc.addTrack(track, stream));

    const transceiver = pc.getTransceivers().find(t => t.sender && t.sender.track === stream.getAudioTracks()[0]);
    const {codecs} = RTCRtpSender.getCapabilities('audio');
    const selectedCodecIndex = codecs.findIndex(c => c.mimeType === 'audio/PCMA');
    transceiver.setCodecPreferences([codecs[selectedCodecIndex]]);


   // pc.addStream(localStream);
    isStarted = true;
    console.log('isInitiator', isInitiator);
    if (isInitiator) {
      doCall();
    }
  }
}

window.onbeforeunload = function() {
    sendMessage({
        room: room,
        type: 'bye'
    });
};

/////////////////////////////////////////////////////////

function createPeerConnection() {
  try {
    //pc = new RTCPeerConnection(null);

     pc = new RTCPeerConnection(
        {
            iceServers         : [{'urls': 'stun:stun.l.google.com:19302'}],
            iceTransportPolicy : 'all',
            bundlePolicy       : 'max-bundle',
            rtcpMuxPolicy      : 'require',
            sdpSemantics       : 'unified-plan'
        });

    pc.addTransceiver('audio');
    pc.addTransceiver('video');


    channelSnd = pc.createDataChannel("chat"); // sende PC1 
    
    channelSnd.onopen = function(event)
    {
        channelSnd.send('Hi you!');
    }
    
    channelSnd.onmessage = function(event)
    {
        //console.log("event.data " + event.data);

        var msg = JSON.parse(event.data);

        
        switch (msg.messageType) {
          case "IDENTITY_NOT_IN_GALLERY":
           

          var base64Url = "data:image/jpeg;base64, " + msg.messagePayload.looselyCroppedImage;
          var imgid = document.getElementById("image");
          imgid.src = base64Url;

          var myJsObj = GenIdentity();

            //   "messageType": "identity",
            //   "messagePayload": {
            //     "configuredGalleryIdentities": {
            //       "76a92b24-31d5-463b-ab7a-b379efab7b30": {
            //         "accuracyMonitorConsent": false,
            //         "identityName": "entername",
            //         "productImprovementConsent": false,
            //         "registrationImageIDs": ["entername"]
            //       }
            //     },
            //     "sequenceNum": 1
            //   }
            // };


          //var obj = JSON.parse(myJsObj);
          myJsObj['registrationImage'] = msg.messagePayload.registrationImage;

          // using JSON.stringify pretty print capability:
          var str = JSON.stringify(myJsObj);

          // display pretty printed object in text area:
          //document.getElementById('myTextArea').innerHTML = str;

          document.getElementById('w3review').value= str;
     
             

          break;
          case "IDENTITY_RECOGNIZED":
          {


            var base64Url = "data:image/jpeg;base64, " + msg.messagePayload.looselyCroppedImage;
            var imgid = document.getElementById("image");
            imgid.src = base64Url;

            document.getElementById('w3review').value=  msg.messagePayload.identityName;

             
          // {
          //   msg.identityID": "76a92b24-31d5-463b-ab7a-b379efab7b30",
          //   "identityName": "arvind",
          // }
          
                
            //document.getElementById('w3review').value = msg.messagePayload;

            break;
            
          }

          case "RECORDING":
          {

             recordlist(msg.messagePayload);

            break;
          }

        }
    }



    // pc.ondatachannel = function(event) {  // receiver /PC2
    // var channel = event.channel;
    // channel.onopen = function(event) {
    // channel.send('ravind back!');
    // }
    // channel.onmessage = function(event) {
    // console.log("ravind " + event.data);
    // }
    // }


    pc.onicecandidate = handleIceCandidate;
    //pc.onaddstream = handleRemoteStreamAdded;
    pc.ontrack = ontrack;
    pc.onremovestream = handleRemoteStreamRemoved;
    console.log('Created RTCPeerConnnection');
  } catch (e) {
    console.log('Failed to create PeerConnection, exception: ' + e.message);
    alert('Cannot create RTCPeerConnection object.');
    return;
  }
}

function handleIceCandidate(event) {
  console.log('icecandidate event: ', event);
  if (event.candidate) {
    sendMessage({
      room: room,
      //to: remotePeerID,
      type: 'candidate',
      candidate: event.candidate
    });
  } else {
    console.log('End of candidates.');
  }
}

function handleCreateOfferError(event) {
    console.log('createOffer() error: ', event);
}

function doCall() {
    console.log('Sending offer to peer');
    pc.createOffer(setLocalAndSendMessage, handleCreateOfferError);
}

function doAnswer() {
    console.log('Sending answer to peer.');
    pc.createAnswer().then(
        setLocalAndSendMessage,
        onCreateSessionDescriptionError
    );
}

function setLocalAndSendMessage(sessionDescription) {
    // for changing bandwidth,bitrate and audio stereo/mono
    // sessionDescription.sdp = sessionDescription.sdp.replace("useinbandfec=1", "useinbandfec=1; minptime=10; cbr=1; stereo=1; sprop-stereo=1; maxaveragebitrate=510000");
    // sessionDescription.sdp = sessionDescription.sdp.replace("useinbandfec=1", "useinbandfec=1; minptime=10; stereo=1; maxaveragebitrate=510000");

    //sessionDescription.sdp = sessionDescription.sdp.replaceAll("level-asymmetry-allowed=1", "level-asymmetry-allowed=1; Enc=" + encType);
    pc.setLocalDescription(sessionDescription);
    console.log('setLocalAndSendMessage sending message', sessionDescription);

    sendMessage({
        room: room,
        type: sessionDescription.type,
        starttime:starttime,
        desc: sessionDescription
    });
}

function onCreateSessionDescriptionError(error) {
    //log('Failed to create session description: ' + error.toString());
    console.log('Failed to create session description: ' + error.toString());
}


function ontrack({
    transceiver,
    receiver,
    streams: [stream]
}) {
    var track = transceiver.receiver.track;
    var trackid = stream.id;

    if (!inboundStream) {
            inboundStream = new MediaStream();
        }
        inboundStream.addTrack(track);
        remoteVideo.srcObject = inboundStream;

       
        stream.onaddtrack = () => console.log("stream.onaddtrack");
        stream.onremovetrack = () => console.log("stream.onremovetrack");
        transceiver.receiver.track.onmute = () => console.log("transceiver.receiver.track.onmute " + track.id);
        transceiver.receiver.track.onended = () => console.log("transceiver.receiver.track.onended " + track.id);
        transceiver.receiver.track.onunmute = () => {
        console.log("transceiver.receiver.track.onunmute " + track.id);

     

  };


}

// function handleRemoteStreamAdded(event) {
//     console.log('Remote stream added.');
//     //remoteStream = event.stream;
//     remoteVideo.srcObject =  event.stream;
// }

function handleRemoteStreamRemoved(event) {
    console.log('Remote stream removed. Event: ', event);
}

function hangup() {
    console.log('Hanging up.');
    stop();
    sendMessage({
        room: room,
        type: 'bye'
    });
}

function handleRemoteHangup() {
    console.log('Session terminated.');
    stop();
}

function stop() {
    isStarted = false;
    pc.close();
    pc = null;

   isChannelReady = false;
   isInitiator = false;


}

function onIceStateChange(pc, event) {
    switch (pc.iceConnectionState) {
        case 'checking': {
            console.log('checking...');
        }
        break;
        case 'connected':
            console.log('connected...');
            break;
        case 'completed':
            console.log('completed...');
            break;
        case 'failed':
            console.log('failed...');
            break;
        case 'disconnected':
            console.log('Peerconnection disconnected...');
            break;
        case 'closed':
            console.log('failed...');
            break;
    }
}



function addIdentity()
{
  //channelSnd.send('Hi arvind you!');

  if(document.getElementById('w3review').value.length)
  channelSnd.send(document.getElementById('w3review').value);
  document.getElementById('w3review').value ="";
}

function GenIdentity()
{

  var txtname =  document.getElementById("txtname").value;

  let nx = Math.floor((Math.random() * 100) + 1);

  txtname += nx.toString(); 

  document.getElementById("txtname").value = txtname;

  return identity(txtname);

}


function identity(name)
{


  let uid = uuidV4();

  var jsonStr = '{}';

  var obj =  {
                          "messageType": "identity",
                          "messagePayload": {
                            "configuredGalleryIdentities": {},
                            "sequenceNum": 1
                          }
              };

  obj["messagePayload"]["configuredGalleryIdentities"][uid]= {};
  obj["messagePayload"]["configuredGalleryIdentities"][uid]['accuracyMonitorConsent'] = false;
  obj["messagePayload"]["configuredGalleryIdentities"][uid]['identityName'] = name;
  obj["messagePayload"]["configuredGalleryIdentities"][uid]['productImprovementConsent'] = false;
  obj["messagePayload"]["configuredGalleryIdentities"][uid]['registrationImageIDs']=[];
  obj["messagePayload"]["configuredGalleryIdentities"][uid]['registrationImageIDs'].push(name);
  
  jsonStr = JSON.stringify(obj);

  console.log(jsonStr);

  return obj;

}

//identity("arvind");

/*
"76a92b24-31d5-463b-ab7a-b379efab7b30": {
      "accuracyMonitorConsent": false,
      "identityName": "arvind",
      "productImprovementConsent": false,
      "registrationImageIDs": [
        "temp_new_identity_1_reg_image.jpg"
      ]
    },
*/


function uuidV4() {
  const uuid = new Array(36);
  for (let i = 0; i < 36; i++) {
    uuid[i] = Math.floor(Math.random() * 16);
  }
  uuid[14] = 4; // set bits 12-15 of time-high-and-version to 0100
  uuid[19] = uuid[19] &= ~(1 << 2); // set bit 6 of clock-seq-and-reserved to zero
  uuid[19] = uuid[19] |= (1 << 3); // set bit 7 of clock-seq-and-reserved to one
  uuid[8] = uuid[13] = uuid[18] = uuid[23] = '-';
  return uuid.map((x) => x.toString(16)).join('');
}


var list = document.getElementById('myUL');

// Add an event listener to the list
list.addEventListener('click', function(event) {
  // Get the clicked item
  var item = event.target;


   var selected;
  
   if(item.tagName === 'LI') {                                      
    selected= document.querySelector('li.selected');
    if(selected) selected.className= ''; 
     item.className= 'selected';
   }



   hangup();

  // Get the item's text
  var text = item.textContent;

  starttime = text;


  isChannelReady = true;
  isInitiator = true;
  maybeStart();

  // if( isNaN(item.id ))
  // {
  //    return;
  // }

  // Alert the item's text
  //var vsend= "starttime:" +  item.id;
  //alert(item.id);
  // var  selectBox = document.getElementById("cameraSel");
  // var camid = selectBox.options[selectBox.selectedIndex].value;
  // if(channelSnd)
  // {
  //    channelSnd.send(vsend);
  // }
});


function recordlist(data)
{


   console.log('first: %o', data);

   var list = document.getElementById('myUL');

   while (list.firstChild) {
    list.removeChild(list.firstChild);
   }


   for( var i=0; i < data.length ; ++ i )
   {

      var li = document.createElement('li');

      if(!i)
      li.className= 'selected'; 


     // const myDate = new Date(Number(data[i]));

      //let dateStr = myDate.getFullYear() + "/" + (myDate.getMonth() + 1) + "/" + myDate.getDate() + "_" + myDate.getHours() + ":" + myDate.getMinutes() + ":" + myDate.getSeconds()
 
      li.innerText = data[i];
      // li.setAttribute('draggable','true');
      // li.setAttribute('class','drag');
      li.setAttribute('id', data[i]);
 
      list.appendChild(li);

    }
 
}



function startRecording()
{
   channelSnd.send("startrec");
}




