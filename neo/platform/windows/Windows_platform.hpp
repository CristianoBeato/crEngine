
#ifndef __WINDOWS_PLATFORM_HPP__
#define __WINDOWS_PLATFORM_HPP__

class crWindowsPlatform : public crPlatform
{
public:
    // disable class copy
    crWindowsPlatform( const crWindowsPlatform& ) = delete;
    crWindowsPlatform operator = ( const crWindowsPlatform& ) = delete;

    crWindowsPlatform( void );
    ~crWindowsPlatform( void );

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
    virtual void SetFatalError( const char* error );
    virtual const char* GetCurrentUser( void );
    
private:
    sysMemoryStats_t    exeLaunchMemoryStats;

};

#endif //!__WINDOWS_PLATFORM_HPP__