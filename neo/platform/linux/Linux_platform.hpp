
#ifndef __LINUX_PLATFORM_HPP__
#define __LINUX_PLATFORM_HPP__

#include "../platform.hpp"

class crLinuxConsole : public crConsole
{
public:
    crLinuxConsole( const crLinuxConsole& ) = delete;
    crLinuxConsole operator =( const crLinuxConsole& ) = delete;

    crLinuxConsole( void );
    ~crLinuxConsole( void );
    void            StartUp( void );
    void            ShutDown( void );
    virtual void    VPrintf( const char *fmt, va_list arg );
    virtual void    DebugVPrintf( const char *fmt, va_list arg );
    virtual void    VError( const char *fmt, va_list arg );
private:

};

class crLinuxPlatform : public crPlatform
{
public:
    // Disable class copy
    crLinuxPlatform( const crLinuxPlatform& ) = delete;
    crLinuxPlatform operator = ( const crLinuxPlatform& ) = delete;

    crLinuxPlatform( void );
    ~crLinuxPlatform( void );

    virtual void StartUp( void );
    virtual void ShutDown( void );
    virtual void Exit( const int code );
    virtual bool CreateInstanceLock( void );
    virtual bool LockMemory( void* ptr, const size_t bytes );
    virtual bool UnlockMemory( void* ptr, const size_t bytes );
    virtual void ReLaunch( void * data, const size_t dataSize );
    virtual void StartProcess( const char *exePath, const bool doexit );
    virtual void OpenURL( const char *url, const bool doexit );
    virtual void GetCurrentMemoryStatus( sysMemoryStats_t& stats );
    virtual void GetExeLaunchMemoryStatus( sysMemoryStats_t &stats );
    virtual const char* GetCurrentUser( void );

private:
    int                 m_setExit;
    sysMemoryStats_t    m_exeLaunchMemoryStats;
    const char          m_exitSpawn[ 1024 ];

    void                SetExitSpawn( const char* exeName );
    void                ClearSigs( void );
};

#endif //!__LINUX_PLATFORM_HPP__