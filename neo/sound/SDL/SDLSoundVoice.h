
#ifndef __SOUND_VOICE_SDL_H__
#define __SOUND_VOICE_SDL_H__

#include <SDL3/SDL_audio.hpp>

class idSoundSampleSDL3;
class idSoundVoiceSDL3 : public idSoundVoice_Base
{
public:
	idSoundVoiceSDL3( void );
	~idSoundVoiceSDL3( void );

	void		Create( const idSoundSample* leadinSample, const idSoundSample* loopingSample );
	/// @brief Start playing at a particular point in the buffer.  Does an Update() too
	void		Start( int offsetMS, int ssFlags );	
	/// @brief Stop playing.
	void		Stop( void ); 
	/// @brief Stop consuming buffers
	void		Pause( void ); 
	/// @brief Start consuming buffers again
	void		UnPause( void ); 
	/// @brief Sends new position/volume/pitch information to the hardware
	bool		Update( void ); 
	/// @brief returns the RMS levels of the most recently processed block of audio, SSF_FLICKER must have been passed to Start
	float		GetAmplitude( void ); 
	/// @brief returns true if we can re-use this voice
	bool		CompatibleFormat( idSoundSample* s ); 
	
	/// @brief Helper function to submit a buffer
	int			SubmitBuffer( idSoundSampleSDL3* sample, int bufferNumber, int offset );
	
	uint32_t	GetSampleRate( void ) const { return m_sampleRate; }

protected:
	friend class idSoundHardwareSDL3;
	
	/// @brief Returns true when all the buffers are finished processing
	bool	IsPlaying( void ) const; 
	
	/// @brief Called after the voice has been stopped
	void	FlushSourceBuffers( void ) const; 
	
	/// @brief Destroy the internal hardware resource
	void	DestroyInternal( void ); 
	
	/// @brief Helper function used by the initial start as well as for looping a streamed buffer
	int		RestartAt( int offsetSamples ); 
	
	/// @brief Adjust the voice frequency based on the new sample rate for the buffer
	void	SetSampleRate( uint32_t newSampleRate, uint32_t operationSet ); 
	
	/// @brief Helper function to submit a buffer
	int		SubmitBuffer( idSoundSample* sample, int bufferNumber, int offset ); 
	
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
	idSoundSampleSDL3*		m_leadinSample;
	idSoundSampleSDL3*		m_loopingSample;
	SDL::Audio::Stream		m_stream;
};

class idSoundVoice : public idSoundVoiceSDL3
{
public:
	idSoundVoice( void ) : idSoundVoiceSDL3()
	{}
};

#endif //!__SOUND_VOICE_SDL_H__