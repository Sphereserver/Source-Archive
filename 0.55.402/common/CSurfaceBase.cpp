//
// CSurfaceBase.cpp
//

// Surface image manipulation.

#ifdef GRAY_CLIENT
#include "../grayclient/grayclient.h"

#elif defined(GRAY_MAKER)
#include "../graymaker/stdafx.h"
#include "../graymaker/graymaker.h"

#elif defined(GRAY_MAP)
#include "../graymap/stdafx.h"
#include "../graymap/graymap.h"

#elif defined(GRAY_PATCH)
#include "../graypatch/stdafx.h"
#include "../graypatch/graypatch.h"

#else

#error no GRAY_ defined !
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////
// -CSurfaceBase

HBITMAP CSurfaceBase::CreateBitmapFromSurface16()
{
	// take the surface and confert to a bitmap.
	// (which we create)
	// used for things like clipboard etc.

	HDC hDC = GetDC( NULL );

	BITMAPINFO bmi;
	memset( &bmi, 0, sizeof( bmi ));
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biHeight = -m_size.cy;	// - Top down
	bmi.bmiHeader.biWidth = m_size.cx;
	bmi.bmiHeader.biBitCount = 16;	// 1,4,8,24
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;	// Not compressed
	bmi.bmiHeader.biSizeImage = m_size.cy * GetWidthBytes();
	bmi.bmiHeader.biXPelsPerMeter= 0;
	bmi.bmiHeader.biYPelsPerMeter= 0;
	bmi.bmiHeader.biClrUsed 	 = 0;
	bmi.bmiHeader.biClrImportant = 0;

	// This is direct ? push the background buffer to the foreground screen
	HBITMAP hBitmap = CreateDIBitmap( 
		hDC, 
		&(bmi.bmiHeader),
		CBM_INIT, 
		GetPixelData(),
		&bmi,
		DIB_RGB_COLORS );

	ReleaseDC( NULL, hDC );

	return( hBitmap );
}

void CSurfaceBase::Flip16( HDC hDC, int XDest, int YDest )
{
	// Set up the bitmap.
	BITMAPINFO bmi;
	memset( &bmi, 0, sizeof( bmi ));
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biHeight = -m_size.cy;	// - Top down
	bmi.bmiHeader.biWidth = m_size.cx;
	bmi.bmiHeader.biBitCount = 16;	// 1,4,8,24
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;	// Not compressed
	bmi.bmiHeader.biSizeImage = m_size.cy * GetWidthBytes();
	bmi.bmiHeader.biXPelsPerMeter= 0;
	bmi.bmiHeader.biYPelsPerMeter= 0;
	bmi.bmiHeader.biClrUsed 	 = 0;
	bmi.bmiHeader.biClrImportant = 0;

	ASSERT(hDC);

	// This is direct ? push the background buffer to the foreground screen
	int iLines = SetDIBitsToDevice( 
		hDC,
		XDest, YDest,
		m_size.cx, m_size.cy,
		0, 0,
		0,
		m_size.cy, 
		GetPixelData(),
		&bmi, 
		DIB_RGB_COLORS );
	// ASSERT( iLines );
}

void CSurfaceBase::ScrollPixels16(int cx, int cy)
{
	// Scroll the bitmap data.
	if (!IsValid()) 
		return;

	int acx = abs(cx);
	int acy = abs(cy);

	LPVOID lp1, lp2;
	if (cx!=0)
	{ // Shift data with x coord
		if (acx<GetWidth())
		{
			for (int i=0; i<GetHeight(); i++)
			{
				lp1=GetLinePtr(i);
				lp2=((BYTE*)lp1)+(2*acx);
				if (cx<0)
					memcpy(lp2, lp1, 2*(GetWidth()-acx));
				else 
					memcpy(lp1, lp2, 2*(GetWidth()-acx));
			}
		}
	}
	if (cy!=0)
	{ // Shift data with x coord
		if (acy<GetHeight())
		{
			if (cy<0)
			{ // shift data down
				for (int i=0; i<GetHeight()-acy; i++)
				{
					lp2=GetLinePtr(i);
					lp1=GetLinePtr(i+acy);
					memcpy(lp2, lp1, 2*GetWidth());
				}
			}
			else
			{
				for (int i=0; i<GetHeight()-acy; i++)
				{
					lp2=GetLinePtr(GetHeight()-i-1);
					lp1=GetLinePtr(GetHeight()-i-acy-1);
					memcpy(lp2, lp1, 2*GetWidth());
				}
			}
		}
	}
}
