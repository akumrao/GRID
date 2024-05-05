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
		controls.innerHTML = '<div id="change">' + '<button title="Reset" class="reset"><svg class="reset" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink"  width="45" height="45" viewBox="0 0 512 512" xml:space="preserve"><g class="reset" transform="matrix(1.48 0 0 1.48 256 256)"><path class="reset" style="stroke: rgb(0,0,0); stroke-width: 0; stroke-dasharray: none; stroke-linecap: butt; stroke-dashoffset: 0; stroke-linejoin: miter; stroke-miterlimit: 4; fill: rgb(77,77,79); fill-rule: nonzero; opacity: 1;" vector-effect="non-scaling-stroke"  transform=" translate(-256, -244.27)" d="M 256 121.07 L 122.88 234.35 L 170.70999999999998 234.35 L 170.70999999999998 347.81 C 170.7980015432894 358.71166102152233 179.6780348828916 367.4933749180086 190.57999999999998 367.46 L 230.27999999999997 367.46 L 230.27999999999997 298.08 L 283.18999999999994 298.08 L 283.18999999999994 367.46 L 321.42999999999995 367.46 C 332.3297231322432 367.4878975083077 341.2065463470775 358.7093581941913 341.29999999999995 347.81 L 341.29999999999995 234.35 L 389.12999999999994 234.35 Z" stroke-linecap="round" /></g></svg></button>'+'<button title="Zoom In" class="zoomin"><svg class="zoomin" viewBox="0 0 512 512"><path class="zoomin" d="M505.75,475.58,378.42,348.25a212.3,212.3,0,0,0,48.25-134.92C426.67,95.7,331,0,213.33,0S0,95.7,0,213.33,95.7,426.67,213.33,426.67a212.3,212.3,0,0,0,134.92-48.25L475.58,505.75a21.33,21.33,0,1,0,30.17-30.17ZM277.33,234.67H234.67v42.67a21.33,21.33,0,1,1-42.67,0V234.67H149.33a21.33,21.33,0,0,1,0-42.67H192V149.33a21.33,21.33,0,0,1,42.67,0V192h42.67a21.33,21.33,0,1,1,0,42.67Z" fill="#5b5b5f"/></svg></button>' + '<button title="Zoom Out" class="zoomout"><svg class="zoomout" viewBox="0 0 512 512"><path class="zoomout" d="M505.75,475.58,378.42,348.25a212.3,212.3,0,0,0,48.25-134.92C426.67,95.7,331,0,213.33,0S0,95.7,0,213.33,95.7,426.67,213.33,426.67a212.3,212.3,0,0,0,134.92-48.25L475.58,505.75a21.33,21.33,0,1,0,30.17-30.17ZM277.33,234.67h-128a21.33,21.33,0,0,1,0-42.67h128a21.33,21.33,0,1,1,0,42.67Z" fill="#5b5b5f"/></svg></button>' + '<button title="Move Left" class="right"><svg class="right" viewBox="0 0 512 512"><polygon class="right" points="91.13 256 255.42 382.35 255.42 302.45 405.63 302.45 405.63 209.56 255.44 209.56 255.44 129.65 91.13 256" fill="#5b5b5f"/></svg></button>' + '<button title="Move Right" class="left"><svg class="left" viewBox="0 0 512 512"><polygon class="left" points="420.88 256 256.57 129.66 256.57 209.57 106.37 209.57 106.37 302.45 256.57 302.45 256.57 382.35 420.88 256" fill="#5b5b5f"/></svg></button>' + '<button title="Move Down" class="up"><svg class="up" viewBox="0 0 512 512"><polygon class="up" points="256 420.89 382.35 256.58 302.44 256.58 302.44 106.38 209.55 106.38 209.55 256.57 129.65 256.57 256 420.89" fill="#5b5b5f"/></svg></button>' + '<button title="Move Up" class="down"><svg class="down" viewBox="0 0 512 512"><polygon class="down" points="256.02 91.13 129.66 255.43 209.57 255.43 209.57 405.62 302.45 405.62 302.45 255.43 382.36 255.43 256.02 91.13" fill="#5b5b5f"/></svg></button>' + '</div>';
	}
	/* If a button was clicked (uses event delegation)...*/
	controls.addEventListener('click', function(e) {
		t = e.target;
		if(t.nodeName.toLowerCase() === 'button' || t.nodeName.toLowerCase() == 'svg' || t.nodeName.toLowerCase() == 'polygon' || t.nodeName.toLowerCase() == 'path') {
			
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


 
