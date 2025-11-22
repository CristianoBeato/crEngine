
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
    m_cursor( 0 )
{
}

idSoundVoiceSDL3::~idSoundVoiceSDL3(void)
{
    DestroyInternal();
}

void idSoundVoiceSDL3::Create(const idSoundSample *leadinSample, const idSoundSample *loopingSample)
{
    if( IsPlaying() )
	{
		// This should never hit
		Stop();
		return;
	}

    m_leadinSample = (idSoundSampleSDL3*)leadinSample;
	m_loopingSample = (idSoundSampleSDL3*)loopingSample;

    if( m_stream != nullptr && CompatibleFormat( const_cast<idSoundSample*>( leadinSample ) ) )
	{
		m_sampleRate = leadinSample->SampleRate();
	}
	else
	{
		DestroyInternal();

        m_audioSpec.channels = leadinSample->NumChannels();
        m_audioSpec.freq = leadinSample->SampleRate();
        m_audioSpec.format = SDL_AUDIO_S16;
 		
		if( !m_stream.Create( &m_audioSpec, &m_audioSpec ) )
		{
			// If this hits, then we are most likely passing an invalid sample format, which should have been caught by the loader (and the sample defaulted)
            common->Error( SDL_GetError() );
			return;
		}
#if 0
		if( s_debugHardware.GetBool() )
		{
			if( loopingSample == nullptr || loopingSample == leadinSample )
			{
				idLib::Printf( "%dms: %p created for %s\n", Sys_Milliseconds(), pSourceVoice, leadinSample ? leadinSample->GetName() : "<null>" );
			}
			else
			{
				idLib::Printf( "%dms: %p created for %s and %s\n", Sys_Milliseconds(), pSourceVoice, leadinSample ? leadinSample->GetName() : "<null>", loopingSample ? loopingSample->GetName() : "<null>" );
			}
		}
#endif
	}
}

void idSoundVoiceSDL3::Start( int offsetMS, int ssFlags )
{
    if( !m_leadinSample )
		return;

	if( !m_stream )
		return;
    
    if( m_leadinSample->IsDefault() )
		idLib::Warning( "Starting defaulted sound sample %s", m_leadinSample->GetName() );

    bool flicker = ( ssFlags & SSF_NO_FLICKER ) == 0;

    assert( offsetMS >= 0 );
	int offsetSamples = MsecToSamples( offsetMS, m_leadinSample->SampleRate() );
	if( m_loopingSample == nullptr && offsetSamples >= m_leadinSample->NumSamples() )
		return;
	
	RestartAt( offsetSamples );
	Update();
	UnPause();
}

void idSoundVoiceSDL3::Stop( void )
{
    if( !m_stream )
        return;

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
    if( m_stream == nullptr || m_leadinSample == nullptr )
		return false;
	
    const int srcChannels = m_leadinSample->NumChannels();
	
	float pLevelMatrix[ MAX_CHANNELS_PER_VOICE * MAX_CHANNELS_PER_VOICE ] = { 0 };
	CalculateSurround( srcChannels, pLevelMatrix, 1.0f );
	
//	if( s_skipHardwareSets.GetBool() )
//		return true;
	
	///assert( idMath::Fabs( gain ) <= XAUDIO2_MAX_VOLUME_LEVEL );
	///pSourceVoice->SetVolume( gain, OPERATION_SET );
    m_stream.SetGain( gain );

	SetSampleRate( m_sampleRate, 1 );
	
	// we don't do this any longer because we pause and unpause explicitly when the soundworld is paused or unpaused
	// UnPause();
	return true;
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
            float s = 2.0f; //TODO: load from sample
            //float s = m_buffer[(m_cursor + frame) * m_channels + ch];
            sumSquares += s * s;
        }
    }

    float meanSquare = sumSquares / (framesToCheck * m_channels);
    return std::sqrt(meanSquare); // 0.0f .. 1.0f
}

bool idSoundVoiceSDL3::CompatibleFormat( idSoundSample *s )
{   
    SDL_AudioSpec format;

    // If this voice has never been allocated, then it's compatible with everything
    if ( m_stream == nullptr )  
        return true;

    m_stream.GetFormat( &format, &format );

    if ( format.channels == s->NumChannels() && format.freq == s->SampleRate() )
        return true;

    return false;
}

int idSoundVoiceSDL3::RestartAt(int offsetSamples)
{
    offsetSamples &= ~127;
	
	idSoundSampleSDL3* sample = m_leadinSample;
	if( offsetSamples >= m_leadinSample->NumSamples() )
	{
		if( m_loopingSample != nullptr )
		{
			offsetSamples %= m_loopingSample->NumSamples();
			sample = m_loopingSample;
		}
		else
			return 0;
	}
	
	int previousNumSamples = 0;
	for( int i = 0; i < sample->NumBuffers(); i++ )
	{
     	if( sample->Buffers()[i].NumSamples() > sample->PlayBegin() + offsetSamples )
			return SubmitBuffer( sample, i, sample->PlayBegin() + offsetSamples - previousNumSamples );
		
		previousNumSamples = sample->Buffers()[i].NumSamples();
	}

    return 0;
}

int idSoundVoiceSDL3::SubmitBuffer(idSoundSampleSDL3 * sample, int bufferNumber, int offset)
{
	if( sample == nullptr || ( bufferNumber < 0 ) || ( bufferNumber >= sample->NumBuffers() ) )
		return 0;
	
	idSoundSystemLocal::bufferContext_t* bufferContext = static_cast<idSoundSystemLocal*>(idSoundSystem::Get())->ObtainStreamBufferContext();
	if( bufferContext == nullptr )
	{
		idLib::Warning( "No free buffer contexts!" );
		return 0;
	}
	
	bufferContext->voice = this;
	bufferContext->sample = sample;
	bufferContext->bufferNumber = bufferNumber;

    m_stream.PutData( sample->Buffers()[bufferNumber].Ptr(), sample->Buffers()[bufferNumber].Size() );

    /*
	XAUDIO2_BUFFER buffer = { 0 };
	if( offset > 0 )
	{
		int previousNumSamples = 0;
		if( bufferNumber > 0 )
		{
			previousNumSamples = sample->buffers[bufferNumber - 1].numSamples;
		}
		buffer.PlayBegin = offset;
		buffer.PlayLength = sample->buffers[bufferNumber].numSamples - previousNumSamples - offset;
	}
	buffer.AudioBytes = sample->buffers[bufferNumber].bufferSize;
	buffer.pAudioData = ( BYTE* )sample->buffers[bufferNumber].buffer;
	buffer.pContext = bufferContext;
	if( ( loopingSample == nullptr ) && ( bufferNumber == sample->buffers.Num() - 1 ) )
	{
		buffer.Flags = XAUDIO2_END_OF_STREAM;
	}
	pSourceVoice->SubmitSourceBuffer( &buffer );
	
	return buffer.AudioBytes;
    */

    return 0;
}

bool idSoundVoiceSDL3::IsPlaying( void ) const
{
    return ( m_looping && m_playing );
}

void idSoundVoiceSDL3::DestroyInternal(void)
{
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
    if( m_stream == nullptr || m_leadinSample == nullptr )
		return;
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