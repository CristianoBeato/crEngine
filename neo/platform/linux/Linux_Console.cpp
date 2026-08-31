
#include "precompiled.h"
#include "Linux_Console.hpp"
#include <signal.h>
#include <termios.h>
#include <fcntl.h>

// terminal support
idCVar in_tty( "in_tty", "1", CVAR_BOOL | CVAR_INIT | CVAR_SYSTEM, "terminal tab-completion and history" );

constexpr int siglist[] =
{
	SIGHUP,
	SIGQUIT,
	SIGILL,
	SIGTRAP,
	SIGIOT,
	SIGBUS,
	SIGFPE,
	SIGSEGV,
	SIGPIPE,
	SIGABRT,
	//	SIGTTIN,
	//	SIGTTOU,
	-1
};

constexpr const char* signames[] =
{
	"SIGHUP",
	"SIGQUIT",
	"SIGILL",
	"SIGTRAP",
	"SIGIOT",
	"SIGBUS",
	"SIGFPE",
	"SIGSEGV",
	"SIGPIPE",
	"SIGABRT",
	//	"SIGTTIN",
	//	"SIGTTOUT"
};

static crLinuxConsole gLinuxConsole = crLinuxConsole();

crConsole* crConsole::Get( void )
{
    return &gLinuxConsole;
}

crLinuxConsole::crLinuxConsole( void ) : m_ttyEnabled( false )
{
}

crLinuxConsole::~crLinuxConsole( void )
{
}

void crLinuxConsole::Startup( void )
{
    InitSigs();
    InitConsoleInput();
}

void crLinuxConsole::Shutdown( void )
{
    if( m_ttyEnabled )
	{
		Printf( "shutdown terminal support\n" );
		if( tcsetattr( 0, TCSADRAIN, &m_tty_tc ) == -1 )
			Printf( "tcsetattr failed: %s\n", strerror( errno ) );
	}

    for( int i = 0; i < COMMAND_HISTORY; i++ )
	{
		m_history[ i ].Clear();
	}
    
	// at this point, too late to catch signals
	ClearSigs();
}

void crLinuxConsole::ShowConsole(const int in_visLevel, const bool in_quitOnClose)
{
}

void crLinuxConsole::VPrintf( const char *fmt, va_list arg )
{
    TTYHide();
	std::vprintf( fmt, arg );
	TTYShow();
}

void crLinuxConsole::SetFatalError( const char *error )
{
	std::strncpy( m_fatalError, error, sizeof( m_fatalError ) );
}

const char *crLinuxConsole::ConsoleInput(void)
{
	if( m_ttyEnabled )
	{
		int		ret;
		char	key;
		bool	hidden = false;
		while( ( ret = read( STDIN_FILENO, &key, 1 ) ) > 0 )
		{
			if( !hidden )
			{
				TTYHide();
				hidden = true;
			}
			switch( key )
			{
				case 1:
					m_inputField.SetCursor( 0 );
					break;
				case 5:
					m_inputField.SetCursor( strlen( m_inputField.GetBuffer() ) );
					break;
				case 127:
				case 8:
					m_inputField.CharEvent( K_BACKSPACE );
					break;
				case '\n':
					idStr::Copynz( m_inputRet, m_inputField.GetBuffer(), sizeof( m_inputRet ) );
					assert( hidden );
					TTYShow();
					write( STDOUT_FILENO, &key, 1 );
					m_inputField.Clear();
					if( m_historyCount < COMMAND_HISTORY )
					{
						m_history[ m_historyCount ] = m_inputRet;
						m_historyCount++;
					}
					else
					{
						m_history[ m_historyStart ] = m_inputRet;
						m_historyStart++;
						m_historyStart %= COMMAND_HISTORY;
					}
					m_historyCurrent = 0;
					return m_inputRet;
				case '\t':
					m_inputField.AutoComplete();
					break;
				case 27:
				{
					// enter escape sequence mode
					ret = read( STDIN_FILENO, &key, 1 );
					if( ret <= 0 )
					{
						Printf( "dropping sequence: '27' " );
						TTYFlushIn();
						assert( hidden );
						TTYShow();
						return nullptr;
					}
					switch( key )
					{
						case 79:
							ret = read( STDIN_FILENO, &key, 1 );
							if( ret <= 0 )
							{
								Printf( "dropping sequence: '27' '79' " );
								TTYFlushIn();
								assert( hidden );
								TTYShow();
								return nullptr;
							}
							switch( key )
							{
								case 72:
									// xterm only
									m_inputField.SetCursor( 0 );
									break;
								case 70:
									// xterm only
									m_inputField.SetCursor( strlen( m_inputField.GetBuffer() ) );
									break;
								default:
									Printf( "dropping sequence: '27' '79' '%d' ", key );
									TTYFlushIn();
									assert( hidden );
									TTYShow();
									return nullptr;
							}
							break;
						case 91:
						{
							ret = read( STDIN_FILENO, &key, 1 );
							if( ret <= 0 )
							{
								Printf( "dropping sequence: '27' '91' " );
								TTYFlushIn();
								assert( hidden );
								TTYShow();
								return nullptr;
							}
							switch( key )
							{
								case 49:
								{
									ret = read( STDIN_FILENO, &key, 1 );
									if( ret <= 0 || key != 126 )
									{
										Printf( "dropping sequence: '27' '91' '49' '%d' ", key );
										TTYFlushIn();
										assert( hidden );
										TTYShow();
										return nullptr;
									}
									// only screen and linux terms
									m_inputField.SetCursor( 0 );
									break;
								}
								case 50:
								{
									ret = read( STDIN_FILENO, &key, 1 );
									if( ret <= 0 || key != 126 )
									{
										Printf( "dropping sequence: '27' '91' '50' '%d' ", key );
										TTYFlushIn();
										assert( hidden );
										TTYShow();
										return nullptr;
									}
									// all terms
									m_inputField.KeyDownEvent( K_INS );
									break;
								}
								case 52:
								{
									ret = read( STDIN_FILENO, &key, 1 );
									if( ret <= 0 || key != 126 )
									{
										Printf( "dropping sequence: '27' '91' '52' '%d' ", key );
										TTYFlushIn();
										assert( hidden );
										TTYShow();
										return NULL;
									}
									// only screen and linux terms
									m_inputField.SetCursor( strlen( m_inputField.GetBuffer() ) );
									break;
								}
								case 51:
								{
									ret = read( STDIN_FILENO, &key, 1 );
									if( ret <= 0 )
									{
										Printf( "dropping sequence: '27' '91' '51' " );
										TTYFlushIn();
										assert( hidden );
										TTYShow();
										return nullptr;
									}
									if( key == 126 )
									{
										m_inputField.KeyDownEvent( K_DEL );
										break;
									}
									Printf( "dropping sequence: '27' '91' '51' '%d'", key );
									TTYFlushIn();
									assert( hidden );
									TTYShow();
									return nullptr;
								}
								case 65:
								case 66:
								{
									// history
									if( m_historyCurrent == 0 )
										m_historyBackup = m_inputField;
									
									if( key == 65 )
										// up
										m_historyCurrent++;
									else
										// down
										m_historyCurrent--;
									
									// history_current cycle:
									// 0: current edit
									// 1 .. Min( COMMAND_HISTORY, history_count ): back in history
									if( m_historyCurrent < 0 )
										m_historyCurrent = Min( (int)COMMAND_HISTORY, m_historyCount );
									else
										m_historyCurrent %= Min( (int)COMMAND_HISTORY, m_historyCount ) + 1;
									
									int index = -1;
									if( m_historyCurrent == 0 )
									{
										m_inputField = m_historyBackup;
									}
									else
									{
										index = m_historyStart + Min( (int)COMMAND_HISTORY, m_historyCount ) - m_historyCurrent;
										index %= COMMAND_HISTORY;
										assert( index >= 0 && index < COMMAND_HISTORY );
										m_inputField.SetBuffer( m_history[ index ] );
									}
									assert( hidden );
									TTYShow();
									return nullptr;
								}
								case 67:
									m_inputField.KeyDownEvent( K_RIGHTARROW );
									break;
								case 68:
									m_inputField.KeyDownEvent( K_LEFTARROW );
									break;
								default:
									Printf( "dropping sequence: '27' '91' '%d' ", key );
									TTYFlushIn();
									assert( hidden );
									TTYShow();
									return nullptr;
							}
							break;
						}
						default:
                        {
							Printf( "dropping sequence: '27' '%d' ", key );
							TTYFlushIn();
							assert( hidden );
							TTYShow();
							return nullptr;
                        }
					}
					break;
				}
				default:
                {
					if( key >= ' ' )
					{
						m_inputField.CharEvent( key );
						break;
					}

					Printf( "dropping sequence: '%d' ", key );
					TTYFlushIn();
					assert( hidden );
					TTYShow();
					return nullptr;
                }
			}
		}

		if( hidden )
			TTYShow();
		
		return nullptr;
	}

	return nullptr;
}

void crLinuxConsole::VDebug( const char *fmt, va_list arg )
{
    TTYHide();
	std::vprintf( fmt, arg );
	TTYShow();
}

void crLinuxConsole::VError( const char *fmt, va_list arg )
{
    va_list argptr; 
	Printf( "Error: " );
	TTYHide();
	std::vprintf( fmt, argptr );
	Printf( "\n" );
}

void crLinuxConsole::InitConsoleInput(void)
{
	struct termios tc;
	
	if( in_tty.GetBool() )
	{
		if( isatty( STDIN_FILENO ) != 1 )
		{
			Printf( "terminal support disabled: stdin is not a tty\n" );
			in_tty.SetBool( false );
			return;
		}

		if( tcgetattr( 0, &m_tty_tc ) == -1 )
		{
			Printf( "tcgetattr failed. disabling terminal support: %s\n", strerror( errno ) );
			in_tty.SetBool( false );
			return;
		}
		// make the input non blocking
		if( fcntl( STDIN_FILENO, F_SETFL, fcntl( STDIN_FILENO, F_GETFL, 0 ) | O_NONBLOCK ) == -1 )
		{
			Printf( "fcntl STDIN non blocking failed.  disabling terminal support: %s\n", strerror( errno ) );
			in_tty.SetBool( false );
			return;
		}

		tc = m_tty_tc;
		
		//  ECHO: don't echo input characters
		//  ICANON: enable canonical mode.  This  enables  the  special
		//  	characters  EOF,  EOL,  EOL2, ERASE, KILL, REPRINT,
		//  	STATUS, and WERASE, and buffers by lines.
		//  ISIG: when any of the characters  INTR,  QUIT,  SUSP,  or
		//  	DSUSP are received, generate the corresponding signal
		tc.c_lflag &= ~( ECHO | ICANON );
		
		//  ISTRIP strip off bit 8
		//  INPCK enable input parity checking
		
		tc.c_iflag &= ~( ISTRIP | INPCK );
		tc.c_cc[VMIN] = 1;
		tc.c_cc[VTIME] = 0;
		if( tcsetattr( 0, TCSADRAIN, &tc ) == -1 )
		{
			Printf( "tcsetattr failed: %s\n", strerror( errno ) );
			Printf( "terminal support may not work correctly. Use +set in_tty 0 to disable it\n" );
		}
#if 0
		// make the output non blocking
		if( fcntl( STDOUT_FILENO, F_SETFL, fcntl( STDOUT_FILENO, F_GETFL, 0 ) | O_NONBLOCK ) == -1 )
			Printf( "fcntl STDOUT non blocking failed: %s\n", strerror( errno ) );
#endif
		m_ttyEnabled = true;
		// check the terminal type for the supported ones
		char* term = getenv( "TERM" );
		if( term )
		{
			if( strcmp( term, "linux" ) && strcmp( term, "xterm" ) && strcmp( term, "xterm-color" ) && strcmp( term, "screen" ) )
				Printf( "WARNING: terminal type '%s' is unknown. terminal support may not work correctly\n", term );
		}
		Printf( "terminal support enabled ( use +set in_tty 0 to disabled )\n" );
	}
	else
		Printf( "terminal support disabled\n" );
}

void crLinuxConsole::TTYHide(void)
{
	int len, buf_len;
	if( !m_ttyEnabled )
		return;

	if( m_inputHide )
	{
		m_inputHide++;
		return;
	}

	// clear after cursor
	len = std::strlen( m_inputField.GetBuffer() ) - m_inputField.GetCursor();
	while( len > 0 )
	{
		TTYRight();
		len--;
	}

	buf_len = std::strlen( m_inputField.GetBuffer() );
	while( buf_len > 0 )
	{
		TTYDel();
		buf_len--;
	}

	m_inputHide++;
}

void crLinuxConsole::TTYShow(void)
{
	//	int i;
	if( !m_ttyEnabled )
		return;
	
	assert( m_inputHide > 0 );
	m_inputHide--;
	if( m_inputHide == 0 )
	{
		char* buf = m_inputField.GetBuffer();
		if( buf[0] )
		{
			write( STDOUT_FILENO, buf, std::strlen( buf ) );
						
			int back = std::strlen( buf ) - m_inputField.GetCursor();
			while( back > 0 )
			{
				TTYLeft();
				back--;
			}
		}
	}
}

void crLinuxConsole::TTYRight(void)
{
    constexpr char key = 27;
	write( STDOUT_FILENO, &key, 1 );
	write( STDOUT_FILENO, "[C", 2 );
}

void crLinuxConsole::TTYLeft(void)
{
    constexpr char key = '\b';
	write( STDOUT_FILENO, &key, 1 );
}

void crLinuxConsole::TTYDel(void)
{
    char key;
	key = '\b';
	write( STDOUT_FILENO, &key, 1 );
	key = ' ';
	write( STDOUT_FILENO, &key, 1 );
	key = '\b';
	write( STDOUT_FILENO, &key, 1 );
}

void crLinuxConsole::TTYFlushIn( void )
{
	char key;
	while( read( STDIN_FILENO, &key, 1 ) != -1 )
	{
        Printf( "'%d' ", key );
	}
	
    Printf( "\n" );
}

/*
===============
Sys_FPE_handler
===============
*/
static void Sys_FPE_handler( int signum, siginfo_t* info, void* context )
{
	assert( signum == SIGFPE );
	Sys_Printf( "FPE\n" );
}

void crLinuxConsole::InitSigs(void)
{
	struct sigaction action;
	int i;
	
	m_fatalError[0] = '\0';
	
	// Set up the structure
	action.sa_sigaction = sig_handler;
	sigemptyset( &action.sa_mask );
	action.sa_flags = SA_SIGINFO | SA_NODEFER;
	
	i = 0;
	while( siglist[ i ] != -1 )
	{
		if( siglist[ i ] == SIGFPE )
		{
			action.sa_sigaction = Sys_FPE_handler;
			if( sigaction( siglist[ i ], &action, NULL ) != 0 )
				Printf( "Failed to set SIGFPE handler: %s\n", strerror( errno ) );
			
			action.sa_sigaction = sig_handler;
		}
		else if( sigaction( siglist[ i ], &action, NULL ) != 0 )
		{
			Printf( "Failed to set %s handler: %s\n", signames[ i ], strerror( errno ) );
		}
		i++;
	}
	
	// if the process is backgrounded (running non interactively)
	// then SIGTTIN or SIGTOU could be emitted, if not caught, turns into a SIGSTP
	signal( SIGTTIN, SIG_IGN );
	signal( SIGTTOU, SIG_IGN );
}

void crLinuxConsole::ClearSigs(void)
{
    struct sigaction action;
	int i;
	
	// Set up the structure
	action.sa_handler = SIG_DFL;
	sigemptyset( &action.sa_mask );
	action.sa_flags = 0;
	
	i = 0;
	while( siglist[ i ] != -1 )
	{
		if( sigaction( siglist[ i ], &action, nullptr ) != 0 )
			Printf( "Failed to reset %s handler: %s\n", signames[ i ], strerror( errno ) );

		i++;
	}
}

void crLinuxConsole::SigHandler(int signum, siginfo_t *info, void *context)
{
	static bool double_fault = false;
	
	if( double_fault )
	{
		Printf( "double fault %s, bailing out\n", strsignal( signum ) );
		crPlatform::Get()->Exit( signum );
	}
	
	double_fault = true;
	
	// NOTE: see sigaction man page, could verbose the whole siginfo_t and print human readable si_code
	Printf( "signal caught: %s\nsi_code %d\n", strsignal( signum ), info->si_code );
	
	if( m_fatalError[ 0 ] )
		Printf( "Was in fatal error shutdown: %s\n", m_fatalError );

	Printf( "Trying to exit gracefully..\n" );
	
	// Posix_SetExit( signum );
	m_setExit = signum;

	common->Quit();
}

void crLinuxConsole::sig_handler(int signum, siginfo_t *info, void *context)
{
    gLinuxConsole.SigHandler( signum, info, context );
}
