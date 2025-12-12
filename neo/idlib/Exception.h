
#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

/*
================================================
idException
================================================
*/
class idException
{
public:
	static const int MAX_ERROR_LEN = 2048;
	
	idException( const char* except, ... )
	{
		va_list		argptr;
		va_start( argptr, except );
		SDL_vsnprintf( error, MAX_ERROR_LEN, except, argptr );
		va_end( argptr );
	}
	
	// this really, really should be a const function, but it's referenced too many places to change right now
	const char* 	GetError( void ) { return error; }
	
protected:
	// if GetError() were correctly const this would be named GetError(), too
	char* 		GetErrorBuffer( void ) { return error; }

	int			GetErrorBufferSize( void ) { return MAX_ERROR_LEN; }
	
private:
	friend class idFatalException;
	static char error[MAX_ERROR_LEN];
};

/*
================================================
idFatalException
================================================
*/
class idFatalException
{
public:
	static const int MAX_ERROR_LEN = 2048;
	
	idFatalException( const char* text = "" )
	{
		strncpy( idException::error, text, MAX_ERROR_LEN );
	}
	
	// this really, really should be a const function, but it's referenced too many places to change right now
	const char* 	GetError()
	{
		return idException::error;
	}
	
protected:
	// if GetError() were correctly const this would be named GetError(), too
	char* 		GetErrorBuffer( void )
	{
		return idException::error;
	}

	int			GetErrorBufferSize( void )
	{
		return MAX_ERROR_LEN;
	}
};

/*
================================================
idNetworkLoadException
================================================
*/
class idNetworkLoadException : public idException
{
public:
	idNetworkLoadException( const char* text = "" ) : idException( text ) { }
};

#endif //!__EXCEPTION_H__