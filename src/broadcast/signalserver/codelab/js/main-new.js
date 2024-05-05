'use strict';

var isChannelReady = true;
var isInitiator = false;
var isStarted = false;
//var localStream;
var pc;
var remoteStream;
var turnReady;

var roomId = 'foo'; /*think as a group  peerName@room */
//var  remotePeerID;
var  peerID;
//var  remotePeerName;
var  peerName;

// var spinner = new jQuerySpinner({
//   parentId: 'videoLoader'
// });
// //showLoadSpinner();
// function showLoadSpinner() {
//spinner.show();
//spinner.hide();
//}

var pcConfig = {
'iceServers': [{
  'urls': 'stun:stun.l.google.com:19302'
}]
};

// Set up audio and video regardless of what devices are present.
/*var sdpConstraints = {
offerToReceiveAudio: true,
offerToReceiveVideo: true
};*/

/////////////////////////////////////////////


// Could prompt for room name:
// room = prompt('Enter room name:');

var socket = io.connect();


socket.on('created', function(room) {
console.log('Created room ' + room);
isInitiator = true;
});



socket.on('join', function (room, id, numClients){
console.log('New peer joins room ' + room + '!' +" client id " + id);
isChannelReady = true;
});
socket.on('joined', function(room, id, numClients) {
console.log('joined: ' + room + ' with peerID: ' + id);
log('joined: ' + room + ' with peerID: ' + id);
isChannelReady = true;
peerID = id;


let number = getUrlVars()["cam"];
 if ( !number ) {
   number =0;
  }

if (isInitiator) {

  // when working with web enable bellow line
  // doCall();
  // disable  send message 
   sendMessage ({
    room: roomId,
     cam: number.toString(),
    type: 'offer',
    desc:'sessionDescription'
  });

}

});

socket.on('log', function(array) {
console.log.apply(console, array);
});

////////////////////////////////////////////////
function getUrlVars() {
  var vars = {};
  var parts = window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
      vars[key] = value;
  });
  return vars;
}

function sendMessage(message) {
console.log('Client sending message: ', message);
log('Client sending message: ', message);
//socket.emit('message', message);
socket.emit('sfu-message', message);
}

// This client receives a message
socket.on('message', function(message) {
console.log('Client received message:', message);
log('Client received message:', message);

if (typeof message.type !== 'undefined' && message.type == 'error') {
  document.getElementById("videoLoader").innerHTML = '';
  document.getElementById("hlsStreamSection").innerHTML = '<div id="errorId" class="liveview_col text-center">We are having trouble connecting to this camera. Please try again. If issue persists, please report to <a target="_blank" href="mailto: customer.support@pro-vigil.com">customer.support@pro-vigil.com</a></div>';
  //spinner.hide();
  if( /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent) ) {
    // var myobj = document.getElementById("controls");
    // myobj.remove();
    document.getElementById("errorId").style.color = "white";
  }
  else {
    var removeControls = document.getElementById("controls");
    removeControls.remove();
  }
  
}

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
  handleRemoteHangup();
}
else if(message.type === 'error') {
 
  console.log('Camera state', message.desc);
  log('Camera state:', message.desc);
  if (typeof message.type !== 'undefined' && message.type == 'error' && message.desc == 'Deleted camera with Rest API') {
    document.getElementById("hlsStreamSection").innerHTML = '';
    document.getElementById("videoLoader").innerHTML = '';
    //spinner.hide();
    if( /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent) ) {
      // var myobj = document.getElementById("controls");
      // myobj.remove();
    }
    else {
      var removeControls = document.getElementById("controls");
      removeControls.remove();
    }
    
  }
  hangup();

}

});

////////////////////////////////////////////////////

// var localVideo = document.querySelector('#localVideo');
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
//   localStream = stream;
//   localVideo.srcObject = stream;
//   sendMessage('got user media');
//     isInitiator = true;
//   if (isInitiator) {
//     maybeStart();
//   }
// }

//arvind else  if no gotStream
isInitiator = true;
if (isInitiator) {
   maybeStart();
 }

// if (location.hostname !== 'localhost') {
//   requestTurn(
//     'https://computeengineondemand.appspot.com/turn?username=41784574&key=4080218913'
//   );
// }

function maybeStart() {
// spinner.show();
console.log('>>>>>>> maybeStart() ', isStarted, isChannelReady);
if (!isStarted  && isChannelReady) {
  console.log('>>>>>> creating peer connection');
  createPeerConnection();
 // pc.addStream(localStream);
  isStarted = true;
  console.log('isInitiator', isInitiator);
 // if (isInitiator) {
   // doCall();
 // }
    if (roomId !== '') {
      socket.emit('create or join', roomId);
      console.log('Attempted to create or  join room', roomId);
    }


}
}

window.onbeforeunload = function() {
  sendMessage({
    room: roomId,
    //to: remotePeerID,
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



  pc.onicecandidate = handleIceCandidate;
  // pc.onaddstream = handleRemoteStreamAdded;
  // pc.onremovestream = handleRemoteStreamRemoved;
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
    room: roomId,
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

// sessionDescription.sdp = sessionDescription.sdp.replace("useinbandfec=1", "useinbandfec=1; minptime=10; cbr=1; stereo=1; sprop-stereo=1; maxaveragebitrate=510000");
sessionDescription.sdp = sessionDescription.sdp.replace("useinbandfec=1", "useinbandfec=1; minptime=10; stereo=1; maxaveragebitrate=510000");
pc.setLocalDescription(sessionDescription);
console.log('setLocalAndSendMessage sending message', sessionDescription);

 sendMessage ({
    room: roomId,
    //to: remotePeerID,
    type: sessionDescription.type,
    desc:sessionDescription
  });
}

function onCreateSessionDescriptionError(error) {
log('Failed to create session description: ' + error.toString());
console.log('Failed to create session description: ' + error.toString());

}

// function requestTurn(turnURL) {
//   var turnExists = false;
//   for (var i in pcConfig.iceServers) {
//     if (pcConfig.iceServers[i].urls.substr(0, 5) === 'turn:') {
//       turnExists = true;
//       turnReady = true;
//       break;
//     }
//   }
//   if (!turnExists) {
//     console.log('Getting TURN server from ', turnURL);
//     // No TURN server. Get one from computeengineondemand.appspot.com:
//     var xhr = new XMLHttpRequest();
//     xhr.onreadystatechange = function() {
//       if (xhr.readyState === 4 && xhr.status === 200) {
//         var turnServer = JSON.parse(xhr.responseText);
//         console.log('Got TURN server: ', turnServer);
//         pcConfig.iceServers.push({
//           'urls': 'turn:' + turnServer.username + '@' + turnServer.turn,
//           'credential': turnServer.password
//         });
//         turnReady = true;
//       }
//     };
//     xhr.open('GET', turnURL, true);
//     xhr.send();
//   }
// }

function handleRemoteStreamAdded(event) {
console.log('Remote stream added.');
remoteStream = event.stream;
remoteVideo.srcObject = remoteStream;
}

function handleRemoteStreamRemoved(event) {
console.log('Remote stream removed. Event: ', event);
}

function hangup() {
console.log('Hanging up.');
stop();
sendMessage({
    room: roomId,
    //to: remotePeerID,
    type: 'bye'
  });
}

function handleRemoteHangup() {
console.log('Session terminated.');
stop();
//isInitiator = false;
}

function stop() {
isStarted = false;
pc.close();
pc = null;
//localStream=null;
}



 


pc.ontrack = ({transceiver, streams: [stream]}) => {
//log("pc.ontrack with transceiver and streams");

if(transceiver.direction != 'inactive' && transceiver.currentDirection != 'inactive')
{   
  var track = transceiver.receiver.track;
  console.log("pc.ontrack with transceiver and streams " + track.kind);
}

stream.onaddtrack = () => console.log("stream.onaddtrack");
stream.onremovetrack = () => console.log("stream.onremovetrack");
transceiver.receiver.track.onmute = () => console.log("transceiver.receiver.track.onmute " + track.id);
transceiver.receiver.track.onended = () => console.log("transceiver.receiver.track.onended " + track.id);
transceiver.receiver.track.onunmute = () => {
console.log("transceiver.receiver.track.onunmute " + track.id);
remoteVideo.srcObject = stream;

   // var atracks =  streams.getAudioTracks();

   //  for (var tsn in atracks) 
   //  {
   //         var trc = atracks[tsn];
   //          trc.enable = false;

   //         var x = 1;
   //  }
          


};
};


pc.addEventListener('iceconnectionstatechange', () =>
{
  
    switch (pc.iceConnectionState)
    {
        case 'checking':
            console.log( 'subscribing...');
            break;
        case 'connected':
        case 'completed':


            console.log( 'subscribed...');
            //spinner.hide();
            break;
        case 'failed':
           // pc2.close();

            console.log( 'failed...');
            break;
        case 'disconnected':
           // pc2.close();
            console.log( 'Peerconnection disconnected...');
            break;
        case 'closed':
            //pc2.close();
            console.log( 'failed...');
            break;
    }
});



function onMuteClick() {
// Get the checkbox
var checkBox = document.getElementById("checkmute");
// Get the output text
// If the checkbox is checked, display the output text
if (checkBox.checked == true){
  //text.style.display = "block";
} else {
  //text.style.display = "none";
}


sendMessage ({
    room: roomId,
    //to: remotePeerID,
    type: 'mute',
    desc: checkBox.checked
  });

}


actionButtons();
function actionButtons() {
/* predefine zoom and rotate */
var zoom = 1,
  rotate = 0;
/* Grab the necessary DOM elements */
var hlsStreamSection = document.getElementById('hlsStreamSection'),
  v = document.getElementsByTagName('video')[0],
  controls = document.getElementById('controls');
/* Array of possible browser specific settings for transformation */
var properties = ['transform', 'WebkitTransform', 'MozTransform', 'msTransform', 'OTransform'],
  prop = properties[0];
/* Iterators and stuff */
var i, j, t;
/* Find out which CSS transform the browser supports */
/*   for(i=0,j=properties.length;i<j;i++){
    if (properties[i] in v.style) {
      prop = properties[i];
      break;
    }
  } */
/* Position video */
v.style.left = 0;
v.style.top = 0;
/* If there is a controls element, add the player buttons */
if(controls) {
  controls.innerHTML = '<div id="change">' + '<button title="Reset" class="reset" style="border-radius: 50% !important;background-color:rgb(244, 119, 56) !important;border:none;"><img class="reset" src="images-new/icon-home-alt.svg" class="img-fluid" /></button>'+'<button title="Move Down" class="up" style="border-radius: 50% !important;background-color:white; !important;border:none;"><img class="up" src="images-new/icon-chav-sm-down.svg" class="img-fluid" /></button>' + '<button title="Move Up" class="down" style="border-radius: 50% !important;background-color:white !important;border:none;"><img class="down" src="images-new/icon-chav-sm-up.svg" class="img-fluid" /></button>' + '<button title="Move Left" class="right" style="border-radius: 50% !important;background-color:white !important;border:none;"><img class="right" src="images-new/icon-chav-sm-left.svg" class="img-fluid" /></button>' + '<button title="Move Right" class="left" style="border-radius: 50% !important;background-color:white !important;border:none;"><img class="left" src="images-new/icon-chav-sm-right.svg" class="img-fluid" /></button>' + '<button title="Zoom Out" class="zoomout" style="border-radius: 50% !important;background-color:rgb(244, 119, 56) !important;border:none;"><img class="zoomout" src="images-new/icon-minus.svg" class="img-fluid" /></button>' + '<button title="Zoom In" class="zoomin" style="border-radius: 50% !important;background-color:rgb(244, 119, 56) !important;border:none;"><img class="zoomin" src="images-new/icon-plus.svg" class="img-fluid" /></button>' + '</div>';
}
/* If a button was clicked (uses event delegation)...*/
controls.addEventListener('click', function(e) {
  t = e.target;
  if(t.nodeName.toLowerCase() === 'button' || t.nodeName.toLowerCase() == 'svg' || t.nodeName.toLowerCase() == 'polygon' || t.nodeName.toLowerCase() == 'path' || t.nodeName.toLowerCase() == 'img') {
    
    var classValue = typeof t.className.baseVal === "undefined" ? t.className : t.className.baseVal;
    /* Check the class name of the button and act accordingly */
    switch (classValue) {
      /* Increase zoom and set the transformation */
      case 'zoomin':
        if(zoom < 6) {
          zoom = zoom + 0.5;
          v.style.transform = 'scale(' + zoom + ') rotate(' + rotate + 'deg)';
        }
        break;
        /* Decrease zoom and set the transformation */
      case 'zoomout':
        if(zoom > 1) {
          zoom = zoom - 0.5;
          v.style.transform = 'scale(' + zoom + ') rotate(' + rotate + 'deg)';
        }
        break;
      case 'left':
        if(zoom > 1) {
          v.style.left = (parseInt(v.style.left, 10) - 25) + 'px';
        }
        break;
      case 'right':
        if(zoom > 1) {
          v.style.left = (parseInt(v.style.left, 10) + 25) + 'px';
        }
        break;
      case 'up':
        if(zoom > 1) {
          v.style.top = (parseInt(v.style.top, 10) - 25) + 'px';
        }
        break;
      case 'down':
        if(zoom > 1) {
          v.style.top = (parseInt(v.style.top, 10) + 25) + 'px';
        }
        break;
        /* Reset all to default */
      case 'reset':
        zoom = 1;
        rotate = 0;
        v.style.top = 0 + 'px';
        v.style.left = 0 + 'px';
        v.style.transform = 'rotate(' + rotate + 'deg) scale(' + zoom + ')';
        break;
    }
    e.preventDefault();
  }
}, false);
document.addEventListener("keydown", function(event) {
  if ((event.keyCode == 187 || event.keyCode == 61) && zoom < 6) {
    zoom = zoom + 0.5;
    v.style.transform = 'scale(' + zoom + ') rotate(' + rotate + 'deg)';
  }
  if ((event.keyCode == 189 || event.keyCode == 173) && zoom > 1) {
    zoom = zoom - 0.5;
    v.style.transform = 'scale(' + zoom + ') rotate(' + rotate + 'deg)';
  }
  if (event.keyCode == 39 && zoom > 1) {
    v.style.left = (parseInt(v.style.left, 10) - 25) + 'px';
  }
  if (event.keyCode == 37 && zoom > 1) {
    v.style.left = (parseInt(v.style.left, 10) + 25) + 'px';
  }
  if (event.keyCode == 40 && zoom > 1) {
    v.style.top = (parseInt(v.style.top, 10) - 25) + 'px';
  }
  if (event.keyCode == 38 && zoom > 1) {
    v.style.top = (parseInt(v.style.top, 10) + 25) + 'px';
  }
  if (event.keyCode == 27) {
    zoom = 1;
    rotate = 0;
    v.style.top = 0 + 'px';
    v.style.left = 0 + 'px';
    v.style.transform = 'rotate(' + rotate + 'deg) scale(' + zoom + ')';
  }
});
};


