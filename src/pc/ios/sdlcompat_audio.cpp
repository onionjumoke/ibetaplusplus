// sdlcompat_audio.cpp — RemoteIO output driving the game's mixer callback,
// plus WAV parsing and the audio-conversion chain SoundManager::decodeWav uses.
#include "sdlcompat/sdlcompat.h"

#include <AudioToolbox/AudioToolbox.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// ═════════════════════════════════════════════════════════════════════════════
// Sample readers
// ═════════════════════════════════════════════════════════════════════════════

namespace
{

bool isSint16Fmt(int f) { return f == AUDIO_S16LSB || f == AUDIO_S16MSB; }
bool isSint32Fmt(int f) { return f == AUDIO_S32LSB || f == AUDIO_S32MSB; }
bool fmtIsBigEndian(int f) { return f == AUDIO_S16MSB || f == AUDIO_S32MSB || f == AUDIO_F32MSB; }
int fmtBytes(int f)
{
	if (f == AUDIO_U8 || f == AUDIO_S8) return 1;
	if (isSint16Fmt(f)) return 2;
	return 4;
}

uint16_t swap16(uint16_t v) { return static_cast<uint16_t>((v << 8) | (v >> 8)); }
uint32_t swap32(uint32_t v) { return ((v & 0xFFu) << 24) | ((v & 0xFF00u) << 8) | ((v >> 8) & 0xFF00u) | (v >> 24); }

float readSample(const Uint8 *&p, int format)
{
	switch (format)
	{
		case AUDIO_U8:
			return (*p++ / 127.5f) - 1.0f;
		case AUDIO_S8:
			return static_cast<int8_t>(*p++) / 128.0f;
		case AUDIO_S16LSB:
		case AUDIO_S16MSB:
		{
			uint16_t u;
			memcpy(&u, p, 2);
			p += 2;
			if (fmtIsBigEndian(format))
				u = swap16(u);
			return static_cast<int16_t>(u) / 32768.0f;
		}
		case AUDIO_S32LSB:
		case AUDIO_S32MSB:
		{
			uint32_t u;
			memcpy(&u, p, 4);
			p += 4;
			if (fmtIsBigEndian(format))
				u = swap32(u);
			return static_cast<int32_t>(u) / 2147483648.0f;
		}
		default: // float32
		{
			uint32_t u;
			memcpy(&u, p, 4);
			p += 4;
			if (fmtIsBigEndian(format))
				u = swap32(u);
			float f;
			memcpy(&f, &u, 4);
			return f;
		}
	}
}

} // namespace

// ═════════════════════════════════════════════════════════════════════════════
// WAV loading
// ═════════════════════════════════════════════════════════════════════════════

extern "C" Uint8 *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, Uint8 **audio_buf, Uint32 *audio_len)
{
	if (spec == nullptr || audio_buf == nullptr || audio_len == nullptr || file == nullptr)
		return nullptr;

	*audio_buf = nullptr;
	*audio_len = 0;

	FILE *fp = fopen(file, "rb");
	if (fp == nullptr)
		return nullptr;

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (size < 12)
	{
		fclose(fp);
		return nullptr;
	}

	std::vector<uint8_t> data(static_cast<size_t>(size));
	if (fread(data.data(), 1, data.size(), fp) != data.size())
	{
		fclose(fp);
		return nullptr;
	}
	fclose(fp);

	const uint8_t *p = data.data();
	const uint8_t *end = p + data.size();
	if (memcmp(p, "RIFF", 4) != 0 || memcmp(p + 8, "WAVE", 4) != 0)
		return nullptr;

	int formatTag = -1, channels = 0, sampleRate = 0, bits = 0;
	const uint8_t *sampleData = nullptr;
	size_t sampleLen = 0;

	p += 12;
	while (p + 8 <= end)
	{
		char id[5] = {};
		memcpy(id, p, 4);
		uint32_t chunkSize;
		memcpy(&chunkSize, p + 4, 4); // little-endian host
		p += 8;
		if (p + chunkSize > end)
			chunkSize = static_cast<uint32_t>(end - p);

		if (memcmp(id, "fmt ", 4) == 0 && chunkSize >= 16)
		{
			uint16_t tag, ch;
			uint32_t rate;
			uint16_t bps;
			memcpy(&tag, p + 0, 2);
			memcpy(&ch, p + 2, 2);
			memcpy(&rate, p + 4, 4);
			memcpy(&bps, p + 14, 2);
			formatTag = tag;
			channels = ch;
			sampleRate = rate;
			bits = bps;
		}
		else if (memcmp(id, "data", 4) == 0)
		{
			sampleData = p;
			sampleLen = chunkSize;
		}

		p += chunkSize + (chunkSize & 1); // chunks are word-aligned
	}

	if (formatTag != 1 && formatTag != 3) // PCM / IEEE float
		return nullptr;
	if (channels <= 0 || channels > 2 || sampleRate <= 0)
		return nullptr;

	int wavFormat;
	if (formatTag == 3 && bits == 32)
		wavFormat = AUDIO_F32SYS;
	else if (formatTag == 1 && bits == 8)
		wavFormat = AUDIO_U8;
	else if (formatTag == 1 && bits == 16)
		wavFormat = AUDIO_S16SYS;
	else if (formatTag == 1 && bits == 32)
		wavFormat = AUDIO_S32SYS;
	else
		return nullptr; // 24-bit and ADPCM oddities are not worth the code

	Uint8 *out = static_cast<Uint8 *>(malloc(sampleLen ? sampleLen : 1));
	if (out == nullptr)
		return nullptr;
	if (sampleData && sampleLen)
		memcpy(out, sampleData, sampleLen);

	spec->freq = sampleRate;
	spec->format = wavFormat;
	spec->channels = static_cast<Uint8>(channels);
	spec->silence = 0;
	spec->samples = 4096;
	spec->callback = nullptr;
	spec->userdata = nullptr;

	*audio_buf = out;
	*audio_len = static_cast<Uint32>(sampleLen);
	return out;
}

extern "C" void SDL_FreeWAV(Uint8 *audio_buf)
{
	free(audio_buf);
}

// ═════════════════════════════════════════════════════════════════════════════
// Conversion chain: any source → Float32 interleaved stereo @ dst rate
// ═════════════════════════════════════════════════════════════════════════════

extern "C" int SDL_BuildAudioCVT(SDL_AudioCVT *cvt,
                                 int src_format, Uint8 src_channels, int src_rate,
                                 int dst_format, Uint8 dst_channels, int dst_rate)
{
	if (cvt == nullptr || src_channels == 0 || dst_channels == 0 ||
	    src_rate <= 0 || dst_rate <= 0 || fmtBytes(src_format) == 4 /*ambiguous*/)
	{
		if (cvt != nullptr)
			cvt->needed = 0;
		return -1;
	}

	cvt->src_format = src_format;
	cvt->dst_format = dst_format;
	cvt->channels_in = src_channels;
	cvt->channels_out = dst_channels;
	cvt->freq_in = src_rate;
	cvt->freq_out = dst_rate;
	cvt->rate_incr = static_cast<double>(dst_rate) / static_cast<double>(src_rate);
	cvt->buf = nullptr;
	cvt->len = 0;
	cvt->len_cvt = 0;

	cvt->needed = !(src_format == dst_format && src_channels == dst_channels && src_rate == dst_rate);
	if (!cvt->needed)
	{
		cvt->len_mult = 1;
		return 0;
	}

	// Worst-case output size per input byte:
	//   frames_out ≈ frames_in * dstRate/srcRate, each frame dstCh * 4 bytes.
	int srcBps = fmtBytes(src_format);
	double expand =
	    (static_cast<double>(dst_rate) / static_cast<double>(src_rate)) *
	    (static_cast<double>(dst_channels) * sizeof(float)) /
	    (static_cast<double>(src_channels) * srcBps);
	double mult = expand > 1.0 ? expand : 1.0;
	cvt->len_mult = static_cast<int>(ceil(mult)) + 1; // one frame of slack
	return 0;
}

extern "C" int SDL_ConvertAudio(SDL_AudioCVT *cvt)
{
	if (cvt == nullptr || cvt->needed == 0 || cvt->buf == nullptr)
		return cvt ? 0 : -1;

	const int srcBps = fmtBytes(cvt->src_format);
	const int srcFrameBytes = cvt->channels_in * srcBps;
	if (cvt->len <= 0 || srcFrameBytes <= 0)
	{
		cvt->len_cvt = 0;
		return 0;
	}

	const size_t framesIn = static_cast<size_t>(cvt->len) / srcFrameBytes;

	// Decode + channel-mix into stereo floats at the source rate.
	std::vector<float> stereo(framesIn * 2);
	for (size_t i = 0; i < framesIn; ++i)
	{
		const Uint8 *p = cvt->buf + i * srcFrameBytes;
		float chans[2] = {0.0f, 0.0f};
		if (cvt->channels_in == 1)
		{
			chans[0] = chans[1] = readSample(p, cvt->src_format);
		}
		else
		{
			chans[0] = readSample(p, cvt->src_format);
			chans[1] = readSample(p, cvt->src_format);
		}
		stereo[i * 2 + 0] = chans[0];
		stereo[i * 2 + 1] = chans[1];
	}

	// Resample: linear interpolation is cheap and hiss-free enough here.
	const double ratio = static_cast<double>(cvt->freq_out) / static_cast<double>(cvt->freq_in);
	size_t framesOut;
	std::vector<float> resampled;
	const float *mixData;
	if (fabs(ratio - 1.0) < 1e-9)
	{
		framesOut = framesIn;
		mixData = stereo.data();
	}
	else
	{
		framesOut = static_cast<size_t>(ceil(framesIn * ratio));
		resampled.resize(framesOut * 2);
		for (size_t i = 0; i < framesOut; ++i)
		{
			double pos = static_cast<double>(i) / ratio;
			size_t i0 = static_cast<size_t>(pos);
			if (framesIn >= 2 && i0 > framesIn - 2)
				i0 = framesIn - 2;
			size_t i1 = (i0 + 1 < framesIn) ? i0 + 1 : i0;
			float frac = static_cast<float>(pos - static_cast<double>(i0));
			for (int c = 0; c < 2; ++c)
			{
				float a = stereo[i0 * 2 + c];
				float b = stereo[i1 * 2 + c];
				resampled[i * 2 + c] = a + (b - a) * frac;
			}
		}
		mixData = resampled.data();
	}

	size_t outBytes = framesOut * 2 * sizeof(float);
	size_t capacity = static_cast<size_t>(cvt->len) * static_cast<size_t>(cvt->len_mult);
	if (outBytes > capacity)
		outBytes = capacity & ~(sizeof(float) * 2 - 1); // keep whole frames

	float *out = reinterpret_cast<float *>(cvt->buf);
	for (size_t i = 0; i < outBytes / (sizeof(float) * 2); ++i)
	{
		out[i * 2 + 0] = mixData[i * 2 + 0];
		out[i * 2 + 1] = mixData[i * 2 + 1];
	}

	cvt->len_cvt = static_cast<int>(outBytes);
	return 0;
}

// ═════════════════════════════════════════════════════════════════════════════
// Output device: AudioUnit RemoteIO
//
// On iOS the RemoteIO component APIs live in AudioToolbox (no AudioUnit
// framework link needed). The game's mixer callback runs on the realtime
// audio render thread; all state it touches lives inside SoundManager.
// ═════════════════════════════════════════════════════════════════════════════

namespace
{

struct IosAudioDevice
{
	AudioUnit unit;
	SDL_AudioSpec have;
	std::vector<Uint8> renderBuf;
	bool running;
};

IosAudioDevice g_device = {};

OSStatus renderProc(void *inRefCon,
                    AudioUnitRenderActionFlags *,
                    const AudioTimeStamp *,
                    UInt32,
                    UInt32 numFrames,
                    AudioBufferList *ioData)
{
	IosAudioDevice *dev = static_cast<IosAudioDevice *>(inRefCon);
	const size_t bytesWanted = static_cast<size_t>(numFrames) * dev->have.channels * sizeof(float);

	if (ioData->mNumberBuffers < 1)
		return noErr;
	AudioBuffer &dst = ioData->mBuffers[0];

	if (dev->renderBuf.size() < bytesWanted)
		dev->renderBuf.resize(bytesWanted);

	if (dev->have.callback)
		dev->have.callback(dev->have.userdata, &dev->renderBuf[0], static_cast<int>(bytesWanted));
	else
		memset(&dev->renderBuf[0], 0, bytesWanted);

	size_t copyLen = bytesWanted < dst.mDataByteSize ? bytesWanted : dst.mDataByteSize;
	memcpy(dst.mData, &dev->renderBuf[0], copyLen);
	dst.mDataByteSize = static_cast<UInt32>(copyLen);
	return noErr;
}

} // namespace

extern "C" SDL_AudioDeviceID SDL_OpenAudioDevice(const char *device, int iscapture,
                                                 const SDL_AudioSpec *want, SDL_AudioSpec *have,
                                                 int allowed_changes)
{
	(void)device;
	(void)allowed_changes;
	if (iscapture || want == nullptr || want->callback == nullptr)
		return 0;

	// One device slot: reopening replaces the previous unit.
	if (g_device.unit != nullptr)
		SDL_CloseAudioDevice(1);

	AudioComponentDescription desc;
	memset(&desc, 0, sizeof(desc));
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_RemoteIO;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;

	AudioComponent comp = AudioComponentFindNext(nullptr, &desc);
	if (comp == nullptr)
		return 0;

	AudioUnit unit;
	if (AudioComponentInstanceNew(comp, &unit) != noErr)
		return 0;

	// Respect the mute switch like a normal iOS app.
	AudioSessionInitialize(nullptr, nullptr, nullptr, nullptr);
	UInt32 category = kAudioSessionCategory_AmbientSound;
	AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
	AudioSessionSetActive(true);

	// Interleaved Float32 stereo at the requested rate on the input scope of
	// the output element (element 0).
	AudioStreamBasicDescription asbd;
	memset(&asbd, 0, sizeof(asbd));
	asbd.mSampleRate = static_cast<Float64>(want->freq);
	asbd.mFormatID = kAudioFormatLinearPCM;
	asbd.mFormatFlags = kLinearPCMFormatFlagIsFloat | kLinearPCMFormatFlagIsPacked;
	asbd.mChannelsPerFrame = 2;
	asbd.mBitsPerChannel = 32;
	asbd.mBytesPerFrame = asbd.mChannelsPerFrame * sizeof(float);
	asbd.mFramesPerPacket = 1;
	asbd.mBytesPerPacket = asbd.mBytesPerFrame;

	if (AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat,
	                         kAudioUnitScope_Input, 0, &asbd, sizeof(asbd)) != noErr)
	{
		AudioComponentInstanceDispose(unit);
		return 0;
	}

	AURenderCallbackStruct cb;
	memset(&cb, 0, sizeof(cb));
	cb.inputProc = renderProc;
	cb.inputProcRefCon = &g_device;
	if (AudioUnitSetProperty(unit, kAudioUnitProperty_SetRenderCallback,
	                         kAudioUnitScope_Input, 0, &cb, sizeof(cb)) != noErr)
	{
		AudioComponentInstanceDispose(unit);
		return 0;
	}

	if (AudioUnitInitialize(unit) != noErr)
	{
		AudioComponentInstanceDispose(unit);
		return 0;
	}

	g_device.unit = unit;
	g_device.have = *want;
	g_device.have.format = AUDIO_F32SYS;
	g_device.have.channels = 2;
	if (g_device.have.samples < 512 || g_device.have.samples > 4096)
		g_device.have.samples = 1024;
	g_device.have.silence = 0;
	g_device.have.size = g_device.have.samples * g_device.have.channels * sizeof(float);
	g_device.renderBuf.reserve(g_device.have.size);
	g_device.running = false;

	if (have != nullptr)
		*have = g_device.have;

	if (AudioOutputUnitStart(unit) != noErr)
	{
		SDL_CloseAudioDevice(1);
		return 0;
	}

	g_device.running = true;
	return 1;
}

extern "C" void SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on)
{
	(void)dev;
	if (g_device.unit == nullptr)
		return;
	if (pause_on && g_device.running)
	{
		AudioOutputUnitStop(g_device.unit);
		g_device.running = false;
	}
	else if (!pause_on && !g_device.running)
	{
		AudioOutputUnitStart(g_device.unit);
		g_device.running = true;
	}
}

extern "C" void SDL_CloseAudioDevice(SDL_AudioDeviceID dev)
{
	(void)dev;
	if (g_device.unit == nullptr)
		return;
	if (g_device.running)
		AudioOutputUnitStop(g_device.unit);
	g_device.running = false;
	AudioUnitUninitialize(g_device.unit);
	AudioComponentInstanceDispose(g_device.unit);
	g_device.unit = nullptr;
}
