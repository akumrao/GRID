# SFU For cloud and local Storage, works with both RTSP and WEBRTC


1. SFU (Selective Forwarding Unit). One instance of SFU in docker stores 15-17 cameras streams on cloud and 40 streams locally
2. Support both WebRTC and plain RTP input and output.
4. A tiny 3.5 mb C++ binary.
5. Be minimalist: just handle the media layer.
6. Be signaling agnostic: do not mandate any signaling protocol.
7. Be super low level API.
8. Support all existing WebRTC endpoints.
9. Enable integration with well known multimedia libraries/tools.
