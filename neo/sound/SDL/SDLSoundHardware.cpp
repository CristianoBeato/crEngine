
#include "precompiled.h"
#include "sound/snd_local.h"
#include "SDLSoundHardware.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_audio.h>

// The whole system runs at this sample rate
constexpr int MAX_OUT_CHANNELS = 6; 
constexpr int SYSTEM_SAMPLE_RATE = 44100;
constexpr float ONE_OVER_SYSTEM_SAMPLE_RATE = 1.0f / SYSTEM_SAMPLE_RATE;


idCVar s_device( "s_device", "-1", CVAR_INTEGER | CVAR_ARCHIVE, "Which audio device to use (listDevices to list, -1 for default)" );
idCVar s_channels( "s_channels", "2", CVAR_INTEGER | CVAR_ARCHIVE, "audio channels output 1 Mono; 2 Estéreo; 4 for 4.0 Quadrifonic or 6 surround 5.1" );

constexpr uint32_t k_MAX_HARDWARE_VOICES = 96;

idSoundHardwareSDL3::idSoundHardwareSDL3(void)
{
}

idSoundHardwareSDL3::~idSoundHardwareSDL3(void)
{
}

void idSoundHardwareSDL3::Init( void )
{
    int availableDevice = 0;
    int numDevices = 0;
    SDL_AudioDeviceID   openDevice = 0;
    SDL_AudioDeviceID*  availableDevices = nullptr;

    if ( !SDL_InitSubSystem(SDL_INIT_AUDIO) ) 
    {
        common->FatalError("SDL audio init failed: %s\n", SDL_GetError());
        return;
    }

    // list the available output devices
    availableDevices = SDL_GetAudioPlaybackDevices( &numDevices );

    m_specs.freq = 48000;
    m_specs.format = SDL_AUDIO_F32;
    m_specs.channels = Min( s_channels.GetInteger(), MAX_OUT_CHANNELS );

    if ( s_device.GetInteger() < 0 )
    {
        // just open the default devie 
        openDevice = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
    }
    else
    {
        openDevice = availableDevices[std::clamp( s_device.GetInteger(), 0, numDevices )];
    }

    if( !m_device.Open( openDevice, &m_specs ) )
    {
        // fallback 
        if ( openDevice == SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK )
            common->FatalError("Failed to open SDL audio device: %s\n", SDL_GetError());
        else
        {
            if( !m_device.Open( SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &m_specs ) )   
                common->FatalError("Failed to open SDL audio device: %s\n", SDL_GetError());
        }
    }

    m_voices.SetNum( k_MAX_HARDWARE_VOICES );
    m_freeVoices.SetNum( k_MAX_HARDWARE_VOICES );
    m_usedVoices.SetNum( k_MAX_HARDWARE_VOICES );
    // set all voices free
    for ( uint32_t i = 0; i < k_MAX_HARDWARE_VOICES; i++)
    {
        m_freeVoices[i] = &m_voices[i];
    }

    m_device.Resume();
}

void idSoundHardwareSDL3::Shutdown(void)
{
    for ( int i = 0; i < m_usedVoices.Num(); i++)
    {
        if ( m_usedVoices[i] != nullptr )
            m_usedVoices[i]->Stop();
    }
    
    m_usedVoices.Clear();
    m_freeVoices.Clear();
    m_voices.Clear();
    
    if ( m_device )
        m_device.Close();   
}

void idSoundHardwareSDL3::Update(void)
{
    if( idSoundSystem::Get()->IsMuted() )
        m_device.SetGain( 0.0f );
    else
    {
        if ( s_volume_dB.IsModified() )
            m_device.SetGain( DBtoLinear( s_volume_dB.GetFloat() ) ); // set the game main volume   
    }
}

/*
========================
idSoundHardwareSDL3::GetAudioDevice
========================
*/
void *idSoundHardwareSDL3::GetAudioDevice( void ) const
{
    return nullptr;
}

/*
========================
idSoundHardwareSDL3::AllocateVoice
========================
*/
idSoundVoice *idSoundHardwareSDL3::AllocateVoice(const idSoundSample *leadinSample, const idSoundSample *loopingSample)
{
	if( leadinSample == nullptr )
		return nullptr;
	
	//if( loopingSample != nullptr )
	//{
	//	if( ( leadinSample->format.basic.formatTag != loopingSample->format.basic.formatTag ) || ( leadinSample->format.basic.numChannels != loopingSample->format.basic.numChannels ) )
	//	{
	//		idLib::Warning( "Leadin/looping format mismatch: %s & %s", leadinSample->GetName(), loopingSample->GetName() );
	//		loopingSample = nullptr;
	//	}
	//}
	
	// Try to find a free voice that matches the format
	// But fallback to the last free voice if none match the format
	idSoundVoice* voice = nullptr;
	for( int i = 0; i < m_freeVoices.Num(); i++ )
	{
		if( m_freeVoices[i]->IsPlaying() )
			continue;
		
		voice = static_cast<idSoundVoice*>( m_freeVoices[i] );
		if( voice->CompatibleFormat( const_cast<idSoundSample*>( leadinSample ) ) )
			break;
	}

	if( voice != nullptr )
	{
		voice->Create( leadinSample, loopingSample );
        
		// remove move from free list to used list 
        m_freeVoices.Remove( voice );
        m_usedVoices.Append( voice );

        // attach stream to device
        m_device.BindStream( voice->Stream() );
		return voice;
	}
	
	return nullptr;
}


/*
========================
idSoundHardwareSDL3::FreeVoice
========================
*/
void idSoundHardwareSDL3::FreeVoice( idSoundVoice *voice )
{
    // remove voices from used put in free
    m_usedVoices.Remove( voice );
    m_freeVoices.Append( voice );

    // signal to stop playing
    voice->Stop();    
}
