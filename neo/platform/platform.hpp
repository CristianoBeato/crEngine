
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

class crConsole
{
public:
    static crConsole*   Get( void );

    crConsole( void ) {};
    virtual ~crConsole( void ) {};
    virtual void            ShowConsole( int visLevel, bool quitOnClose ) = 0;
    virtual void            VPrintf( const char *fmt, va_list arg ) = 0;
    virtual void            DebugVPrintf( const char *fmt, va_list arg ) = 0;
    virtual void            VError( const char *fmt, va_list arg ) = 0; 
    virtual const char * Sys_GetCmdLine( void ) = 0;
};

struct sysMemoryStats_t
{
	size_t memoryLoad;
	size_t totalPhysical;
	size_t availPhysical;
	size_t totalPageFile;
	size_t availPageFile;
	size_t totalVirtual;
	size_t availVirtual;
    size_t availExtendedVirtual;
};

class crPlatform
{
public:
    static crPlatform*  Get( void );

    crPlatform( void ) {};
    virtual ~crPlatform( void ) {};

    virtual void StartUp( void ) = 0;
    virtual void ShutDown( void ) = 0;

    virtual void Exit( const int code ) = 0;

    virtual bool CreateInstanceLock( void ) = 0;

    virtual bool LockMemory( void* ptr, const size_t bytes ) = 0;
    virtual bool UnlockMemory( void* ptr, const size_t bytes ) = 0;

    virtual void ReLaunch( void * data, const size_t dataSize ) = 0;
    virtual void StartProcess( const char *exePath, const bool doexit ) = 0;
    virtual void OpenURL( const char *url, const bool doexit ) = 0;

    /// @brief returns current OS mem info, all values are in kB except the memoryload
    virtual void GetCurrentMemoryStatus( sysMemoryStats_t& stats ) = 0;

    /// @brief get the OS mem info, from engine boot
    virtual void GetExeLaunchMemoryStatus( sysMemoryStats_t &stats ) = 0;

    virtual const char* GetCurrentUser( void ) = 0;
};

#endif //__PLATFORM_H__