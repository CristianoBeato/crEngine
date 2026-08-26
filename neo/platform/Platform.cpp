
#include "precompiled.h"
#include "platform.hpp"

#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_loadso.h>
#include <SDL3/SDL_cpuinfo.h>

const char* k_sysLanguageNames[] =
{
	ID_LANG_ENGLISH, ID_LANG_FRENCH, ID_LANG_ITALIAN, ID_LANG_GERMAN, ID_LANG_SPANISH, ID_LANG_JAPANESE, nullptr
};

constexpr uint32_t k_numLanguages = sizeof( k_sysLanguageNames ) / sizeof k_sysLanguageNames[ 0 ] - 1;

idCVar sys_lang( "sys_lang", ID_LANG_ENGLISH, CVAR_SYSTEM | CVAR_INIT, "", k_sysLanguageNames, idCmdSystem::ArgCompletion_String<k_sysLanguageNames> );

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
		Sys_Printf( "failed to get clipboard content %s\n", SDL_GetError() );
	
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
		Sys_Printf( "failed to set clipboard content %s\n",  SDL_GetError() );
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