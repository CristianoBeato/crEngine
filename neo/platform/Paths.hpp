
#ifndef __PATHS_HPP__
#define __PATHS_HPP__


class crPaths
{
public:
    static crPaths* Get( void );
	
	virtual const char* EXEPath( void ) = 0;
	
	/// @brief Get the default base path
	/// 		- binary image path
	/// 		- current directory
	/// 		- hardcoded
	/// Try to be intelligent: if there is no BASE_GAMEDIR, try the next path 
	virtual const char* DefaultBasePath( void );
	
	virtual const char*	DefaultSavePath( void ) = 0;

	/// @brief Retrieve current work dir
	/// @return 
	virtual const char*	CWD( void ) = 0;
	
	const char*			LaunchPath( void ) { return CWD(); }

	uint64_t 			GetDriveFreeSpace( const char* path );
	uint64_t 			GetDriveFreeSpaceInBytes( const char* path );

	/// @brief use fs_debug to verbose Sys_ListFiles
	/// @param path
	/// @param extension
	/// @return -1 if directory was not found (the list is cleared), or n for the element count in the list 
	//int					ListFiles( const char *path, const char* extension, idList<class idStr>& list );
	bool 				DirExist( const char *path );

	/// @brief Create a path three 
	/// @param path 
	//void 				Mkdir( const char* path ) const;
	
	/// @brief Remove a folder in the path 
	/// @param path 
	/// @return false on erro, folder not enpty, not acessible 
	//bool				Rmdir( const char* path );
	
	/// @brief check if file is protected form writing
	/// @param path 
	/// @return true if are unlocked 
	virtual bool		IsFileWritable( const char* path );
	
	/// @brief returns FOLDER_YES if the specified path is a folder
	/// @param path 
	/// @return FOLDER_ERROR on error, FOLDER_NO for not folder, FOLDER_YES for a folder
	Folder_t			IsFolder( const char* path );
};

#endif //!__PATHS_HPP__
