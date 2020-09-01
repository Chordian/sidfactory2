#pragma once

#include <memory>
#include "SDL.h"

namespace Foundation
{
	class IAudioStreamFeeder
	{
	protected:
		IAudioStreamFeeder() { }
	public:
		virtual ~IAudioStreamFeeder() { }

		virtual void Start() = 0;
		virtual void Stop() = 0;

		virtual bool IsStarted() const = 0;

		virtual unsigned int GetBytesFed() const = 0;								// Returns number of bytes of data fed to the stream!
		virtual unsigned int GetFeedCount() const = 0;								// Returns the number of times feed procedure as been called

		virtual void PreFeedPCM(void* inBuffer, unsigned int inByteCount) = 0;		// Called when pre feeding the buffer before starting it
		virtual void FeedPCM(void* inBuffer, unsigned int inByteCount) = 0;			// Called when ever the stream needs more data while running
	};

	class AudioStream final
	{
	public:
		AudioStream(unsigned int inFrequency, unsigned int inBitDepth, unsigned int inBufferDuration, IAudioStreamFeeder* inStreamFeeder);
		~AudioStream();

		void Start();
		void Stop();
	
	private:
		unsigned int m_Frequency;
		unsigned int m_BitDepth;
		unsigned int m_BufferDuration;

		IAudioStreamFeeder* m_StreamFeeder;

		SDL_AudioDeviceID m_AudioDeviceID;

		static void AudioCallback(void* inUserData, unsigned char* inStream, int inByteCount);
	};
}
