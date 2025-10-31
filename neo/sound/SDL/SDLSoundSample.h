
#ifndef __SOUND_SAMPLE_H__
#define __SOUND_SAMPLE_H__

class idSoundSampleSDL3
{
public:
	idSoundSampleSDL3( void );
	
    // destructor should be public so lists of  soundsamples can be destroyed etc
	~idSoundSampleSDL3( void ); 
	
	// Loads and initializes the resource based on the name.
	virtual void	 LoadResource( void );
	
	void			SetName( const char* n ) { name = n; }
	const char* 	GetName( void ) const { return name; }
	ID_TIME_T		GetTimestamp( void ) const { return timestamp; }
	
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
	struct sampleBuffer_t
	{
		uint32_t 	bufferSize;
		uint32_t	numSamples;
		void*		buffer = nullptr;
	};	

	bool								loaded;
	bool								neverPurge;
	bool								levelLoadReferenced;
	bool								usesMapHeap;
	int									playBegin;
	int									playLength;
	idStr								name;
	uint32_t							lastPlayedTime;
	ID_TIME_T							timestamp;
	size_t								totalBufferSize;	// total size of all the buffers
	idList<sampleBuffer_t, TAG_AUDIO>	buffers;
	idWaveFile::waveFmt_t				format;
	idList<byte, TAG_AMPLITUDE>			amplitude;

	bool			LoadWav( const idStr& name );
	bool			LoadOgg( const idStr& name );
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