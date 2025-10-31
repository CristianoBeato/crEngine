#include "precompiled.h"
#include "sys/sys_public.h"

// DG: SDL.h somehow needs the following functions, so #undef those silly
//     "don't use" #defines from Str.h
#undef strncmp
#undef strcasecmp
#undef vsnprintf
// DG end

// STD 17
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sys/stat.h>

namespace fs = std::filesystem;

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_storage.h>
#include <SDL3/SDL_iostream.h>

constexpr char* DEFALT_STRING = "detect";

static idCVar sys_defaultbasepath( "sys_defaultbasepath", DEFALT_STRING, CVAR_SYSTEM | CVAR_ROM, "the local game source base path" );
static idCVar sys_defaultsavepath( "sys_defaultsavepath", DEFALT_STRING, CVAR_SYSTEM | CVAR_ROM, "the game saves folder" );

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
	if ( strncmp( DEFALT_STRING, sys_defaultsavepath.GetString(), strlen( DEFALT_STRING ) ) == 0 )
    {
#if __PLATFORM_LINUX__
		char path[1024];
    	SDL_snprintf( path, 1024, "%s/.%s", getenv( "HOME" ), GAME_NAME );
		sys_defaultsavepath.SetString( path );
#else
		char* path = SDL_GetPrefPath( "crEngine", GAME_NAME );
		sys_defaultsavepath.SetString( path );
		SDL_free( path );
#endif
    }
	
	return sys_defaultsavepath.GetString();
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
	static idStr basepath;
    if ( basepath.IsEmpty() )
	{
#if 0
        basepath = SDL_GetBasePath();
#else
		fs::path cwd = fs::current_path();
		basepath = cwd.c_str(); 
#endif
		sys_defaultbasepath.SetString( basepath.c_str() );
	}

	return basepath.c_str();
}

// ---------------------------------------------------------------------------

#include <SDL3/SDL.h>

/*
================
Sys_GetDriveFreeSpace
returns in megabytes
================
*/
uint32_t Sys_GetDriveFreeSpace( const char* path )
{
	uint64_t bytes = Sys_GetDriveFreeSpaceInBytes(path);
    return static_cast<uint32_t>( bytes / ( 1024 * 1024 ) ); // convert to MB
}

/*
========================
Sys_GetDriveFreeSpaceInBytes
========================
*/
uint64_t Sys_GetDriveFreeSpaceInBytes( const char* path )
{
	try 
	{
        fs::space_info info = fs::space(path);
        return static_cast<uint64_t>(info.available);
    } 
	catch (const fs::filesystem_error& e) 
	{
		common->Error( "Sys_Rmdir: erro '%s' ao remover '%s'\n", e.what(), path );
		return 0;
	}
}

/*
================
Sys_Mkdir
================
*/
void Sys_Mkdir( const char* path )
{
	if (!path || !*path)
		return;

#if 0
	if ( !SDL_CreateDirectory( path ) )
		Sys_Printf( "ERROR: mkdir (%s) failed %s\n", path, SDL_GetError() );
#else
	// check if already exists 
	if ( fs::exists( fs::path( path ) ) )
		return; 

	// try create
	try 
	{
		fs::create_directories( fs::path( path ) );
	}
	catch (const fs::filesystem_error& e) 
	{
		common->Warning("Sys_Mkdir: error '%s' wen creating '%s'\n", e.what(), path);	
	}
#endif
}



/*
================
Sys_Rmdir
================
*/
bool Sys_Rmdir( const char* path )
{
	if (!path || !*path)
		return false;

#if 0
	if ( !SDL_RemovePath( path ) )
	{
		Sys_Printf( "ERROR: rmdir (%s) failed %s\n", path, SDL_GetError() );
		return false;
	}

#else		
	try 
	{
		fs::path dir(path);

		// remove apenas se for diretório existente
		if ( fs::exists(dir) && fs::is_directory(dir) )
			return fs::remove(dir);
	} 
	catch (const fs::filesystem_error& e) 
	{
		common->DPrintf( "Sys_Rmdir: erro '%s' ao remover '%s'\n", e.what(), path );
	}
#endif
		
	return false;
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

/*
================
Sys_ListFiles
================
*/
int Sys_ListFiles( const char* directory, const char* extension, idStrList& list )
{
	if (!directory || !*directory)
        return 0;

#if 0
	int count = 0;
	char** pathlist = SDL_GlobDirectory( directory, extension, SDL_GLOB_CASEINSENSITIVE , &count );
	for ( uint32_t i = 0; i < count; i++)
	{
		idStr path;
		if ( pathlist[i] == nullptr )
			continue;
		
		path = pathlist[i];


		list.Append( path );
	}
	
	SDL_free( pathlist );
#else
	try
	{
		// check for a valid path
		fs::path dirPath(directory);
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath))
            return 0;

		// get the extensions
        std::string extFilter;
        if (extension && *extension) 
		{
            extFilter = extension;
            if (extFilter[0] != '.')
                extFilter = "." + extFilter;
        }

        list.Clear(); // idStrList tem Clear()

        for (const auto& entry : fs::directory_iterator(dirPath)) 
		{
			const fs::path& filePath = entry.path();
          
			// we are listing subpaths
			if ( entry.is_directory() && std::strncmp( extension, PATHSEPARATOR_STR, strlen( extension ) ) )
			{
            	list.Append(filePath.filename().string().c_str());
			}
			else if( entry.is_regular_file() && !extFilter.empty() )
			{
				std::string ext = filePath.extension(); 
            	if ( ext != extFilter)
            	    continue;

				list.Append(filePath.filename().string().c_str());
			}	
        }
	}
	catch(const std::exception& e)
	{
		 common->DPrintf("Sys_ListFiles: fail '%s' to acess '%s'\n", e.what(), directory);
        return 0;
	}
	
	return list.Num(); // idStrList.Num() retorna count
#endif
}

// ---------------------------------------------------------------------------

// only relevant when specified on command line
const char* Sys_DefaultCDPath()
{
	return "";
}


ID_TIME_T Sys_FileTimeStamp( idFileHandle fp )
{
#if __PLATFORM_WINDOWS__
	struct _stat st;
	_fstat( fileno( fp ), &st );
	return st.st_mtime;
#else
	struct stat st;
	fstat( fileno( fp ), &st );
	return st.st_mtime;
#endif
}

#if 0
ID_TIME_T Sys_FileTimeStamp(const char* path) 
{
    try 
	{
        auto ftime = last_write_time(path);
        // converte de filesystem::file_time_type para time_t
        auto sctp = decltype(ftime)::clock::to_sys(ftime);
        return std::chrono::system_clock::to_time_t(sctp);
    } 
	catch (const std::filesystem::filesystem_error&) 
	{
        return 0; // falha ou arquivo inexistente
    }
}
#endif
