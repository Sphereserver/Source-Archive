//
// CWindow = a base window class for controls.
// Copyright Menace Software (www.menasoft.com).
//

#ifndef _INC_CWINDOW_H
#define _INC_CWINDOW_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
#ifdef _AFXDLL
#error This is not compatible with MFC AFX
#endif

class CWindow    // similar to Std MFC class CWnd
{
public:
	HWND m_hWnd;
public:
	operator HWND () const       // cast as a HWND
	{
		return( m_hWnd );
	}
	CWindow()
	{
		m_hWnd = NULL;
	}
	~CWindow()
	{
		DestroyWindow();
	}

	// Standard message handlers.
	BOOL OnCreate( HWND hwnd, LPCREATESTRUCT lpCreateStruct = NULL )
	{
		m_hWnd = hwnd;
		return( TRUE );
	}
	void OnDestroy()
	{
		m_hWnd = NULL;
	}
	void OnDestroy( HWND hwnd )
	{
		m_hWnd = NULL;
	}

	// Standard windows commands.
	BOOL IsWindow( void ) const
	{
		if ( this == NULL ) 
			return( FALSE );
		if ( m_hWnd == NULL ) 
			return( false );
		return( ::IsWindow( m_hWnd ));
	}
	BOOL IsWindowVisible( void ) const
	{
		return( ::IsWindowVisible( m_hWnd ));
	}
	BOOL IsZoomed( void ) const
	{
		return( ::IsZoomed( m_hWnd ));
	}
	BOOL IsIconic( void ) const
	{
		return( ::IsIconic( m_hWnd ));
	}
	HDC GetDC( void ) const
	{
		return( ::GetDC( m_hWnd ));
	}
	HMENU GetMenu( void ) const
	{
		return( (HMENU) ::GetMenu( m_hWnd ));
	}
	void ReleaseDC( HDC hDC ) const
	{
		::ReleaseDC( m_hWnd, hDC );
	}
	void ValidateRect( RECT * pRect = NULL ) const
	{
		if ( m_hWnd == NULL )
			return;
		::ValidateRect( m_hWnd, pRect );
	}
	void InvalidateRect( RECT * pRect = NULL, BOOL fErase = FALSE ) const
	{
		if ( m_hWnd == NULL )
			return;
		::InvalidateRect( m_hWnd, pRect, fErase );
	}
	void InvalidateRect( int sx, int sy, int iWidth, int iHeight ) const
	{
		RECT rect;
		rect.left = sx;
		rect.top = sy;
		rect.right = sx + iWidth;
		rect.bottom = sy + iHeight;
		InvalidateRect( &rect );
	}
	void GetClientRect( LPRECT pRect ) const
	{
		ASSERT( m_hWnd );
		::GetClientRect( m_hWnd, pRect );
	}
	void GetWindowRect( LPRECT pRect ) const
	{
		ASSERT( m_hWnd );
		::GetWindowRect( m_hWnd, pRect );
	}
	void DestroyWindow( void )
	{
		if ( m_hWnd == NULL ) 
			return;
		::DestroyWindow( m_hWnd );
		ASSERT( m_hWnd == NULL );
	}
	BOOL SetWindowText( LPCSTR lpszText )
	{
		ASSERT( m_hWnd );
		return ::SetWindowText( m_hWnd, lpszText );
	}
	int MessageBox( LPCSTR lpszText, LPCSTR lpszTitle, UINT fuStyle = MB_OK	) const
	{
		// ASSERT( m_hWnd );
		return( ::MessageBox( m_hWnd, lpszText, lpszTitle, fuStyle ));
	}
	HWND GetDlgItem( int id ) const
	{
		return( ::GetDlgItem( m_hWnd, id ));
	}
	LONG SendDlgItemMessage( int nIDDlgItem, UINT Msg, WPARAM wParam, LPARAM lParam ) const
	{
		return( ::SendDlgItemMessage( m_hWnd, nIDDlgItem, Msg, wParam, lParam ));
	}

	BOOL MoveWindow( int X, int Y, int nWidth, int nHeight, BOOL bRepaint = TRUE )
	{
		return( ::MoveWindow( m_hWnd, X, Y, nWidth, nHeight, bRepaint ));
	}

	HWND SetFocus()
	{
		ASSERT( m_hWnd );
		return( ::SetFocus( m_hWnd ));
	}

#ifdef _WIN32	
	int SetDlgItemText( int ID, LPCSTR lpszText ) const
	{
		return( ::SetDlgItemText( m_hWnd, ID, lpszText ));
	}
#else
	void SetDlgItemText( int ID, LPCSTR lpszText ) const
	{
		::SetDlgItemText( m_hWnd, ID, lpszText );
	}
#endif
	void CenterWindow( void );

	void ClientToScreen( POINT * pPoint ) const
	{
		::ClientToScreen( m_hWnd, pPoint );
	}
	void ClientToScreen( RECT * pRect ) const
	{
		ClientToScreen( (POINT*)&(pRect->left));
		ClientToScreen( (POINT*)&(pRect->right));
	}
	void ScreenToClient( POINT * pPoint ) const
	{
		::ScreenToClient( m_hWnd, pPoint );
	}
	void ScreenToClient( RECT * pRect ) const
	{
		ScreenToClient( (POINT*)&(pRect->left));
		ScreenToClient( (POINT*)&(pRect->right));
	}

	// Special methods.
	// Cursor selection stuff.
	void SetCursorStart() const;
	void SetCursorEnd() const;
	void SetSize( int ID, BOOL bInit );
	void SetEnableRange( int ID, int IDEnd, BOOL State );
};

class CDialogBase : public CWindow
{
public:
	static BOOL CALLBACK DlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
public:
	virtual BOOL DefDlgProc( UINT message, WPARAM wParam, LPARAM lParam )
	{
		return FALSE;
	}
};

class CWindowBase : public CWindow
{
protected:
	static LRESULT WINAPI WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
public:
	static ATOM RegisterClass( WNDCLASS & wc );
	virtual LRESULT DefWindowProc( UINT message, WPARAM wParam, LPARAM lParam )
	{
		return ::DefWindowProc( m_hWnd, message, wParam, lParam );
	}
};

class CWinApp	// Similar to MFC type
{
public:
	LPCSTR	 	m_pszAppName;	// Specifies the name of the application.
	HINSTANCE 	m_hInstance;	// Identifies the current instance of the application.
	LPSTR 		m_lpCmdLine;	// Points to a null-terminated string that specifies the command line for the application.
	CWindow *	m_pMainWnd;		// Holds a pointer to the application's main window. For an example of how to initialize m_pMainWnd, see InitInstance.
	CGString	m_pszExeName;	// The module name of the application.
	CGString	m_pszHelpFilePath;	// The path to the application's Help file.
	CGString	m_pszProfileName;
	int			m_nCmdShow;

public:
	CWinApp()
	{
		m_hInstance = NULL;
		m_pMainWnd = NULL;
		m_nCmdShow = 0;
	}

	void InitInstance( const char * pszAppName, HINSTANCE hInstance, LPSTR lpszCmdLine )
	{
		m_pszAppName = pszAppName;	// assume this is a static data pointer valid forever.
		m_hInstance	= hInstance;
		m_lpCmdLine	= lpszCmdLine;
		
		char szFileName[ _MAX_PATH ];
		if ( ! GetModuleFileName( m_hInstance, szFileName, sizeof( szFileName )))
			return;
		m_pszExeName = szFileName;
        
        LPSTR pszTmp = strrchr( const_cast<TCHAR *>(m_pszExeName.GetPtr()), '\\' );	// Get title
        lstrcpy( szFileName, ( pszTmp == NULL ) ? m_pszExeName : ( pszTmp + 1 ));
		pszTmp = strrchr( szFileName, '.' );	// Get extension.
		if ( pszTmp != NULL )
			pszTmp[0] = '\0';
		lstrcat( szFileName, ".INI" );
				
		OFSTRUCT ofs;
		if ( OpenFile( szFileName, &ofs, OF_EXIST ) != HFILE_ERROR)
		{
			m_pszProfileName = ofs.szPathName;
		}
		else
		{
			m_pszProfileName = szFileName;
		}
	}

	BOOL WriteProfileString( LPCSTR pszSection, LPCSTR pszEntry, LPCSTR pszVal )
	{
		return( ::WritePrivateProfileString( pszSection, pszEntry, pszVal, m_pszProfileName ));
	}
	BOOL WriteProfileInt( LPCSTR pszSection, LPCSTR pszEntry, int iVal )
	{
		char szValue[16];
		wsprintf( szValue, "%d", iVal );
		return( WriteProfileString( pszSection, pszEntry, szValue ));
	}
	int GetProfileString( LPCSTR pszSection, LPCSTR pszEntry, LPCSTR pszDefault, LPSTR pszReturnBuffer, int cbReturnBuffer )
	{
		return( ::GetPrivateProfileString( pszSection, pszEntry, pszDefault, pszReturnBuffer, cbReturnBuffer, m_pszProfileName ));
	}
	UINT GetProfileInt( LPCSTR pszSection, LPCSTR pszEntry, int iDefault )
	{
		return( ::GetPrivateProfileInt( pszSection, pszEntry, iDefault, m_pszProfileName ));
	}

	HMENU LoadMenu( int id ) const
	{
		return( ::LoadMenu( m_hInstance, MAKEINTRESOURCE( id )));
	}
	FARPROC MakeProcInst( FARPROC lpProc ) const
	{
		return( MakeProcInstance( lpProc, m_hInstance ));
	}
};

#endif	// _INC_CWINDOW_H

