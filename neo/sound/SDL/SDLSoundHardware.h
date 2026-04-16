#ifndef __SOUND_HARDWARE_H__
#define __SOUND_HARDWARE_H__

#include <SDL3/SDL_audio.hpp>

#include "SDLSoundSample.h"
#include "SDLSoundVoice.h"

inline constexpr uint32_t k_MAX_HARDWARE_VOICES = 128;

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
	idStaticList<idSoundVoiceSDL3, k_MAX_HARDWARE_VOICES>	m_voices; // available voices
	idStaticList<idSoundVoiceSDL3*, k_MAX_HARDWARE_VOICES>	m_usedVoices;
	idStaticList<idSoundVoiceSDL3*, k_MAX_HARDWARE_VOICES>	m_freeVoices;
};

class idSoundHardware : public idSoundHardwareSDL3
{
public:
	idSoundHardware( void ) : idSoundHardwareSDL3()
	{
	}
};

#endif //__SOUND_HARDWARE_H__