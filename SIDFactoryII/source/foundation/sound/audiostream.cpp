#include "audiostream.h"
#include "SDL.h"
#include "foundation/base/assert.h"
#include "utils/config/configtypes.h"
#include "utils/configfile.h"
#include "utils/global.h"
#include "utils/logging.h"

using namespace Utility;
using namespace Utility::Config;

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

		ConfigFile& config = Global::instance().GetConfig();
		std::string preferred_device = GetSingleConfigurationValue<ConfigValueString>(config, "Sound.Output.Device", std::string(""));

		const int nr_output_devices = SDL_GetNumAudioDevices(0);
		if (nr_output_devices == 0) {
			Logging::instance().Error("No audio output devices found.");
		}

		bool unknown_preferred_device = true;
		for (int i = 0; i < nr_output_devices; ++i)
		{
			const std::string audio_device = std::string(SDL_GetAudioDeviceName(i, 0));
			Logging::instance().Info("Found audio output device \"%s\"", audio_device.c_str());
			unknown_preferred_device = unknown_preferred_device && audio_device != preferred_device;
		}
		if (unknown_preferred_device && preferred_device.length() > 0)
		{
			Logging::instance().Warning("Unknown audio output device: %s, falling back to default.", preferred_device.c_str());
			preferred_device = "";
		}

		const char * effective_device = preferred_device.length() > 0 ? preferred_device.c_str() : nullptr;
		m_AudioDeviceID = SDL_OpenAudioDevice(effective_device, 0, &audio_spec, &audio_spec_created, 0);

		// TODO: could this occur at all if we opened audio device without ALLOW flags?
		if (audio_spec.freq != audio_spec_created.freq)
		{
			Logging::instance().Warning("Requested %d Hz, but got %d Hz, sound could be affected by this.", audio_spec.freq, audio_spec_created.freq);
		}

		if (m_AudioDeviceID == 0)
		{
			Logging::instance().Error("Could not open audio device. SDL Error: %s", SDL_GetError());
		}
		else
		{
			Logging::instance().Info("Succesfully opened audio output device @ %d Hz", audio_spec_created.freq);
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
