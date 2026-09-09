
#include "precompiled.h"
#include "Linux_Path.hpp"
#include "../Platform.hpp"

#include <unistd.h>

static idStr s_savepath;
/*
================
crLinuxPaths::EXEPath
================
*/
const char *crLinuxPaths::EXEPath( void )
{
    static char	buf[ 1024 ];
    idStr		linkpath;
	int			len;
	buf[ 0 ] = '\0';
	sprintf( linkpath, "/proc/%d/exe", getpid() );
	len = readlink( linkpath.c_str(), buf, sizeof( buf ) );
	if( len == -1 )
	{
		crConsole::Get()->Printf( "couldn't stat exe path link %s\n", linkpath.c_str() );
		// RB: fixed array subscript is below array bounds
		buf[ 0 ] = '\0';
		// RB end
	}


    return buf;
}

/*
================
crLinuxPaths::DefaultSavePath
================
*/
const char *crLinuxPaths::DefaultSavePath( void )
{
	if( s_savepath.IsEmpty() )
    {
		char path[1024];
    	SDL_snprintf( path, 1024, "%s/.%s", getenv( "HOME" ), GAME_NAME );
		s_savepath = path;
        sys_DefaultSavePath.SetString( path );
    }
	
	return s_savepath.c_str();
}

/*
================
crLinuxPaths::IsFileWritable
================
*/
bool crLinuxPaths::IsFileWritable( const char *path )
{
    return access( path, W_OK ) == 0;
}
