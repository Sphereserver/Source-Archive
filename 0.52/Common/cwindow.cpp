//
// CWindow.cpp
//

#if defined(GRAY_CLIENT)
#include "graycom.h"
#else
#define STRICT
#include <windows.h>
#include <stdio.h>
#include "cfile.h"
#endif
#include "cwindow.h"

///////////////////////
// -CDialogBase

BOOL WINAPI CDialogBase::DlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) // static
{
	CDialogBase * pDlg;
	if ( message == WM_INITDIALOG )	// NOTE: I get a useless WM_SETFONT msg first !
	{
		pDlg = (CDialogBase *)( lParam );
		ASSERT( pDlg );
		pDlg->m_hWnd = hWnd;	// OnCreate()
		SetWindowLong( hWnd, GWL_USERDATA, (DWORD) pDlg );
	}
	else
	{
		pDlg = (CDialogBase *)(LPVOID)GetWindowLong( hWnd, GWL_USERDATA );
	}
	if ( pDlg )
	{
		return pDlg->DefDlgProc( message, wParam, lParam );
	}
	else
	{
		return FALSE;
	}
}

///////////////////////
// -CWindowBase

ATOM CWindowBase::RegisterClass( WNDCLASS & wc )	// static
{
	wc.lpfnWndProc = WndProc;
	return ::RegisterClass( &wc );
}

LRESULT WINAPI CWindowBase::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) // static 
{
	// NOTE: It is important to call OnDestroy() for asserts to work.
	CWindowBase * pWnd;
	if ( message == WM_NCCREATE || message == WM_CREATE )
	{
		pWnd = (CWindowBase *)( ((LPCREATESTRUCT) lParam)->lpCreateParams );
		ASSERT( pWnd );
		pWnd->m_hWnd = hWnd;	// OnCreate()
		SetWindowLong( hWnd, GWL_USERDATA, (DWORD) pWnd );
	}
	else
	{
		pWnd = (CWindowBase *)(LPVOID)GetWindowLong( hWnd, GWL_USERDATA );
	}
	if ( pWnd )
	{
		return pWnd->DefWindowProc( message, wParam, lParam );
	}
	else
	{
		return ::DefWindowProc( hWnd, message, wParam, lParam );
	}
}

