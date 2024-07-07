
#include "webrtc/peer.h"
#include "webrtc/peermanager.h"
#include "webrtc/peerfactorycontext.h"
#include "base/logger.h"
#include "p2p/base/transport_info.h"
#include "pc/media_session.h"
//#include "pc/peer_connection_wrapper.h"
#include "pc/sdp_utils.h"
#include "Settings.h"
#include <sys/reboot.h>

#if defined(__x86_64__)
#else
#include "sample-common.h"
#endif

using std::endl;

extern std::atomic<int>  HDVideo ;

namespace base
{
namespace web_rtc
{


cricket::Candidate CreateLocalUdpCandidate(const rtc::SocketAddress &address)
{
    cricket::Candidate candidate;
    candidate.set_component(cricket::ICE_CANDIDATE_COMPONENT_DEFAULT);
    candidate.set_protocol(cricket::UDP_PROTOCOL_NAME);
    candidate.set_address(address);
    candidate.set_type(cricket::LOCAL_PORT_TYPE);
    return candidate;
}

cricket::Candidate CreateLocalTcpCandidate(const rtc::SocketAddress &address)
{
    cricket::Candidate candidate;
    candidate.set_component(cricket::ICE_CANDIDATE_COMPONENT_DEFAULT);
    candidate.set_protocol(cricket::TCP_PROTOCOL_NAME);
    candidate.set_address(address);
    candidate.set_type(cricket::LOCAL_PORT_TYPE);
    candidate.set_tcptype(cricket::TCPTYPE_PASSIVE_STR);
    return candidate;
}


bool AddCandidateToFirstTransport(cricket::Candidate *candidate, webrtc::SessionDescriptionInterface *sdesc)
{
    auto *desc = sdesc->description();
    // RTC_DCHECK(desc->contents().size() > 0);
    const auto &first_content = desc->contents()[0];
    candidate->set_transport_name(first_content.name);
    std::unique_ptr<webrtc::IceCandidateInterface> jsep_candidate
        = webrtc::CreateIceCandidate(first_content.name, 0, *candidate);
    return sdesc->AddCandidate(jsep_candidate.get());
}


Peer::Peer(
    PeerManager *manager,
    PeerFactoryContext *context,
    st_track &trackInfo,
    Mode mode)
    : _manager(manager),
      _context(context),
      trackInfo(trackInfo),
      _mode(mode)
      //, _context->factory(manager->factory())
      ,
      _peerConnection(nullptr)
{
    SInfo << trackInfo.peerID << ": Creating ";

    webrtc::PeerConnectionInterface::IceServer stun;
    // stun.uri = kGoogleStunServerUri;
    //_config.servers.push_back(stun);

    stun.uri = "stun:stun.l.google.com:19302";
    
    _config.servers.push_back(stun);
    
    
    webrtc::PeerConnectionInterface::IceServer turn;
    turn.uri = "turn:13.235.182.183:3478?transport=udp";
    turn.username = "test"; 
    turn.password = "test123";         
    _config.servers.push_back(turn);        
      
      
    //  config_.sdp_semantics = sdp_semantics;

    // _constraints.SetMandatoryReceiveAudio(true);
    // _constraints.SetMandatoryReceiveVideo(true);
    // _constraints.SetAllowDtlsSctpDataChannels();

    //_config.servers.clear();
    //_config.servers.empty();
    _config.enable_rtp_data_channel = false;
    _config.enable_dtls_srtp = true;
    _config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    _config.rtcp_mux_policy = webrtc::PeerConnectionInterface::kRtcpMuxPolicyRequire;
    _config.bundle_policy = webrtc::PeerConnectionInterface::kBundlePolicyMaxBundle;
    _config.type = webrtc::PeerConnectionInterface::kAll;
    _config.candidate_network_policy = webrtc::PeerConnectionInterface::kCandidateNetworkPolicyLowCost;
    //_config.disable_ipv6 = false;
   // _config.disable_ipv6_on_wifi = false;
    _config.tcp_candidate_policy = webrtc::PeerConnectionInterface::kTcpCandidatePolicyDisabled;

    // _config.min_port =80000;
    //_config.max_port =100000;
    // _config.enable_ice_renomination = true;
    //_config.ice_candidate_pool_size=1;
}


Peer::~Peer()
{
    // LInfo( trackInfo.peerID, ": Destroying " , this )
    SInfo << "Destroying " << trackInfo.peerID ;
    // closeConnection();  // Do not uncomment it it will cause memory leaks
}


// rtc::scoped_refptr<webrtc::MediaStreamInterface> Peer::createMediaStream()
//{
//    // assert(_mode == Offer);
//     //assert(_context->factory);
//     assert(!_stream);
//     _stream = _context->factory->CreateLocalMediaStream(kStreamLabel);
//     return _stream;
// }


// void Peer::setPortRange(int minPort, int maxPort)
// {
//     assert(!_peerConnection);

//     if (!_context->networkManager) {
//         throw std::runtime_error("Must initialize custom network manager to set port range");
//     }

//     if (!_context->socketFactory) {
//         throw std::runtime_error("Must initialize custom socket factory to set port range");
//     }

//     if (!_portAllocator)
//         _portAllocator.reset(new cricket::BasicPortAllocator(
//             _context->networkManager.get(),
//             _context->socketFactory.get()));
//     _portAllocator->SetPortRange(minPort, maxPort);
// }


void Peer::createConnection()
{
    assert(_context->factory);
    _peerConnection = _context->factory->CreatePeerConnection(_config, nullptr, nullptr, this);

    //    if (_stream) {
    //        if (!_peerConnection->AddStream(_stream)) {
    //            throw std::runtime_error("Adding stream to Peer failed");
    //        }
    //    }
}


void Peer::closeConnection()
{
     SInfo << "Closing " << trackInfo.peerID;

        if (_peerConnection)
    {
        _peerConnection->Close();
    }
    else
    {
        // Call onClosed if no connection has been
        // made so callbacks are always run.
        _manager->onClosed(this);
    }
}


void Peer::createOffer( bool video , bool audio)
{
    // assert(_mode == Offer);
    // assert(_peerConnection);
    SInfo << "Create Offer " << trackInfo.peerID;

    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    options.offer_to_receive_audio = audio ;
    options.offer_to_receive_video = video;


    _peerConnection->CreateOffer(this, options);
}


void Peer::recvSDP(
    const std::string &type,
    const std::string &sdp,
    en_EncType &encType,
    std::string &vtrackid,
    std::string &atrackid)
{
    SDebug << "Received SDP " <<  type  <<   ": " <<  sdp ;

        webrtc::SdpParseError error;
    webrtc::SessionDescriptionInterface *desc(webrtc::CreateSessionDescription(type, sdp, &error));
    if (!desc) { SError << "Can't parse remote SDP: " <<  error.description; }
    _peerConnection->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), desc);


    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
    options.offer_to_receive_audio = true;
    options.offer_to_receive_video = true;

    if (type == "offer")
    {
        // assert(_mode == Answer);
        //SInfo <<  trackInfo.peerID << ": wrong state Received " <<  type ;
                
        _peerConnection->CreateAnswer(this, options);
    }
    else
    {}

    std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = _peerConnection->GetSenders();

    for (const rtc::scoped_refptr<webrtc::RtpSenderInterface> &sender : senders)
    {
        std::string str = sender->id();

        // cricket::MediaType media_type = sender->media_type();
        webrtc::MediaStreamTrackInterface *track = sender->track();
        if (!track) { continue; }

        if (track && track->kind() == "video")
        {
            vtrackid = track->id();  // removed prefixed vl
        }
        else if (track && track->kind() == "audio")
        {
            atrackid = track->id();  // removed prefixed vl
        }

        SInfo << track->kind() << " " << track->id();
        
        encType = EN_NATIVE;

        //webrtc::RtpParameters getParameters = sender->GetParameters();

      //  std::vector<webrtc::RtpCodecParameters> vtCodecs = getParameters.codecs;

//        for (webrtc::RtpCodecParameters &codecs : vtCodecs)
//        {
//            SDebug << codecs.name;
//            if (codecs.name == "H264")
//            {
//                std::unordered_map<std::string, std::string> parameters = codecs.parameters;
//
//                if (parameters["Enc"] == "NATIVE")
//                {
//                    encType = EN_NATIVE;
//                    break;
//                }
//                else if (parameters["Enc"] == "NVIDIA")
//                {
//                    encType = EN_NVIDIA;
//                    break;
//                }
//                else if (parameters["Enc"] == "QUICKSYNC")
//                {
//                    encType = EN_QUICKSYNC;
//                    break;
//                }
//                else if (parameters["Enc"] == "X264")  /// never use this better to use VP9
//                {
//                    encType = EN_X264;
//                    break;
//                }
//
//
//                //                for( auto &parameter :parameters)
//                //                {
//                //                     SInfo <<  parameter.first << " " <<  parameter.second;
//                //                }
//            }
//            else if (codecs.name == "VP9")
//            {
//                encType = EN_VP9;
//            }
//        }
    }
}


void Peer::recvCandidate(const std::string &mid, int mlineindex, const std::string &sdp)
{
    webrtc::SdpParseError error;
    std::unique_ptr<webrtc::IceCandidateInterface> candidate(
        webrtc::CreateIceCandidate(mid, mlineindex, sdp, &error));
    if (!candidate) { SError << "Can't parse remote candidate: " <<  error.description; }
    _peerConnection->AddIceCandidate(candidate.get());
}


void Peer::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
{
    LInfo( trackInfo.peerID, ": On signaling state change: ", new_state)

        switch (new_state)
    {
    case webrtc::PeerConnectionInterface::kStable:
        _manager->onStable(this);
        break;
    case webrtc::PeerConnectionInterface::kClosed:
        // _manager->onClosed(this);  // Do not uncomment it, it will cause memory leaks
        break;
    case webrtc::PeerConnectionInterface::kHaveLocalOffer:
    case webrtc::PeerConnectionInterface::kHaveRemoteOffer:
    case webrtc::PeerConnectionInterface::kHaveLocalPrAnswer:
    case webrtc::PeerConnectionInterface::kHaveRemotePrAnswer:
        break;
    }
}


void Peer::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
{
    LDebug( trackInfo.peerID, ": On ICE connection change: ", new_state)
}


void Peer::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
{
    LDebug( trackInfo.peerID, ": On ICE gathering change: ", new_state)

    // if( new_state == webrtc::PeerConnectionInterface::kIceGatheringComplete)
    // {
    //     createOffer();
    //     hasIceLiteOffer=true;
    // }
}


void Peer::OnRenegotiationNeeded()
{
    LInfo( trackInfo.peerID, ": On renegotiation needed")
}


void Peer::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
{
    LInfo( trackInfo.peerID, ": OnAddStream")
        // proxy to deprecated OnAddStream method
        OnAddStream(stream.get());
}

void Peer::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)
{
    LInfo( trackInfo.peerID, ": OnTrack")
    //_manager->onAddRemoteTrack(this, transceiver.get());

    //    const char * pMid  = transceiver->mid()->c_str();
    //    int iMid = atoi(pMid);
    //    RTC_LOG(INFO)  << "OnAddTrack " <<  " mid "  << pMid;
    //    if(  transceiver->current_direction() !=  webrtc::RtpTransceiverDirection::kInactive &&
    //    transceiver->direction() !=  webrtc::RtpTransceiverDirection::kInactive  )
    //    {
    //
    //        rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track =
    //                transceiver->receiver()->track();
    //        RTC_LOG(INFO)  << "OnAddTrack " << track->id() <<  " kind " << track->kind() ;
    //
    //        if (track && remote_video_observer_[0] &&
    //            track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
    //            static_cast<webrtc::VideoTrackInterface*>(track.get())
    //                    ->AddOrUpdateSink(remote_video_observer_[0].get(), rtc::VideoSinkWants());
    //            RTC_LOG(LS_INFO) << "Remote video sink set up: " << track;
    //
    //        }
    //
    //    }
    
/*
    if (transceiver->media_type() == cricket::MEDIA_TYPE_AUDIO)
    {
            webrtc::AudioTrackInterface& AudioTrack = static_cast<webrtc::AudioTrackInterface&>(*transceiver->receiver()->track());
            AudioTrack.AddSink(this);
    }
*/

}

/*
void Peer::OnData(const void* audio_data,
                      int bits_per_sample,
                      int sample_rate,
                      size_t number_of_channels,
                      size_t number_of_frames) 
{

                      SInfo << "bits_per_sample " << bits_per_sample <<  " sample_rate " << sample_rate << " number_of_channels " <<  number_of_channels  <<  " number_of_frames "  << number_of_frames;
}
*/
    
void Peer::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
{
    LInfo( trackInfo.peerID, ": OnRemoveTrack")
}

void Peer::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
{
    LInfo( trackInfo.peerID, ": OnRemoveStream") OnRemoveStream(stream.get());
}


void Peer::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
    channel->RegisterObserver(this);
    //LInfo( trackInfo.peerID, ": OnDataChannel") assert(0 && "virtual");
     data_channel_ = channel;
    //DataChannelSend("hello data channel");
}

 void Peer::OnStateChange() {
  if (data_channel_) {
    webrtc::DataChannelInterface::DataState state = data_channel_->state();
    if (state == webrtc::DataChannelInterface::kOpen) {
      //if (OnLocalDataChannelReady)
      //  OnLocalDataChannelReady();
         cnfg::Configuration identity;
        identity.load(Settings::configuration.storage + "manifest.js");

        if(identity.loaded())
        {
            json m;
            m["messageType"] = "RECORDING";
            m["messagePayload"] =  identity.root;
            DataChannelSend(m.dump());

        }
      RTC_LOG(LS_INFO) << "Data channel is open";
    }
  }
}





//  A data buffer was successfully received.
void Peer::OnMessage(const webrtc::DataBuffer& buffer) {
  size_t size = buffer.data.size();
  char* msg = new char[size + 1];
  memcpy(msg, buffer.data.data(), size);
  msg[size] = 0;

  if(  size < 11 )
  {
        if(!strncmp(msg, "startrec",   8 )  )
        {
            
           ((LiveThread*)_manager->ctx->liveThread)->t31rgba->record = true;
                     
            //ATOMIC_STORE_BOOL(&gSampleConfiguration->startrec, TRUE); 

        }else if(!strncmp(msg, "stoprec",   7 )  )
        {
           // ATOMIC_STORE_BOOL(&gSampleConfiguration->startrec, FALSE); 

        }else if(!strncmp(msg, "recDates",   8 )  )
        {
           char json[256]={'\0'};

//           MUTEX_LOCK(gSampleConfiguration->recordReadLock);
//           getJson(json);
//           MUTEX_UNLOCK(gSampleConfiguration->recordReadLock);
//
//           printf("final %s\n", json); 
//
//          STATUS retStatus = STATUS_SUCCESS;
//          retStatus = dataChannelSend(pDataChannel, FALSE, (PBYTE) json, STRLEN(json));
//          if (retStatus != STATUS_SUCCESS) {
//            DLOGI("[KVS Master] dataChannelSend(): operation returned status code: 0x%08x \n", retStatus);
//          }


        }
        else if(!strncmp(msg, "starttime:",   10 )  )
        {
            //strcpy( gSampleConfiguration->timeStamp, &pMessage[10]);

            //ATOMIC_STORE_BOOL(&gSampleConfiguration->newRecording, TRUE);
        }
    }
   else
   {
        
    if(binfile)
    {
        fwrite(msg, 1, size, binfile);

        OTALen += size;
        
        
        if(OTALen >= OTAsize  )
        {
            fclose(binfile);
            binfile = nullptr;

            if(OTALen != size)
            {
               // SError << " OTA file is bad:" << name;
               // std::remove(name.c_str());
                OTAsize = -1;
            }
            OTALen = 0;
            OTAsize = 0;
        }
        
        
        return;
    }
        
       

	json jsonMsg ;
	try
	{   
	   jsonMsg = json::parse(msg); 

	}
	catch(...)
	{
	    SError << "Json from client is not in correct format " << msg;
	    delete[] msg;
	    return;
	}
	   

	if (jsonMsg.find("messageType") != jsonMsg.end())
	{
	    std::string type = jsonMsg["messageType"].get<std::string>();  
	    if(type == "identity")
	    {
               
		  if( _manager->ctx->liveThread)
		  ((LiveThread*)_manager->ctx->liveThread)->onMessage(jsonMsg );
	    }
        else if(type == "OTA")
        {

            std::string name = jsonMsg["name"].get<std::string>();  
            name = "/mnt/OTA/" + name;
            int size  = jsonMsg["size"].get<int>();  
            if(size)
            {
                
                if(!binfile)
                {
                    binfile = fopen(name.c_str(), "wb");
                    OTAsize = size;  
                }
            }
            else
            { 
           // OTA = false;
                if(binfile)
                  fclose(binfile);
                binfile = nullptr;
            
                if(OTAsize < 0)
                {
                    SError << " OTA file is bad:" << name;
                    std::remove(name.c_str());
                }
                OTALen = 0;
                OTAsize = 0;

            }
        }
        else if(type == "REBOOT")
        {
          reboot(RB_AUTOBOOT);
        }
        else if(type == "RESET")
        {
          std::remove("/mnt/config.js");
        }
        else if(type == "DEBUG")
        {
            
           SInfo << "Set Log Debug " ;
             
           base::Logger::instance().get("webrtcserver")->setLevel(Level::Debug);
        }
        else if(type == "SETIRCUT")
        {

          if (jsonMsg.find("enable") != jsonMsg.end())
          { 

             bool ircut = jsonMsg["enable"].get<bool>();  
             SInfo << "ircut " << ircut;
              sample_SetIRCUT(ircut);
           }
        }  
        else if(type == "HD")
        {
          if (jsonMsg.find("enable") != jsonMsg.end())
          { 

             bool hd = jsonMsg["enable"].get<bool>();  
            
              if(hd)
                HDVideo =0;
              else 
               HDVideo =2; 
           }
        }  
                
          
	    //SInfo << jsonMsg.dump(4);
	    
	}


   }


  delete[] msg;
}


void Peer::OnAddStream(webrtc::MediaStreamInterface *stream)
{
    // assert(_mode == Answer);

    LInfo( trackInfo.peerID, ": On add stream") _manager->onAddRemoteStream(this, stream);
}


bool Peer::DataChannelSend(std::string data)
{
	webrtc::DataBuffer buff = webrtc::DataBuffer(data);
	if (!this->data_channel_)
	{
		LInfo("No Data Channel");
		return false;
	}
	if (this->data_channel_->state() != webrtc::DataChannelInterface::kOpen)
	{
		LInfo("Data Channel Not Open");
		return false;
	}
	bool ret = this->data_channel_->Send(buff);

	if (ret == false)
	{
		LInfo("Data not sent: " + data);
	}
	return ret;
}

void Peer::OnRemoveStream(webrtc::MediaStreamInterface *stream)
{
    // assert(_mode == Answer);

    LInfo( trackInfo.peerID, ": On remove stream") _manager->onRemoveRemoteStream(this, stream);
}


void Peer::OnIceCandidate(const webrtc::IceCandidateInterface *candidate)
{
    
 
            
            
    /*  std::string mid = candidate->sdp_mid() ;

      int line = candidate->sdp_mline_index();
     // Only for use internally.
      cricket::Candidate& can =  (cricket::Candidate& ) candidate->candidate();


      rtc::SocketAddress& add =  ( rtc::SocketAddress& ) can.address();

      bool x  =add.IsPrivateIP();
      if(x)
      add.SetIP("44.226.10.202");

     */

    std::string sdp;
    if (!candidate->ToString(&sdp))
    {
        LError( trackInfo.peerID, ": Failed to serialize candidate") assert(0);
        return;
    }

    SInfo << "Sending " << trackInfo.peerID <<  " " <<  sdp;
    _manager->sendCandidate(this, candidate->sdp_mid(), candidate->sdp_mline_index(), sdp);
}


void Peer::OnSuccess(webrtc::SessionDescriptionInterface *desc)
{
    LDebug( trackInfo.peerID, ": Set local description")

        cricket::SessionDescription *desc1
        = desc->description();


    /*for (const auto& content : desc1->contents()) {
       auto* transport_info = desc1->GetTransportInfoByName(content.name);
       transport_info->description.ice_mode = cricket::IceMode::ICEMODE_LITE;
      // transport_info->description.connection_role =  cricket::CONNECTIONROLE_ACTIVE;
       transport_info->description.transport_options.clear();
        transport_info->description.transport_options.push_back("renomination");

     }*/

    /*
    const rtc::SocketAddress kCallerAddress1("192.168.0.17", 1111);
    cricket::Candidate candidate1 = CreateLocalUdpCandidate(kCallerAddress1);
    AddCandidateToFirstTransport(&candidate1, SDP);
    const rtc::SocketAddress kCallerAddress2("192.168.0.17", 1001);
    cricket::Candidate candidate2 = CreateLocalTcpCandidate(kCallerAddress2);
    AddCandidateToFirstTransport(&candidate2, SDP);
    */

    _peerConnection->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);

    // Send an SDP offer to the peer

    // if(hasIceLiteOffer)
    {
        std::string sdp;
        if (!desc->ToString(&sdp))
        {
            LError( trackInfo.peerID, ": Failed to serialize local sdp") assert(0);
            return;
        }
        _manager->sendSDP(this, desc->type(), sdp);
    }
}


void Peer::OnFailure(const std::string &error)
{
    LError( trackInfo.peerID, ": On failure: ", error)

        _manager->onFailure(this, error);
}


void Peer::setPeerFactory(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory)
{
    assert(!_context->factory);  // should not be already set via PeerManager
    _context->factory = factory;
}


std::string Peer::peerid() const
{
    return  trackInfo.peerID;
}


void Peer::CloseDataChannel() {
  if (data_channel_.get()) {
    data_channel_->UnregisterObserver();
    data_channel_->Close();
  }
  data_channel_ = nullptr;
}

// void Peer::mute( const json& m)
// {
//     bool val = m.get<bool>();
//
//     SInfo <<  trackInfo.peerID <<  ": On mute: " <<  val ;
//
//    std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders =  _peerConnection->GetSenders();
//
//    std::string trackid;
//
//    std::string cmd =  "pause";
//
//    for (const auto& sender : senders)
//    {
//        rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track = sender->track();
//        if( track && track->kind() == "video")
//        {
//           trackid =  track->id();  // removed prefixed vl
//           SInfo <<  track->kind() << " " <<  trackid ;
//           track->set_enabled(!val);
//
//        }
//
//    }
//
//
//    _manager->_capturer.oncommand( this, trackid , cmd, val, 0 );
//
// }



    

// webrtc::FakeConstraints& Peer::constraints()
// {
//     return _constraints;
// }


webrtc::PeerConnectionFactoryInterface *Peer::factory() const
{
    return _context->factory.get();
}


rtc::scoped_refptr<webrtc::PeerConnectionInterface> Peer::peerConnection() const
{
    return _peerConnection;
}


// rtc::scoped_refptr<webrtc::MediaStreamInterface> Peer::stream() const
//{
//     return _stream;
// }


//
// Dummy Set Peer Description Observer
//


void DummySetSessionDescriptionObserver::OnSuccess()
{
    LDebug("On SDP parse success")
}


void DummySetSessionDescriptionObserver::OnFailure(const std::string &error)
{
    LError("On SDP parse error: ", error)
    // assert(0);
}


}  // namespace web_rtc
}  // namespace base
