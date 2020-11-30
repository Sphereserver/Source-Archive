//
// CImageDC.cpp
//
// A Surface image with an attaced DC.
//

#if defined(GRAY_MAP)
#include "../graymap/stdafx.h"
#include "../graymap/graymap.h"
#elif defined(GRAY_MAKER)
#include "../graymaker/stdafx.h"
#include "../graymaker/graymaker.h"
#else
#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#endif

#include "CImageDC.h"

CImageDC::CImageDC()
{
	m_hDC = NULL;
}

CImageDC::~CImageDC()
{
	DeleteObject();
}

void CImageDC::DeleteObject()
{
	if ( m_hDC )
	{
		::DeleteDC( m_hDC );
		m_hDC = NULL;
		Init();	// CSurfaceBase
		m_Bitmap.DeleteObject();
	}
}

bool CImageDC::CreateSurface( int cx, int cy, int iBits )
{
	// Create a device independant bitmap permenantly attached to the device.
	if ( cx == GetWidth() && cy == GetHeight())
		return true;

	DeleteObject();
	
	// NOTE: CreateCompatibleDC( NULL ) will create a B&W hDC !
	//
	HDC hDCMain = ::GetDC( NULL );
	m_hDC = ::CreateCompatibleDC(hDCMain);
	::ReleaseDC( NULL, hDCMain );

	if ( ! m_hDC )
		return false;

	// create BITMAPINFO structure
	BITMAPINFO bmi;
	memset( &bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth=cx;
	bmi.bmiHeader.biHeight=-cy;
	bmi.bmiHeader.biPlanes=1;
	bmi.bmiHeader.biBitCount=iBits;

	HBITMAP hDibBMP = ::CreateDIBSection( m_hDC, &bmi,
		DIB_PAL_COLORS,
		(void**) &m_pPixelData,
		NULL,
		NULL );
	if ( hDibBMP == NULL || m_pPixelData == NULL )
	{
		// ASSERT(FALSE);
		// AfxMessageBox(TEXT("There is no memory for offscreens!"));
		DWORD dwError = ::GetLastError();
		::DeleteDC(m_hDC);
		return false;
	}

	m_uWidthBytes = CImageDIB::Bitmap_GetWidthBytes(&bmi);	// iPitch = bytes per row. in most cases it must be 4 or 8 byte aligned ?
	m_size.cx = CImageDIB::Bitmap_GetWidth(&bmi);		// size in pixels.
	m_size.cy = CImageDIB::Bitmap_GetHeight(&bmi);		// size in pixels.

	m_Bitmap.Attach( hDibBMP );
	::SelectObject( m_hDC, hDibBMP );

	return( true );
}

void CImageDC::Draw( HDC hDC, RECT rect )
{
	if (m_pPixelData!=NULL)
	{
#if 1
		BitBlt( hDC, 0, 0, GetWidth(), GetHeight(), m_hDC, 0, 0, SRCCOPY );
#else
		BITMAPINFO bmi;
		memset( &bmi, 0, sizeof(bmi));
		bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth=GetWidth();
		bmi.bmiHeader.biHeight=GetHeight();
		bmi.bmiHeader.biPlanes=1;
		bmi.bmiHeader.biBitCount=16;

		SetDIBitsToDevice( hDC,
			rect.left,
			rect.top,
			rect.right - rect.left,
			rect.bottom - rect.top,
			rect.left,
			GetHeight()-rect.bottom, 
			0,
			GetHeight(), m_pPixelData,
			&bmi, DIB_RGB_COLORS );
#endif
	}

}

