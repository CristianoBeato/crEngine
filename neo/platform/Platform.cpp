
#include "precompiled.h"
#include "Platform.hpp"

// STD 17
#include <filesystem>
namespace fs = std::filesystem;

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_messagebox.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_loadso.h>
#include <SDL3/SDL_cpuinfo.h>
#include <SDL3/SDL_filesystem.h>

static idStr s_basepath;

const char* k_sysLanguageNames[] =
{
	ID_LANG_ENGLISH, ID_LANG_FRENCH, ID_LANG_ITALIAN, ID_LANG_GERMAN, ID_LANG_SPANISH, ID_LANG_JAPANESE, nullptr
};

constexpr uint32_t k_numLanguages = sizeof( k_sysLanguageNames ) / sizeof k_sysLanguageNames[ 0 ] - 1;

static idCVar sys_lang( "sys_lang", ID_LANG_ENGLISH, CVAR_SYSTEM | CVAR_INIT, "", k_sysLanguageNames, idCmdSystem::ArgCompletion_String<k_sysLanguageNames> );
idCVar crPaths::sys_DefaultBasePath( "sys_defaultbasepath", DEFALT_STRING, CVAR_SYSTEM | CVAR_ROM, "the local game source base path" );
idCVar crPaths::sys_DefaultSavePath( "sys_defaultsavepath", DEFALT_STRING, CVAR_SYSTEM | CVAR_ROM, "the game saves folder" );


void crConsole::Printf( const char *fmt, ... )
{
	va_list argptr;
	va_start( argptr, fmt );
	VPrintf( fmt, argptr );
	va_end( argptr );
}

void crConsole::Debug( const char *fmt, ... )
{
	va_list argptr;
	va_start( argptr, fmt );
	VDebug( fmt, argptr );
	va_end( argptr );
}

void crConsole::Error( const char *fmt, ... )
{
	va_list argptr;
	va_start( argptr, fmt );
	VError( fmt, argptr );
	va_end( argptr );
	crPlatform::Get()->Exit( EXIT_FAILURE );
}

void crPlatform::SetFatalError(const char *error)
{
	///
	SDL_Log( error );

	/// present a message box
	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Error!", error, nullptr );

	/// close aplication
	crPlatform::Get()->Exit( EXIT_FAILURE );
}

/*
=====================
crPlatform::NumLangs
=====================
*/
const uint32_t crPlatform::NumLangs( void ) const
{
	return k_numLanguages;
}

/*
=====================
crPlatform::Language
=====================
*/
const char* crPlatform::Language( const uint32_t in_idx ) const
{
	if( in_idx >= 0 && in_idx < k_numLanguages )
		return k_sysLanguageNames[ in_idx ];

	return "";
}

/*
================
crPlatform::SetLanguageFromSystem
================
*/
void crPlatform::SetLanguageFromSystem( void ) 
{
	sys_lang.SetString( DefaultLanguage() );
}

/*
================
crPlatform::SetLanguageFromSystem
================
*/
const char* crPlatform::DefaultLanguage( void ) const
{
	// sku breakdowns are as follows
	//  EFIGS	Digital
	//  EF  S	North America
	//   FIGS	EU
	//  E		UK
	// JE    	Japan
	
	// If japanese exists, default to japanese
	// else if english exists, defaults to english
	// otherwise, french
	if( !fileSystem->UsingResourceFiles() ) // TODO: Fix this
		return ID_LANG_ENGLISH;
	
	return ID_LANG_ENGLISH;
}

/*
=====================
crPlatform::SharedLibLoad
=====================
*/
void *crPlatform::SharedLibLoad( const char *in_libraryName ) const
{
	auto lib = SDL_LoadObject( in_libraryName );
	if ( !lib )
		throw idException( SDL_GetError() );
	
	return static_cast<void*>( lib );
}

/*
=====================
crPlatform::SharedLibProcAddress
=====================
*/
void *crPlatform::SharedLibProcAddress( void* in_handle, const char *in_procName ) const
{
    if( in_procName == nullptr || in_procName[0] == '/0' )
        return nullptr;

	// RB: added missing cast
	return reinterpret_cast<void*>( SDL_LoadFunction( static_cast<SDL_SharedObject*>( in_handle ), in_procName ) );
}

/*
=====================
crPlatform::SharedLibUnload
=====================
*/
void crPlatform::SharedLibUnload( void* in_handle ) const
{
	if( in_handle == nullptr )
		return;

	SDL_UnloadObject( static_cast< SDL_SharedObject*>( in_handle ) );
}

/*
==============
crPlatform::GetClipboardData
==============
*/
const char* crPlatform::GetClipboardData( void ) const
{
	size_t strl = 0;
	char* clpbrd = nullptr;
	char* copy = nullptr;
	if ( !SDL_HasClipboardText() )
		return nullptr;

	clpbrd = SDL_GetClipboardText();
	if (  clpbrd == nullptr )
		crConsole::Get()->Printf( "failed to get clipboard content %s\n", SDL_GetError() );
	
	copy = static_cast<char*>( Mem_Alloc( SDL_strlen( clpbrd ) + 1, TAG_CRAP ) );

	SDL_free( clpbrd );

	return copy;
}

/*
==============
crPlatform::SetClipboardData
==============
*/
void crPlatform::SetClipboardData( const char* in_string ) const
{
	if ( in_string == nullptr || in_string[0] == '/0' )
		return;

	SDL_ClearClipboardData();

	if( !SDL_SetClipboardText( in_string ) )
		crConsole::Get()->Printf( "failed to set clipboard content %s\n",  SDL_GetError() );
}

/*
==============
crPlatform::Sleep
==============
*/
const bool crPlatform::Sleep(const uint32_t in_milliseconds) const
{
    SDL_Delay( in_milliseconds );
    return true;
}

/*
==============
crPlatform::Milliseconds
==============
*/
const uint32_t crPlatform::Milliseconds( void ) const
{
    return SDL_GetTicks();
}

/*
========================
crPlatform::Microseconds
========================
*/
const uint64_t crPlatform::Microseconds( void ) const
{
	static uint64_t baseCounter = 0;
	static uint64_t frequency = 0;

	// init the timer 
	if ( frequency == 0)
	{
		frequency = SDL_GetPerformanceFrequency();
    	baseCounter = SDL_GetPerformanceCounter();
	}
	
	return ( ( SDL_GetPerformanceCounter() - baseCounter ) * 1000000ULL) / frequency;
}

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

	if( info.type != SDL_PATHTYPE_NONE )
		return false; 

	return true;
#endif
}

/*
==============
crPaths::Mkdir
==============
*/
void crPaths::Mkdir( const char* path )
{
	if (!path || !*path)
		return;

#if 0
	std::error_code ec;
	auto fpath = fs::path( path );
	
	// check if already exists 
	if ( !fs::exists( fpath, ec ) )
		idLib::Warning("Sys_Mkdir: error '%s' wen creating '%s'\n", ec.message().c_str(), path );

	if( !fs::create_directories( fpath, ec ) )
		idLib::Warning("Sys_Mkdir: error '%s' wen creating '%s'\n", ec.message().c_str(), path );
#else
	SDL_PathInfo info;
	if( SDL_GetPathInfo( path, &info ) )
	{
		if ( info.type == SDL_PATHTYPE_FILE )
		{
			idLib::Warning( "Failed create %s path, Already exists!", path );
			return;
		}
		else if( info.type == SDL_PATHTYPE_DIRECTORY )
		{
			idLib::Warning( "Already exists!" );
			return;
		}	
	}

	if( !SDL_CreateDirectory( path ) )
		idLib::Error( SDL_GetError() );
#endif
}

/*
==============
crPaths::Rmdir
==============
*/
bool crPaths::Rmdir(const char *path)
{
	if( !SDL_RemovePath( path ) )
	{
		crConsole::Get()->Error( SDL_GetError() );
		return false;
	}

	return true;
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

/*
========================
crPaths::IsFolder
========================
*/
crPaths::sysFolder_t crPaths::IsFolder( const char* path )
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
