//
// NTWindow.CPP
// Copyright Menace Software (www.menasoft.com).
//
// Put up a window for data (other than the console)
//

#ifdef _WIN32
#ifndef VISUAL_SPHERE

#include "graysvr.h"	// predef header.
#include <commctrl.h>	// NM_RCLICK
#include "resource.h"
#include "../common/cwindow.h"
#include "../common/grayver.h"	// sphere version
// #include "windowsx.h"

#define WM_USER_POST_MSG		(WM_USER+10)
#define WM_USER_TRAY_NOTIFY		(WM_USER+12)
#define IDC_M_LOG	10
#define IDC_M_INPUT 11
#define IDT_ONTICK	1

class CAboutDlg : public CDialogBase
{
private:
	bool OnInitDialog();
	bool OnCommand( WORD wNotifyCode, int wID, HWND hwndCtl );
public:
	virtual BOOL DefDialogProc( UINT message, WPARAM wParam, LPARAM lParam );
};

class COptionsDlg : public CDialogBase
{
private:
	bool OnInitDialog();
	bool OnCommand( WORD wNotifyCode, int wID, HWND hwndCtl );
public:
	virtual BOOL DefDialogProc( UINT message, WPARAM wParam, LPARAM lParam );
};

class CListTextConsole : public CTextConsole
{
	CListbox m_wndList;
public:
	CListTextConsole( HWND hWndList )
	{
		m_wndList.m_hWnd = hWndList;
	}
	~CListTextConsole()
	{
		m_wndList.OnDestroy();
	}
	virtual PLEVEL_TYPE GetPrivLevel() const
	{
		return PLEVEL_Admin;
	}
	virtual LPCTSTR GetName() const
	{
		return( _TEXT("Stats"));
	}
	virtual void SysMessage( LPCTSTR pszMessage ) const
	{
		if ( pszMessage == NULL || ISINTRESOURCE(pszMessage))
			return;
		m_wndList.AddString( pszMessage );
	}
};

class CStatusWnd : public CDialogBase
{
	CListbox m_wndListClients;
	CListbox m_wndListStats;
private:
	bool OnInitDialog();
	bool OnCommand( WORD wNotifyCode, int wID, HWND hwndCtl );
public:
	void FillClients();
	void FillStats();
	virtual BOOL DefDialogProc( UINT message, WPARAM wParam, LPARAM lParam );
};

class CNTWindow : public CWindow
{
	// Create a dialog to do stuff.
public:
	COLORREF		m_dwColorNew;	// setthe color for the next block written.
	COLORREF		m_dwColorPrv;
	CRichEditCtrl	m_wndLog;
	int				m_iLogTextLen;

	CEdit			m_wndInput;		// the text input portion at the bottom.
	int				m_iHeightInput;
   	
   	HFONT			m_hLogFont;

	bool m_fLogScrollLock;	// lock with the rolling text ?

private:
	int OnCreate( HWND hWnd, LPCREATESTRUCT lParam );
	bool OnSysCommand( UINT uCmdType, int xPos, int yPos );
	void OnSize( UINT nType, int cx, int cy );
	void OnDestroy();
	void OnSetFocus( HWND hWndLoss );
	bool OnClose();
	void OnUserPostMessage( COLORREF color, CGString * psMsg );
	LRESULT OnUserTrayNotify( WPARAM wID, LPARAM lEvent );
	LRESULT OnNotify( int idCtrl, NMHDR * pnmh );
	void	SetLogFont( const char * pszFont );

public:
	bool OnCommand( WORD wNotifyCode, int wID, HWND hwndCtl );

	static bool RegisterClass();
	static LRESULT WINAPI WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

	void List_Clear();
	void List_Add( COLORREF color, LPCTSTR pszText );

	CNTWindow();
	virtual ~CNTWindow();

	char	m_zCommands[5][256];
};

class CNTApp : public CWinApp
{
public:
	CNTWindow m_wndMain;	// m_pMainWnd
	CStatusWnd m_wndStatus;
	COptionsDlg m_dlgOptions;
};

CNTApp theApp;

//************************************
// -CAboutDlg

bool CAboutDlg::OnInitDialog()
{
	SetDlgItemText( IDC_ABOUT_VERSION, EVO_TITLE " Version " GRAY_VERSION );
	return( false );
}

bool CAboutDlg::OnCommand( WORD wNotifyCode, int wID, HWND hwndCtl )
{
	// WM_COMMAND
	switch ( wID )
	{
	case IDOK:
	case IDCANCEL:
		EndDialog( m_hWnd, wID );
		break;
	}
	return( TRUE );
}

BOOL CAboutDlg::DefDialogProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	switch ( message )
	{
	case WM_INITDIALOG:
		return( OnInitDialog());
	case WM_COMMAND:
		return( OnCommand(  HIWORD(wParam), LOWORD(wParam), (HWND) lParam ));
	case WM_DESTROY:
		OnDestroy();
		return( TRUE );
	}
	return( FALSE );
}

//************************************
// -COptionsDlg

bool COptionsDlg::OnInitDialog()
{
	return( false );
}

bool COptionsDlg::OnCommand( WORD wNotifyCode, int wID, HWND hwndCtl)
{
	// WM_COMMAND
	switch ( wID )
	{
	case IDOK:
	case IDCANCEL:
		DestroyWindow();
		break;
	}
	return( FALSE );
}

BOOL COptionsDlg::DefDialogProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	switch ( message )
	{
	case WM_INITDIALOG:
		return( OnInitDialog());
	case WM_COMMAND:
		return( OnCommand(  HIWORD(wParam), LOWORD(wParam), (HWND) lParam ));
	case WM_DESTROY:
		OnDestroy();
		return( TRUE );
	}
	return( FALSE );
}

//************************************
// -CStatusWnd

void CStatusWnd::FillClients()
{
	if ( m_wndListClients.m_hWnd == NULL )
		return;
	m_wndListClients.ResetContent();
	CListTextConsole capture( m_wndListClients.m_hWnd );
	g_Serv.ListClients( &capture );
	int iCount = m_wndListClients.GetCount();
	iCount++;
}

void CStatusWnd::FillStats()
{
	if ( m_wndListStats.m_hWnd == NULL )
		return;
	m_wndListStats.ResetContent();

	CListTextConsole capture( m_wndListStats.m_hWnd );
	for ( int i=0; i < PROFILE_QTY; i++ )
	{
		capture.SysMessagef( "'%s' = %s\n", g_Serv.m_Profile.GetName((PROFILE_TYPE) i), g_Serv.m_Profile.GetDesc((PROFILE_TYPE) i ));
	}

}

bool CStatusWnd::OnInitDialog()
{
	m_wndListClients.m_hWnd = GetDlgItem(IDC_STAT_CLIENTS);
	FillClients();
	m_wndListStats.m_hWnd = GetDlgItem(IDC_STAT_STATS);
	FillStats();
	return( false );	
}

bool CStatusWnd::OnCommand( WORD wNotifyCode, int wID, HWND hwndCtl )
{
	// WM_COMMAND
	switch ( wID )
	{
	case IDOK:
	case IDCANCEL:
		DestroyWindow();
		break;
	}
	return( FALSE );
}

BOOL CStatusWnd::DefDialogProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	// IDM_STATUS
	switch ( message )
	{
	case WM_INITDIALOG:
		return( OnInitDialog());
	case WM_COMMAND:
		return( OnCommand( HIWORD(wParam), LOWORD(wParam), (HWND) lParam ));
	case WM_DESTROY:
		m_wndListClients.OnDestroy();
		m_wndListStats.OnDestroy();
		OnDestroy();
		return( TRUE );
	}
	return( FALSE );
}

//************************************
// -CNTWindow

CNTWindow::CNTWindow()
{
	m_iLogTextLen		= 0;
	m_fLogScrollLock	= false;
	m_dwColorNew		= RGB( 0xaf,0xaf,0xaf );
	m_dwColorPrv		= RGB( 0xaf,0xaf,0xaf );
	m_iHeightInput		= 0;
   	m_hLogFont			= NULL;
	memset(m_zCommands, 0, sizeof(m_zCommands));
}

CNTWindow::~CNTWindow()
{
	DestroyWindow();
}

void CNTWindow::List_Clear()
{
	m_wndLog.SetWindowText( "");
	m_wndLog.SetSel( 0, 0 );
	m_iLogTextLen = 0;
}

void CNTWindow::List_Add( COLORREF color, LPCTSTR pszText )
{
	int iTextLen = strlen( pszText );
	int iNewLen = m_iLogTextLen + iTextLen;

	if ( iNewLen > (32*1024) )
	{
		int iCut = iNewLen - (32*1024);
		m_wndLog.SetSel( 0, iCut );
		m_wndLog.ReplaceSel( "" );
		m_iLogTextLen = (32*1024);
	}

	m_wndLog.SetSel( m_iLogTextLen, m_iLogTextLen );

	// set the blocks color.
	CHARFORMAT cf;
	memset( &cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = color; 
	BOOL fRet = m_wndLog.SetSelectionCharFormat( cf );

	m_wndLog.ReplaceSel( pszText );

	m_iLogTextLen += iTextLen;
	m_wndLog.SetSel( m_iLogTextLen, m_iLogTextLen );

	int iSelBegin;
	int iSelEnd;
	m_wndLog.GetSel( iSelBegin, iSelEnd );
	//ASSERT( iSelEnd == iSelBegin );
	//ASSERT( m_iLogTextLen == iSelBegin );
	m_iLogTextLen = iSelBegin;	// make sure it's correct.

	// If the select is on screen then keep scrolling.
	if ( ! m_fLogScrollLock && ! GetCapture())
	{
		if ( GRAY_GetOSInfo()->dwPlatformId == VER_PLATFORM_WIN32_NT )
		{
			m_wndLog.Scroll();
		}
	}
}

bool CNTWindow::RegisterClass()	// static
{
	WNDCLASS wc;
	memset( &wc, 0, sizeof(wc));

	wc.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = theApp.m_hInstance;
	wc.hIcon = theApp.LoadIcon( IDR_MAINFRAME );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH) NULL;
	wc.lpszMenuName = NULL; // MAKEINTRESOURCE( 1 );
	wc.lpszClassName = GRAY_TITLE "Svr";

	ATOM frc = ::RegisterClass( &wc );
	if ( !frc )
	{
		return( false );
	}

    HMODULE hMod = LoadLibrary("Riched20.dll"); // Load the RichEdit DLL to activate the class
	DEBUG_CHECK(hMod);
	return( true );
}

//****************************

int CNTWindow::OnCreate( HWND hWnd, LPCREATESTRUCT lParam )
{
	CWindow::OnCreate(hWnd);

	m_wndLog.m_hWnd = ::CreateWindow( RICHEDIT_CLASS, NULL,
		ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY | // ES_OEMCONVERT | 
		WS_CHILD|WS_VISIBLE|WS_VSCROLL,
		0, 0, 10, 10, 
		m_hWnd,
		(HMENU)(UINT) IDC_M_LOG, theApp.m_hInstance, NULL );
	ASSERT( m_wndLog.m_hWnd );


	SetLogFont( "Courier" );
    
	// TEXTMODE
	// m_wndLog.SetTextMode( TM_RICHTEXT );	// |TM_MULTICODEPAGE.. 
	m_wndLog.SetBackgroundColor( false, RGB(0,0,0));
	CHARFORMAT cf;
	memset( &cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = m_dwColorPrv;
	cf.bCharSet = ANSI_CHARSET;
	cf.bPitchAndFamily = FF_MODERN | FIXED_PITCH;
	m_wndLog.SetDefaultCharFormat( cf );
	m_wndLog.SetAutoUrlDetect( true );
	m_wndLog.SetEventMask( ENM_LINK | ENM_MOUSEEVENTS | ENM_KEYEVENTS );

	m_wndInput.m_hWnd = ::CreateWindow( _TEXT( "EDIT" ), NULL,
		ES_LEFT | ES_AUTOHSCROLL |	// | ES_OEMCONVERT
		WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP,
		0, 0, 10, 10, 
		m_hWnd,
		(HMENU)(UINT) IDC_M_INPUT, theApp.m_hInstance, NULL );
	ASSERT( m_wndInput.m_hWnd );

	if ( GRAY_GetOSInfo()->dwPlatformId > VER_PLATFORM_WIN32s )
	{
		NOTIFYICONDATA pnid;
		memset(&pnid,0,sizeof(pnid));
		pnid.cbSize = sizeof(NOTIFYICONDATA);
		pnid.hWnd   = m_hWnd;
		pnid.uFlags = NIF_TIP | NIF_ICON | NIF_MESSAGE;
		pnid.uCallbackMessage = WM_USER_TRAY_NOTIFY;
		pnid.hIcon  = theApp.LoadIcon( IDR_MAINFRAME );
		strcpy( pnid.szTip, theApp.m_pszAppName );
		Shell_NotifyIcon(NIM_ADD, &pnid);
	}

	NTWindow_SetWindowTitle();

	return( 0 );
}

void CNTWindow::OnDestroy()
{
	if ( GRAY_GetOSInfo()->dwPlatformId > VER_PLATFORM_WIN32s )
	{
	    NOTIFYICONDATA pnid;
		memset(&pnid,0,sizeof(pnid));
		pnid.cbSize = sizeof(NOTIFYICONDATA);
		pnid.hWnd   = m_hWnd;
		Shell_NotifyIcon(NIM_DELETE, &pnid);
	}

	m_wndLog.OnDestroy();	// these are automatic.
	m_wndInput.OnDestroy();
	CWindow::OnDestroy();
}

void CNTWindow::OnSetFocus( HWND hWndLoss )
{
	m_wndInput.SetFocus();
}

LRESULT CNTWindow::OnUserTrayNotify( WPARAM wID, LPARAM lEvent )
{
	// WM_USER_TRAY_NOTIFY
	switch ( lEvent )
	{
	case WM_MOUSEMOVE:
		return 0;
	case WM_RBUTTONDOWN:
		// Context menu ?
		{
			HMENU hMenu = theApp.LoadMenu( IDM_POP_TRAY );
			if ( hMenu == NULL )
				break;
			HMENU hMenuPop = GetSubMenu(hMenu,0);
			if ( hMenuPop ) 
			{
				POINT point;
				if ( GetCursorPos( &point ))
				{
					TrackPopupMenu( hMenuPop, TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL );
				}
			}
			DestroyMenu( hMenu );
		}
		return 1;
	case WM_LBUTTONDBLCLK:
	    ShowWindow(SW_NORMAL);	
		SetForegroundWindow();
	    return 1; // handled
	}
	return 0;	// not handled.
}

void CNTWindow::OnUserPostMessage( COLORREF color, CGString * psMsg )
{
	// WM_USER_POST_MSG
	ASSERT(psMsg);
	List_Add( color, *psMsg );
	delete psMsg;
}

void CNTWindow::OnSize( UINT nType, int cx, int cy )
{
	if ( nType != SIZE_MINIMIZED && nType != SIZE_MAXHIDE && m_wndLog.m_hWnd )
	{
		if ( ! m_iHeightInput )
		{
			HFONT hFont = GetFont();
			if ( hFont == NULL )
				hFont = (HFONT) GetStockObject(SYSTEM_FONT);
			ASSERT(hFont);
			LOGFONT logfont;
			int iRet = ::GetObject(hFont, sizeof(logfont),&logfont );
			ASSERT(iRet==sizeof(logfont));
			m_iHeightInput = abs( logfont.lfHeight );
			ASSERT(m_iHeightInput);
		}

		m_wndLog.MoveWindow( 0, 0, cx, cy-m_iHeightInput, TRUE );
		m_wndInput.MoveWindow( 0, cy-m_iHeightInput, cx, m_iHeightInput, TRUE );
	}
}

bool CNTWindow::OnClose()
{
	// WM_CLOSE
	ASSERT( CThread::GetCurrentThreadId() == g_Serv.m_dwParentThread );

	if ( g_Serv.m_iExitFlag == 0 )
	{
		int iRet = theApp.m_wndMain.MessageBox( _TEXT("Are you sure you want to close the server?"),
			theApp.m_pszAppName, MB_YESNO|MB_ICONQUESTION );
		if ( iRet == IDNO )
			return( false );
	}

	PostQuitMessage(0);
	g_Serv.SetExitFlag( 5 );
	return( true );	// ok to close.
}

bool CNTWindow::OnCommand( WORD wNotifyCode, int wID, HWND hwndCtl )
{
	// WM_COMMAND

	switch ( wID )
	{
	case IDC_M_LOG:
		break;
	case IDM_STATUS:
		if ( theApp.m_wndStatus.m_hWnd == NULL )
		{
			theApp.m_wndStatus.m_hWnd = ::CreateDialogParam( 
				theApp.m_hInstance,
				MAKEINTRESOURCE(IDM_STATUS), 
				HWND_DESKTOP, 
				CDialogBase::DialogProc, 
				(DWORD) static_cast <CDialogBase*>(&theApp.m_wndStatus) );
		}
		theApp.m_wndStatus.ShowWindow(SW_NORMAL);	
		theApp.m_wndStatus.SetForegroundWindow();
		break;
	case IDR_ABOUT_BOX:
		{
			CAboutDlg wndAbout;
			int iRet = DialogBoxParam(
				theApp.m_hInstance,  // handle to application instance
				MAKEINTRESOURCE(IDR_ABOUT_BOX),   // identifies dialog box template
				m_hWnd,      // handle to owner window
				CDialogBase::DialogProc, 
				(DWORD) static_cast <CDialogBase*>(&wndAbout) );  // pointer to dialog box procedure
		}
		break;

	case IDM_MINIMIZE:
		// SC_MINIMIZE
	    ShowWindow(SW_HIDE);	
		break;
	case IDM_RESTORE:
		// SC_RESTORE
	    ShowWindow(SW_NORMAL);	
		SetForegroundWindow();
		break;
	case IDM_EXIT:
		PostMessage( WM_CLOSE );
		break;

	case IDM_RESYNC_PAUSE:
		if ( ! g_Serv.m_fConsoleTextReadyFlag )	// busy ?
		{
			g_Serv.m_sConsoleText = "R";
			g_Serv.m_fConsoleTextReadyFlag = true;
			return( true );
		}
		return( false );

	case IDM_EDIT_COPY:
		m_wndLog.SendMessage( WM_COPY );
		break;

	case IDOK:
		// We just entered the text.

		if ( ! g_Serv.m_fConsoleTextReadyFlag )	// busy ?
		{
			TCHAR szTmp[ MAX_TALK_BUFFER ];
			m_wndInput.GetWindowText( szTmp, sizeof(szTmp));
			strcpy(m_zCommands[4], m_zCommands[3]);
			strcpy(m_zCommands[3], m_zCommands[2]);
			strcpy(m_zCommands[2], m_zCommands[1]);
			strcpy(m_zCommands[1], m_zCommands[0]);
			strcpy(m_zCommands[0], szTmp);
			m_wndInput.SetWindowText("");
			g_Serv.m_sConsoleText = szTmp;
			g_Serv.m_fConsoleTextReadyFlag = true;
			return( true );
		}
		return( false );
	}
	return( true );
}

bool CNTWindow::OnSysCommand( UINT uCmdType, int xPos, int yPos )
{
	// WM_SYSCOMMAND
	// return : 1 = i processed this.
	switch ( uCmdType )
	{
	case SC_MINIMIZE:
		if ( GRAY_GetOSInfo()->dwPlatformId > VER_PLATFORM_WIN32s )
		{
		    ShowWindow(SW_HIDE);	
			return( true );
		}
		break;
	}
	return( false );	
}

void	CNTWindow::SetLogFont( const char * pszFont )
{
	// use an even spaced font
	if ( pszFont == NULL )
	{
		m_hLogFont	= (HFONT) GetStockObject(SYSTEM_FONT);
	}
	else
	{
		LOGFONT		logfont;
   		memset( &logfont, 0, sizeof(logfont) );
   		strcpy( logfont.lfFaceName, pszFont );
   		m_hLogFont = CreateFontIndirect( &logfont );		
	}
   	m_wndLog.SetFont( m_hLogFont, true );
}	


LRESULT CNTWindow::OnNotify( int idCtrl, NMHDR * pnmh )
{
	ASSERT(pnmh);
	if ( idCtrl == IDC_M_LOG )
	{
		if ( pnmh->code == EN_LINK )	// ENM_LINK
		{
			ENLINK * pLink = (ENLINK *)(pnmh);
			if ( pLink->msg == WM_LBUTTONDOWN )
			{
				// Open the web page.
				return 1;
			}
		}
		else if ( pnmh->code == EN_MSGFILTER )	// ENM_MOUSEEVENTS
		{
			MSGFILTER * pMsg = (MSGFILTER *)(pnmh);
			ASSERT(pMsg);
			if ( pMsg->msg == WM_MOUSEMOVE )
				return 0;
			if ( pMsg->msg == WM_RBUTTONDOWN )
			{
				HMENU hMenu = theApp.LoadMenu( IDM_POP_LOG );
				if ( hMenu == NULL )
					return 0;
				HMENU hMenuPop = GetSubMenu(hMenu,0);
				if ( hMenuPop ) 
				{
					POINT point;
					if ( GetCursorPos( &point ))
					{
						TrackPopupMenu( hMenuPop, TPM_RIGHTBUTTON, point.x, point.y, 0, m_hWnd, NULL );
					}
				}
				DestroyMenu( hMenu );
				return 1;
			}
			if ( pMsg->msg == WM_CHAR )	// ENM_KEYEVENTS
			{
				// We normally have no business typing into this window.
				// Should we allow CTL C etc ?
				if ( pMsg->lParam & (1<<29))	// ALT
					return 0;
				SHORT sState = GetKeyState( VK_CONTROL );
				if ( sState & 0xff00 )
					return 0;
				m_wndInput.SetFocus();
				m_wndInput.PostMessage( WM_CHAR, pMsg->wParam, pMsg->lParam );
				return 1;	// mostly ignored.
			}
		}
	}

	return 0;	// mostly ignored.
}

LRESULT WINAPI CNTWindow::WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )	// static
{
#ifndef _DEBUG
	try
	{
#endif

	switch( message )
	{
	case WM_CREATE:
		return( theApp.m_wndMain.OnCreate( hWnd, (LPCREATESTRUCT) lParam ));
	case WM_SYSCOMMAND:
		if ( theApp.m_wndMain.OnSysCommand( wParam&~0x0f, LOWORD(lParam), HIWORD(lParam)))
			return( 0 );
		break;
	case WM_COMMAND:
		if ( theApp.m_wndMain.OnCommand( HIWORD(wParam), LOWORD(wParam), (HWND) lParam ))
			return( 0 );
		break;
	case WM_CLOSE:
		if ( ! theApp.m_wndMain.OnClose())
			return( false );
		break;
	case WM_ERASEBKGND:	// don't bother with this.
		return 1;
	case WM_SIZE:	// get the client rectangle
		theApp.m_wndMain.OnSize( wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_DESTROY:
		theApp.m_wndMain.OnDestroy();
		return 0;
	case WM_SETFOCUS:
		theApp.m_wndMain.OnSetFocus( (HWND) wParam ); 
		return 0;
	case WM_NOTIFY:
		theApp.m_wndMain.OnNotify( (int) wParam, (NMHDR *) lParam );
		return 0;
	case WM_USER_POST_MSG:
		theApp.m_wndMain.OnUserPostMessage( (COLORREF) wParam, (CGString*) lParam );
		return 1;
	case WM_USER_TRAY_NOTIFY:
		return theApp.m_wndMain.OnUserTrayNotify( wParam, lParam );
	}
#ifndef _DEBUG
	}
	catch (CGrayError &e)
	{
		g_Log.CatchEvent( &e, "Window" );
	}
	catch (...)	// catch all
	{
		g_Log.CatchEvent( NULL, "Window" );
	}
#endif

	return ::DefWindowProc( hWnd, message, wParam, lParam);
}

//************************************

bool NTWindow_Init( HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow )
{
	ASSERT(!g_Cfg.m_fUseNTService);
	theApp.InitInstance( EVO_TITLE " Server V" GRAY_VERSION, hInstance, lpCmdLine );
	CNTWindow::RegisterClass();

	theApp.m_wndMain.m_hWnd = ::CreateWindow( 
		GRAY_TITLE "Svr", // registered class name
		EVO_TITLE " V" GRAY_VERSION, // window name
		WS_OVERLAPPEDWINDOW,   // window style
		CW_USEDEFAULT,  // horizontal position of window
		CW_USEDEFAULT,  // vertical position of window
		CW_USEDEFAULT,  // window width
		CW_USEDEFAULT,	// window height
		HWND_DESKTOP,      // handle to parent or owner window
		NULL,          // menu handle or child identifier
		theApp.m_hInstance,  // handle to application instance
		NULL        // window-creation data
		);

	theApp.m_wndMain.ShowWindow( nCmdShow );
	return( true );
}

void NTWindow_Exit()
{
	// Unattach the window.
	ASSERT( CThread::GetCurrentThreadId() == g_Serv.m_dwParentThread );

	// theApp.m_wndMain.DetachThread();

	if ( g_Serv.m_iExitFlag < 0 )
	{
		CGString sMsg;
		sMsg.Format( _TEXT("Server terminated by error %d!"), g_Serv.m_iExitFlag );
		int iRet = theApp.m_wndMain.MessageBox( sMsg, theApp.m_pszAppName, MB_OK|MB_ICONEXCLAMATION );
		// just sit here for a bit til the user wants to close the window.
		while ( NTWindow_OnTick( 500 ))
		{
		}
	}
}

void NTWindow_SetWindowTitle( LPCTSTR pszText )
{
	if ( theApp.m_wndMain.m_hWnd == NULL )
		return;
	// set the title to reflect mode.

	LPCTSTR pszMode;
	switch ( g_Serv.m_iModeCode )
	{
	case SERVMODE_RestockAll:	// Major event.
		pszMode = "Restocking";
		break;
	case SERVMODE_Saving:		// Forced save freezes the system.
		pszMode = "Saving";
		break;
	case SERVMODE_ScriptBook:	// Run at Lower Priv Level
	case SERVMODE_Run:			// Game is up and running
		pszMode = "Running";
		break;
	case SERVMODE_Loading:		// Initial load.
		pszMode = "Loading";
		break;
	case SERVMODE_ResyncPause:
		pszMode = "Resync Pause";
		break;
	case SERVMODE_ResyncLoad:	// Loading after resync
		pszMode = "Resync Load";
		break;
	case SERVMODE_Exiting:		// Closing down
		pszMode = "Exiting";
		break;
	case SERVMODE_Test5:			// Sometest modes.
	case SERVMODE_Test8:			// Sometest modes.
		pszMode = "Testing";
		break;
	default:
		pszMode = "Unknown";
		break;
	}

	// Number of connections ?

	char *psTitle = Str_GetTemp();
	sprintf(psTitle, _TEXT("%s - %s (%s) %s"), theApp.m_pszAppName, g_Serv.GetName(), pszMode, pszText ? pszText : "" );
	theApp.m_wndMain.SetWindowText( psTitle );

	if ( GRAY_GetOSInfo()->dwPlatformId > VER_PLATFORM_WIN32s )
	{
		NOTIFYICONDATA pnid;
		memset(&pnid,0,sizeof(pnid));
		pnid.cbSize = sizeof(NOTIFYICONDATA);
		pnid.hWnd   = theApp.m_wndMain.m_hWnd;
		pnid.uFlags = NIF_TIP;
		strcpy( pnid.szTip, psTitle );
		Shell_NotifyIcon(NIM_MODIFY, &pnid);
	}
}

bool NTWindow_PostMsgColor( COLORREF color )
{
	// Set the color for the next text.
	if ( theApp.m_wndMain.m_hWnd == NULL )
		return( false );

	if ( ! color )
	{
		// set to default color.
		color = theApp.m_wndMain.m_dwColorPrv;
	}

	theApp.m_wndMain.m_dwColorNew = color;
	return( true );
}

bool NTWindow_PostMsg( LPCTSTR pszMsg )
{
	// Post a message to print out on the main display.
	// If we use AttachThreadInput we don't need to post ?
	// RETURN:
	//  false = post did not work.

	if ( theApp.m_wndMain.m_hWnd == NULL )
		return( false );

	COLORREF color = theApp.m_wndMain.m_dwColorNew;

	if ( g_Serv.m_dwParentThread != CThread::GetCurrentThreadId())
	{
		// A thread safe way to do text.
		CGString * psMsg = new CGString( pszMsg );
		ASSERT(psMsg);
		if ( ! theApp.m_wndMain.PostMessage( WM_USER_POST_MSG, (WPARAM) color, (LPARAM)psMsg ))
		{
			delete psMsg;
			return( false );
		}
	}
	else
	{
		// just add it now.
		theApp.m_wndMain.List_Add( color, pszMsg );
	}

	return( true );
}

bool NTWindow_OnTick( int iWaitmSec )
{
	// RETURN: false = exit the app.

	ASSERT( CThread::GetCurrentThreadId() == g_Serv.m_dwParentThread );

	if ( iWaitmSec )
	{
		if ( theApp.m_wndMain.m_hWnd )
		{
			UINT uRet = theApp.m_wndMain.SetTimer( IDT_ONTICK, iWaitmSec );
			if ( ! uRet )
			{
				iWaitmSec = 0;
			}
		}
		else
		{
			iWaitmSec = 0;
		}
	}

	// Give the windows message loops a tick.
	while ( true )
	{
#if !defined( _DEBUG ) && !defined( NO_INTERNAL_EXCEPTIONS )
		try
		{
#endif
			MSG msg;

			// any windows messages ? (blocks until a message arrives)
			if ( iWaitmSec )
			{
				if ( ! GetMessage( &msg, NULL, 0, 0 ))
				{
					g_Serv.SetExitFlag( 5 );
					return( false );
				}
				if ( msg.hwnd == theApp.m_wndMain.m_hWnd && 
					msg.message == WM_TIMER && 
					msg.wParam == IDT_ONTICK )
				{
					theApp.m_wndMain.KillTimer( IDT_ONTICK );
					iWaitmSec = 0;	// empty the queue and bail out.
					continue;
				}
			}
			else
			{
				if (! PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
					return true;
				if ( msg.message == WM_QUIT )
				{
					g_Serv.SetExitFlag( 5 );
					return( false );
				}
			}

			//	Got char in edit box
			if ( theApp.m_wndMain.m_wndInput.m_hWnd && 
				msg.hwnd == theApp.m_wndMain.m_wndInput.m_hWnd )
			{
				if ( msg.message == WM_CHAR )	//	char to edit box
				{
					if ( msg.wParam == '\r' )	//	ENTER
						if ( theApp.m_wndMain.OnCommand( 0, IDOK, msg.hwnd ))
							return(true);
				}
				else if ( msg.message == WM_KEYUP )	//	key released
				{
					if ( msg.wParam == VK_UP )	//	UP
					{
						theApp.m_wndMain.m_wndInput.SetWindowText(theApp.m_wndMain.m_zCommands[0]);
						strcpy(theApp.m_wndMain.m_zCommands[0], theApp.m_wndMain.m_zCommands[1]);
						strcpy(theApp.m_wndMain.m_zCommands[1], theApp.m_wndMain.m_zCommands[2]);
						strcpy(theApp.m_wndMain.m_zCommands[2], theApp.m_wndMain.m_zCommands[3]);
						strcpy(theApp.m_wndMain.m_zCommands[3], theApp.m_wndMain.m_zCommands[4]);
						theApp.m_wndMain.m_wndInput.GetWindowText(theApp.m_wndMain.m_zCommands[4], sizeof(theApp.m_wndMain.m_zCommands[4]));
					}
				}
			}
			if ( theApp.m_dlgOptions.m_hWnd && 
				IsDialogMessage( theApp.m_dlgOptions.m_hWnd, &msg ))
			{
				return( true );
			}
		
			TranslateMessage(&msg);
			DispatchMessage(&msg);
#if !defined( _DEBUG ) && !defined( NO_INTERNAL_EXCEPTIONS )
		}
		catch ( CGrayError &e )
		{
			g_Log.CatchEvent( &e, "Window OnTick" );
		}
		catch (...)	// catch all
		{
			g_Log.CatchEvent( NULL, "Window OnTick" );
		}
#endif

	}

	return true;
}

#endif // VISUAL_SPHERE
#endif // _WIN32
