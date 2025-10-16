#include "precompiled.h"
#include "sys/sys_public.h"

// STD 17
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_storage.h>
#include <SDL3/SDL_iostream.h>

static idStr	basepath;
static idStr	savepath;

/*
==============
Sys_EXEPath
==============
*/
const char* Sys_EXEPath( void )
{
    static char	buf[ 1024 ];
#if __PLATFORM_WINDOWS__
	GetModuleFileName( NULL, buf, sizeof( buf ) - 1 );
#else
	idStr		linkpath;
	int			len;
	buf[ 0 ] = '\0';
	sprintf( linkpath, "/proc/%d/exe", getpid() );
	len = readlink( linkpath.c_str(), buf, sizeof( buf ) );
	if( len == -1 )
	{
		Sys_Printf( "couldn't stat exe path link %s\n", linkpath.c_str() );
		// RB: fixed array subscript is below array bounds
		buf[ 0 ] = '\0';
		// RB end
	}
#endif
	return buf;
}

/*
 ==============
 Sys_DefaultSavePath
 ==============
 */
const char* Sys_DefaultSavePath( void )
{
    if ( !savepath.IsEmpty() )
    {
#if __PLATFORM_LINUX__
    	sprintf( savepath, "%s/.%s", getenv( "HOME" ), GAME_NAME );
#else
    savepath = SDL_GetPrefPath( "crEngine", GAME_NAME);
#endif
    }
	
	return savepath.c_str();
}

/*
================
Sys_DefaultBasePath

Get the default base path
- binary image path
- current directory
- hardcoded
Try to be intelligent: if there is no BASE_GAMEDIR, try the next path
================
*/
const char* Sys_DefaultBasePath( void )
{
    if ( basepath.IsEmpty() )
        basepath = SDL_GetBasePath();

	return basepath.c_str();
}


/*
================
Sys_Mkdir
================
*/
void Sys_Mkdir( const char* path )
{
	if ( !SDL_CreateDirectory( path ) )
		Sys_Printf( "ERROR: mkdir (%s) failed %s\n", path, SDL_GetError() );
}

/*
================
Sys_Rmdir
================
*/
bool Sys_Rmdir( const char* path )
{
	if ( !SDL_RemovePath( path ) )
	{
		Sys_Printf( "ERROR: rmdir (%s) failed %s\n", path, SDL_GetError() );
		return false;
	}	
		
	return true;
}

/*
========================
Sys_IsFileWritable
========================
*/
bool Sys_IsFileWritable( const char* path )
{
    try 
	{
        if (!path || !*path)
            return false;

        fs::path p(path);

        // If the file already exists, try opening it as an attachment (non-destructive)
        if (fs ::exists( p ) ) 
		{
            std::ofstream f(p, std::ios::app);
            return f.is_open();
        }

		// If it doesn't exist, check if the parent directory is writable
		fs::path dir = p.parent_path();
        if (dir.empty())
            dir = fs::current_path();

        if ( !fs::exists( dir ) )
            return false;
#if 0
        // tries to create and remove a temporary file
		fs::path testFile = dir / ".write_test.tmp";
        std::ofstream test(testFile);
        if (!test.is_open())
            return false;

        test.close();
        fs::remove(testFile);
#endif	
        return true;
    } 
	catch (const fs::filesystem_error&) 
	{
        return false;
    }
}

/*
========================
Sys_IsFolder
========================
*/
sysFolder_t Sys_IsFolder( const char* path )
{
	if (!path || !*path)
		return FOLDER_ERROR;

	try 
	{
		fs::path p(path);
		if (fs::exists(p)) 
		{
			if (fs::is_directory(p))
				return FOLDER_YES;
			else
				return FOLDER_NO;
		}
		return FOLDER_NO;
	} catch (const fs::filesystem_error& e) 
	{
		// Em idTech4x, você poderia logar isso com common->DPrintf
		common->DPrintf("Sys_IsFolder: erro '%s' '%s'\n", e.what(), path);
		return FOLDER_ERROR;
	}
}