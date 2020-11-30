//
// CBITMAP.CPP
//
// Read, write and convert (DIB/DDB) bitmap BMP files.
//

#ifdef GRAY_MAP
#include "graycom.h"
#else
#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#endif

#include "cbitmap.h"
#include "cfile.h"

///////////////////////////////////////////////////////////////
// Support functions.

static WORD _LOCCALL Bitmap_GetNumColors( LPBITMAPINFO pDIB )
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//   Given a pointer to a DIB, returns a number of colors in
	//   the DIB's color table.
	// ARGS:
	//   pDIB == pointer to DIB header (either BITMAPINFOHEADER or BITMAPCOREHEADER)
	// NOTE:
	//  If this is a Windows style DIB, the number of colors in the
	//  color table can be less than the number of bits per pixel
	//  allows for (i.e. pDIB->biClrUsed can be set to some value).
	//  If this is the case, return the appropriate value.
	// RETURN:
	//  Number of colors.
	//@------------------------------------------------------------------------

	DWORD dwClrUsed = pDIB->bmiHeader.biClrUsed;     // # May be supplied.
	if ( dwClrUsed )
		return( (WORD) dwClrUsed );

	//
	// If ClrUsed is 0 then assume all colors used.
	// Calculate the number of colors in the color table based on
	//  the number of bits per pixel for the DIB.
	//
	WORD wBitCount = pDIB->bmiHeader.biBitCount;
	if ( ! wBitCount || wBitCount >= 24 )	// 16 ?
		return( 0 );                       // Don't need to store the palette.

	return( _BITMASK( wBitCount ));
}

static WORD _LOCCALL Bitmap_GetColorSize( LPBITMAPINFO pDIB )
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Given a pointer to a DIB, returns number of bytes
	//   in the DIB's color table.
	// ARGS:
	//   pDIB == pointer to DIB header (either BITMAPINFOHEADER or BITMAPCOREHEADER)
	// RETURN:
	//  sizeof the color table.
	//@------------------------------------------------------------------------

	return( Bitmap_GetNumColors( pDIB ) * sizeof( RGBQUAD ));
}

static LONG _LOCCALL Bitmap_GetWidthBytes( LPBITMAPINFO pDIB )
{
	// NOTE:
	//  Must use the 32 bit scan line boundary.

#define DIB_WIDTH_BYTES(bits) ((((bits) + 31) / 32 ) * 4 )

	return( DIB_WIDTH_BYTES((DWORD)( pDIB->bmiHeader.biWidth ) * pDIB->bmiHeader.biBitCount ));
}

static LONG _LOCCALL Bitmap_GetImageSize( LPBITMAPINFO pDIB )
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Size of the image info for the DIB.
	// NOTE:
	//  bmi.bmiHeader.biPlanes will ALWAYS be 1 in the file !
	// RETURN:
	//  MAX size of the bitmap image data.
	//@------------------------------------------------------------------------

	LONG lSize = Bitmap_GetWidthBytes( pDIB ) * abs( pDIB->bmiHeader.biHeight );

	if ( pDIB->bmiHeader.biCompression == BI_RGB ) return( lSize );

	//
	// The RLE compression forms should always provide a lSize.
	// This should never happen ! provide a max size.
	//
	return(( lSize * 3 ) / 2 );
}

static LONG _LOCCALL Bitmap_GetSize( LPBITMAPINFO pDIB )
{
	// Get the total size needed.

	if ( ! pDIB->bmiHeader.biSizeImage )              // May not always be filled in.
	{
		pDIB->bmiHeader.biSizeImage = Bitmap_GetImageSize( pDIB );
	}

	return( pDIB->bmiHeader.biSize + Bitmap_GetColorSize( pDIB ) + pDIB->bmiHeader.biSizeImage );
}

///////////////////////////////////////////////////////////////
// CDIBitmap publics

void CDIBitmap::Init( void )
{
	m_pBMI = NULL;
}

void CDIBitmap::Empty()
{
	if ( m_pBMI == NULL )
		return;

	HGLOBAL hMem = GlobalHandle(m_pBMI);
	GlobalUnlock(hMem);
	GlobalFree(hMem);
	Init();
}

bool CDIBitmap::Set( LPBITMAPINFO pDIB )
{
	Empty();
	if ( pDIB->bmiHeader.biSize < sizeof( BITMAPINFOHEADER ))
		return( false );

#if 0
	// Fix the bit numbers to decent values (why should we do this ?)
	if (wBits <= 1)        // Mono.
		wBits = 1;
	else if (wBits <= 4)   // 16 colors.
		wBits = 4;
	else if (wBits <= 8)   // 256 colors.
		wBits = 8;
	else
		wBits = 24;          // No palllette stored.
#endif

	m_pBMI = (LPBITMAPINFO) GlobalLock( GlobalAlloc( GHND, Bitmap_GetSize( pDIB )));
	if ( m_pBMI == NULL )
		return( false );
	m_pBMI->bmiHeader = pDIB->bmiHeader;			// Copy header to it.
	m_pBits = ((BYTE FAR *) GetColors()) + GetColorSize();
	return( true );
}

bool CDIBitmap::Set( int x, int y, int iBits, int iColorMapSize )
{
	// Set to a new bitmap of this size.

	BITMAPINFO bmi;
	if ( m_pBMI == NULL )
	{
		// Must set up a bitmap from scratch.
		memset( &bmi, 0, sizeof( bmi ));
		bmi.bmiHeader.biSize = sizeof( bmi.bmiHeader );
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = iBits;
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

	return( Set( &bmi ));
}

long CDIBitmap::GetSize( void ) const
{
	return( Bitmap_GetSize( m_pBMI ));
}

long CDIBitmap::GetColorSize( void ) const
{
	return( Bitmap_GetColorSize( m_pBMI ));
}

int CDIBitmap::GetWidthBytes( void ) const
{
	return( (int) Bitmap_GetWidthBytes( m_pBMI ));
}

////////////////////////////////////////////////////////////////
// File I/O

bool CDIBitmap::Read( CGFile * pFile, bool fHeader )
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

	DEBUG_MSG(( "CDIBitmap:Read" ));

	if ( fHeader )	// Read a true Windows BMP file.
	{
		BITMAPFILEHEADER bmfh;
		if ( pFile->Read( & bmfh, sizeof( bmfh )) != 1 )
			return( false );

#define DIB_HEADER_MARKER     ((WORD) ('M' << 8) | 'B')

		if ( bmfh.bfType != DIB_HEADER_MARKER )                 // NOT 'BM' Type
			return( false );
	}

	BITMAPINFO bmi;
	if ( pFile->Read( &( bmi.bmiHeader ), sizeof( bmi.bmiHeader )) <= 0 )
		return( false );

	if ( ! pFile->Seek( bmi.bmiHeader.biSize - sizeof( bmi.bmiHeader ), SEEK_CUR )) // skip any extra ?
		return( false );

	//
	// The size of all the stored pixels.
	//
	if ( ! Set( &bmi ))
		return( false );

	//
	// Get the color and pallette info from the file. (if needed)
	// Number of colors. biClrUsed may be all we need ?!
	//
	long lSize = GetColorSize();
	if ( lSize )
	{
		if ( pFile->Read( (LPSTR) GetColors(), lSize ) != 1 )
			return( false );
	}

	//
	// Store the pixels.
	//
	lSize = GetImageSize();
	if ( pFile->Read( GetBits(), lSize ) != 1 )
	{
		return( false );
	}

	return( true );
}

bool CDIBitmap::Write( CGFile * pFile, bool fHeader )
{
	//
	// Write out the DIB in the specified format.
	// Create a DIB from the DDB provided.
	//
    long lSize = GetSize();
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
		if ( ! pFile->Write( (LPSTR) & bmfh, sizeof( BITMAPFILEHEADER )))
			return( false );
	}

	//
	// Write the DIB header and the bits
	//
	if ( ! pFile->Write( (LPSTR) m_pBMI, lSize ))
		return( false );

	return( true );
}

#if 0

CDIBitmap::CDIBitmap( LPCSTR pszFileName )
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Read a bitmap BMP file.
	// ARGS:
	//  pszFileName = the name of the bitmap BMP file.
	//@------------------------------------------------------------------------

	Init();

	DEBUG_MSG(( "CDIBitmap(%s)", (LPCSTR) pszFileName ));

	CFileBin File;
	if ( ! File.Open( pszFileName, OF_READ ))
		return;

	Read( &File, true );
	File.Close();
}

#endif

///////////////////////////////////////////////////////////////
// CBitmap publics

void CBitmap::Init()
{
	m_hBitmap = NULL;
	m_Size.cx = 0;
	m_Size.cy = 0;
}

void CBitmap::Empty()
{
	if ( m_hBitmap != NULL )
	{
		DeleteObject( m_hBitmap );
		Init();
	}
}

bool CBitmap::Set( HBITMAP hBitmap )
{
	Empty();
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
		DEBUG_ERR(( "CBitmap:Set BAD %xh", hBitmap ));
	}
	return( false );
}

bool CBitmap::SetDIB( LPBITMAPINFO pDIB, const void FAR * pBits )
{
	//
	// Create the bitmap to be used in the display context.
	// NOTE: CreateCompatibleDC( NULL ) will create a B&W hDC !
	//
	if ( pDIB == NULL )
		return( false );
	HDC hDC = GetDC( NULL );
	HBITMAP hBitmap = CreateDIBitmap( hDC, &(pDIB->bmiHeader), CBM_INIT, pBits, pDIB, DIB_RGB_COLORS );
	ReleaseDC( NULL, hDC );
	return( Set( hBitmap ));
}

bool CBitmap::Fill_DIB_Header( LPBITMAPINFO pDIB, DWORD dwStyle, WORD wBits ) const
{
	//
	// PURPOSE>
	//  Make the DIB header.
	// ARGS:
	//  hBitmap = the bitmap to get the header from.
	//  pDIB = Put the bitmap header here.
	//  dwStyle = Compression form. BI_*
	//  wBits = Number of color bits.
	// RETURN:
	//  false = can't make a header !
	//  true = OK.
	//

	if ( ! IsLoaded()) return( false );

	BITMAP bm;
	if ( ! GetObject( &bm ))
	{
		DEBUG_ERR(( "CBitmap: Bad %xh", m_hBitmap ));
		return( false );
	}

	//
	// Get number of bits per pixel.
	//
	if (wBits == 0)
		wBits = bm.bmPlanes * bm.bmBitsPixel;

	//
	// Build the DIB header.
	//
	pDIB->bmiHeader.biSize          = sizeof(pDIB->bmiHeader);
	pDIB->bmiHeader.biWidth         = bm.bmWidth;
	pDIB->bmiHeader.biHeight        = bm.bmHeight;
	pDIB->bmiHeader.biPlanes        = 1;
	pDIB->bmiHeader.biBitCount      = wBits;
	pDIB->bmiHeader.biCompression   = dwStyle;
	pDIB->bmiHeader.biSizeImage     = 0;
	pDIB->bmiHeader.biXPelsPerMeter = 0;
	pDIB->bmiHeader.biYPelsPerMeter = 0;
	pDIB->bmiHeader.biClrUsed       = 0;	// ??? Our new bitmaps may want a palette
	pDIB->bmiHeader.biClrImportant  = 0;

	return( true );
}

LONG CBitmap::GetDIBSize( void ) const
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
	return( Bitmap_GetSize( &bmi ));
}

bool CBitmap::Read( CGFile * pFile, bool fHeader )
{
	// RETURN:
	//  true = Success.
	//  false = ERROR.

	CDIBitmap * pDIB = new CDIBitmap;
	if ( pDIB == NULL ) 
		return( false );
	bool fRet = pDIB->Read( pFile, fHeader );
	if ( fRet )
	{
		fRet = SetDIB( pDIB );
	}
	delete pDIB;
	return( fRet );
}

bool CBitmap::Load( LPCSTR pszFileName )
{
	// RETURN:
	//  true = Success.
	//  false = ERROR.

	bool  fRet = false;
	CFileBin File;
	if ( File.Open( pszFileName, OF_READ ))
	{
		fRet = Read( &File, true );
		File.Close();
	}
	return( fRet );
}

////////////////////////////////////////////////////////////
// Memory image.

static HBITMAP _LOCCALL Bitmap_MakeCopy( HBITMAP hBitmap )
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

	DEBUG_MSG(( "Bitmap:MakeCopy" ));

	if ( hBitmap == NULL ) return( NULL );

	BITMAP Bitmap;
	if ( GetObject( hBitmap, sizeof( Bitmap ), &Bitmap ) != sizeof( Bitmap ))
		return( NULL );

	ASSERT( Bitmap.bmWidthBytes );
	long lSize = Bitmap.bmWidthBytes * (long) Bitmap.bmHeight;
	HGLOBAL hMem = GlobalAlloc( GMEM_MOVEABLE, lSize );
	Bitmap.bmBits = GlobalLock( hMem );
	if ( Bitmap.bmBits == NULL )
		return( NULL );

	GetBitmapBits( hBitmap, lSize, Bitmap.bmBits );
	HBITMAP hBitmapNew = CreateBitmapIndirect( & Bitmap );

	GlobalFree(hMem);
	return( hBitmapNew );
}

void CBitmap::Copy( HBITMAP hBitmap )
{	// copy in
	Set( Bitmap_MakeCopy( hBitmap ));
}

HBITMAP CBitmap::MakeCopy() const
{	// copy out
	return( Bitmap_MakeCopy( m_hBitmap ));
}

CDIBitmap * CBitmap::CreateDIB( DWORD dwStyle, WORD wBits, HPALETTE hPal ) const
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Will create a global memory block in DIB format that
	//   represents the Device-dependent bitmap (DDB) passed in.
	// ARGS:
	//  hBitmap = the bitmap to copy.
	//  dwStyle = the compression format of interest.
	//  wBits = the number of color bits.
	//  hPal = any special pallette we may be interested in.
	// RETURN:
	//  A handle to the DIB
	//@------------------------------------------------------------------------

	BITMAPINFO bmi;
	if ( ! Fill_DIB_Header( &bmi, dwStyle, wBits ))
		return( NULL );

	CDIBitmap * pDIB = new CDIBitmap( &bmi );
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
	if ( GetDIBits( hDC, m_hBitmap, 0, (WORD) pDIB->GetHeight(), pDIB->GetBits(),
		pDIB->GetInfo(), DIB_RGB_COLORS ) == 0 )
	{
		delete pDIB;  // no good I guess.
		pDIB = NULL;
	}

	SelectPalette( hDC, hPrevPal, false );                       // Restore old.
	ReleaseDC( NULL, hDC );
	return pDIB;
}

bool CBitmap::Write( CGFile * pFile, bool fHeader ) const
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

	CDIBitmap * pDIB = CreateDIB( BI_RGB, 0, NULL );
	if ( pDIB == NULL )
		return( false );                       // No space ?
	bool fRet = pDIB->Write( pFile, fHeader );
	delete pDIB;		// Get rid of it.
	return( fRet );
}

bool CBitmap::Save( LPCSTR pszFileName ) const
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
	CFileBin File;
	if ( File.Open( pszFileName, OF_CREATE | OF_WRITE ))
	{
		fRet = Write( &File, true );
		File.Close();
	}
	return( fRet );
}

COLORREF CBitmap::BltCopy( HDC hDC, int x, int y ) const
{
	HDC hDCBit = CreateCompatibleDC( hDC );
	HBITMAP hBitmapPrv = (HBITMAP) SelectObject( hDCBit, m_hBitmap );
	COLORREF color = GetPixel( hDCBit, 0, 0 );	// Why do this ?
	BitBlt( hDC, x, y, GetWidth(), GetHeight(), hDCBit, 0, 0, SRCCOPY );
	SelectObject( hDCBit, hBitmapPrv );
	DeleteDC( hDCBit );
	return( color );
}

void CBitmap::BltStretch( HDC hDC, int cx, int cy, int x, int y ) const
{
	HDC hDCBit = CreateCompatibleDC( hDC );
	if ( hDCBit != NULL )
	{
		HBITMAP hBitmapPrv = (HBITMAP) SelectObject( hDCBit, m_hBitmap );
		StretchBlt( hDC, x, y, cx, cy, hDCBit, 0, 0, GetWidth(), GetHeight(), SRCCOPY );
		SelectObject( hDCBit, hBitmapPrv );
		DeleteDC( hDCBit );
	}
}

#if 0

HBITMAP CBitmap::CreateAnd( void ) const
{
	// Prepare DC
	HDC hDCPaint = CreateCompatibleDC( NULL );
	SelectObject( hDCPaint, m_hBitmap );
	SetBkColor( hDCPaint, GetPixel( hDCPaint, 0, 0 ) );

	POINT   ptSize;
	ptSize.x = GetWidth();            // Get width of bitmap
	ptSize.y = GetHeight();           // Get height of bitmap
	DPtoLP( hDCPaint, &ptSize, 1 );    // Convert from device
	// to logical points
	// Prepare AND bitmap
	HBITMAP hbmpAnd = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
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

void CBitmap::Paint( HDC hDC, HBITMAP hbmpAnd,
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

