//
// CBITMAP.H
// Copyright Menace Software (www.menasoft.com).
//

#ifndef _INC_CBITMAP_H
#define _INC_CBITMAP_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "common.h"
// #include "windowsx.h"

////////////////////////////////////////////////////////////

class CGFile;

class CDIBitmap		// The device independant bitmap.
{
private:
	LPBITMAPINFO m_pBMI;	// The base pointer.
	BYTE FAR * m_pBits;		// GetColors()) + GetColorSize()
private:
	void Init();

public:
	void Empty();
	CDIBitmap()
	{
		Init();
	}
	~CDIBitmap()
	{
		Empty();
	}
	CDIBitmap( LPCSTR pszFileName );

	LPBITMAPINFO GetInfo() const
	{
		return( m_pBMI );
	}
	bool IsLoaded() const
	{
		return( this != NULL && m_pBMI != NULL );
	}
	int GetHeight( void ) const
	{	// This can go negative if top down.
		return( abs( (int) m_pBMI->bmiHeader.biHeight ));
	}
	int GetWidth( void ) const
	{
		return( (int) m_pBMI->bmiHeader.biWidth );
	}
	int GetBitCount( void ) const
	{
		return( m_pBMI->bmiHeader.biBitCount );
	}
	int GetWidthBytes( void ) const;

	bool Set( LPBITMAPINFO pBMI );
	bool Set( int x, int y, int iBits = 8, int iColorMapSize = 0 );

	CDIBitmap( LPBITMAPINFO pBMI )
	{
		Init();
		Set( pBMI );
	}

	long GetSize( void ) const;			// Total Size
	long GetHeaderSize( void ) const
	{
		return( m_pBMI->bmiHeader.biSize );
	}
	long GetColorSize( void ) const;    // Size of the palette info.
	long GetImageSize( void ) const		// Bit Size
	{
		return( m_pBMI->bmiHeader.biSizeImage );
	}

	RGBQUAD FAR * GetColors() const
	{
		return( (RGBQUAD FAR *) ( ((BYTE FAR *) m_pBMI) + GetHeaderSize() ));
	}
	void FAR * GetBits() const
	{
		return( m_pBits );
	}

	virtual bool Read( CGFile * pFile, bool fHeader = true );
	virtual bool Write( CGFile * pFile, bool fHeader = true );
};

////////////////////////////////////////////////////////////

class CBitmap	// Basic device dependant bitmap handling class.
{
protected:
	HBITMAP m_hBitmap;
	SIZE	m_Size;		// Keep this around as we use it alot.

protected:
	void Init();
	bool Fill_DIB_Header( LPBITMAPINFO pBI, DWORD dwStyle, WORD wBits ) const;

public:
	void Empty();
	CBitmap()
	{
		Init();
	}
	~CBitmap()
	{
		Empty();
	}

	// Info functions.
	int GetWidth() const
	{
		return( m_Size.cx );
	}
	int GetHeight() const
	{
		return( m_Size.cy );
	}
	operator HBITMAP() const
	{
		return( m_hBitmap );
	}
	bool IsLoaded() const
	{
		return( this != NULL && m_hBitmap != NULL );
	}
	bool GetObject( BITMAP * pBitmap ) const
	{	// Get the system object data.
		return( :: GetObject( m_hBitmap, sizeof( BITMAP ), (LPSTR) pBitmap )
			== sizeof( BITMAP ));
	}
	int GetWidthBytes() const;
	LONG GetDIBSize() const; 		// if it was a DIB ?

	CDIBitmap * CreateDIB( DWORD dwStyle, WORD wBits, HPALETTE hPal ) const;

	bool Set( HBITMAP hBitmap );
	bool SetDIB( LPBITMAPINFO pDIB, const void FAR * pBits );
	bool SetDIB( CDIBitmap * pDIB )
	{
		if ( ! pDIB->IsLoaded()) return( false );
		return( SetDIB( pDIB->GetInfo(), pDIB->GetBits()));
	}

	void Copy( HBITMAP hBitmap );
	HBITMAP MakeCopy() const;

	bool Write( CGFile * pFile, bool fHeader = true ) const;
	bool Read( CGFile * pFile, bool fHeader = true );

	virtual bool Load( LPCSTR pFileName );
	virtual bool Save( LPCSTR pFileName ) const;

	CBitmap( LPCSTR pFileName )
	{
		Init();
		Load( pFileName );
	}
	CBitmap( CDIBitmap * pDIB )
	{
		Init();
		SetDIB( pDIB );
	}

	bool Load( HINSTANCE hInst, int id )
	{
		// from resource.
		return( Set( LoadBitmap( hInst, MAKEINTRESOURCE( id ))));
	}

	COLORREF BltCopy( HDC hDC, int x = 0, int y = 0 ) const;
	void BltStretch( HDC hDC, int cx, int cy, int x = 0, int y = 0 ) const;

	HBITMAP CreateAnd( void ) const;
	void Paint(
		HDC hDC, HBITMAP hbmpAnd, int xDst = 0, int yDst = 0,
		int cxDst = -1, int cyDst = -1,
		int xSrc = 0, int ySrc = 0 ) const;
};

#endif // _INC_CBITMAP_H
