#include "precompiled.h"
#include "sys/sys_public.h"

// DG: SDL.h somehow needs the following functions, so #undef those silly
//     "don't use" #defines from Str.h
#undef strncmp
#undef strcasecmp
#undef vsnprintf
// DG end


#include <sys/stat.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_filesystem.h>
#include <fstream>

// STD 17
#include <filesystem>
namespace fs = std::filesystem;

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
#else //!__PLATFORM_WINDOWS__
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
#endif //!__PLATFORM_WINDOWS__

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
        basepath = SDL_GetBasePath();
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
	if (!path || !*path)
		return false;

#if 1
	std::error_code ec;
	auto fpath = fs::path( path );
	
	if ( fs::exists( fpath, ec ) )
	{
		fs::file_status status = fs::status( fpath, ec );
        if ( ec ) 
			return false;

		// Checks if the owner's write bit (owner_write) is active.
		return ( status.permissions() & fs::perms::owner_write ) != fs::perms::none;
	}

	// The file does NOT exist. We need to check the parent folder.
	fs::path parent = fpath.parent_path();

	// If the path doesn't have an explicit parent (e.g., "my_file.txt"), 
	// it uses the current directory.
	if ( parent.empty() )
    {
        parent = fs::current_path( ec );
        if ( ec ) 
			return false;
    }

	// Check if the parent folder exists and if we have write permission to it.
    if ( fs::exists( parent, ec ) )
    {
        fs::file_status parent_status = fs::status( parent, ec );
        if ( ec )
			return false;

        // If we can write to the folder, we can create the file inside it.
        return ( parent_status.permissions() & fs::perms::owner_write ) != fs::perms::none;
    }

    return false;

#else
#	if __PLATFORM_WINDOWS__
	return _access( path, 2 ) == 0;
#	else
	return access( path, W_OK ) == 0;
#	endif 
#endif
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
