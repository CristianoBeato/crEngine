
#ifndef __WINDOWS_CONSOLE_HPP__
#define __WINDOWS_CONSOLE_HPP__

#include "platform/Platform.hpp"

class crWindowsConsole : public crConsole
{
public:
    crWindowsConsole( void );
    ~crWindowsConsole( void );
	virtual void			Startup( void );
	virtual void			Shutdown( void );
	virtual void			ShowConsole( const int in_visLevel, const bool in_quitOnClose );
	virtual const char* 	ConsoleInput( void );
    virtual void			SetFatalError( const char* error );
	virtual void            VPrintf( const char *fmt, va_list arg );
    virtual void            VDebug( const char *fmt, va_list arg );
    virtual void            VError( const char *fmt, va_list arg ); 

private:
    bool		m_quitOnClose;
    int			m_windowWidth; 
    int         m_windowHeight;
    int         m_nextHistoryLine;  // the last line in the history buffer, not masked
	int         m_historyLine;      // the line being displayed from history buffer will be <= nextHistoryLine

    HWND		m_hWnd;             // console window handle
    HWND		m_hwndBuffer;       // console log scroll handle
    
    HWND		m_hwndInputLine;    // comand input line
    
    HWND		m_hwndButtonClear;  // clear button handle
	HWND		m_hwndButtonCopy;   // copy button handle
	HWND		m_hwndButtonQuit;   // quit button handle
    
    HWND		m_hwndErrorBox;     // Error text box
	HWND		m_hwndErrorText;    // Error text 

    HFONT		m_hfBufferFont;
    HINSTANCE   m_hInstance;

    HBRUSH		m_hbrEditBackground;
    HBRUSH		m_hbrErrorBackground;

    WNDPROC		m_sysInputLineWndProc;

    char		m_consoleText[512];
    char        m_returnedText[512];
    char		m_errorString[80];

    idEditField	m_historyEditLines[COMMAND_HISTORY];
    idEditField	m_consoleField;

    static LONG_PTR WINAPI ConWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static LONG_PTR WINAPI InputLineWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    // Show the early console as an error dialog
    void    SetErrorText( const char* buf ); 
    void    AppendText( const char* pMsg );
};

#endif //!__WINDOWS_CONSOLE_HPP__