const SUPABASE_URL = 'https://dabwulkpyquthvearbfw.supabase.co';
const SUPABASE_ANON_KEY = 'sb_publishable_1V4Zaw720lrIPx8IqmuxmA_ri32Bdqu';

// Renamed client instance variable to avoid collision with CDN window.supabase
let supabaseClient;
let channel;
let myRole = null; 
const myClientId = 'client_' + Math.random().toString(36).substr(2, 9);

// Data tracking structures
const peerConnections = {}; 
const dataChannels = {}; 
const iceTimeouts = {}; 
let activeDataChannel = null; 

function appendLog(text) {
  console.log(`[APP LOG] ${text}`);
  const logDiv = document.getElementById('log');
  if (logDiv) {
    logDiv.innerHTML += `<div>[${new Date().toLocaleTimeString()}] ${text}</div>`;
    logDiv.scrollTop = logDiv.scrollHeight;
  }
}

function setupSupabase(roomName) {
  console.log(`[Supabase] Initializing client for room: ${roomName}...`);
  if (!supabaseClient) {
    // Referencing CDN global window.supabase object
    supabaseClient = supabase.createClient(SUPABASE_URL, SUPABASE_ANON_KEY);
  }
  
  channel = supabaseClient.channel(`room-${roomName}`);

  document.getElementById('setupPanel').style.display = 'none';
  document.getElementById('controlsPanel').style.display = 'block';
  document.getElementById('myIdDisplay').innerText = myClientId;
  document.getElementById('currentRoomDisplay').innerText = `room-${roomName}`;
}

// ==========================================
// HOST ENGINE CODE
// ==========================================
function initHost() {
  myRole = 'host';
  const uniqueRoomCode = Math.random().toString(36).substr(2, 7).toUpperCase();
  
  console.log(`[Host Engine] Initializing Host mode with room code: ${uniqueRoomCode}`);
  setupSupabase(uniqueRoomCode);
  document.getElementById('roleTitle').innerText = "Role: Host (Broadcaster)";
  appendLog(`Host created unique room token: [ ${uniqueRoomCode} ]. Share this code with viewers!`);

  channel
    .on('broadcast', { event: 'viewer-joined' }, async ({ payload }) => {
      const { clientId } = payload;
      console.log(`[Host Broadcast] 'viewer-joined' received from: ${clientId}`);
      appendLog(`Signaling: Viewer ${clientId} requested handshake. Generating WebRTC link...`);
      await createHostPeerConnection(clientId);
    })
    .on('broadcast', { event: 'signal-to-host' }, async ({ payload }) => {
      const { clientId, signal } = payload;
      const pc = peerConnections[clientId];
      if (!pc) {
        console.warn(`[Host Broadcast] Received signal for unknown peer connection: ${clientId}`);
        return;
      }

      if (signal.type === 'answer') {
        console.log(`[Host RTCPeerConnection] SDP Answer received from ${clientId}:`, signal);
        appendLog(`Signaling: Received SDP Answer from Viewer: ${clientId}`);
        await pc.setRemoteDescription(new RTCSessionDescription(signal));
      } else if (signal.candidate) {
        console.log(`[Host RTCPeerConnection] ICE Candidate received from ${clientId}:`, signal.candidate);
        appendLog(`Signaling: Received ICE candidate from Viewer: ${clientId}`);
        await pc.addIceCandidate(new RTCIceCandidate(signal.candidate));
      }
    })
    .on('presence', { event: 'leave' }, ({ leftPresences }) => {
      leftPresences.forEach((presence) => {
        if (presence.clientId) {
          console.log(`[Host Presence] Viewer departed: ${presence.clientId}`);
          appendLog(`Presence: Viewer ${presence.clientId} vanished. Running cleanup.`);
          destroyHostPeerConnection(presence.clientId);
        }
      });
    })
    .subscribe((status) => {
      console.log(`[Supabase Host Channel Status]: ${status}`);
      if (status === 'SUBSCRIBED') appendLog("Host successfully linked to custom room mesh.");
    });
}

async function createHostPeerConnection(clientId) {
  console.log(`[Host RTCPeerConnection] Constructing new RTCPeerConnection for viewer: ${clientId}`);
  const pc = new RTCPeerConnection({
    iceServers: [{ urls: 'stun:stun.l.google.com:19302' }]
  });
  peerConnections[clientId] = pc;

  // Create Data Channel
  console.log(`[Host RTCDataChannel] Creating data channel 'bidirectional-chat' for client: ${clientId}`);
  const dataChannel = pc.createDataChannel("bidirectional-chat", { ordered: true });
  dataChannels[clientId] = dataChannel;

  dataChannel.onopen = () => {
    console.log(`[Host RTCDataChannel] Data channel OPENED with client: ${clientId}`);
    appendLog(`SCTP Stream: Connected with viewer: ${clientId}`);
    dataChannel.send(JSON.stringify({ text: "System: Welcome to this Unique Broadcast Room!" }));
  };

  dataChannel.onmessage = (event) => {
    console.log(`[Host RTCDataChannel] Raw Message received from [${clientId}]:`, event.data);
    const data = JSON.parse(event.data);
    appendLog(`Chat Received from [${clientId}]: ${data.text}`);
  };

  dataChannel.onerror = (error) => {
    console.error(`[Host RTCDataChannel Error] [${clientId}]:`, error);
  };

  dataChannel.onclose = () => {
    console.log(`[Host RTCDataChannel] Data channel CLOSED for client: ${clientId}`);
    appendLog(`SCTP Stream: Data link dropped for ${clientId}`);
  };

  pc.onconnectionstatechange = () => {
    console.log(`[Host RTCPeerConnection] State Change for [${clientId}]: ${pc.connectionState}`);
    appendLog(`WebRTC Link State for ${clientId}: ${pc.connectionState}`);
    
    if (pc.connectionState === "connected" && iceTimeouts[clientId]) {
      clearTimeout(iceTimeouts[clientId]);
      delete iceTimeouts[clientId];
    } else if (pc.connectionState === "disconnected") {
      iceTimeouts[clientId] = setTimeout(() => destroyHostPeerConnection(clientId), 15000);
    } else if (pc.connectionState === "failed") {
      destroyHostPeerConnection(clientId);
    }
  };

  pc.onicecandidate = (event) => {
    if (event.candidate) {
      console.log(`[Host RTCPeerConnection] Generated local ICE Candidate for viewer [${clientId}]:`, event.candidate);
      channel.send({
        type: 'broadcast',
        event: 'signal-to-viewer',
        payload: { targetId: clientId, signal: { candidate: event.candidate } }
      });
    } else {
      console.log(`[Host RTCPeerConnection] All local ICE Candidates gathered for viewer: ${clientId}`);
    }
  };

  console.log(`[Host RTCPeerConnection] Creating SDP Offer for viewer: ${clientId}`);
  const offer = await pc.createOffer({ offerToReceiveAudio: false, offerToReceiveVideo: false });
  await pc.setLocalDescription(offer);
  console.log(`[Host RTCPeerConnection] Set Local Description (Offer):`, offer);

  channel.send({
    type: 'broadcast',
    event: 'signal-to-viewer',
    payload: { targetId: clientId, signal: offer }
  });
}

function destroyHostPeerConnection(clientId) {
  console.log(`[Host RTCPeerConnection] Cleaning up connection resources for viewer: ${clientId}`);
  if (iceTimeouts[clientId]) clearTimeout(iceTimeouts[clientId]);
  delete iceTimeouts[clientId];
  delete dataChannels[clientId];
  if (peerConnections[clientId]) {
    peerConnections[clientId].close();
    delete peerConnections[clientId];
    appendLog(`Cleanup: All resources freed for ${clientId}`);
  }
}

// ==========================================
// VIEWER ENGINE CODE
// ==========================================
function initViewer() {
  const roomInput = document.getElementById('roomInput').value.trim();
  
  if (!roomInput) {
    alert("Please enter a room code to join.");
    return;
  }

  const cleanedRoomToken = roomInput.replace('room-', '').toUpperCase();

  myRole = 'viewer';
  console.log(`[Viewer Engine] Initializing Viewer mode joining room code: ${cleanedRoomToken}`);
  setupSupabase(cleanedRoomToken);
  
  document.getElementById('roleTitle').innerText = "Role: Viewer (Client)";
  appendLog(`Viewer joining channel for room code: ${cleanedRoomToken}...`);

  console.log(`[Viewer RTCPeerConnection] Constructing new RTCPeerConnection...`);
  const pc = new RTCPeerConnection({
    iceServers: [{ urls: 'stun:stun.l.google.com:19302' }]
  });

  // Handle incoming RTCDataChannel from Host
  pc.ondatachannel = (event) => {
    console.log(`[Viewer RTCDataChannel] OnDataChannel event fired with label: '${event.channel.label}'`);
    if (event.channel.label === "bidirectional-chat") {
      activeDataChannel = event.channel;
      appendLog("SCTP Stream: Bidirectional Data link initialized successfully.");

      activeDataChannel.onopen = () => {
        console.log(`[Viewer RTCDataChannel] Data channel OPENED with Host.`);
        appendLog("SCTP Stream channel opened on Viewer side.");
      };

      activeDataChannel.onmessage = (msgEvent) => {
        console.log(`[Viewer RTCDataChannel] Raw Message received from Host:`, msgEvent.data);
        const data = JSON.parse(msgEvent.data);
        appendLog(`Host message received: ${data.text}`);
      };

      activeDataChannel.onerror = (error) => {
        console.error(`[Viewer RTCDataChannel Error]:`, error);
      };
      
      activeDataChannel.onclose = () => {
        console.log(`[Viewer RTCDataChannel] Data channel CLOSED by Host.`);
        appendLog("SCTP Stream: Data link severed by Host.");
        activeDataChannel = null;
      };
    }
  };

  pc.onconnectionstatechange = () => {
    console.log(`[Viewer RTCPeerConnection] Connection State Changed: ${pc.connectionState}`);
    appendLog(`Viewer WebRTC Link State: ${pc.connectionState}`);
  };

  pc.onicecandidate = (event) => {
    if (event.candidate) {
      console.log(`[Viewer RTCPeerConnection] Generated local ICE Candidate:`, event.candidate);
      channel.send({
        type: 'broadcast',
        event: 'signal-to-host',
        payload: { clientId: myClientId, signal: { candidate: event.candidate } }
      });
    } else {
      console.log(`[Viewer RTCPeerConnection] All local ICE Candidates gathered.`);
    }
  };

  channel
    .on('broadcast', { event: 'signal-to-viewer' }, async ({ payload }) => {
      const { targetId, signal } = payload;
      if (targetId !== myClientId) return;

      if (signal.type === 'offer') {
        console.log(`[Viewer RTCPeerConnection] SDP Offer received from Host:`, signal);
        appendLog("Signaling: Offer received from Host. Constructing SDP Answer...");
        await pc.setRemoteDescription(new RTCSessionDescription(signal));
        
        const answer = await pc.createAnswer();
        await pc.setLocalDescription(answer);
        console.log(`[Viewer RTCPeerConnection] Set Local Description (Answer) and sending to Host:`, answer);

        channel.send({
          type: 'broadcast',
          event: 'signal-to-host',
          payload: { clientId: myClientId, signal: answer }
        });
      } else if (signal.candidate) {
        console.log(`[Viewer RTCPeerConnection] ICE Candidate received from Host:`, signal.candidate);
        appendLog("Signaling: Received ICE Candidate from Host.");
        await pc.addIceCandidate(new RTCIceCandidate(signal.candidate));
      }
    })
    .subscribe(async (status) => {
      console.log(`[Supabase Viewer Channel Status]: ${status}`);
      if (status === 'SUBSCRIBED') {
        appendLog("Viewer signed into specific Supabase channel mesh.");
        await channel.track({ clientId: myClientId });
        
        // Timeout ensures listener readiness before dispatching viewer-joined signal
        setTimeout(() => {
          console.log(`[Viewer Broadcast] Dispatching 'viewer-joined' signal for client: ${myClientId}`);
          channel.send({ type: 'broadcast', event: 'viewer-joined', payload: { clientId: myClientId } });
        }, 500);
      }
    });
}

// Send message via Data Channel for Host or Viewer
function triggerSend() {
  const input = document.getElementById('chatInput');
  const messageText = input.value.trim();

  if (!messageText) return;

  if (myRole === 'host') {
    const targetClients = Object.keys(dataChannels);
    console.log(`[Host triggerSend] Sending message to ${targetClients.length} connected channel(s)...`);

    let sentCount = 0;
    targetClients.forEach((clientId) => {
      const dc = dataChannels[clientId];
      if (dc && dc.readyState === "open") {
        console.log(`[Host RTCDataChannel] Transmitting message to client [${clientId}]:`, messageText);
        dc.send(JSON.stringify({ text: messageText }));
        sentCount++;
      } else {
        console.warn(`[Host RTCDataChannel] DataChannel for client [${clientId}] is not open. ReadyState: ${dc?.readyState}`);
      }
    });

    if (sentCount > 0) {
      appendLog(`Host Sent to ${sentCount} viewer(s): ${messageText}`);
      input.value = "";
    } else {
      appendLog("Error: No open WebRTC data channel streams available to send message.");
    }
  } else if (myRole === 'viewer') {
    if (activeDataChannel && activeDataChannel.readyState === "open") {
      console.log(`[Viewer RTCDataChannel] Transmitting message to Host:`, messageText);
      activeDataChannel.send(JSON.stringify({ text: messageText }));
      appendLog(`Viewer Sent to Host: ${messageText}`);
      input.value = "";
    } else {
      console.warn(`[Viewer RTCDataChannel] Cannot send message. ReadyState: ${activeDataChannel?.readyState}`);
      appendLog("Error: Data channel pipeline is not open yet.");
    }
  }
}