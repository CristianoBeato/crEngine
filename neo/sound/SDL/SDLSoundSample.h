
#ifndef __SOUND_SAMPLE_H__
#define __SOUND_SAMPLE_H__

class crSDLSampleBuffer
{
public:
	crSDLSampleBuffer( void );
	~crSDLSampleBuffer( void );

	void Alloc( const size_t size, const uint32_t samples, const idStr &name );
	void Free( void );
	void Copy( void* data, const size_t len );

	uint32_t	NumChannels( void ) const { return m_numChannels; }
	uint32_t	NumSamples( void ) const { return m_numSamples; }
	size_t		Size( void ) const { return m_size; };
	void*		Ptr( void ) const { return m_buffer; }

private:
	idStr		m_name;
	uint32_t	m_numSamples;
	uint32_t	m_numChannels;
	size_t		m_size;
	void*		m_buffer;
};

class idSoundSampleSDL3
{
public:

	idSoundSampleSDL3( void );
	
    // destructor should be public so lists of  soundsamples can be destroyed etc
	~idSoundSampleSDL3( void ); 
	
	// Loads and initializes the resource based on the name.
	virtual void	 		LoadResource( void );
	
	void					SetName( const char* n ) { name = n; }
	const char* 			GetName( void ) const { return name; }
	ID_TIME_T				GetTimestamp( void ) const { return timestamp; }

// BEATO Begin:
	const uint32_t								NumBuffers( void ) const { return buffers.Num(); }
	const idList<crSDLSampleBuffer, TAG_AUDIO>	Buffers( void ) const {return buffers; }
	const int									PlayBegin( void ) const { return playBegin; }
// BEATO End

	// turns it into a beep
	void			MakeDefault( void );
	
	// frees all data
	void			FreeData( void );
	
	int				LengthInMsec( void ) const { return SamplesToMsec( NumSamples(), SampleRate() ); }

	int				SampleRate( void ) const { return format.basic.samplesPerSec; }

	int				NumSamples( void ) const { return playLength; }

	int				NumChannels( void ) const { return format.basic.numChannels; }

	int				BufferSize( void ) const { return totalBufferSize; }
	
	bool			IsCompressed( void ) const { return ( format.basic.formatTag != idWaveFile::FORMAT_PCM ); }
	
	bool			IsDefault( void ) const { return timestamp == FILE_NOT_FOUND_TIMESTAMP; }
	bool			IsLoaded( void ) const { return loaded; }
	
	void			SetNeverPurge( void ) { neverPurge = true; }

	bool			GetNeverPurge( void ) const { return neverPurge; }
	
	void			SetLevelLoadReferenced( void ) { levelLoadReferenced = true; }
	void			ResetLevelLoadReferenced( void ) { levelLoadReferenced = false; }
	bool			GetLevelLoadReferenced( void ) const { return levelLoadReferenced; }
	
	int				GetLastPlayedTime( void ) const { return lastPlayedTime; }

	void			SetLastPlayedTime( int t ) { lastPlayedTime = t; }
	
	float			GetAmplitude( int timeMS ) const;
	
protected:
	bool									loaded;
	bool									neverPurge;
	bool									levelLoadReferenced;
	bool									usesMapHeap;
	int										playBegin;
	int										playLength;
	idStr									name;
	uint32_t								lastPlayedTime;
	ID_TIME_T								timestamp;
	size_t									totalBufferSize;	// total size of all the buffers
	idList<crSDLSampleBuffer, TAG_AUDIO>	buffers;
	idWaveFile::waveFmt_t					format;
	idList<byte, TAG_AMPLITUDE>				amplitude;

// BEATO Begin:
	bool			LoadOgg( const idStr& name );
// BEATO End
	bool			LoadWav( const idStr& name );
	bool			LoadAmplitude( const idStr& name );
	void			WriteAllSamples( const idStr& sampleName );
	bool			LoadGeneratedSample( const idStr& name );
	void			WriteGeneratedSample( idFile* fileOut );
	
	
};

/*
================================================
idSoundSample

This reverse-inheritance purportedly makes working on
multiple platforms easier.
================================================
*/
class idSoundSample : public idSoundSampleSDL3
{
public:
};


#endif //!__SOUND_SAMPLE_H__