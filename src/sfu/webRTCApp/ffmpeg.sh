#!/usr/bin/env bash

videoTransportIp="127.0.0.1"
videoTransportPort="$1"
videoTransportRtcpPort="$2"
MEDIA_FILE="./test.mp4"

# handler.cpp enable  #define RTSP 1 . Please open two tabs of  https://127.0.0.1:9090/  select video only and click webcam.  at other tab click subscribe twice (Knowin bug )
# ./configure --disable-shared  --enable-ffplay --enable-libopus --enable-debug=3  --disable-optimizations --disable-stripping -enable-libfdk-aac    --enable-nonfree --enable-libx264 --enable-gpl --enable-libvpx

if [ "$#" -eq 0 ]; then
  echo "No arguments provided. Please copy ./ffmpeg.sh parameter from output of ./runwebrtc"
  exit 1
else
  echo "Arguments provided: $@"
fi

echo  "rtp://${videoTransportIp}:${videoTransportPort}?rtcpport=${videoTransportRtcpPort}"

set -e

BROADCASTER_ID=$(LC_CTYPE=C tr -dc A-Za-z0-9 < /dev/urandom | fold -w ${1:-32} | head -n 1)
HTTPIE_COMMAND="http --check-status"
AUDIO_SSRC=1111
AUDIO_PT=100
VIDEO_SSRC=2222


#

#
echo ">>> running ffmpeg..."

#
# NOTES:
# - We can add ?pkt_size=1200 to each rtp:// URI to limit the max packet size
#   to 1200 bytes.


VIDEO_PT=101
ffmpeg \
	-re \
	-v info \
	-stream_loop -1 \
	-i ${MEDIA_FILE} \
	-map 0:v:0 \
	-pix_fmt yuv420p -c:v libvpx -b:v 1000k -deadline realtime -cpu-used 4 \
	-f tee \
	"[select=v:f=rtp:ssrc=${VIDEO_SSRC}:payload_type=${VIDEO_PT}]rtp://${videoTransportIp}:${videoTransportPort}?rtcpport=${videoTransportRtcpPort}"

# VIDEO_PT=96
# ffmpeg \
# 	-re \
# 	-v info \
# 	-stream_loop -1 \
# 	-i ${MEDIA_FILE} \
# 	-map 0:v:0 \
# 	-pix_fmt yuv420p -c:v libx264 -b:v 1000k  \
# 	-f tee \
# 	"[select=v:f=rtp:ssrc=${VIDEO_SSRC}:payload_type=${VIDEO_PT}]rtp://${videoTransportIp}:${videoTransportPort}?rtcpport=${videoTransportRtcpPort}"
