//
// CSurfaceBase.h
//

#ifndef _INC_CSURFACEBASE_H
#define _INC_CSURFACEBASE_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CSurfaceBase
{
	// A simple surface. (we do not allocate this)
	// Assume nothing about pixel format.
	friend class CSurfaceDCData;
	friend class CSurfaceDC;

protected:
	void * m_pPixelData;	// Unknown ownership of this memory block.
	SIZE m_size;		// size in pixels.
	WORD m_uWidthBytes;	// iPitch = bytes per row. in most cases it must be 4 or 8 byte aligned ?

protected:
	void Copy( const CSurfaceBase & Surf )
	{
		m_pPixelData = Surf.m_pPixelData;	// this is dangerous ! (who owns this now ?)
		m_size = Surf.m_size;
		m_uWidthBytes = Surf.m_uWidthBytes;
	}
	CSurfaceBase& operator = ( const CSurfaceBase & Surf )
	{
		Copy( Surf );
		return( *this );
	}

	void Init()
	{
		m_pPixelData = NULL;
		m_size.cx = 0; 
		m_size.cy = 0;
		m_uWidthBytes = 0;
	}
	bool InitSurface( int cx, int cy )
	{
		if ( IsValid() && cx == m_size.cx && cy == m_size.cy )
			return false;
		m_size.cx = cx;
		m_size.cy = cy;
		// Release the current memory (if any) must be done after this.
		return( true );
	}
	void SetWidthAlign( int iAlignBytes, int nBytesPerPixel = 2 )
	{
		// Set the pitch to some byte alignment
		ASSERT(iAlignBytes>0);
		iAlignBytes --;
		m_uWidthBytes = ((( m_size.cx * nBytesPerPixel ) + iAlignBytes ) &~ iAlignBytes );	// Round up to mult 4
	}

public:

	// basic attributes.
	bool IsValid() const
	{
		return( m_pPixelData != NULL );
	}
	void * GetPixelData() const
	{
		// pointer to the pixel data. We assume nothing about pixel format.
		ASSERT(m_pPixelData);
		return( m_pPixelData );
	}
	int GetWidthBytes() const
	{
		// width in bytes not pixels
		return m_uWidthBytes;
	}
	int GetWidth() const
	{
		// width in pixels not bytes.
		return m_size.cx;
	}
	int GetHeight() const
	{
		// height in pixels not bytes.
		return m_size.cy;
	}

	void * GetLinePtr( int y ) const
	{
		assert( ((WORD)y) < m_size.cy );
		// assert( GetPixelData() );
		return( ((BYTE*) GetPixelData()) + (GetWidthBytes() * y));
	}
	void * GetNextLine( const void * pDst ) const
	{
		return( ((BYTE*) pDst) + GetWidthBytes() );
	}

	void Erase()
	{
		// NOTE: This is not a good idea if 
		//  m_uWidthBytes is much different than size.cx
		memset( m_pPixelData, 0, GetWidthBytes() * GetHeight());
	}

	bool Lock()
	{
		return( true );
	}
	void Unlock()
	{
	}

	// Assume 16 bit colors
	HBITMAP CreateBitmapFromSurface16();
	void Flip16( HDC hDC, int XDest = 0, int YDest = 0 );
	void ScrollPixels16(int cx, int cy);
	WORD * GetPixPtr16( int x, int y ) const
	{
		assert( ((WORD)x) < m_size.cx );
		return( ((WORD *) GetLinePtr(y)) + x );
	}
	WORD GetPixColor16( int x, int y ) const
	{
		return( * GetPixPtr16( x, y ));
	}

	CSurfaceBase()
	{
		Init();
	}
};

#endif
