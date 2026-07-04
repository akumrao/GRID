

#ifndef RTC_TRACK_H
#define RTC_TRACK_H

#include "channel.h"
#include "common.hpp"
#include "description.hpp"
#include "mediahandler.hpp"

#if RTC_ENABLE_MEDIA
//#include "dtlssrtptransport.hpp"
//#include "impl/dtlssrtptransport.hpp"

#include "sctptransport.hpp"

#endif

#include "queue.hpp"
#include <shared_mutex>

namespace rtc {

struct PeerConnection;

class Track  :  public std::enable_shared_from_this<Track>, public Channel {
public:
    
     	Track(weak_ptr<PeerConnection> pc, Description::Media desc);
        
	//Track();
	~Track() override;

	string mid() const;
	Description::Direction direction() const;
	Description::Media description() const;

	void setDescription(Description::Media description);

	void close(void) override;
	bool send(message_variant data) override;
	bool send(const byte *data, size_t size) override;

	bool isOpen(void) const override;
	bool isClosed(void) const override;
	size_t maxMessageSize() const override;

	void onFrame(std::function<void(binary data, FrameInfo frame)> callback);

	bool requestKeyframe();
	bool requestBitrate(unsigned int bitrate);

	void setMediaHandler(shared_ptr<MediaHandler> handler);
	void chainMediaHandler(shared_ptr<MediaHandler> handler);
	//shared_ptr<MediaHandler> getMediaHandler();

	// Deprecated, use setMediaHandler() and getMediaHandler()
	inline void setRtcpHandler(shared_ptr<MediaHandler> handler) { setMediaHandler(handler); }
	inline shared_ptr<MediaHandler> getRtcpHandler() { return getMediaHandler(); }
        
        
        
        
        
        
        
                
        
        
        
        

public:
    
    
   
	void ImpDesTrack();

	void Impclose();
	void incoming(message_ptr message);
	bool outgoing(message_ptr message);

	optional<message_variant> receive() override;
	optional<message_variant> peek() override;
	size_t availableAmount() const override;
	void flushPendingMessages() override;
	message_variant trackMessageToVariant(message_ptr message);

	void ImponFrame(std::function<void(binary data, FrameInfo frame)> callback);

	bool ImpisOpen() const;
	bool ImpisClosed() const;
	size_t ImpmaxMessageSize() const;

	string Impmid() const;
	Description::Direction Impdirection() const;
	Description::Media Impdescription() const;
	void ImpsetDescription(Description::Media desc);

	shared_ptr<MediaHandler> getMediaHandler();
	void ImpsetMediaHandler(shared_ptr<MediaHandler> handler);

#if RTC_ENABLE_MEDIA
	void open(shared_ptr<SctpTransport> transport);

#endif

	bool transportSend(message_ptr message);

private:
	const weak_ptr<PeerConnection> mPeerConnection;
#if RTC_ENABLE_MEDIA
	weak_ptr<SctpTransport> mDtlsSrtpTransport;

	//shared_ptr<SctpTransport> mDtlsSrtpTransport;
#endif

	Description::Media mMediaDescription;
	shared_ptr<MediaHandler> mMediaHandler;

	std::shared_mutex mMutex;

	std::atomic<bool> mIsClosed = false;

//	Queue<message_ptr> mRecvQueue;
        
        std::queue<message_ptr> mRecvQueue;

	synchronized_callback<binary, FrameInfo> frameCallback;
    
    
    
    
    
	
};

} // namespace rtc

#endif
