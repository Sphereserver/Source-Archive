//
// CImageDC.h
//

#ifndef _INC_CIMAGEDC_H
#define _INC_CIMAGEDC_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "cimagebitmap.h"

class CImageDC : public CSurfaceBase
{
	// Create the Bitmap with attached access to the DIB info.
public:
	HDC m_hDC;
	CImageBitmap m_Bitmap;	// The bitmap that owns the m_pDibBits pointer.

protected:
	void DeleteObject();

public:
	//Draw capabilities
	void Draw( HDC dc, RECT rect );
	bool CreateSurface( int cx, int cy, int iBits = 16 );

	CImageDC();
	~CImageDC();
};

#endif  // _INC_CIMAGEDC_H



