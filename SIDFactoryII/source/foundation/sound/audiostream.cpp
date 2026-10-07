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
		, m_ChannelCount(1)
		, m_IsRunning(false)
		, m_StreamFeeder(inStreamFeeder)
		, m_AudioDeviceID(0)
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

		m_BufferDuration = buffer_size_power_of_two;

		const int count = SDL_GetNumAudioDevices(0);

		for (int i = 0; i < count; ++i)
		{
			Utility::Logging::instance().Info("Audio device %d: %s", i, SDL_GetAudioDeviceName(i, 0));
		}

		Open();
	}

	AudioStream::~AudioStream()
	{
		Close();
	}


	void AudioStream::Open()
	{
		SDL_AudioSpec audio_spec;

		audio_spec.callback = &AudioStream::AudioCallback;
		audio_spec.userdata = this;
		audio_spec.channels = static_cast<unsigned char>(m_ChannelCount);
		audio_spec.format = m_BitDepth == 16 ? AUDIO_S16LSB : AUDIO_U8;
		audio_spec.freq = m_Frequency;
		audio_spec.samples = static_cast<unsigned short>(m_BufferDuration);

		SDL_AudioSpec audio_spec_created;

		// No callback runs before the device is unpaused: the feeder learns the layout first
		if (m_StreamFeeder != nullptr)
			m_StreamFeeder->SetChannelCount(m_ChannelCount);

		m_AudioDeviceID = SDL_OpenAudioDevice(nullptr, 0, &audio_spec, &audio_spec_created, 0);

		if (m_AudioDeviceID == 0)
		{
			Utility::Logging::instance().Error("Could not open audio device. SDL Error: %s", SDL_GetError());
		}
		Utility::Logging::instance().Info("Audio device frequency: %d, channels: %d", audio_spec_created.freq, audio_spec_created.channels);
	}


	void AudioStream::Close()
	{
		// Returns once a running callback is done
		if (m_AudioDeviceID != 0)
			SDL_CloseAudioDevice(m_AudioDeviceID);

		m_AudioDeviceID = 0;
	}


	void AudioStream::Start()
	{
		m_IsRunning = true;

		if (m_AudioDeviceID != 0)
			SDL_PauseAudioDevice(m_AudioDeviceID, 0);
	}


	void AudioStream::Stop()
	{
		m_IsRunning = false;

		if (m_AudioDeviceID != 0)
			SDL_PauseAudioDevice(m_AudioDeviceID, 1);
	}


	void AudioStream::SetChannelCount(unsigned int inChannelCount)
	{
		const unsigned int channel_count = inChannelCount == 2 ? 2 : 1;

		if (channel_count == m_ChannelCount)
			return;

		Close();

		m_ChannelCount = channel_count;

		Open();

		if (m_IsRunning && m_AudioDeviceID != 0)
			SDL_PauseAudioDevice(m_AudioDeviceID, 0);
	}
}