
#ifndef __WINDOWS_PLATFORM_HPP__
#define __WINDOWS_PLATFORM_HPP__

#include <windows.h>

class crWindowsPlatform : public crPlatform
{
public:
    // disable class copy
    crWindowsPlatform( const crWindowsPlatform& ) = delete;
    crWindowsPlatform operator = ( const crWindowsPlatform& ) = delete;

    crWindowsPlatform( void );
    ~crWindowsPlatform( void );
    virtual void        Init( void );
    virtual void        Shutdown( void );
    virtual void        Exit( const int code );
    virtual void        Quit( void );
    virtual bool        AlreadyRunning( void );
    virtual bool        LockMemory( void* ptr, const size_t bytes );
    virtual bool        UnlockMemory( void* ptr, const size_t bytes );
    virtual void        ReLaunch( void * data, const size_t dataSize );
    virtual void        StartProcess( const char *exePath, const bool doexit );
    virtual void        OpenURL( const char *url, const bool doexit );
    virtual void        GetCurrentMemoryStatus( sysMemoryStats_t& stats );
    virtual void        GetExeLaunchMemoryStatus( sysMemoryStats_t &stats );
    virtual const char*	GetCmdLine( void );
    virtual const char* GetCurrentUser( void );
    
private:

    sysMemoryStats_t    exeLaunchMemoryStats;
    HANDLE              hProcessMutex;
    char		        m_cmdline[MAX_STRING_CHARS];

};

#endif //!__WINDOWS_PLATFORM_HPP__