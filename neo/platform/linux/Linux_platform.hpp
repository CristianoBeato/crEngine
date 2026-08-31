
#ifndef __LINUX_PLATFORM_HPP__
#define __LINUX_PLATFORM_HPP__

#include "../Platform.hpp"

class crLinuxPlatform : public crPlatform
{
public:
    // Disable class copy
    crLinuxPlatform( const crLinuxPlatform& ) = delete;
    crLinuxPlatform operator = ( const crLinuxPlatform& ) = delete;

    crLinuxPlatform( void );
    ~crLinuxPlatform( void );

    virtual void Init( void );
    virtual void Shutdown( void );
    virtual void Quit( void );
    virtual void Exit( const int code );
    virtual bool AlreadyRunning( void );
    virtual bool LockMemory( void* ptr, const size_t bytes );
    virtual bool UnlockMemory( void* ptr, const size_t bytes );
    virtual void ReLaunch( void * data, const size_t dataSize );
    virtual void StartProcess( const char *exePath, const bool doexit );
    virtual void OpenURL( const char *url, const bool doexit );
    virtual void GetCurrentMemoryStatus( sysMemoryStats_t& stats );
    virtual void GetExeLaunchMemoryStatus( sysMemoryStats_t &stats );
//    virtual void SetFatalError(const char *error);
    virtual const char* GetCurrentUser( void );

private:
    int                 m_instanceLock;
    int                 m_setExit;
    sysMemoryStats_t    m_exeLaunchMemoryStats;
    const char          m_exitSpawn[ 1024 ];
    void                SetExitSpawn( const char* exeName );
};

#endif //!__LINUX_PLATFORM_HPP__