
#include "Windows_Path.hpp"
#include "../Platform.hpp"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>

static idStr s_basepath;
static idStr s_savepath;


crPaths* crPaths::Get( void )
{
    static crWindowsPaths gWindowsPaths = crWindowsPaths();
    return &gWindowsPaths;
}

bool crWindowsPaths::IsFileWritable( const char *path )
{
	return _access( path, 2 ) == 0;
}

/*
==============
Sys_DefaultSavePath
==============
*/
const char *crWindowsPaths::DefaultBasePath( void )
{
    if ( s_basepath.IsEmpty() )
	{
		// Retrieve base path
        s_basepath = SDL_GetBasePath();

		// remove last slash
		s_basepath.StripTrailing( '\\' );

		// in windows make surre that we are using black slashes;
		s_basepath.BackSlashesToSlashes(); 	

		/// so we can query on console
		sys_DefaultBasePath.SetString( s_basepath.c_str() );
	}

	return s_basepath.c_str();
}

/*
==============
crWindowsPaths::DefaultSavePath
==============
*/
const char *crWindowsPaths::DefaultSavePath( void )
{
	if( s_savepath.IsEmpty() )
    {
		auto path = SDL_GetPrefPath( "crEngine", GAME_NAME );
		s_savepath = path;
		SDL_free( path );

		// remove last slash
		s_savepath.StripTrailing( '\\' );

		// in windows make surre that we are using black slashes;
		s_savepath.BackSlashesToSlashes();

		sys_DefaultSavePath.SetString( s_savepath.c_str() );
    }
	
	return s_savepath.c_str();
}

/*
==============
crWindowsPaths::EXEPath
==============
*/
const char* crWindowsPaths::EXEPath( void )
{
    static char	buf[ 1024 ];
	GetModuleFileName( nullptr, buf, sizeof( buf ) - 1 );
    return buf;
}
