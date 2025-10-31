#ifndef __SOUND_HARDWARE_H__
#define __SOUND_HARDWARE_H__

#include <SDL3/SDL_audio.hpp>

#include "SDLSoundSample.h"
#include "SDLSoundVoice.h"

class idSoundHardwareSDL3
{
public:
	idSoundHardwareSDL3( void );
	~idSoundHardwareSDL3( void );
	void			Init( void );
	void			Shutdown( void );
	void 			Update( void );
	void* 		    GetAudioDevice( void ) const;
	idSoundVoice* 	AllocateVoice( const idSoundSample* leadinSample, const idSoundSample* loopingSample );
	void			FreeVoice( idSoundVoice* voice );
	int				GetNumZombieVoices( void ) const { return m_freeVoices.Num(); }
	int				GetNumFreeVoices( void ) const { return 0; }

private:
    SDL_AudioSpec       					m_specs;
    SDL::Audio::Device  					m_device;
    SDL::Audio::Stream  					m_stream;
	idList<idSoundVoiceSDL3, TAG_AUDIO>		m_voices;
	idList<idSoundVoiceSDL3*, TAG_AUDIO>	m_usedVoices;
	idList<idSoundVoiceSDL3*, TAG_AUDIO>	m_freeVoices;
};

class idSoundHardware : public idSoundHardwareSDL3
{
public:
	idSoundHardware( void ) : idSoundHardwareSDL3()
	{
	}
};

#endif //__SOUND_HARDWARE_H__