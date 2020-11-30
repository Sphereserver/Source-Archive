//
// CImageBITMAP.CPP
//
// Read, write and convert (DIB/DDB) bitmap BMP files.
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

#include "CImageBitmap.h"

///////////////////////////////////////////////////////////////
// Support functions.

int CImageDIB::Bitmap_GetHeight( LPBITMAPINFO pBMI ) // static
{
	ASSERT(pBMI);
	return( abs( (int) pBMI->bmiHeader.biHeight ));
}

int CImageDIB::Bitmap_GetWidth( LPBITMAPINFO pBMI ) // static
{
	ASSERT(pBMI);
	return( (int) pBMI->bmiHeader.biWidth );
}

int CImageDIB::Bitmap_GetNumColors( LPBITMAPINFO pBMI )	// static
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//   Given a pointer to a DIB, returns a number of colors in
	//   the DIB's color table.
	// ARGS:
	//   pBMI == pointer to DIB header (BITMAPINFOHEADER NOT BITMAPCOREHEADER)
	// NOTE:
	//  If this is a Windows style DIB, the number of colors in the
	//  color table can be less than the number of bits per pixel
	//  allows for (i.e. pBMI->biClrUsed can be set to some value).
	//  If this is the case, return the appropriate value.
	// RETURN:
	//  Number of colors.
	//@------------------------------------------------------------------------

	ASSERT(pBMI);
	DWORD dwClrUsed = pBMI->bmiHeader.biClrUsed;     // # May be supplied.
	if ( dwClrUsed )
		return( (WORD) dwClrUsed );

	//
	// If ClrUsed is 0 then assume all colors used.
	// Calculate the number of colors in the color table based on
	//  the number of bits per pixel for the DIB.
	//
	WORD wBitCount = pBMI->bmiHeader.biBitCount * pBMI->bmiHeader.biPlanes;
	if ( ! wBitCount || wBitCount >= 24 )	// 16 ?
		return( 0 );                       // Don't need to store the palette.

	return( 1 << wBitCount );
}

int CImageDIB::Bitmap_GetWidthBytes( LPBITMAPINFO pBMI )	// static 
{
	// NOTE:
	//  Must use the 32 bit scan line boundary.

#define DIB_WIDTH_BYTES(bits) ((((bits) + 31) / 32 ) * 4 )

	ASSERT(pBMI);
	return( DIB_WIDTH_BYTES((DWORD)( pBMI->bmiHeader.biWidth ) * pBMI->bmiHeader.biBitCount ));
}

long CImageDIB::Bitmap_GetColorSize( LPBITMAPINFO pBMI )	// static 
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Given a pointer to a DIB, returns number of bytes
	//   in the DIB's color table.
	// ARGS:
	//   pBMI == pointer to DIB header (BITMAPINFOHEADER NOT BITMAPCOREHEADER)
	// RETURN:
	//  sizeof the color table.
	//@------------------------------------------------------------------------

	ASSERT(pBMI);
	return( Bitmap_GetNumColors( pBMI ) * sizeof( RGBQUAD ));
}

long CImageDIB::Bitmap_GetImageSize( LPBITMAPINFO pBMI ) // static
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Size of the image info for the DIB.
	// NOTE:
	//  bmi.bmiHeader.biPlanes MUST ALWAYS be 1 in the file !
	// RETURN:
	//  MAX size of the bitmap image data.
	//@------------------------------------------------------------------------

	ASSERT(pBMI);
	LONG lSize = Bitmap_GetWidthBytes( pBMI ) *
		abs( pBMI->bmiHeader.biHeight );

	if ( pBMI->bmiHeader.biCompression == BI_RGB ) 
		return( lSize );

	//
	// The RLE compression forms should always provide a lSize.
	// This should never happen ! provide a max size.
	//
	return(( lSize * 3 ) / 2 );
}

long CImageDIB::Bitmap_GetTotalSize( LPBITMAPINFO pBMI ) // static
{
	// Get the total size needed.

	ASSERT(pBMI);
	if ( ! pBMI->bmiHeader.biSizeImage )              // May not always be filled in.
	{
		pBMI->bmiHeader.biSizeImage = Bitmap_GetImageSize( pBMI );
	}

	return( pBMI->bmiHeader.biSize + Bitmap_GetColorSize( pBMI ) + pBMI->bmiHeader.biSizeImage );
}

RGBQUAD FAR * CImageDIB::Bitmap_GetColorTablePtr( LPBITMAPINFO pBMI ) // static
{
	// Get the palette table.
	ASSERT(pBMI);
	return( (RGBQUAD FAR *) ( ((BYTE FAR *) pBMI) + pBMI->bmiHeader.biSize ));
}

BYTE FAR * CImageDIB::Bitmap_GetPixelDataPtr( LPBITMAPINFO pBMI ) // static
{
	ASSERT(pBMI);
	return(((BYTE FAR *) Bitmap_GetColorTablePtr(pBMI)) + Bitmap_GetColorSize(pBMI));	
}

///////////////////////////////////////////////////////////////
// CImageDIB publics

void CImageDIB::Init( void )
{
	m_pBMI = NULL;
}

void CImageDIB::Empty()
{
	if ( m_pBMI == NULL )
		return;

	HGLOBAL hMem = GlobalHandle(m_pBMI);
	GlobalUnlock(hMem);
	GlobalFree(hMem);
	Init();
}

bool CImageDIB::CreateSurface( LPBITMAPINFO pBMI )
{
	Empty();
	if ( pBMI->bmiHeader.biSize < sizeof( BITMAPINFOHEADER ))
		return( false );
	m_pBMI = (LPBITMAPINFO) GlobalLock( GlobalAlloc( GHND, Bitmap_GetTotalSize( pBMI )));
	if ( m_pBMI == NULL )
		return( false );
	m_pBMI->bmiHeader = pBMI->bmiHeader;			// Copy header to it.

	m_pPixelData = Bitmap_GetPixelDataPtr(m_pBMI);
	m_uWidthBytes = Bitmap_GetWidthBytes(m_pBMI);	// iPitch = bytes per row. in most cases it must be 4 or 8 byte aligned ?
	m_size.cx = Bitmap_GetWidth(m_pBMI);		// size in pixels.
	m_size.cy = Bitmap_GetHeight(m_pBMI);		// size in pixels.

	return( true );
}

bool CImageDIB::CreateSurface( int x, int y, int iBits, int iColorMapSize )
{
	// Set to a new bitmap of this size.

	BITMAPINFO bmi;
	if ( m_pBMI == NULL )
	{
		// Must set up a bitmap from scratch.
		memset( &bmi, 0, sizeof( bmi ));
		bmi.bmiHeader.biSize = sizeof( bmi.bmiHeader );
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = (WORD) iBits;
		bmi.bmiHeader.biCompression = BI_RGB;
	}
	else
	{
		// No change ?
		if ( GetWidth() == x && GetHeight() == y )
			return( true );
		bmi = *m_pBMI;
	}

	bmi.bmiHeader.biWidth = x;
	bmi.bmiHeader.biHeight = - y;
	bmi.bmiHeader.biClrUsed = iColorMapSize; // May Lose colors this way ?

	return( CreateSurface( &bmi ));
}

////////////////////////////////////////////////////////////////
// File I/O

bool CImageDIB::Read( CFile * pFile, bool fHeader )
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Read the bitmap file data.
	// NOTE:
	//  File format for DIB file:
	//    BITMAPFILEHEADER bmfh;
	//    BITMAPINFOHEADER bmih;
	//    RGBQUAD          aColors[];
	//    BYTE             aBitmapBits[];
	// NOTE:
	//  Scan lines must pad to end on a 32 bit boundary.
	// ARGS:
	//  pFile = the handle to the open file.
	// RETURN:
	//  false = ERROR.
	//@------------------------------------------------------------------------

	// DEBUG_MSG(( "CImageDIB:Read" ));

	if ( fHeader )	// Read a true Windows BMP file.
	{
		BITMAPFILEHEADER bmfh;
		if ( pFile->Read( & bmfh, sizeof( bmfh )) <= 0 )
			return( false );

#define DIB_HEADER_MARKER     ((WORD) ('M' << 8) | 'B')

		if ( bmfh.bfType != DIB_HEADER_MARKER )                 // NOT 'BM' Type
			return( false );
	}

	DWORD dwSize;
	if ( pFile->Read( &dwSize, sizeof(dwSize)) != sizeof(dwSize))
		return( false );

	DWORD dwSizeRead;
	BITMAPINFO bmi;
	memset( &bmi, 0, sizeof( bmi ));
	if ( dwSize <= sizeof( BITMAPCOREHEADER ))
	{
		BITMAPCOREINFO bmci;
		memset( &bmci, 0, sizeof( bmci ));
		dwSizeRead = sizeof( bmci.bmciHeader ) - sizeof( DWORD );
		if ( pFile->Read( ((BYTE*)&( bmci.bmciHeader )) + sizeof(DWORD), dwSizeRead) != dwSizeRead )
		return( false );

		bmi.bmiHeader.biSize = dwSize;
		bmi.bmiHeader.biSize = sizeof( bmi );
		bmi.bmiHeader.biWidth = bmci.bmciHeader.bcWidth;
		bmi.bmiHeader.biHeight = bmci.bmciHeader.bcHeight;
		if ( bmci.bmciHeader.bcPlanes != 1 )
			return( false );
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = bmci.bmciHeader.bcBitCount;
	}
	else
	{
		bmi.bmiHeader.biSize = dwSize;
		dwSizeRead = sizeof( bmi.bmiHeader ) - sizeof(DWORD);
		if ( pFile->Read( ((BYTE*)&( bmi.bmiHeader )) + sizeof(DWORD), dwSizeRead) != dwSizeRead )
		{
			return( false );
		}
	}

	dwSizeRead += sizeof( DWORD );
    pFile->Seek( dwSize - dwSizeRead, CFile::current ); // skip any extra ?

	//
	// The size of all the stored pixels.
	//
	if ( ! CreateSurface( &bmi ))
		return( false );

	//
	// Get the color and pallette info from the file. (if needed)
	// Number of colors. biClrUsed may be all we need ?!
	//
	if ( dwSize <= sizeof( BITMAPCOREHEADER ))
	{
		// the colors are stored in 3 bytes.
		int iCount = Bitmap_GetNumColors( m_pBMI );
		RGBQUAD * pColor = GetColors();
		while ( iCount-- )
		{
			RGBTRIPLE rgbtri;
			if ( pFile->Read( (LPSTR) &rgbtri, sizeof(rgbtri)) != sizeof(rgbtri))
			return( false );
			pColor->rgbBlue = rgbtri.rgbtBlue;
			pColor->rgbGreen = rgbtri.rgbtGreen;
			pColor->rgbRed = rgbtri.rgbtRed;
			pColor->rgbReserved = 0;
			pColor++;
		}
	}
	else
	{
		dwSizeRead = GetColorSize();
		if ( dwSizeRead )
		{
			if ( pFile->Read( (LPSTR) GetColors(), dwSizeRead ) != dwSizeRead )
				return( false );
		}
	}

	//
	// Store the pixels.
	//
	dwSizeRead = GetImageSize();
	if ( pFile->Read( GetPixelData(), dwSizeRead ) != dwSizeRead )
	{
		return( false );
	}

	return( true );
}

bool CImageDIB::Write( CFile * pFile, bool fHeader )
{
	//
	// Write out the DIB in the specified format.
	// Create a DIB from the DDB provided.
	//
    long lSize = GetTotalSize();
	if ( fHeader )
	{
		//
		// Fill in the fields of the file header.
		//
		BITMAPFILEHEADER   bmfh;
		bmfh.bfType       = DIB_HEADER_MARKER;
		bmfh.bfSize       = lSize + sizeof(BITMAPFILEHEADER);
		bmfh.bfReserved1  = 0;
		bmfh.bfReserved2  = 0;
		bmfh.bfOffBits    = (DWORD) sizeof(BITMAPFILEHEADER) + GetHeaderSize()
			+ GetColorSize();

		//
		// Write the file header.
		//
		pFile->Write( (LPSTR) & bmfh, sizeof( BITMAPFILEHEADER ));
	}

	//
	// Write the DIB header and the bits
	//
	pFile->Write( (LPSTR) m_pBMI, lSize );
	return( true );
}

#if 1

CImageDIB::CImageDIB( LPCTSTR pszFileName )
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Read a bitmap BMP file.
	// ARGS:
	//  pszFileName = the name of the bitmap BMP file.
	//@------------------------------------------------------------------------

	Init();

	//DEBUG_MSG(( "CImageDIB(%s)", (LPCTSTR) pszFileName ));

	CGFile File;
	if ( ! File.Open( pszFileName, OF_READ|OF_BINARY ))
		return;

	Read( &File, true );
	File.Close();
}

#endif

///////////////////////////////////////////////////////////////
// CImageBitmap publics

void CImageBitmap::Init()
{
	m_hBitmap = NULL;
	m_Size.cx = 0;
	m_Size.cy = 0;
}

void CImageBitmap::DeleteObject()
{
	if ( m_hBitmap != NULL )
	{
		::DeleteObject( m_hBitmap );
		Init();
	}
}

bool CImageBitmap::Attach( HBITMAP hBitmap )
{
	DeleteObject();
	if ( hBitmap != NULL )
	{
		m_hBitmap = hBitmap;
		BITMAP Bitmap;
		if ( GetObject( &Bitmap ))
		{
			m_Size.cx = Bitmap.bmWidth;
			m_Size.cy = Bitmap.bmHeight;
			return( true );
		}
		m_hBitmap = NULL;
		// DEBUG_ERR(( "CImageBitmap:CreateBitmap BAD %xh", hBitmap ));
	}
	return( false );
}

bool CImageBitmap::CreateBitmap( LPBITMAPINFO pBMI, const void FAR * pBits )
{
	//
	// Create the bitmap to be used in the display context.
	// NOTE: CreateCompatibleDC( NULL ) will create a B&W hDC !
	//
	if ( pBMI == NULL )
		return( false );
	HDC hDC = ::GetDC( NULL );

#ifdef UNDER_CE

	BYTE FAR * pBitsDst;
	HBITMAP hBitmap = CreateDIBSection(
		hDC,
        pBMI,
        DIB_RGB_COLORS,
        (void**)&pBitsDst, NULL, NULL );

	// Now copy the bits in myself? weird.
	memcpy( pBitsDst, pBits, Bitmap_GetImageSize( pBMI ));

#else
	HBITMAP hBitmap = CreateDIBitmap( hDC, 
		&(pBMI->bmiHeader), 
		CBM_INIT, 
		pBits, 
		pBMI, 
		DIB_RGB_COLORS );
#endif

	ReleaseDC( NULL, hDC );
	return( Attach( hBitmap ));
}

bool CImageBitmap::Fill_DIB_Header( LPBITMAPINFO pBMI, DWORD dwStyle, WORD wBits ) const
{
	//
	// PURPOSE>
	//  Make the DIB header.
	// ARGS:
	//  hBitmap = the bitmap to get the header from.
	//  pBMI = Put the bitmap header here.
	//  dwStyle = Compression form. BI_*
	//  wBits = Number of color bits.
	// RETURN:
	//  false = can't make a header !
	//  true = OK.
	//

	if ( ! IsLoaded()) 
		return( false );

	BITMAP bm;
	if ( ! GetObject( &bm ))
	{
		//DEBUG_ERR(( "CImageBitmap: Bad %xh", m_hBitmap ));
		return( false );
	}

	//
	// Get number of bits per pixel.
	//
	if (wBits == 0)
	{
		wBits = (WORD)( bm.bmPlanes * bm.bmBitsPixel );
	}

	//
	// Build the DIB header.
	//
	pBMI->bmiHeader.biSize          = sizeof(pBMI->bmiHeader);
	pBMI->bmiHeader.biWidth         = bm.bmWidth;
	pBMI->bmiHeader.biHeight        = bm.bmHeight;
	pBMI->bmiHeader.biPlanes        = 1;
	pBMI->bmiHeader.biBitCount      = wBits;
	pBMI->bmiHeader.biCompression   = dwStyle;
	pBMI->bmiHeader.biSizeImage     = 0;
	pBMI->bmiHeader.biXPelsPerMeter = 0;
	pBMI->bmiHeader.biYPelsPerMeter = 0;
	pBMI->bmiHeader.biClrUsed       = 0;	// ??? Our new bitmaps may want a palette
	pBMI->bmiHeader.biClrImportant  = 0;

	return( true );
}

LONG CImageBitmap::GetDIBSize( void ) const
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  How big would this bitmap ( ddb ) be to store as a dib.
	// ARGS:
	//  hBitmap = handle to the bitmap.
	// RETURN:
	//  size in bytes.
	//@------------------------------------------------------------------------

	BITMAPINFO bmi;
	if ( ! Fill_DIB_Header( &bmi, BI_RGB, 0 ))
		return( 0 );
	return( CImageDIB::Bitmap_GetTotalSize( &bmi ));
}

bool CImageBitmap::Read( CFile * pFile, bool fHeader )
{
	// RETURN:
	//  true = Success.
	//  false = ERROR.

	CImageDIB * pDIB = new CImageDIB;
	if ( pDIB == NULL )
		return( false );
	bool fRet = pDIB->Read( pFile, fHeader );
	if ( fRet )
	{
		fRet = CreateBitmap( pDIB );
	}
	delete pDIB;
	return( fRet );
}

bool CImageBitmap::Load( LPCTSTR pszFileName )
{
	// RETURN:
	//  true = Success.
	//  false = ERROR.

	bool  fRet = false;

   ASSERT(pszFileName && *pszFileName);
    if (!pszFileName || !*pszFileName)
        return false;

#ifdef UNDER_CE
   m_hBitmap = SHLoadDIBitmap(pszFileName);
   //ASSERT(m_hBitmap != NULL);
   if (m_hBitmap != NULL)
       fRet = true; // Success!
#else
	CGFile File;
	if ( File.Open( pszFileName, OF_READ|OF_BINARY ))
	{
		fRet = Read( &File, true );
		File.Close();
	}
#endif

	return( fRet );
}

////////////////////////////////////////////////////////////
// Memory image.

HBITMAP CImageBitmap::Bitmap_MakeCopy( HBITMAP hBitmap )	// static
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Create an independant duplicate bitmap.
	// ARGS:
	//  hBitmap = the bitmap to copy.
	// RETURN:
	//  The new bitmap.
	// NOTE:
	//  All bitmaps must be destroyed manually.
	//  Once created a bitmap is owned by the GDC Task.
	//  It is not destroyed when the task exits !
	//@------------------------------------------------------------------------

	// DEBUG_MSG(( "Bitmap:MakeCopy" ));

	if ( hBitmap == NULL ) 
		return( NULL );

#ifdef UNDER_CE
	CImageBitmap bitmapSrc( hBitmap );

	HDC hDCScreen = ::GetDC(NULL);
	ASSERT( hDCScreen );
	HBITMAP hBitmapNew = ::CreateCompatibleBitmap(
		hDCScreen,
		bitmapSrc.GetWidth(), 
		bitmapSrc.GetHeight());
	ASSERT(hBitmapNew);

	HDC hDCDst = ::CreateCompatibleDC( hDCScreen );
	ASSERT(hDCDst);
	SelectObject( hDCDst, hBitmapNew );

	bitmapSrc.BltCopy( hDCDst );

	::ReleaseDC( NULL, hDCScreen );
	::DeleteDC( hDCDst );
	bitmapSrc.Detach();

#else
	BITMAP Bitmap;
	if ( ::GetObject( hBitmap, sizeof( Bitmap ), &Bitmap ) != sizeof( Bitmap ))
		return( NULL );

	ASSERT( Bitmap.bmWidthBytes );
	long lSize = Bitmap.bmWidthBytes * (long) Bitmap.bmHeight;
	HGLOBAL hMem = GlobalAlloc( GMEM_MOVEABLE, lSize );
	Bitmap.bmBits = GlobalLock( hMem );
	if ( Bitmap.bmBits == NULL )
		return( NULL );

	::GetBitmapBits( hBitmap, lSize, Bitmap.bmBits );
	HBITMAP hBitmapNew = ::CreateBitmapIndirect( & Bitmap );
	GlobalUnlock(hMem);
	GlobalFree(hMem);
#endif

	return( hBitmapNew );
}

void CImageBitmap::Copy( HBITMAP hBitmap )
{	// copy in
	Attach( Bitmap_MakeCopy( hBitmap ));
}

HBITMAP CImageBitmap::MakeCopy() const
{	// copy out
	return( Bitmap_MakeCopy( m_hBitmap ));
}

CImageDIB * CImageBitmap::CreateDIB( DWORD dwStyle, WORD wBits, HPALETTE hPal ) const
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Will create a global memory block in DIB format that
	//   represents the Device-dependent bitmap (DDB) HBITMAP.
	// ARGS:
	//  dwStyle = the compression format of interest.
	//  wBits = the number of color bits i want in my DIB.
	//  hPal = any special pallette we may be interested in.
	// RETURN:
	//  A handle to the DIB
	//@------------------------------------------------------------------------

	BITMAPINFO bmi;
	if ( ! Fill_DIB_Header( &bmi, dwStyle, wBits ))
		return( NULL );

	CImageDIB * pDIB = new CImageDIB( &bmi );
	if ( pDIB == NULL )
		return( NULL );

	//
	// Set up the pallette.
	//
	if ( hPal == NULL )
		hPal = (HPALETTE) GetStockObject( DEFAULT_PALETTE );
	HDC hDC = GetDC( NULL );
	HPALETTE hPrevPal = SelectPalette( hDC, hPal, false );
	RealizePalette( hDC );

	//
	// call GetDIBits with a NON-NULL pBits param, and actualy get the
	//  bits this time
	//
#ifdef UNDER_CE
    AfxThrowNotSupportedException();
#else
	if ( GetDIBits( hDC, m_hBitmap, 0, (WORD) pDIB->GetHeight(), pDIB->GetPixelData(),
		pDIB->GetInfo(), DIB_RGB_COLORS ) == 0 )
	{
		delete pDIB;  // no good I guess.
		pDIB = NULL;
	}
#endif
	SelectPalette( hDC, hPrevPal, false );                       // Restore old.
	ReleaseDC( NULL, hDC );
	return pDIB;
}

bool CImageBitmap::Write( CFile * pFile, bool fHeader ) const
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Write the DIB to the open file.
	// ARGS:
	//  hFile = the open file.
	//  hDib = the Device independant bitmap in memory.
	// RETURN:
	//  true = Success.
	//  false = ERROR.
	//@------------------------------------------------------------------------

	CImageDIB * pDIB = CreateDIB( BI_RGB, 0, NULL );
	if ( pDIB == NULL )
		return( false );                       // No space ?
	bool fRet = pDIB->Write( pFile, fHeader );
	delete pDIB;		// Get rid of it.
	return( fRet );
}

bool CImageBitmap::Save( LPCTSTR pszFileName ) const
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Write out the bitmap file to SP.BMP.
	// ARGS:
	//  pszFileName = save the bitmap to this file.
	// RETURN:
	//  true = the file was written correctly.
	//  false = ERROR.
	//@------------------------------------------------------------------------

	bool  fRet = false;
	CGFile File;
	if ( File.Open( pszFileName, OF_CREATE | OF_WRITE|OF_BINARY ))
	{
		fRet = Write( &File, true );
		File.Close();
	}
	return( fRet );
}

COLORREF CImageBitmap::BltCopy( HDC hDC, int x, int y ) const
{
	HDC hDCBit = CreateCompatibleDC( hDC );
	ASSERT( hDCBit );
	if ( hDCBit == NULL ) 
		return( CLR_INVALID );
	HBITMAP hBitmapPrv = (HBITMAP) SelectObject( hDCBit, m_hBitmap );
	COLORREF color = GetPixel( hDCBit, 0, 0 );	// Why do this ?
	BitBlt( hDC, x, y, GetWidth()-x, GetHeight()-y, hDCBit, 0, 0, SRCCOPY );
	SelectObject( hDCBit, hBitmapPrv );
	DeleteDC( hDCBit );
	return( color );
}

void CImageBitmap::BltStretch( HDC hDC, int cx, int cy, int x, int y ) const
{
	HDC hDCBit = CreateCompatibleDC( hDC );
	if ( hDCBit == NULL ) 
		return;
	HBITMAP hBitmapPrv = (HBITMAP) SelectObject( hDCBit, m_hBitmap );
	StretchBlt( hDC, x, y, cx, cy, hDCBit, 0, 0, GetWidth(), GetHeight(), SRCCOPY );
	SelectObject( hDCBit, hBitmapPrv );
	DeleteDC( hDCBit );
}

#if 1

HBITMAP CImageBitmap::CreateAnd( void ) const
{
	// Prepare DC
	HDC hDCPaint = CreateCompatibleDC( NULL );
	SelectObject( hDCPaint, m_hBitmap );
	SetBkColor( hDCPaint, GetPixel( hDCPaint, 0, 0 ) );

	POINT   ptSize;
	ptSize.x = GetWidth();            // Get width of bitmap
	ptSize.y = GetHeight();           // Get height of bitmap
#ifndef UNDER_CE	// Unnecessary & unsupported under CE
	DPtoLP( hDCPaint, &ptSize, 1 );    // Convert from device
#endif
	// to logical points
	// Prepare AND bitmap
	HBITMAP hbmpAnd = ::CreateBitmap( ptSize.x, ptSize.y, 1, 1, NULL );
	HDC hDCAnd = CreateCompatibleDC( NULL );
	SelectObject( hDCAnd, hbmpAnd );
	BitBlt( hDCAnd, 0, 0, ptSize.x, ptSize.y, hDCPaint, 0, 0, SRCCOPY );

	// Prepare PAINT bitmap
	SetTextColor( hDCPaint, RGB( 255, 255, 255 ) );
	SetBkColor( hDCPaint, RGB( 0, 0, 0 ) );
	BitBlt( hDCPaint, 0, 0, ptSize.x, ptSize.y, hDCAnd, 0, 0, SRCAND );
	DeleteDC( hDCAnd );
	DeleteDC( hDCPaint );

	return( hbmpAnd );
}

void CImageBitmap::Paint( HDC hDC, HBITMAP hbmpAnd,
	int xDst, int yDst, int cxDst, int cyDst,
	int xSrc, int ySrc ) const
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  In order to paint a bitmap on top of a background of unknown color,
	//  we must use the AND bitmap above.
	//@------------------------------------------------------------------------

	COLORREF cBk   = SetBkColor( hDC, RGB( 255, 255, 255 ) );
	COLORREF cText = SetTextColor( hDC, RGB( 0, 0, 0 ) );
	HDC hDCBit = CreateCompatibleDC( hDC );

	if ( cxDst < 0 ) cxDst = GetWidth();
	if ( cyDst < 0 ) cyDst = GetHeight();

	HBITMAP hBitOld = (HBITMAP) SelectObject( hDCBit, hbmpAnd );
	BitBlt( hDC, xDst, yDst, cxDst, cxDst, hDCBit, xSrc, ySrc, SRCAND   );
	SelectObject( hDCBit, m_hBitmap );
	BitBlt( hDC, xDst, yDst, cxDst, cxDst, hDCBit, xSrc, ySrc, SRCPAINT );
	SelectObject( hDCBit, hBitOld );

	DeleteDC( hDCBit );
	SetBkColor( hDC, cBk );
	SetTextColor( hDC, cText );
}

#endif

