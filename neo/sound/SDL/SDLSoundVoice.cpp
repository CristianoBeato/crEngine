
#include "precompiled.h"
#include "sound/snd_local.h"
#include "SDLSoundVoice.h"

enum
{
    LEFT_FRONT,
    RIGHT_FRONT,
    LEFT_BACK,
    RIGHT_BACK
};

idSoundVoiceSDL3::idSoundVoiceSDL3( void ) :
    m_looping( false ),
    m_playing( false ),
    m_paused( false ),
    m_channels( 0 ),
    m_volume( 1.0f ),
    m_sampleRate( 0 ),
    m_size( 0 ),
    m_cursor( 0 ),
    m_buffer( nullptr )
{
}

idSoundVoiceSDL3::~idSoundVoiceSDL3(void)
{
    DestroyInternal();
}

void idSoundVoiceSDL3::Create(const idSoundSample *leadinSample, const idSoundSample *loopingSample)
{
    m_audioSpec.channels = leadinSample->NumChannels();
    m_audioSpec.freq = leadinSample->SampleRate();
    m_audioSpec.format = SDL_AUDIO_S16;

    if( IsPlaying() )
	{
		// This should never hit
		Stop();
		return;
	}

    if( !m_stream.Create( &m_audioSpec, &m_audioSpec ) )
    {
        // TODO: call error here
        common->Error( "" );
    }
}

void idSoundVoiceSDL3::Start( int offsetMS, int ssFlags )
{
    m_playing = true;
    m_paused = false;
    m_cursor = 0;
}

void idSoundVoiceSDL3::Stop( void )
{
    m_playing = false;
    m_paused = false;
    m_cursor = 0;
    // release stream from logic devie
    m_stream.Unbind();
}

void idSoundVoiceSDL3::Pause(void)
{
    m_paused = true;
}

void idSoundVoiceSDL3::UnPause(void)
{
    m_paused = false;
}

bool idSoundVoiceSDL3::Update(void)
{
    return false;
}

float idSoundVoiceSDL3::GetAmplitude(void)
{
    if (!m_playing || m_paused)
        return 0.0f;

    float sumSquares = 0.0f;
    size_t framesToCheck = std::min<size_t>(128, m_size / m_channels - m_cursor);

    for (size_t frame = 0; frame < framesToCheck; frame++)
    {
        for (size_t ch = 0; ch < m_channels; ch++)
        {
            float s = m_buffer[(m_cursor + frame) * m_channels + ch];
            sumSquares += s * s;
        }
    }

    float meanSquare = sumSquares / (framesToCheck * m_channels);
    return std::sqrt(meanSquare); // 0.0f .. 1.0f
}

bool idSoundVoiceSDL3::CompatibleFormat(idSoundSample *s)
{
	if( m_buffer == nullptr )
        return true; // If this voice has never been allocated, then it's compatible with everything
	
	return false;
}

bool idSoundVoiceSDL3::IsPlaying( void ) const
{
    return ( m_looping && m_playing );
}

void idSoundVoiceSDL3::DestroyInternal(void)
{
    if ( m_buffer != nullptr )
    {
        SDL_free( m_buffer );
        m_buffer = nullptr;
    }
    
    if ( m_stream )
        m_stream.Clear();

    m_looping = false;
    m_playing = false;
    m_paused = false;
    m_channels = 0;
    m_sampleRate = 0;
    m_volume = 0.0f;
    m_pitch = 0.0f;
    m_size = 0;
    m_cursor = 0;
}

void idSoundVoiceSDL3::SetSampleRate(uint32_t newSampleRate, uint32_t operationSet)
{
}

/*
int idSoundVoiceSDL3::MixSamples( const int frames )
{
    if ( !m_playing || m_paused ) 
        return 0;

        
    // Calcula quantos frames ainda podemos misturar
    size_t framesAvailable = ( m_size / m_channels ) - m_cursor;
    size_t framesToMix = Min<size_t>( frames, framesAvailable );
        
    // not mutch inteligent, but will serve for now 
    float * out = static_cast<float*>( SDL_malloc( sizeof( float ) * framesToMix ) );
    
    for ( size_t frame = 0; frame < framesToMix; frame++ )
    {
        for (size_t channel = 0; channel < m_channels; channel++)
        {
            float pos = 1.0f;

            // Aplica pan apenas nos canais frontais
            if ( channel == LEFT_FRONT)
                pos = 1.0f - m_pan * 0.5f;
            else if ( channel == RIGHT_FRONT )
                pos = 1.0f + m_pan * 0.5f;

            out[frame * m_channels + channel] += m_buffer[(m_cursor + frame) * m_channels + channel] * m_volume * pos;
        }
    }

    m_cursor += framesToMix;

    // apply loop
    if ( m_cursor >= m_size / m_channels )
    {
        if (m_looping)
            m_cursor = 0;
        else
            m_playing = false;
    }

    // send data to stream 
    m_stream.PutData( out, sizeof( float ) * framesToMix );
    SDL_free( out );

    return static_cast<int>(framesToMix); // mixed frames
    
}
*/