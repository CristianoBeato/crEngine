
#include "precompiled.h"
#include "sound/snd_local.h"
#include "SDLSoundSample.h"

#define GPU_CONVERT_CPU_TO_CPU_CACHED_READONLY_ADDRESS( x ) x
constexpr uint32_t SOUND_MAGIC_IDMSA = 0x6D7A7274;
constexpr uint32_t MIN_SAMPLE_RATE = 1000;	// Minimum audio sample rate supported (this value is from XAudio2)
extern idCVar sys_lang;

// BEATO Begin:
#include <vorbis/vorbisfile.h>
#include <vorbis/vorbisenc.h>
// BEATO End

/*
========================
crSDLSampleBuffer::crSDLSampleBuffer
========================
*/
crSDLSampleBuffer::crSDLSampleBuffer( void ) : 
	m_numSamples( 0 ),
	m_size( 0 ),
	m_buffer( nullptr )
{
}

/*
========================
crSDLSampleBuffer::~crSDLSampleBuffer
========================
*/
crSDLSampleBuffer::~crSDLSampleBuffer( void )
{
}

/*
========================
crSDLSampleBuffer::Alloc
========================
*/
void crSDLSampleBuffer::Alloc( const size_t size, const uint32_t samples, const idStr &name )
{
	Free();
	m_size = size;
	m_name = name;
	m_numSamples = samples;
	m_buffer = Mem_Alloc16( m_size, TAG_AUDIO ); 
}

/*
========================
crSDLSampleBuffer::Free
========================
*/
void crSDLSampleBuffer::Free( void )
{
	if ( m_buffer != nullptr )
	{
		Mem_Free16( m_buffer );
		m_buffer = nullptr;
	}

	m_name.Clear();

	m_numSamples = 0;
	m_size = 0;
}

void crSDLSampleBuffer::Copy(void *data, const size_t len)
{
	std::memcpy( m_buffer, data, len );
}

/*
========================
idSoundSampleSDL3::idSoundSampleSDL3
========================
*/
idSoundSampleSDL3::idSoundSampleSDL3( void )
{
	timestamp = FILE_NOT_FOUND_TIMESTAMP;
	loaded = false;
	neverPurge = false;
	levelLoadReferenced = false;
	
	std::memset( &format, 0, sizeof( format ) );
	
	totalBufferSize = 0;
	
	playBegin = 0;
	playLength = 0;
	
	lastPlayedTime = 0;
}

/*
========================
idSoundSampleSDL3::~idSoundSampleSDL3
========================
*/
idSoundSampleSDL3::~idSoundSampleSDL3()
{
	FreeData();
}

/*
========================
idSoundSampleSDL3::WriteGeneratedSample
========================
*/
void idSoundSampleSDL3::WriteGeneratedSample( idFile* fileOut )
{
	fileOut->WriteBig( SOUND_MAGIC_IDMSA );
	fileOut->WriteBig( timestamp );
	fileOut->WriteBig( loaded );
	fileOut->WriteBig( playBegin );
	fileOut->WriteBig( playLength );
	idWaveFile::WriteWaveFormatDirect( format, fileOut );
	fileOut->WriteBig( ( int )amplitude.Num() );
	fileOut->Write( amplitude.Ptr(), amplitude.Num() );
	fileOut->WriteBig( totalBufferSize );
	fileOut->WriteBig( ( int )buffers.Num() );
	for( int i = 0; i < buffers.Num(); i++ )
	{
		fileOut->WriteBig( buffers[ i ].NumSamples() );
		fileOut->WriteBig( buffers[ i ].Size() );
		fileOut->Write( buffers[ i ].Ptr(), buffers[ i ].Size() );
	};
}
/*
========================
idSoundSampleSDL3::WriteAllSamples
========================
*/
void idSoundSampleSDL3::WriteAllSamples( const idStr& sampleName )
{
	idSoundSampleSDL3* samplePC = new idSoundSampleSDL3();
	{
		idStrStatic< MAX_OSPATH > inName = sampleName;
		inName.Append( ".msadpcm" );
		idStrStatic< MAX_OSPATH > inName2 = sampleName;
		inName2.Append( ".wav" );
		
		idStrStatic< MAX_OSPATH > outName = "generated/";
		outName.Append( sampleName );
		outName.Append( ".idwav" );
		
		if( samplePC->LoadWav( inName ) || samplePC->LoadWav( inName2 ) )
		{
			idFile* fileOut = fileSystem->OpenFileWrite( outName, "fs_basepath" );
			samplePC->WriteGeneratedSample( fileOut );
			delete fileOut;
		}
	}
	delete samplePC;
}

/*
========================
idSoundSampleSDL3::LoadGeneratedSound
========================
*/
bool idSoundSampleSDL3::LoadGeneratedSample( const idStr& filename )
{
	idFileLocal fileIn( fileSystem->OpenFileReadMemory( filename ) );
	if( fileIn != nullptr )
	{
		uint32_t magic;
		fileIn->ReadBig( magic );
		fileIn->ReadBig( timestamp );
		fileIn->ReadBig( loaded );
		fileIn->ReadBig( playBegin );
		fileIn->ReadBig( playLength );
		idWaveFile::ReadWaveFormatDirect( format, fileIn );
		int num;
		fileIn->ReadBig( num );
		amplitude.Clear();
		amplitude.SetNum( num );
		fileIn->Read( amplitude.Ptr(), amplitude.Num() );
		fileIn->ReadBig( totalBufferSize );
		fileIn->ReadBig( num );
		buffers.SetNum( num );
		for( int i = 0; i < num; i++ )
		{
			int numSamples = 0;
			int bufferSize = 0;

			fileIn->ReadBig( numSamples );
			fileIn->ReadBig( bufferSize );
			buffers[ i ].Alloc(  bufferSize, numSamples, GetName() );
			fileIn->Read( buffers[ i ].Ptr(), bufferSize );
			// buffers[ i ].buffer = GPU_CONVERT_CPU_TO_CPU_CACHED_READONLY_ADDRESS( buffers[ i ].buffer );
		}
		return true;
	}
	return false;
}
/*
========================
idSoundSampleSDL3::Load
========================
*/
void idSoundSampleSDL3::LoadResource( void )
{
	FreeData();
	
	if( idStr::Icmpn( GetName(), "_default", 8 ) == 0 )
	{
		MakeDefault();
		return;
	}
	
	if( s_noSound.GetBool() )
	{
		MakeDefault();
		return;
	}
	
	loaded = false;
	
	for( int i = 0; i < 2; i++ )
	{
		idStrStatic< MAX_OSPATH > sampleName = GetName();
		if( ( i == 0 ) && !sampleName.Replace( "/vo/", va( "/vo/%s/", sys_lang.GetString() ) ) )
			i++;
		
// BEATO Begin:
		for ( uint32_t j = 0; j < 4; j++)
		{
			switch ( j )
			{
			case 0:
			{
				idStrStatic< MAX_OSPATH > generatedName = "generated/";
				generatedName.Append( sampleName );
				generatedName.SetFileExtension( "idwav" );
				// try load generated sound files
				loaded = LoadGeneratedSample( generatedName ); 
			}	break;
			case 1:
			{
				idStrStatic< MAX_OSPATH > formatName = sampleName;
				formatName.SetFileExtension( "ogg" );
				loaded = LoadOgg( formatName );
			} break;
			case 2:
			{
				if( !s_useCompression.GetBool() )
					continue;

				idStrStatic< MAX_OSPATH > formatName = sampleName;
				sampleName.SetFileExtension( "msadpcm" );
				// load WAV files
				loaded = LoadWav( sampleName );
				}	break;
			case 3:
			{
				idStrStatic< MAX_OSPATH > formatName = sampleName;
				formatName.SetFileExtension( "wav" );
				// load WAV files
				loaded = LoadWav( sampleName );
			} break;			
			};
			
			if( loaded )
				break;
		}		
// BEATO End

		if( loaded )
		{
			if( cvarSystem->GetCVarBool( "fs_buildresources" ) )
			{
				fileSystem->AddSamplePreload( GetName() );
				WriteAllSamples( GetName() );
				
				if( sampleName.Find( "/vo/" ) >= 0 )
				{
					for( int i = 0; i < crPlatform::Get()->NumLangs(); i++ )
					{
						const char* lang = crPlatform::Get()->Language( i );
						if( idStr::Icmp( lang, ID_LANG_ENGLISH ) == 0 )
							continue;
						
						idStrStatic< MAX_OSPATH > locName = GetName();
						locName.Replace( "/vo/", va( "/vo/%s/", crPlatform::Get()->Language( i ) ) );
						WriteAllSamples( locName );
					}
				}
			}
			return;
		}
	}
	
	if( !loaded )
        MakeDefault(); // make it default if everything else fails
}

/*
========================
idSoundSampleSDL3::LoadWav
========================
*/
bool idSoundSampleSDL3::LoadWav( const idStr& filename )
{
	// load the wave
	idWaveFile wave;
	if( !wave.Open( filename ) )
		return false;
	
	idStrStatic< MAX_OSPATH > sampleName = filename;
	sampleName.SetFileExtension( "amp" );
	LoadAmplitude( sampleName );
	
	const char* formatError = wave.ReadWaveFormat( format );
	if( formatError != nullptr )
	{
		idLib::Warning( "LoadWav( %s ) : %s", filename.c_str(), formatError );
		MakeDefault();
		return false;
	}
	timestamp = wave.Timestamp();
	
	totalBufferSize = wave.SeekToChunk( 'data' );
	
	if( format.basic.formatTag == idWaveFile::FORMAT_PCM || format.basic.formatTag == idWaveFile::FORMAT_EXTENSIBLE )
	{
	
		if( format.basic.bitsPerSample != 16 )
		{
			idLib::Warning( "LoadWav( %s ) : %s", filename.c_str(), "Not a 16 bit PCM wav file" );
			MakeDefault();
			return false;
		}
		
		playBegin = 0;
		playLength = ( totalBufferSize ) / format.basic.blockSize;
		
		buffers.SetNum( 1 );
		buffers[0].Alloc( totalBufferSize, playLength, GetName() );
				
		wave.Read( buffers[0].Ptr(), totalBufferSize );
		
		if( format.basic.bitsPerSample == 16 )
		{
			idSwap::LittleArray( ( short* )buffers[0].Ptr(), totalBufferSize / sizeof( short ) );
		}
		
		// buffers[0].buffer = GPU_CONVERT_CPU_TO_CPU_CACHED_READONLY_ADDRESS( buffers[0].buffer );
		
	}
	else if( format.basic.formatTag == idWaveFile::FORMAT_ADPCM )
	{
	
		playBegin = 0;
		playLength = ( ( totalBufferSize / format.basic.blockSize ) * format.extra.adpcm.samplesPerBlock );
		
		buffers.SetNum( 1 );
		buffers[0].Alloc( totalBufferSize, playLength, GetName() );

		wave.Read( buffers[0].Ptr(), totalBufferSize );
		
		// buffers[0].buffer = GPU_CONVERT_CPU_TO_CPU_CACHED_READONLY_ADDRESS( buffers[0].buffer );
		
	}
#if 0 // TODO: fix this ( really needed ?) i really prefer to use OGG on the future projects
	else if( format.basic.formatTag == idWaveFile::FORMAT_XMA2 )
	{
		idList<size_t>		sizes;
		idList<int>			samples;
		idList<void*>		sources;

		if( format.extra.xma2.blockCount == 0 )
		{
			idLib::Warning( "LoadWav( %s ) : %s", filename.c_str(), "No data blocks in file" );
			MakeDefault();
			return false;
		}
		
		int bytesPerBlock = format.extra.xma2.bytesPerBlock;
		assert( format.extra.xma2.blockCount == ALIGN( totalBufferSize, bytesPerBlock ) / bytesPerBlock );
		assert( format.extra.xma2.blockCount * bytesPerBlock >= totalBufferSize );
		assert( format.extra.xma2.blockCount * bytesPerBlock < totalBufferSize + bytesPerBlock );
		
		buffers.SetNum( format.extra.xma2.blockCount );
		
		sizes.SetNum( format.extra.xma2.blockCount );
		samples.SetNum( format.extra.xma2.blockCount );
		sources.SetNum( format.extra.xma2.blockCount );

		for( int i = 0; i < buffers.Num(); i++ )
		{
			size_t bufferSize = 0;
			if( i == buffers.Num() - 1 )
				sizes[i] = totalBufferSize - ( i * bytesPerBlock );
			else
				sizes[i] = bytesPerBlock;
			
			sources[i] = Mem_Alloc16( sizes[i], TAG_AUDIO );
			wave.Read( sources[i], sizes[i] );
		}
		
		int seekTableSize = wave.SeekToChunk( 'seek' );
		if( seekTableSize != 4 * buffers.Num() )
		{
			idLib::Warning( "LoadWav( %s ) : %s", filename.c_str(), "Wrong number of entries in seek table" );
			MakeDefault();
			return false;
		}
		
		for( int i = 0; i < buffers.Num(); i++ )
		{
			wave.Read( &samples[i], sizeof( int ) );
			idSwap::Big( samples[i] );
		}
		
		playBegin = format.extra.xma2.loopBegin;
		playLength = format.extra.xma2.loopLength;
		
		if( buffers[buffers.Num() - 1].numSamples < playBegin + playLength )
		{
			// This shouldn't happen, but it's not fatal if it does
			playLength = buffers[buffers.Num() - 1].numSamples - playBegin;
		}
		else
		{
			// Discard samples beyond playLength
			for( int i = 0; i < buffers.Num(); i++ )
			{
				if( buffers[i].NumSamples() > playBegin + playLength )
				{
					buffers[i].NumSamples() = playBegin + playLength;
					// Ideally, the following loop should always have 0 iterations because playBegin + playLength ends in the last block already
					// But there is no guarantee for that, so to be safe, discard all buffers beyond this one
					for( int j = i + 1; j < buffers.Num(); j++ )
					{
						buffers[j].Free();
					}
					buffers.SetNum( i + 1 );
					break;
				}
			}
		}
		
	}
#endif
	else
	{
		idLib::Warning( "LoadWav( %s ) : Unsupported wave format %d", filename.c_str(), format.basic.formatTag );
		MakeDefault();
		return false;
	}
	
	wave.Close();
	
	if( format.basic.formatTag == idWaveFile::FORMAT_EXTENSIBLE )
	{
		// HACK: XAudio2 doesn't really support FORMAT_EXTENSIBLE so we convert it to a basic format after extracting the channel mask
		format.basic.formatTag = format.extra.extensible.subFormat.data1;
	}
	
	// sanity check...
	assert( buffers[buffers.Num() - 1].NumSamples() == playBegin + playLength );
	
	return true;
}

bool idSoundSampleSDL3::LoadOgg(const idStr &name)
{
	// load the wave
	crOGGFile ogg;
	if( !ogg.Open( name ) )
		return false;

	format.basic = ogg.GetFormat();
	
	// OGG aways load in PCM format 
	playBegin = 0;
	playLength = ogg.TotalSamples();
	totalBufferSize = playLength * format.basic.blockSize;
	
	buffers.SetNum( 1 );
	buffers[0].Alloc( totalBufferSize, playLength, GetName() );
			
	// read ogg content
	ogg.Read( buffers[0].Ptr(), totalBufferSize );
		
	if( format.basic.bitsPerSample == 16 )
		idSwap::LittleArray( ( short* )buffers[0].Ptr(), totalBufferSize / sizeof( short ) );
	
	ogg.Close();

	// sanity check...
	assert( buffers[buffers.Num() - 1].NumSamples() == playBegin + playLength );
	
    return true;
}

/*
========================
idSoundSampleSDL3::MakeDefault
========================
*/
void idSoundSampleSDL3::MakeDefault( void )
{
	FreeData();
	
	static const int DEFAULT_NUM_SAMPLES = 256;
	
	timestamp = FILE_NOT_FOUND_TIMESTAMP;
	loaded = true;
	
	std::memset( &format, 0, sizeof( format ) );
	format.basic.formatTag = idWaveFile::FORMAT_PCM;
	format.basic.numChannels = 1;
	format.basic.bitsPerSample = 16;
	format.basic.samplesPerSec = MIN_SAMPLE_RATE;
	format.basic.blockSize = format.basic.numChannels * format.basic.bitsPerSample / 8;
	format.basic.avgBytesPerSec = format.basic.samplesPerSec * format.basic.blockSize;
	
	assert( format.basic.blockSize == 2 );
	
	totalBufferSize = DEFAULT_NUM_SAMPLES * 2;
	
	buffers.SetNum( 1 );
	buffers[0].Alloc( totalBufferSize, DEFAULT_NUM_SAMPLES, GetName() );
	short* defaultBuffer = ( short* )buffers[0].Ptr();
	for( int i = 0; i < DEFAULT_NUM_SAMPLES; i += 2 )
	{
		defaultBuffer[i + 0] = SHRT_MIN;
		defaultBuffer[i + 1] = SHRT_MAX;
	}
		
	playBegin = 0;
	playLength = DEFAULT_NUM_SAMPLES;
}

/*
========================
idSoundSampleSDL3::FreeData

Called before deleting the object and at the start of LoadResource()
========================
*/
void idSoundSampleSDL3::FreeData( void )
{
	if( buffers.Num() > 0 )
	{
		static_cast<idSoundSystemLocal*>(idSoundSystem::Get())->StopVoicesWithSample( static_cast<idSoundSample*>( this ) );
		for( int i = 0; i < buffers.Num(); i++ )
		{
			buffers[i].Free();
		}
		buffers.Clear();
	}
	amplitude.Clear();
	
	timestamp = FILE_NOT_FOUND_TIMESTAMP;
	std::memset( &format, 0, sizeof( format ) );
	loaded = false;
	totalBufferSize = 0;
	playBegin = 0;
	playLength = 0;
}

/*
========================
idSoundSampleSDL3::LoadAmplitude
========================
*/
bool idSoundSampleSDL3::LoadAmplitude( const idStr& name )
{
	amplitude.Clear();
	idFileLocal f( fileSystem->OpenFileRead( name ) );
	if( f == nullptr )
		return false;

	amplitude.SetNum( f->Length() );
	f->Read( amplitude.Ptr(), amplitude.Num() );
	return true;
}

/*
========================
idSoundSampleSDL3::GetAmplitude
========================
*/
float idSoundSampleSDL3::GetAmplitude( int timeMS ) const
{
	if( timeMS < 0 || timeMS > LengthInMsec() )
		return 0.0f;
	
	if( IsDefault() )
		return 1.0f;
	
	int index = timeMS * 60 / 1000;
	if( index < 0 || index >= amplitude.Num() )
		return 0.0f;
	
	return ( float )amplitude[index] / 255.0f;
}
