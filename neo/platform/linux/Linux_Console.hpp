#ifndef __LINUX_CONSOLE_HPP__
#define __LINUX_CONSOLE_HPP__

#include <cstdio>
#include "../Platform.hpp"

inline constexpr uint32_t COMMAND_HISTORY = 64;

class crLinuxConsole : public crConsole
{
public:
    crLinuxConsole( void );
    ~crLinuxConsole( void );
    
    virtual void        StartUp( void );
    virtual void        Shutdown( void );
    virtual void        ShowConsole( const int in_visLevel, const bool in_quitOnClose );
    virtual const char* GetCmdLine( void );
    virtual const char* ConsoleInput( void );
    virtual void        VPrintf( const char *fmt, va_list arg );
    virtual void        VDebug( const char *fmt, va_list arg );
    virtual void        VError( const char *fmt, va_list arg );

protected:
    void    InitConsoleInput( void );
    void    TTYHide( void );
    void    TTYShow( void );
    void    TTYRight( void );
    void    TTYLeft( void );
    void    TTYDel( void );
    void    TTYFlushIn( void );
    void    InitSigs( void );
    void    ClearSigs( void );
    void    SigHandler( int signum, siginfo_t* info, void* context );
    static void sig_handler( int signum, siginfo_t* info, void* context );

private:
    bool            m_ttyEnabled;
    int				m_inputHide;
    int             m_historyStart; 
    int             m_historyCount; // buffer fill up
    int	            m_historyCurrent;   // goes back in history
    int             m_setExit;
    char            m_inputRet[256];
    char            m_fatalError[1024];
    idStr		    m_history[ COMMAND_HISTORY ];	// cycle buffer
    idEditField     m_inputField;
    idEditField	    m_historyBackup;				// the base edit line
    struct termios	m_tty_tc;
};

#endif //__LINUX_CONSOLE_HPP__