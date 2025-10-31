
#ifndef __SOUND_VOICE_SDL_H__
#define __SOUND_VOICE_SDL_H__

#include <SDL3/SDL_audio.hpp>

class idSoundVoiceSDL3 : public idSoundVoice_Base
{
public:
	idSoundVoiceSDL3( void );
	~idSoundVoiceSDL3( void );

	void		Create( const idSoundSample* leadinSample, const idSoundSample* loopingSample );
	void		Start( int offsetMS, int ssFlags );	// Start playing at a particular point in the buffer.  Does an Update() too
	void		Stop( void ); // Stop playing.
	void		Pause( void ); // Stop consuming buffers
	void		UnPause( void ); // Start consuming buffers again
	bool		Update( void ); // Sends new position/volume/pitch information to the hardware
	float		GetAmplitude( void ); // returns the RMS levels of the most recently processed block of audio, SSF_FLICKER must have been passed to Start
	bool		CompatibleFormat( idSoundSample* s ); // returns true if we can re-use this voice
	uint32_t	GetSampleRate( void ) const { return m_sampleRate; }

protected:
	friend class idSoundHardwareSDL3;
	bool	IsPlaying( void ) const; // Returns true when all the buffers are finished processing
	void	FlushSourceBuffers( void ) const; // Called after the voice has been stopped
	void	DestroyInternal( void ); // Destroy the internal hardware resource
	int		RestartAt( int offsetSamples ); // Helper function used by the initial start as well as for looping a streamed buffer
	void	SetSampleRate( uint32_t newSampleRate, uint32_t operationSet ); // Adjust the voice frequency based on the new sample rate for the buffer
	int		SubmitBuffer( idSoundSample* sample, int bufferNumber, int offset ); // Helper function to submit a buffer
	SDL::Audio::Stream	&Stream( void ) { return m_stream; }


	// Chamado pelo mixer
    //int MixSamples( const int frames );

private:
	bool 					m_looping;
    bool 					m_playing;
    bool 					m_paused;
	uint8_t					m_channels;
	uint32_t				m_sampleRate;
	float 					m_volume;
    float 					m_pitch;
	size_t					m_size;
	uintptr_t				m_cursor;
	SDL_AudioSpec			m_audioSpec;
	float*					m_buffer;
	SDL::Audio::Stream		m_stream;
};

class idSoundVoice : public idSoundVoiceSDL3
{
public:
	idSoundVoice( void ) : idSoundVoiceSDL3()
	{}
};

#endif //!__SOUND_VOICE_SDL_H__