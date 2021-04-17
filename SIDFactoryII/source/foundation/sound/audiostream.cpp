#include "audiostream.h"
#include "SDL.h"
#include "foundation/base/assert.h"
#include "utils/logging.h"

namespace Foundation
{
	void AudioStream::AudioCallback(void* inUserData, unsigned char* inStream, int inByteCount)
	{
		FOUNDATION_ASSERT(inUserData != nullptr);

		AudioStream* audio_stream_instance = static_cast<AudioStream*>(inUserData);

		if (audio_stream_instance->m_StreamFeeder != nullptr)
			audio_stream_instance->m_StreamFeeder->FeedPCM(static_cast<void*>(inStream), inByteCount);
	}

	AudioStream::AudioStream(unsigned int inFrequency, unsigned int inBitDepth, unsigned int inBufferDuration, IAudioStreamFeeder* inStreamFeeder)
		: m_Frequency(inFrequency)
		, m_BitDepth(inBitDepth)
		, m_BufferDuration(inBufferDuration)
		, m_StreamFeeder(inStreamFeeder)
	{
		const unsigned int buffer_size = inBufferDuration;
		const unsigned int buffer_size_power_of_two = [&buffer_size]() {
			unsigned int bits = 0;
			unsigned int size = buffer_size;

			while (true)
			{
				size >>= 1;
				if (size == 0)
					break;

				bits++;
			}

			return static_cast<unsigned int>(1 << bits);
		}();

		SDL_AudioSpec audio_spec;

		audio_spec.callback = &AudioStream::AudioCallback;
		audio_spec.userdata = this;
		audio_spec.channels = 1;
		audio_spec.format = inBitDepth == 16 ? AUDIO_S16LSB : AUDIO_U8;
		audio_spec.freq = inFrequency;
		audio_spec.samples = static_cast<unsigned short>(buffer_size_power_of_two);

		SDL_AudioSpec audio_spec_created;

		const int count = SDL_GetNumAudioDevices(0);

		for (int i = 0; i < count; ++i)
		{
			Utility::Logging::instance().Info("Audio device %d: %s", i, SDL_GetAudioDeviceName(i, 0));
		}

		m_AudioDeviceID = SDL_OpenAudioDevice(nullptr, 0, &audio_spec, &audio_spec_created, 0);

		if (m_AudioDeviceID == 0)
		{
			Utility::Logging::instance().Error("Could not open audio device. SDL Error: %s", SDL_GetError());
		}
	}

	AudioStream::~AudioStream()
	{
		if (m_AudioDeviceID != 0)
			SDL_CloseAudioDevice(m_AudioDeviceID);
	}


	void AudioStream::Start()
	{
		if (m_AudioDeviceID != 0)
			SDL_PauseAudioDevice(m_AudioDeviceID, 0);
	}


	void AudioStream::Stop()
	{
		if (m_AudioDeviceID != 0)
			SDL_PauseAudioDevice(m_AudioDeviceID, 1);
	}
}