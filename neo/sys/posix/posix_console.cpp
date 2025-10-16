
#include "precompiled.h"
#include "posix_public.h"
#include "sys/sys_types.h"

#include <termios.h>
#include <fcntl.h>

#if defined(__ANDROID__)
#include <android/log.h>
#endif

static const uint32_t COMMAND_HISTORY = 64;

static struct posix_console_t
{
    bool				tty_enabled = false;
    int			        input_hide = 0;
    int				    history_count = 0;			// buffer fill up
    int				    history_start = 0;			// current history start
    int				    history_current = 0;        // goes back in history
    char				input_ret[256];
    struct termios      tty_tc;
    idEditField         input_field;
    idEditField			history_backup;				// the base edit line
    idStr			    history[ COMMAND_HISTORY ];	// cycle buffer
}console;

// terminal support
idCVar in_tty( "in_tty", "1", CVAR_BOOL | CVAR_INIT | CVAR_SYSTEM, "terminal tab-completion and history" );

/*
================
Sys_ShowConsole
================
*/
void Sys_ShowConsole( int visLevel, bool quitOnClose ) 
{
}

/*
===============
Posix_InitConsoleInput
===============
*/
void Posix_InitConsoleInput()
{
	struct termios tc;
	
	if( in_tty.GetBool() )
	{
		if( isatty( STDIN_FILENO ) != 1 )
		{
			Sys_Printf( "terminal support disabled: stdin is not a tty\n" );
			in_tty.SetBool( false );
			return;
		}
		if( tcgetattr( 0, &console.tty_tc ) == -1 )
		{
			Sys_Printf( "tcgetattr failed. disabling terminal support: %s\n", strerror( errno ) );
			in_tty.SetBool( false );
			return;
		}
		// make the input non blocking
		if( fcntl( STDIN_FILENO, F_SETFL, fcntl( STDIN_FILENO, F_GETFL, 0 ) | O_NONBLOCK ) == -1 )
		{
			Sys_Printf( "fcntl STDIN non blocking failed.  disabling terminal support: %s\n", strerror( errno ) );
			in_tty.SetBool( false );
			return;
		}
		tc = console.tty_tc;
		/*
		  ECHO: don't echo input characters
		  ICANON: enable canonical mode.  This  enables  the  special
		  	characters  EOF,  EOL,  EOL2, ERASE, KILL, REPRINT,
		  	STATUS, and WERASE, and buffers by lines.
		  ISIG: when any of the characters  INTR,  QUIT,  SUSP,  or
		  	DSUSP are received, generate the corresponding signal
		*/
		tc.c_lflag &= ~( ECHO | ICANON );
		/*
		  ISTRIP strip off bit 8
		  INPCK enable input parity checking
		*/
		tc.c_iflag &= ~( ISTRIP | INPCK );
		tc.c_cc[VMIN] = 1;
		tc.c_cc[VTIME] = 0;
		if( tcsetattr( 0, TCSADRAIN, &tc ) == -1 )
		{
			Sys_Printf( "tcsetattr failed: %s\n", strerror( errno ) );
			Sys_Printf( "terminal support may not work correctly. Use +set in_tty 0 to disable it\n" );
		}
#if 0
		// make the output non blocking
		if( fcntl( STDOUT_FILENO, F_SETFL, fcntl( STDOUT_FILENO, F_GETFL, 0 ) | O_NONBLOCK ) == -1 )
		{
			Sys_Printf( "fcntl STDOUT non blocking failed: %s\n", strerror( errno ) );
		}
#endif
		console.tty_enabled = true;
		// check the terminal type for the supported ones
		char* term = getenv( "TERM" );
		if( term )
		{
			if( std::strcmp( term, "linux" ) && std::strcmp( term, "xterm" ) && std::strcmp( term, "xterm-color" ) && std::strcmp( term, "screen" ) )
			{
				Sys_Printf( "WARNING: terminal type '%s' is unknown. terminal support may not work correctly\n", term );
			}
		}
		Sys_Printf( "terminal support enabled ( use +set in_tty 0 to disabled )\n" );
	}
	else
	{
		Sys_Printf( "terminal support disabled\n" );
	}
}

/*
================
terminal support utilities
================
*/

void tty_Del()
{
	char key;
	key = '\b';
	write( STDOUT_FILENO, &key, 1 );
	key = ' ';
	write( STDOUT_FILENO, &key, 1 );
	key = '\b';
	write( STDOUT_FILENO, &key, 1 );
}

void tty_Left()
{
	char key = '\b';
	write( STDOUT_FILENO, &key, 1 );
}

void tty_Right()
{
	char key = 27;
	write( STDOUT_FILENO, &key, 1 );
	write( STDOUT_FILENO, "[C", 2 );
}

// clear the display of the line currently edited
// bring cursor back to beginning of line
void tty_Hide()
{
	int len, buf_len;
	if( !console.tty_enabled )
	{
		return;
	}
	if( console.input_hide )
	{
		console.input_hide++;
		return;
	}
	// clear after cursor
	len = strlen( console.input_field.GetBuffer() ) - console.input_field.GetCursor();
	while( len > 0 )
	{
		tty_Right();
		len--;
	}
	buf_len = strlen( console.input_field.GetBuffer() );
	while( buf_len > 0 )
	{
		tty_Del();
		buf_len--;
	}
	console.input_hide++;
}

// show the current line
void tty_Show( void )
{
	//	int i;
	if( !console.tty_enabled )
	{
		return;
	}
	assert( console.input_hide > 0 );
	console.input_hide--;
	if( console.input_hide == 0 )
	{
		char* buf = console.input_field.GetBuffer();
		if( buf[0] )
		{
			write( STDOUT_FILENO, buf, strlen( buf ) );
			
			// RB begin
#if defined(__ANDROID__)
			//__android_log_print(ANDROID_LOG_DEBUG, "RBDoom3_DEBUG", "%s", buf);
#endif
			// RB end
			
			int back = strlen( buf ) - console.input_field.GetCursor();
			while( back > 0 )
			{
				tty_Left();
				back--;
			}
		}
	}
}

void tty_FlushIn()
{
	char key;
	while( read( 0, &key, 1 ) != -1 )
	{
		Sys_Printf( "'%d' ", key );
	}
	Sys_Printf( "\n" );
}

/*
================
Posix_ConsoleInput
Checks for a complete line of text typed in at the console.
Return NULL if a complete line is not ready.
================
*/
char* Posix_ConsoleInput( void )
{
	if( console.tty_enabled )
	{
		int		ret;
		char	key;
		bool	hidden = false;
		while( ( ret = read( STDIN_FILENO, &key, 1 ) ) > 0 )
		{
			if( !hidden )
			{
				tty_Hide();
				hidden = true;
			}
			switch( key )
			{
				case 1:
					console.input_field.SetCursor( 0 );
					break;
				case 5:
					console.input_field.SetCursor( strlen( console.input_field.GetBuffer() ) );
					break;
				case 127:
				case 8:
					console.input_field.CharEvent( K_BACKSPACE );
					break;
				case '\n':
					idStr::Copynz( console.input_ret, console.input_field.GetBuffer(), sizeof( console.input_ret ) );
					assert( hidden );
					tty_Show();
					write( STDOUT_FILENO, &key, 1 );
					console.input_field.Clear();
					if( console.history_count < COMMAND_HISTORY )
					{
						console.history[ console.history_count ] = console.input_ret;
						console.history_count++;
					}
					else
					{
						console.history[ console.history_start ] = console.input_ret;
						console.history_start++;
						console.history_start %= COMMAND_HISTORY;
					}
					console.history_current = 0;
					return console.input_ret;
				case '\t':
					console.input_field.AutoComplete();
					break;
				case 27:
				{
					// enter escape sequence mode
					ret = read( STDIN_FILENO, &key, 1 );
					if( ret <= 0 )
					{
						Sys_Printf( "dropping sequence: '27' " );
						tty_FlushIn();
						assert( hidden );
						tty_Show();
						return NULL;
					}
					switch( key )
					{
						case 79:
							ret = read( STDIN_FILENO, &key, 1 );
							if( ret <= 0 )
							{
								Sys_Printf( "dropping sequence: '27' '79' " );
								tty_FlushIn();
								assert( hidden );
								tty_Show();
								return NULL;
							}
							switch( key )
							{
								case 72:
									// xterm only
									console.input_field.SetCursor( 0 );
									break;
								case 70:
									// xterm only
									console.input_field.SetCursor( strlen( console.input_field.GetBuffer() ) );
									break;
								default:
									Sys_Printf( "dropping sequence: '27' '79' '%d' ", key );
									tty_FlushIn();
									assert( hidden );
									tty_Show();
									return NULL;
							}
							break;
						case 91:
						{
							ret = read( STDIN_FILENO, &key, 1 );
							if( ret <= 0 )
							{
								Sys_Printf( "dropping sequence: '27' '91' " );
								tty_FlushIn();
								assert( hidden );
								tty_Show();
								return NULL;
							}
							switch( key )
							{
								case 49:
								{
									ret = read( STDIN_FILENO, &key, 1 );
									if( ret <= 0 || key != 126 )
									{
										Sys_Printf( "dropping sequence: '27' '91' '49' '%d' ", key );
										tty_FlushIn();
										assert( hidden );
										tty_Show();
										return NULL;
									}
									// only screen and linux terms
									console.input_field.SetCursor( 0 );
									break;
								}
								case 50:
								{
									ret = read( STDIN_FILENO, &key, 1 );
									if( ret <= 0 || key != 126 )
									{
										Sys_Printf( "dropping sequence: '27' '91' '50' '%d' ", key );
										tty_FlushIn();
										assert( hidden );
										tty_Show();
										return NULL;
									}
									// all terms
									console.input_field.KeyDownEvent( K_INS );
									break;
								}
								case 52:
								{
									ret = read( STDIN_FILENO, &key, 1 );
									if( ret <= 0 || key != 126 )
									{
										Sys_Printf( "dropping sequence: '27' '91' '52' '%d' ", key );
										tty_FlushIn();
										assert( hidden );
										tty_Show();
										return NULL;
									}
									// only screen and linux terms
									console.input_field.SetCursor( strlen( console.input_field.GetBuffer() ) );
									break;
								}
								case 51:
								{
									ret = read( STDIN_FILENO, &key, 1 );
									if( ret <= 0 )
									{
										Sys_Printf( "dropping sequence: '27' '91' '51' " );
										tty_FlushIn();
										assert( hidden );
										tty_Show();
										return NULL;
									}
									if( key == 126 )
									{
										console.input_field.KeyDownEvent( K_DEL );
										break;
									}
									Sys_Printf( "dropping sequence: '27' '91' '51' '%d'", key );
									tty_FlushIn();
									assert( hidden );
									tty_Show();
									return nullptr;
								}
								case 65:
								case 66:
								{
									// history
									if( console.history_current == 0 )
									{
										console.history_backup = console.input_field;
									}
									if( key == 65 )
                                        console.history_current++; // up
									else
                                        console.history_current--;// down
									
									// history_current cycle:
									// 0: current edit
									// 1 .. Min( COMMAND_HISTORY, history_count ): back in history
									if( console.history_current < 0 )
									{
										console.history_current = Min( COMMAND_HISTORY, console.history_count );
									}
									else
									{
										console.history_current %= Min( COMMAND_HISTORY, console.history_count ) + 1;
									}
									int index = -1;
									if( console.history_current == 0 )
									{
										console.input_field = console.history_backup;
									}
									else
									{
										index = console.history_start + Min( COMMAND_HISTORY, console.history_count ) - console.history_current;
										index %= COMMAND_HISTORY;
										assert( index >= 0 && index < COMMAND_HISTORY );
										console.input_field.SetBuffer( console.history[ index ] );
									}
									assert( hidden );
									tty_Show();
									return NULL;
								}
								case 67:
									console.input_field.KeyDownEvent( K_RIGHTARROW );
									break;
								case 68:
									console.input_field.KeyDownEvent( K_LEFTARROW );
									break;
								default:
									Sys_Printf( "dropping sequence: '27' '91' '%d' ", key );
									tty_FlushIn();
									assert( hidden );
									tty_Show();
									return NULL;
							}
							break;
						}
						default:
							Sys_Printf( "dropping sequence: '27' '%d' ", key );
							tty_FlushIn();
							assert( hidden );
							tty_Show();
							return NULL;
					}
					break;
				}
				default:
					if( key >= ' ' )
					{
						console.input_field.CharEvent( key );
						break;
					}
					Sys_Printf( "dropping sequence: '%d' ", key );
					tty_FlushIn();
					assert( hidden );
					tty_Show();
					return NULL;
			}
		}

		if( hidden )
			tty_Show();
		
		return nullptr;
	}
	else
	{
		// disabled on OSX. works fine from a terminal, but launching from Finder is causing trouble
		// I'm pretty sure it could be re-enabled if needed, and just handling the Finder failure case right (TTimo)
#ifndef MACOS_X
		// no terminal support - read only complete lines
		int				len;
		fd_set			fdset;
		struct timeval	timeout;
		
		FD_ZERO( &fdset );
		FD_SET( STDIN_FILENO, &fdset );
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		if( select( 1, &fdset, NULL, NULL, &timeout ) == -1 || !FD_ISSET( 0, &fdset ) )
			return nullptr;
		
		len = read( 0, console.input_ret, sizeof( console.input_ret ) );
		if( len == 0 )
            return nullptr; // EOF
		
		if( len < 1 )
		{
			Sys_Printf( "read failed: %s\n", strerror( errno ) );	// something bad happened, cancel this line and print an error
			return nullptr;
		}
		
		if( len == sizeof( console.input_ret ) )
			Sys_Printf( "read overflow\n" );	// things are likely to break, as input will be cut into pieces
		
		console.input_ret[ len - 1 ] = '\0';		// rip off the \n and terminate
		return console.input_ret;
#endif
	}
	return nullptr;
}

void Posix_ConsoleExit( void )
{
    if( console.tty_enabled )
	{
		Sys_Printf( "shutdown terminal support\n" );
		if( tcsetattr( 0, TCSADRAIN, &console.tty_tc ) == -1 )
		{
			Sys_Printf( "tcsetattr failed: %s\n", strerror( errno ) );
		}
	}

	for( int i = 0; i < COMMAND_HISTORY; i++ )
	{
		console.history[ i ].Clear();
	}
}

/*
called during frame loops, pacifier updates etc.
this is only for console input polling and misc mouse grab tasks
the actual mouse and keyboard input is in the Sys_Poll logic
*/
/*
void Sys_GenerateEvents()
{
	char* s;
	if( ( s = Posix_ConsoleInput() ) )
	{
		char* b;
		int len;

		len = strlen( s ) + 1;
		b = ( char* )Mem_Alloc( len );
		strcpy( b, s );
		Posix_QueEvent( SE_CONSOLE, 0, 0, len, b );
	}
}
*/



/*
===============
low level output
===============
*/

void Sys_DebugPrintf( const char* fmt, ... )
{
#if defined(__ANDROID__)
	va_list		argptr;
	char		msg[4096];
	
	va_start( argptr, fmt );
	idStr::vsnPrintf( msg, sizeof( msg ), fmt, argptr );
	va_end( argptr );
	msg[sizeof( msg ) - 1] = '\0';
	
	__android_log_print( ANDROID_LOG_DEBUG, "RBDoom3_Debug", msg );
#else
	va_list argptr;
	
	tty_Hide();
	va_start( argptr, fmt );
	vprintf( fmt, argptr );
	va_end( argptr );
	tty_Show();
#endif
}

void Sys_DebugVPrintf( const char* fmt, va_list arg )
{
#if defined(__ANDROID__)
	__android_log_vprint( ANDROID_LOG_DEBUG, "RBDoom3_Debug", fmt, arg );
#else
	tty_Hide();
	vprintf( fmt, arg );
	tty_Show();
#endif
}

void Sys_Printf( const char* fmt, ... )
{
#if defined(__ANDROID__)
	va_list		argptr;
	char		msg[4096];
	
	va_start( argptr, fmt );
	idStr::vsnPrintf( msg, sizeof( msg ), fmt, argptr );
	va_end( argptr );
	msg[sizeof( msg ) - 1] = '\0';
	
	__android_log_print( ANDROID_LOG_DEBUG, "RBDoom3", msg );
#else
	va_list argptr;
	
	tty_Hide();
	va_start( argptr, fmt );
	vprintf( fmt, argptr );
	va_end( argptr );
	tty_Show();
#endif
}

void Sys_VPrintf( const char* fmt, va_list arg )
{
#if defined(__ANDROID__)
	__android_log_vprint( ANDROID_LOG_DEBUG, "RBDoom3", fmt, arg );
#else
	tty_Hide();
	vprintf( fmt, arg );
	tty_Show();
#endif
}

/*
================
Sys_Error
================
*/
void Sys_Error( const char* error, ... )
{
	va_list argptr;
	
	Sys_Printf( "Sys_Error: " );
	va_start( argptr, error );
	Sys_DebugVPrintf( error, argptr );
	va_end( argptr );
	Sys_Printf( "\n" );
	
	Posix_Exit( EXIT_FAILURE );
}