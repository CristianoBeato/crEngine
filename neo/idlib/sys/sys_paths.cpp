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

/// TODO: Future, use SDL3 storage 

namespace fs = std::filesystem;
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_filesystem.h>

constexpr char DEFALT_STRING[7] = { "detect" };
static idStr basepath;

static idCVar sys_defaultbasepath( "sys_defaultbasepath", DEFALT_STRING, CVAR_SYSTEM | CVAR_ROM, "the local game source base path" );
static idCVar sys_defaultsavepath( "sys_defaultsavepath", DEFALT_STRING, CVAR_SYSTEM | CVAR_ROM, "the game saves folder" );

/*
==============
Sys_DirExist
==============
*/
bool Sys_DirExist( const char *path )
{
	std::error_code ec;
	auto fpath = fs::path( path );
	if ( fs::exists( fpath, ec ) )
		return true; 	
    return false;
}

/*
================
Sys_Mkdir
================
*/
void Sys_Mkdir( const char* path )
{
	std::error_code ec;
	
	if (!path || !*path)
		return;
	
	auto fpath = fs::path( path );
	
	// check if already exists 
	if ( !fs::exists( fpath, ec ) )
		idLib::Warning("Sys_Mkdir: error '%s' wen creating '%s'\n", ec.message().c_str(), path );

	if( !fs::create_directories( fpath, ec ) )
		idLib::Warning("Sys_Mkdir: error '%s' wen creating '%s'\n", ec.message().c_str(), path );
}

/*
==============
Sys_EXEPath
==============
*/
const char* Sys_EXEPath( void )
{
    static char	buf[ 1024 ];
#if __PLATFORM_WINDOWS__
	GetModuleFileName( nullptr, buf, sizeof( buf ) - 1 );
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
    if ( basepath.IsEmpty() )
	{
#if 1
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
	std::error_code ec;
	auto fpath = fs::path( path );
	if ( fs::exists( fpath, ec ) )
		return 0;

	fs::space_info info = fs::space(fpath, ec );
	if( ec )
	{
		idLib::Error( "Sys_GetDriveFreeSpaceInBytes: erro '%s' ao remover '%s'\n", ec.message().c_str(), path );
		return 0;
	}

	return static_cast<uint64_t>( info.available );
}



/*
================
Sys_Rmdir
================
*/
bool Sys_Rmdir( const char* path )
{
	std::error_code ec;
	
	if (!path || !*path)
		return false;

	auto fpath = fs::path( path );

	// check if already exists 
	if ( !fs::exists( fpath, ec ) )
	{
		idLib::Warning("Sys_Rmdir: error '%s' wen removing '%s'\n", ec.message().c_str(), path );
		return false;
	}

	if( !fs::is_directory( fpath, ec ) )
	{
		idLib::Warning("Sys_Rmdir: error is not a dir wen removing '%s'\n", ec.message().c_str(), path );
		return false;
	}

	if( !fs::remove( fpath, ec ) )
	{
		idLib::Warning( "Sys_Rmdir: error '%s' removing '%s'\n", ec.message().c_str(), path );
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
	std::error_code ec;

	if (!path || !*path)
		return false;

	auto fpath = fs::path( path );
	
	// check if exists 
	if ( fs::exists( fpath, ec ) )
	{
		// If the file already exists, try opening it as an attachment (non-destructive)
		std::ofstream file( fpath, std::ios::app );
        return file.is_open();
	}
	else
	{
		// If it doesn't exist, check if the parent directory is writable
		fs::path parent = fpath.parent_path();
        if ( parent.empty())
            parent = fs::current_path();

		// TODO:

		return false;
	}

	return false;
}

/*
========================
Sys_IsFolder
========================
*/
sysFolder_t Sys_IsFolder( const char* path )
{
	std::error_code ec;

	if (!path || !*path)
		return FOLDER_ERROR;
		
	auto fpath = fs::path( path );

	if ( fs::exists( fpath, ec ) ) 
	{
		if ( fs::is_directory( fpath, ec ) )
			return FOLDER_YES;
	}
	
	return FOLDER_NO;
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
				std::string ext = filePath.extension().string(); 
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
}

// ---------------------------------------------------------------------------

ID_TIME_T Sys_FileTimeStamp( idFileHandle fp )
{
#if __PLATFORM_WINDOWS__
	struct _stat st;
	_fstat( _fileno( fp ), &st );
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
