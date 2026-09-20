
#include "precompiled.h"
#include "Paths.hpp"

#include <SDL3/SDL_filesystem.h>

#ifdef vsnprintf
#undef vsnprintf
#endif //vsnprintf

// STD 17
#include <filesystem>
namespace fs = std::filesystem;

static idStr s_basepath;
idCVar sys_DefaultBasePath( "sys_defaultbasepath", "", CVAR_SYSTEM | CVAR_ROM, "the local game source base path" );
idCVar sys_DefaultSavePath( "sys_defaultsavepath", "", CVAR_SYSTEM | CVAR_ROM, "the game saves folder" );

/*
================
crPaths::DefaultBasePath
================
*/
const char *crPaths::DefaultBasePath( void )
{
	if ( s_basepath.IsEmpty() )
	{
		// Retrieve base path
        s_basepath = SDL_GetBasePath();
		s_basepath.StripTrailing( '/' );

		/// so we can query on console
		sys_DefaultBasePath.SetString( s_basepath.c_str() );
	}

	return s_basepath.c_str();
}

/*
==============
crPaths::GetDriveFreeSpace
==============
*/
uint64_t crPaths::GetDriveFreeSpace(const char *path)
{
	uint64_t bytes = GetDriveFreeSpaceInBytes(path);
    return static_cast<uint32_t>( bytes / ( 1024 * 1024 ) ); // convert to MB
}

/*
==============
crPaths::GetDriveFreeSpaceInBytes
==============
*/
uint64_t crPaths::GetDriveFreeSpaceInBytes(const char *path)
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
==============
crPaths::DirExist
==============
*/
bool crPaths::DirExist( const char *path )
{
#if 0
	std::error_code ec;
	auto fpath = fs::path( path );
	if ( fs::exists( fpath, ec ) )
		return true; 	
    return false;
#else
	SDL_PathInfo info;
	if( !SDL_GetPathInfo( path, &info ) )
		return false;

	if( info.type != SDL_PATHTYPE_NONE || info.type != SDL_PATHTYPE_OTHER /* simbolic link ?*/ )
		return false; 

	return true;
#endif
}

/*
========================
Sys_IsFileWritable
========================
*/
bool crPaths::IsFileWritable( const char* path )
{
	if (!path || !*path)
		return false;

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
}
