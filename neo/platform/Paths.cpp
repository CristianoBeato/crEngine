
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
int crPaths::ListFiles(const char * in_path, const char * in_extension, idList<class idStr> & out_list )
{
	if ( !in_path || !(*in_path) )
        return 0;

#if 
	try
	{
		// check for a valid path
		fs::path dirPath( in_path );
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath))
            return 0;

		// get the extensions
        std::string extFilter;
        if ( in_extension && *in_extension ) 
		{
            extFilter = in_extension;
            if (extFilter[0] != '.')
                extFilter = "." + extFilter;
        }

        out_list.Clear(); // idStrList tem Clear()

        for (const auto& entry : fs::directory_iterator(dirPath)) 
		{
			const fs::path& filePath = entry.path();
          
			// we are listing subpaths
			if ( entry.is_directory() && std::strncmp( in_extension, PATHSEPARATOR_STR, std::strlen( in_extension ) ) )
			{
            	out_list.Append(filePath.filename().string().c_str());
			}
			else if( entry.is_regular_file() && !extFilter.empty() )
			{
				std::string ext = filePath.extension().string(); 
            	if ( ext != extFilter)
            	    continue;

				out_list.Append(filePath.filename().string().c_str());
			}	
        }
	}
	catch(const std::exception& e)
	{
		 common->DPrintf("Sys_ListFiles: fail '%s' to acess '%s'\n", e.what(), in_extension );
        return 0;
	}
#else
	 // 1. Limpa a lista de destino conforme o padrão da idTech4
    list.Clear();

    // 2. Trata a extensão para o formato de wildcard exigido pelo SDL3
    idStr pattern;
    if ( extension == NULL || extension[0] == '\0' ) {
        pattern = "*"; // Se vazio, pega todos os arquivos
    } else if ( extension[0] == '*' ) {
        pattern = extension; // Se já tiver '*', usa como está
    } else if ( extension[0] == '.' ) {
        pattern = idStr( "*" ) + extension; // Se for '.pk4', vira '*.pk4'
    } else {
        pattern = idStr( "*." ) + extension; // Se for 'pk4', vira '*.pk4'
    }

    int count = 0;
    // O flag SDL_GLOB_CASEINSENSITIVE garante portabilidade (essencial para Linux/Android)
    char** arquivos = SDL_GlobDirectory( directory, pattern.c_str(), SDL_GLOB_CASEINSENSITIVE, &count );

    // 3. Se falhar ou não achar nada, retorna zero
    if ( arquivos == NULL ) {
        return 0; 
    }

    // 4. Alimenta a lista interna da idTech4 (idStrList)
    list.AssureSize( count );
    for ( int i = 0; i < count; i++ ) {
        // SDL_GlobDirectory retorna apenas o nome do arquivo/pasta relativo ao 'directory'
        list.Append( arquivos[i] );
    }

    // 5. OBRIGATÓRIO: Liberar a alocação única feita pelo SDL3
    SDL_free( arquivos );

    return list.Num();
#endif
	
	return out_list.Num();
}
*/

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
==============
crPaths::Mkdir
==============
*/
/*
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
*/

/*
==============
crPaths::Rmdir
==============
*/
/*
bool crPaths::Rmdir(const char *path)
{
	if( !SDL_RemovePath( path ) )
	{
		crConsole::Get()->Error( SDL_GetError() );
		return false;
	}

	return true;
}	
*/

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

/*
crPaths::Folder_t crPaths::IsFolder( const char* path ) const
{
#if 0
	std::error_code ec;

	if (!path || !*path)
		return FOLDER_ERROR;
		
	auto fpath = fs::path( path );

	if ( fs::exists( fpath, ec ) ) 
	{
		if ( fs::is_directory( fpath, ec ) )
			return FOLDER_YES;
	}
	
#else
	SDL_PathInfo info;
	if( !SDL_GetPathInfo( path, &info ) )
		return FOLDER_ERROR;

	// is a folder 
	if( info.type != SDL_PATHTYPE_DIRECTORY )
		return FOLDER_YES; 
#endif 
	return FOLDER_NO;
}
*/