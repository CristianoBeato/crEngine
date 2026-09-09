
#include "precompiled.h"
#include "Windows_Console.hpp"

static idCVar win_viewlog( "win_viewlog", "0", CVAR_SYSTEM | CVAR_INTEGER, "" );

constexpr uint32_t COPY_ID = 1;
constexpr uint32_t QUIT_ID = 2;
constexpr uint32_t CLEAR_ID = 3;

constexpr uint32_t ERRORBOX_ID = 10;
constexpr uint32_t ERRORTEXT_ID = 11;

constexpr uint32_t EDIT_ID = 100;
constexpr uint32_t INPUT_ID = 101;

constexpr uint32_t	COMMAND_HISTORY	= 64;

constexpr uint32_t CONSOLE_BUFFER_SIZE = 16384;

constexpr uint32_t MAXPRINTMSG = 4096;

static crWindowsConsole gConsole = crWindowsConsole();

/*
=====================
crConsole::Get
=====================
*/
crConsole* crConsole::Get( void )
{
    return &gConsole;
}

/*
=====================
crWindowsConsole::Startup
=====================
*/
void crWindowsConsole::Startup(void)
{
	HDC hDC;
	WNDCLASS wc;
	RECT rect;
	const char* DEDCLASS = WIN32_CONSOLE_CLASS;
	int nHeight;
	int swidth, sheight;
	int DEDSTYLE = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX;
	int i;
	
	std::memset( &wc, 0, sizeof( wc ) );
	
	wc.style         = 0;
	wc.lpfnWndProc   = ( WNDPROC ) ConWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = GetModuleHandle( NULL );
	wc.hIcon         = LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON1 ) );
	wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = ( struct HBRUSH__* )COLOR_WINDOW;
	wc.lpszMenuName  = 0;
	wc.lpszClassName = DEDCLASS;
	
	if( !RegisterClass( &wc ) )
		return;
	
	rect.left = 0;
	rect.right = 540;
	rect.top = 0;
	rect.bottom = 450;
	AdjustWindowRect( &rect, DEDSTYLE, FALSE );
	
	hDC = GetDC( GetDesktopWindow() );
	swidth = GetDeviceCaps( hDC, HORZRES );
	sheight = GetDeviceCaps( hDC, VERTRES );
	ReleaseDC( GetDesktopWindow(), hDC );
	
	m_windowWidth = rect.right - rect.left + 1;
	m_windowHeight = rect.bottom - rect.top + 1;
	
	//s_wcd.hbmLogo = LoadBitmap( win32.hInstance, MAKEINTRESOURCE( IDB_BITMAP_LOGO) );
	
	m_hWnd = CreateWindowEx( 0,
								 DEDCLASS,
								 GAME_NAME,
								 DEDSTYLE,
								 ( swidth - 600 ) / 2, ( sheight - 450 ) / 2 , rect.right - rect.left + 1, rect.bottom - rect.top + 1,
								 NULL,
								 NULL,
								 m_hInstance,
								 NULL );

	if( m_hWnd == NULL )
		return;
	
	//
	// create fonts
	//
	hDC = GetDC( m_hWnd );
	nHeight = -MulDiv( 8, GetDeviceCaps( hDC, LOGPIXELSY ), 72 );
	
	m_hfBufferFont = CreateFont( nHeight, 0, 0, 0, FW_LIGHT, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN | FIXED_PITCH, "Courier New" );
	
	ReleaseDC( m_hWnd, hDC );
	
	//
	// create the input line
	//
	m_hwndInputLine = CreateWindow( "edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
										ES_LEFT | ES_AUTOHSCROLL,
										6, 400, 528, 20,
										m_hWnd,
										( HMENU ) INPUT_ID,	// child window ID
										m_hInstance, NULL );
										
	//
	// create the buttons
	//
	m_hwndButtonCopy = CreateWindow( "button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
										 5, 425, 72, 24,
										 m_hWnd,
										 ( HMENU ) COPY_ID,	// child window ID
										 m_hInstance, NULL );

	SendMessage( m_hwndButtonCopy, WM_SETTEXT, 0, ( LPARAM ) "copy" );
	
	m_hwndButtonClear = CreateWindow( "button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
										  82, 425, 72, 24,
										  m_hWnd,
										  ( HMENU ) CLEAR_ID,	// child window ID
										  m_hInstance, NULL );

	SendMessage( m_hwndButtonClear, WM_SETTEXT, 0, ( LPARAM ) "clear" );
	
	m_hwndButtonQuit = CreateWindow( "button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
										 462, 425, 72, 24,
										 m_hWnd,
										 ( HMENU ) QUIT_ID,	// child window ID
										 m_hInstance, NULL );

	SendMessage( m_hwndButtonQuit, WM_SETTEXT, 0, ( LPARAM ) "quit" );
	
	
	//
	// create the scrollbuffer
	//
	m_hwndBuffer = CreateWindow( "edit", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
									 ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
									 6, 40, 526, 354,
									 m_hWnd,
									 ( HMENU ) EDIT_ID,	// child window ID
									 m_hInstance, NULL );

	SendMessage( m_hwndBuffer, WM_SETFONT, ( WPARAM ) m_hfBufferFont, 0 );
	
	// RB begin
#if defined(_WIN64)
	m_sysInputLineWndProc = ( WNDPROC ) SetWindowLong( m_hwndInputLine, GWLP_WNDPROC, ( LONG_PTR ) InputLineWndProc );
#else
	m_sysInputLineWndProc = ( WNDPROC ) SetWindowLong( m_hwndInputLine, GWL_WNDPROC, ( LONG ) InputLineWndProc );
#endif
	// RB end
	SendMessage( m_hwndInputLine, WM_SETFONT, ( WPARAM ) m_hfBufferFont, 0 );
	
// don't show it now that we have a splash screen up
	if( win_viewlog.GetBool() )
	{
		ShowWindow( m_hWnd, SW_SHOWDEFAULT );
		UpdateWindow( m_hWnd );
		SetForegroundWindow( m_hWnd );
		SetFocus( m_hwndInputLine );
	}
	
	m_consoleField.Clear();
	
	for( i = 0 ; i < COMMAND_HISTORY ; i++ )
	{
		m_historyEditLines[i].Clear();
	}
}

/*
=====================
crWindowsConsole::ConWndProc
=====================
*/
LONG WINAPI crWindowsConsole::ConWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	char* cmdString;
	static bool s_timePolarity;
	
	switch( uMsg )
	{
		case WM_ACTIVATE:
			if( LOWORD( wParam ) != WA_INACTIVE )
			{
				SetFocus( gConsole.m_hwndInputLine );
			}
			break;
		case WM_CLOSE:
			if( gConsole.m_quitOnClose )
			{
				PostQuitMessage( 0 );
			}
			else
			{
				gConsole.ShowConsole( 0, false );
				win_viewlog.SetBool( false );
			}
			return 0;
		case WM_CTLCOLORSTATIC:
			if( ( HWND ) lParam == gConsole.m_hwndBuffer )
			{
				SetBkColor( ( HDC ) wParam, RGB( 0x00, 0x00, 0x80 ) );
				SetTextColor( ( HDC ) wParam, RGB( 0xff, 0xff, 0x00 ) );
				return ( long ) gConsole.m_hbrEditBackground;
			}
			else if( ( HWND ) lParam == gConsole.m_hwndErrorBox )
			{
				if( s_timePolarity & 1 )
				{
					SetBkColor( ( HDC ) wParam, RGB( 0x80, 0x80, 0x80 ) );
					SetTextColor( ( HDC ) wParam, RGB( 0xff, 0x0, 0x00 ) );
				}
				else
				{
					SetBkColor( ( HDC ) wParam, RGB( 0x80, 0x80, 0x80 ) );
					SetTextColor( ( HDC ) wParam, RGB( 0x00, 0x0, 0x00 ) );
				}
				return ( long ) gConsole.m_hbrErrorBackground;
			}
			break;
		case WM_SYSCOMMAND:
			if( wParam == SC_CLOSE )
			{
				PostQuitMessage( 0 );
			}
			break;
		case WM_COMMAND:
			if( wParam == COPY_ID )
			{
				SendMessage( gConsole.m_hwndBuffer, EM_SETSEL, 0, -1 );
				SendMessage( gConsole.m_hwndBuffer, WM_COPY, 0, 0 );
			}
			else if( wParam == QUIT_ID )
			{
				if( gConsole.m_quitOnClose )
				{
					PostQuitMessage( 0 );
				}
				else
				{
					cmdString = Mem_CopyString( "quit" );
					crEvents::Get()->QueEvent( SE_CONSOLE, 0, 0, std::strlen( cmdString ) + 1, cmdString, 0 );
				}
			}
			else if( wParam == CLEAR_ID )
			{
				SendMessage( gConsole.m_hwndBuffer, EM_SETSEL, 0, -1 );
				SendMessage( gConsole.m_hwndBuffer, EM_REPLACESEL, FALSE, ( LPARAM ) "" );
				UpdateWindow( gConsole.m_hwndBuffer );
			}
			break;
		case WM_CREATE:
			gConsole.m_hbrEditBackground = CreateSolidBrush( RGB( 0x00, 0x00, 0x80 ) );
			gConsole.m_hbrErrorBackground = CreateSolidBrush( RGB( 0x80, 0x80, 0x80 ) );
			SetTimer( hWnd, 1, 1000, NULL );
			break;
			/*
					case WM_ERASEBKGND:
						HGDIOBJ oldObject;
						HDC hdcScaled;
						hdcScaled = CreateCompatibleDC( ( HDC ) wParam );
						assert( hdcScaled != 0 );
						if ( hdcScaled ) {
							oldObject = SelectObject( ( HDC ) hdcScaled, s_wcd.hbmLogo );
							assert( oldObject != 0 );
							if ( oldObject )
							{
								StretchBlt( ( HDC ) wParam, 0, 0, s_wcd.windowWidth, s_wcd.windowHeight,
									hdcScaled, 0, 0, 512, 384,
									SRCCOPY );
							}
							DeleteDC( hdcScaled );
							hdcScaled = 0;
						}
						return 1;
			*/
		case WM_TIMER:
			if( wParam == 1 )
			{
				s_timePolarity = ( bool )!s_timePolarity;
				if( gConsole.m_hwndErrorBox )
				{
					InvalidateRect( gConsole.m_hwndErrorBox, NULL, FALSE );
				}
			}
			break;
	}
	
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

/*
=====================
crWindowsConsole::InputLineWndProc
=====================
*/
LONG WINAPI crWindowsConsole::InputLineWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int key, cursor;
	switch( uMsg )
	{
		case WM_KILLFOCUS:
			if( ( HWND ) wParam == gConsole.m_hWnd || ( HWND ) wParam == gConsole.m_hwndErrorBox )
			{
				SetFocus( hWnd );
				return 0;
			}
			break;
			
		case WM_KEYDOWN:
			key = ( ( lParam >> 16 ) & 0xFF ) | ( ( ( lParam >> 24 ) & 1 ) << 7 );
			
			// command history
			if( ( key == K_UPARROW ) || ( key == K_KP_8 ) )
			{
				if( gConsole.m_nextHistoryLine - gConsole.m_historyLine < COMMAND_HISTORY && gConsole.m_historyLine > 0 )
					gConsole.m_historyLine--;
				
				gConsole.m_consoleField = gConsole.m_historyEditLines[ gConsole.m_historyLine % COMMAND_HISTORY ];
				
				SetWindowText( gConsole.m_hwndInputLine, gConsole.m_consoleField.GetBuffer() );
				SendMessage( gConsole.m_hwndInputLine, EM_SETSEL, gConsole.m_consoleField.GetCursor(), gConsole.m_consoleField.GetCursor() );
				return 0;
			}
			
			if( ( key == K_DOWNARROW ) || ( key == K_KP_2 ) )
			{
				if( gConsole.m_historyLine == gConsole.m_nextHistoryLine )
					return 0;
				
				gConsole.m_historyLine++;
				gConsole.m_consoleField = gConsole.m_historyEditLines[ gConsole.m_historyLine % COMMAND_HISTORY ];
				
				SetWindowText( gConsole.m_hwndInputLine, gConsole.m_consoleField.GetBuffer() );
				SendMessage( gConsole.m_hwndInputLine, EM_SETSEL, gConsole.m_consoleField.GetCursor(), gConsole.m_consoleField.GetCursor() );
				return 0;
			}
			break;
			
		case WM_CHAR:
			key = ( ( lParam >> 16 ) & 0xFF ) | ( ( ( lParam >> 24 ) & 1 ) << 7 );
			
			GetWindowText( gConsole.m_hwndInputLine, gConsole.m_consoleField.GetBuffer(), MAX_EDIT_LINE );
			SendMessage( gConsole.m_hwndInputLine, EM_GETSEL, ( WPARAM ) NULL, ( LPARAM ) &cursor );
			gConsole.m_consoleField.SetCursor( cursor );
			
			// enter the line
			if( key == K_ENTER || key == K_KP_ENTER )
			{
				std::strncat( gConsole.m_consoleText, gConsole.m_consoleField.GetBuffer(), sizeof( gConsole.m_consoleText ) - std::strlen( gConsole.m_consoleText ) - 5 );
				std::strcat( gConsole.m_consoleText, "\n" );
				SetWindowText( gConsole.m_hwndInputLine, "" );
				
				gConsole.Printf( "]%s\n", gConsole.m_consoleField.GetBuffer() );
				
				// copy line to history buffer
				gConsole.m_historyEditLines[gConsole.m_nextHistoryLine % COMMAND_HISTORY] = gConsole.m_consoleField;
				gConsole.m_nextHistoryLine++;
				gConsole.m_historyLine = gConsole.m_nextHistoryLine;
				
				gConsole.m_consoleField.Clear();
				
				return 0;
			}
			
			// command completion
			if( key == K_TAB )
			{
				gConsole.m_consoleField.AutoComplete();
				
				SetWindowText( gConsole.m_hwndInputLine, gConsole.m_consoleField.GetBuffer() );
				//s_wcd.consoleField.SetWidthInChars( strlen( s_wcd.consoleField.GetBuffer() ) );
				SendMessage( gConsole.m_hwndInputLine, EM_SETSEL, gConsole.m_consoleField.GetCursor(), gConsole.m_consoleField.GetCursor() );
				
				return 0;
			}
			
			// clear autocompletion buffer on normal key input
			if( ( key >= K_SPACE && key <= K_BACKSPACE ) || 
                ( key >= K_KP_SLASH && key <= K_KP_PLUS ) || 
                ( key >= K_KP_STAR && key <= K_KP_EQUALS ) )
			{
				gConsole.m_consoleField.ClearAutoComplete();
			}
			break;
	}
	
	return CallWindowProc( gConsole.m_sysInputLineWndProc, hWnd, uMsg, wParam, lParam );
}

/*
=====================
crWindowsConsole::Shutdown
=====================
*/
void crWindowsConsole::Shutdown(void)
{
	if( m_hWnd )
	{
		ShowWindow( m_hWnd, SW_HIDE );
		CloseWindow( m_hWnd );
		DestroyWindow( m_hWnd );
		m_hWnd = nullptr;
	}
}

/*
=====================
crWindowsConsole::ShowConsole
=====================
*/
void crWindowsConsole::ShowConsole(const int in_visLevel, const bool in_quitOnClose)
{
	m_quitOnClose = in_quitOnClose;
	
	if( !m_hWnd )
		return;
	
	switch( in_visLevel )
	{
		case 0:
			ShowWindow( m_hWnd, SW_HIDE );
			break;
		case 1:
			ShowWindow( m_hWnd, SW_SHOWNORMAL );
			SendMessage( m_hwndBuffer, EM_LINESCROLL, 0, 0xffff );
			break;
		case 2:
			ShowWindow( m_hWnd, SW_MINIMIZE );
			break;
		default:
			Error( "Invalid visLevel %d sent to Sys_ShowConsole\n", in_visLevel );
			break;
	}
}

/*
=====================
crWindowsConsole::ConsoleInput
=====================
*/
const char * crWindowsConsole::ConsoleInput(void)
{
    if( m_consoleText[0] == 0 )
		return nullptr;
	
	std::strcpy( m_returnedText, m_consoleText );
	m_consoleText[0] = 0;
	
	return m_returnedText;
}

/*
=====================
crWindowsConsole::SetFatalError
=====================
*/
void crWindowsConsole::SetFatalError( const char * error )
{
    Error( error );

    // Abre o arquivo no modo "a" (append - adiciona ao final)
    FILE* log_file = fopen("crash_report.txt", "a");
    
    if ( log_file != nullptr ) 
    {
        // Captura e formata a data/hora atual
        time_t now;
        time(&now);
        struct tm* time_info = localtime(&now);
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);
        
        fprintf(log_file, "========================================\n");
        fprintf(log_file, "DATE/TIME: %s\n", time_str);
        fprintf(log_file, "DETAIL:  %s\n", error );
        fprintf(log_file, "========================================\n\n");
        fprintf(log_file, "Console log:  %s\n", m_consoleText );
        
        fclose(log_file);
    } 
    else 
    {
        fprintf( stderr, "Erro fatal: Nao foi possivel abrir o arquivo de log.\n" );
    }
}

/*
=====================
crWindowsConsole::VPrintf
=====================
*/
void crWindowsConsole::VPrintf(const char * fmt, va_list arg)
{
    char msg[MAXPRINTMSG];
	idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, arg );
	msg[ sizeof(msg)-1 ] = '\0';
	OutputDebugString( msg );
}

/*
=====================
crWindowsConsole::VDebug
=====================
*/
void crWindowsConsole::VDebug(const char * fmt, va_list arg )
{
    char msg[MAXPRINTMSG];
	idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, arg );
	msg[ sizeof(msg)-1 ] = '\0';
	OutputDebugString( msg );
}

/*
=====================
crWindowsConsole::VError
=====================
*/
void crWindowsConsole::VError(const char * error, va_list argptr )
{
	va_list		argptr;
	char		text[4096];
    MSG        msg;

	std::vsprintf( text, error, argptr );
	
	AppendText( text );
	AppendText( "\n" );

	SetErrorText( text );
	ShowConsole( 1, true );

    crInputSystem::Get()->Shutdown();
    // Todo clear and release the renderer here 
    crRenderAPI::Get()->ShutDown(); 

	extern idCVar com_productionMode;
	if ( com_productionMode.GetInteger() == 0 ) 
    {
		// wait for the user to quit
		while ( 1 ) 
        {
			if ( !GetMessage( &msg, NULL, 0, 0 ) ) 
            {
				common->Quit();
			}

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	Shutdown();
}

/*
=====================
crWindowsConsole::SetErrorText
=====================
*/
void crWindowsConsole::SetErrorText( const char* buf )
{
	idStr::Copynz( m_errorString, buf, sizeof( m_errorString ) );
	if( !m_hwndErrorBox )
	{
		m_hwndErrorBox = CreateWindow( "static", NULL, WS_CHILD | WS_VISIBLE | SS_SUNKEN,
										   6, 5, 526, 30,
										   m_hWnd,
										   ( HMENU ) ERRORBOX_ID,	// child window ID
										   m_hInstance, NULL );

		SendMessage( m_hwndErrorBox, WM_SETFONT, ( WPARAM ) m_hfBufferFont, 0 );
		SetWindowText( m_hwndErrorBox, m_errorString );
		
		DestroyWindow( m_hwndInputLine );
		m_hwndInputLine = nullptr;
	}
}

/*
=====================
crWindowsConsole::AppendText
=====================
*/
void crWindowsConsole::AppendText(const char * pMsg)
{
	char buffer[CONSOLE_BUFFER_SIZE * 2];
	char* b = buffer;
	const char* msg;
	int bufLen;
	int i = 0;
	static unsigned long s_totalChars;
	
	//
	// if the message is REALLY long, use just the last portion of it
	//
	if( std::strlen( pMsg ) > CONSOLE_BUFFER_SIZE - 1 )
		msg = pMsg + strlen( pMsg ) - CONSOLE_BUFFER_SIZE + 1;
	else
		msg = pMsg;
	
	//
	// copy into an intermediate buffer
	//
	while( msg[i] && ( ( b - buffer ) < sizeof( buffer ) - 1 ) )
	{
		if( msg[i] == '\n' && msg[i + 1] == '\r' )
		{
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
			i++;
		}
		else if( msg[i] == '\r' )
		{
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		}
		else if( msg[i] == '\n' )
		{
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		}
		else if( idStr::IsColor( &msg[i] ) )
		{
			i++;
		}
		else
		{
			*b = msg[i];
			b++;
		}
		i++;
	}
	*b = 0;
	bufLen = b - buffer;
	
	s_totalChars += bufLen;
	
	//
	// replace selection instead of appending if we're overflowing
	//
	if( s_totalChars > 0x7000 )
	{
		SendMessage( m_hwndBuffer, EM_SETSEL, 0, -1 );
		s_totalChars = bufLen;
	}
	
	//
	// put this text into the windows console
	//
	SendMessage( m_hwndBuffer, EM_LINESCROLL, 0, 0xffff );
	SendMessage( m_hwndBuffer, EM_SCROLLCARET, 0, 0 );
	SendMessage( m_hwndBuffer, EM_REPLACESEL, 0, ( LPARAM ) buffer );
}
