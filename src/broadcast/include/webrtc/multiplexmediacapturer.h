

#ifndef WebRTC_MultiplexMediaCapturer_H
#define WebRTC_MultiplexMediaCapturer_H


#include "base/base.h"

//#ifdef HAVE_FFMPEG




#include "webrtc/audiopacketmodule.h"
#include "webrtc/videopacketsource.h"

#include "api/peer_connection_interface.h"
#include "webrtc/peer.h"


#include "webrtc/localAudioSouce.h"


namespace base
{
namespace web_rtc
{


// class VideoObserver : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
//  public:
//   VideoObserver() {}
//   ~VideoObserver() {}
//   //oid SetVideoCallback(I420FRAMEREADY_CALLBACK callback);
//
//  protected:
//   // VideoSinkInterface implementation
//   void OnFrame(const webrtc::VideoFrame& frame)
//   {
//       int x = 1;
//   }
//
//  private:
//
//   //std::mutex mutex;
// };


class MultiplexMediaCapturer
{
public:
    MultiplexMediaCapturer(LiveConnectionContext  *ctx, Signaler *sig);
    ~MultiplexMediaCapturer();

    void openFile(const std::string &dir, const std::string &file, const std::string &type, bool loop = true);

    std::string random_string();

   void addMediaTracks(webrtc::PeerConnectionFactoryInterface* factory,
                        webrtc::PeerConnectionInterface* conn, web_rtc::Peer *peer);

    //void start(std::string & cam );
    void stop(std::string & cam ,  std::set< std::string> & tmp);
    
    void remove(web_rtc::Peer* conn );


    void getPeerids(st_track &trackInfo, std::set<std::string> &sPeerIds);

    // void start(std::string & cam );




    void oncommand(web_rtc::Peer *conn, std::string &trackId, std::string &cmd, int arg1, int arg2);

    void mute(web_rtc::Peer *conn, std::string &trackId, std::string &cmd, bool act);

    void onAnswer(web_rtc::Peer *conn, en_EncType &encType, std::string &vtrackId, std::string &atrackId);

    rtc::scoped_refptr<AudioPacketModule> getAudioModule();
    
    void startRecording( std::string &cam );
    void stopRecording( std::string &cam );
    
    // VideoPacketSource* createVideoSource();

protected:
    //    PacketStream _stream;
#if MP4File
    ff::MediaCapture::Ptr _videoCapture;
#endif

    rtc::scoped_refptr<AudioPacketModule> _audioModule;


public:
    std::map<std::string, rtc::scoped_refptr<LocalAudioSource>> mapAudioSource;

    std::map<std::string, rtc::scoped_refptr<VideoPacketSource>> mapVideoSource;

protected:
    std::map<std::string, rtc::scoped_refptr<webrtc::AudioTrackInterface>> mapaudio_track;

    std::map<std::string, rtc::scoped_refptr<webrtc::VideoTrackInterface>> mapvideo_track;

    // std::unique_ptr<VideoObserver> local_video_observer_;

    int PlayerID;


private:
    std::string fileName;
    std::mutex mutexCap;
    
    LiveConnectionContext  *ctx;
    

};

}  // namespace web_rtc
}  // namespace base


//#endif // HAVE_FFMPEG
#endif
