///
#include "webrtc/fakeaudiodevicemodule.h"

//#include "rtc_base/refcountedobject.h"
#include "rtc_base/thread.h"
//#include "rtc_base/timeutils.h"

#include "base/logger.h"


using std::endl;


namespace base
{
namespace web_rtc
{


// Same value as src/modules/audio_device/main/source/audio_device_config.h in
// https://code.google.com/p/webrtc/
static const int kAdmMaxIdleTimeProcess = 1000;
static const uint32_t kMaxVolume = 14392;


FakeAudioDeviceModule::FakeAudioDeviceModule()
    : _lastProcessTimeMS(0),
      _audioCallback(nullptr),
      _recording(false),
      _playing(false),
      _playIsInitialized(false),
      _recIsInitialized(false),
      _currentMicLevel(kMaxVolume)
{}

FakeAudioDeviceModule::~FakeAudioDeviceModule() {}

rtc::scoped_refptr<FakeAudioDeviceModule> FakeAudioDeviceModule::Create()
{
    rtc::scoped_refptr<FakeAudioDeviceModule> capture_module(
        new rtc::RefCountedObject<FakeAudioDeviceModule>());
    return capture_module;
}

void FakeAudioDeviceModule::OnMessage(rtc::Message *msg) {}

// int64_t FakeAudioDeviceModule::TimeUntilNextProcess()
// {
//     const int64_t current_time = rtc::TimeMillis();
//     if (current_time < _lastProcessTimeMS) {
//         // TODO: wraparound could be handled more gracefully.
//         return 0;
//     }
//     const int64_t elapsed_time = current_time - _lastProcessTimeMS;
//     if (kAdmMaxIdleTimeProcess < elapsed_time) {
//         return 0;
//     }
//     return kAdmMaxIdleTimeProcess - elapsed_time;
// }
//
// void FakeAudioDeviceModule::Process()
// {
//     _lastProcessTimeMS = rtc::TimeMillis();
// }

int32_t FakeAudioDeviceModule::ActiveAudioLayer(AudioLayer * /*audio_layer*/) const
{
    // assert(false);
    return 0;
}

// webrtc::AudioDeviceModule::ErrorCode FakeAudioDeviceModule::LastError() const
// {
//     //assert(false);
//     return webrtc::AudioDeviceModule::kAdmErrNone;
// }

// int32_t FakeAudioDeviceModule::RegisterEventObserver(webrtc::AudioDeviceObserver* /*event_callback*/)
// {
//     // Only used to report warnings and errors. This fake implementation won't
//     // generate any so discard this callback.
//     return 0;
// }

int32_t FakeAudioDeviceModule::RegisterAudioCallback(webrtc::AudioTransport *audio_callback)
{
    // rtc::CritScope cs(&_critCallback);
    // _audioCallback = audio_callback;
    return 0;
}

int32_t FakeAudioDeviceModule::Init()
{
    // Initialize is called by the factory method.
    // Safe to ignore this Init call.
    return 0;
}

int32_t FakeAudioDeviceModule::Terminate()
{
    // Clean up in the destructor. No action here, just success.
    return 0;
}

bool FakeAudioDeviceModule::Initialized() const
{
    // assert(false);
    return 0;
}

int16_t FakeAudioDeviceModule::PlayoutDevices()
{
    // assert(false);
    return 0;
}

int16_t FakeAudioDeviceModule::RecordingDevices()
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::PlayoutDeviceName(
    uint16_t /*index*/, char /*name*/[webrtc::kAdmMaxDeviceNameSize], char /*guid*/[webrtc::kAdmMaxGuidSize])
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::RecordingDeviceName(
    uint16_t /*index*/, char /*name*/[webrtc::kAdmMaxDeviceNameSize], char /*guid*/[webrtc::kAdmMaxGuidSize])
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::SetPlayoutDevice(uint16_t /*index*/)
{
    // No playout device, just playing from file. Return success.
    return 0;
}

int32_t FakeAudioDeviceModule::SetPlayoutDevice(WindowsDeviceType /*device*/)
{
    if (_playIsInitialized) { return -1; }
    return 0;
}

int32_t FakeAudioDeviceModule::SetRecordingDevice(uint16_t /*index*/)
{
    // No recording device, just dropping audio. Return success.
    return 0;
}

int32_t FakeAudioDeviceModule::SetRecordingDevice(WindowsDeviceType /*device*/)
{
    if (_recIsInitialized) { return -1; }
    return 0;
}

int32_t FakeAudioDeviceModule::PlayoutIsAvailable(bool * /*available*/)
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::InitPlayout()
{
    _playIsInitialized = true;
    return 0;
}

bool FakeAudioDeviceModule::PlayoutIsInitialized() const
{
    return _playIsInitialized;
}

int32_t FakeAudioDeviceModule::RecordingIsAvailable(bool * /*available*/)
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::InitRecording()
{
    _recIsInitialized = true;
    return 0;
}

bool FakeAudioDeviceModule::RecordingIsInitialized() const
{
    return _recIsInitialized;
}

int32_t FakeAudioDeviceModule::StartPlayout()
{
    LDebug("Start playout") if (!_playIsInitialized) { return -1; }
    {
        rtc::CritScope cs(&_crit);
        _playing = true;
    }
    return 0;
}

int32_t FakeAudioDeviceModule::StopPlayout()
{
    LDebug("Stop playout")
    {
        rtc::CritScope cs(&_crit);
        _playing = false;
    }
    return 0;
}

bool FakeAudioDeviceModule::Playing() const
{
    rtc::CritScope cs(&_crit);
    return _playing;
}

int32_t FakeAudioDeviceModule::StartRecording()
{
    LDebug("Start recording") if (!_recIsInitialized) { return -1; }
    {
        rtc::CritScope cs(&_crit);
        _recording = true;
    }
    return 0;
}

int32_t FakeAudioDeviceModule::StopRecording()
{
    LDebug("Stop recording")
    {
        rtc::CritScope cs(&_crit);
        _recording = false;
    }
    return 0;
}

bool FakeAudioDeviceModule::Recording() const
{
    rtc::CritScope cs(&_crit);
    return _recording;
}

// int32_t FakeAudioDeviceModule::SetAGC(bool /*enable*/)
// {
//     // No AGC but not needed since audio is pregenerated. Return success.
//     return 0;
// }
//
// bool FakeAudioDeviceModule::AGC() const
// {
//     //assert(false);
//     return 0;
// }

int32_t FakeAudioDeviceModule::InitSpeaker()
{
    // No speaker, just playing from file. Return success.
    return 0;
}

bool FakeAudioDeviceModule::SpeakerIsInitialized() const
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::InitMicrophone()
{
    // No microphone, just playing from file. Return success.
    return 0;
}

bool FakeAudioDeviceModule::MicrophoneIsInitialized() const
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::SpeakerVolumeIsAvailable(bool * /*available*/)
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::SetSpeakerVolume(uint32_t /*volume*/)
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::SpeakerVolume(uint32_t * /*volume*/) const
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::MaxSpeakerVolume(uint32_t * /*max_volume*/) const
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::MinSpeakerVolume(uint32_t * /*min_volume*/) const
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::MicrophoneVolumeIsAvailable(bool * /*available*/)
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::SetMicrophoneVolume(uint32_t volume)
{
    rtc::CritScope cs(&_crit);
    _currentMicLevel = volume;
    return 0;
}

int32_t FakeAudioDeviceModule::MicrophoneVolume(uint32_t *volume) const
{
    rtc::CritScope cs(&_crit);
    *volume = _currentMicLevel;
    return 0;
}

int32_t FakeAudioDeviceModule::MaxMicrophoneVolume(uint32_t *max_volume) const
{
    *max_volume = kMaxVolume;
    return 0;
}

int32_t FakeAudioDeviceModule::MinMicrophoneVolume(uint32_t * /*min_volume*/) const
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::SpeakerMuteIsAvailable(bool * /*available*/)
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::SetSpeakerMute(bool /*enable*/)
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::SpeakerMute(bool * /*enabled*/) const
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::MicrophoneMuteIsAvailable(bool * /*available*/)
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::SetMicrophoneMute(bool /*enable*/)
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::MicrophoneMute(bool * /*enabled*/) const
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::StereoPlayoutIsAvailable(bool *available) const
{
    // No recording device, just dropping audio. Stereo can be dropped just
    // as easily as mono.
    *available = true;
    return 0;
}

int32_t FakeAudioDeviceModule::SetStereoPlayout(bool /*enable*/)
{
    // No recording device, just dropping audio. Stereo can be dropped just
    // as easily as mono.
    return 0;
}

int32_t FakeAudioDeviceModule::StereoPlayout(bool * /*enabled*/) const
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::StereoRecordingIsAvailable(bool *available) const
{
    // Keep thing simple. No stereo recording.
    *available = false;
    return 0;
}

int32_t FakeAudioDeviceModule::SetStereoRecording(bool enable)
{
    if (!enable) { return 0; }
    return -1;
}

int32_t FakeAudioDeviceModule::StereoRecording(bool * /*enabled*/) const
{
    // assert(false);
    return 0;
}

int32_t FakeAudioDeviceModule::PlayoutDelay(uint16_t *delay_ms) const
{
    // No delay since audio frames are dropped.
    *delay_ms = 0;
    return 0;
}


}  // namespace web_rtc
}  // namespace base


#if 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/prctl.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#include <imp/imp_audio.h>
#include <imp/imp_log.h>

#define TAG "Sample-Ao"

#define AO_TEST_SAMPLE_RATE 16000
#define AO_TEST_SAMPLE_TIME 20
#define AO_TEST_BUF_SIZE (AO_TEST_SAMPLE_RATE * sizeof(short) * AO_TEST_SAMPLE_TIME / 1000)
#define AO_BASIC_TEST_PLAY_FILE  "./ao_paly.pcm"

static void *_ao_test_play_thread(void *argv)
{
	unsigned char *buf = NULL;
	int size = 0;
	int ret = -1;

	buf = (unsigned char *)malloc(AO_TEST_BUF_SIZE);
	if (buf == NULL) {
		IMP_LOG_ERR(TAG, "[ERROR] %s: malloc audio buf error\n", __func__);
		return NULL;
	}

	FILE *play_file = fopen(AO_BASIC_TEST_PLAY_FILE, "rb");
	if (play_file == NULL) {
		IMP_LOG_ERR(TAG, "[ERROR] %s: fopen %s failed\n", __func__, AO_BASIC_TEST_PLAY_FILE);
		return NULL;
	}

	/* Step 1: set public attribute of AO device. */
	int devID = 0;
	IMPAudioIOAttr attr;
	attr.samplerate = AUDIO_SAMPLE_RATE_16000;
	attr.bitwidth = AUDIO_BIT_WIDTH_16;
	attr.soundmode = AUDIO_SOUND_MODE_MONO;
	attr.frmNum = 20;
	attr.numPerFrm = 640;
	attr.chnCnt = 1;
	ret = IMP_AO_SetPubAttr(devID, &attr);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "set ao %d attr err: %d\n", devID, ret);
		return NULL;
	}

	memset(&attr, 0x0, sizeof(attr));
	ret = IMP_AO_GetPubAttr(devID, &attr);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "get ao %d attr err: %d\n", devID, ret);
		return NULL;
	}

	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr samplerate:%d\n", attr.samplerate);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr   bitwidth:%d\n", attr.bitwidth);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr  soundmode:%d\n", attr.soundmode);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr     frmNum:%d\n", attr.frmNum);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr  numPerFrm:%d\n", attr.numPerFrm);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr     chnCnt:%d\n", attr.chnCnt);

	/* Step 2: enable AO device. */
	ret = IMP_AO_Enable(devID);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "enable ao %d err\n", devID);
		return NULL;
	}

	/* Step 3: enable AI channel. */
	int chnID = 0;
	ret = IMP_AO_EnableChn(devID, chnID);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio play enable channel failed\n");
		return NULL;
	}

	/* Step 4: Set audio channel volume. */
	int chnVol = 80;
	ret = IMP_AO_SetVol(devID, chnID, chnVol);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Play set volume failed\n");
		return NULL;
	}

	ret = IMP_AO_GetVol(devID, chnID, &chnVol);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Play get volume failed\n");
		return NULL;
	}
	IMP_LOG_INFO(TAG, "Audio Out GetVol    vol:%d\n", chnVol);

	int aogain = 28;
	ret = IMP_AO_SetGain(devID, chnID, aogain);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record Set Gain failed\n");
		return NULL;
	}

	ret = IMP_AO_GetGain(devID, chnID, &aogain);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record Get Gain failed\n");
		return NULL;
	}
	IMP_LOG_INFO(TAG, "Audio Out GetGain    gain : %d\n", aogain);

	int i = 0;
	while (1) {
		size = fread(buf, 1, AO_TEST_BUF_SIZE, play_file);
		if (size < AO_TEST_BUF_SIZE)
			break;

		/* Step 5: send frame data. */
		IMPAudioFrame frm;
		frm.virAddr = (uint32_t *)buf;
		frm.len = size;
		ret = IMP_AO_SendFrame(devID, chnID, &frm, BLOCK);
		if (ret != 0) {
			IMP_LOG_ERR(TAG, "send Frame Data error\n");
			return NULL;
		}

		IMPAudioOChnState play_status;
		ret = IMP_AO_QueryChnStat(devID, chnID, &play_status);
		if (ret != 0) {
			IMP_LOG_ERR(TAG, "IMP_AO_QueryChnStat error\n");
			return NULL;
		}

		IMP_LOG_INFO(TAG, "Play: TotalNum %d, FreeNum %d, BusyNum %d\n",
				play_status.chnTotalNum, play_status.chnFreeNum, play_status.chnBusyNum);

		if (++i == 40) {
			ret = IMP_AO_PauseChn(devID, chnID);
			if (ret != 0) {
				IMP_LOG_ERR(TAG, "IMP_AO_PauseChn error\n");
				return NULL;
			}

			printf("[INFO] Test : Audio Play Pause test.\n");
			printf("[INFO]      : Please input any key to continue.\n");
			getchar();

			ret = IMP_AO_ClearChnBuf(devID, chnID);
			if (ret != 0) {
				IMP_LOG_ERR(TAG, "IMP_AO_ClearChnBuf error\n");
				return NULL;
			}

			ret = IMP_AO_ResumeChn(devID, chnID);
			if (ret != 0) {
				IMP_LOG_ERR(TAG, "IMP_AO_ResumeChn error\n");
				return NULL;
			}
		}
	}
	ret = IMP_AO_FlushChnBuf(devID, chnID);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_AO_FlushChnBuf error\n");
		return NULL;
	}
	/* Step 6: disable the audio channel. */
	ret = IMP_AO_DisableChn(devID, chnID);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio channel disable error\n");
		return NULL;
	}

	/* Step 7: disable the audio devices. */
	ret = IMP_AO_Disable(devID);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio device disable error\n");
		return NULL;
	}

	fclose(play_file);
	free(buf);
	pthread_exit(0);
}

int _ao_basic_test(void)
{
	int ret = -1;

	pthread_t play_thread_id;

	printf("[INFO] Test ao basic:\n");
	printf("[INFO]        : Can read the %s file.\n", AO_BASIC_TEST_PLAY_FILE);
	printf("[INFO]        : Please input any key to continue.\n");
	getchar();

	ret = pthread_create(&play_thread_id, NULL, _ao_test_play_thread, NULL);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "[ERROR] %s: pthread_create Audio Record failed\n", __func__);
		return -1;
	}
	pthread_join(play_thread_id, NULL);
	return ret;
}

int main(void)
{
	int ret = -1;
	ret = _ao_basic_test();
	if (0 != ret) {
		printf("err: _ao_basic_test\n");
	}
	return ret;
}
 
#endif