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

constexpr char DEFALT_STRING[7] = { "detect" };


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
