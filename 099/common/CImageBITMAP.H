    // CImageBITMAP.H
    // Copyright Menace Software (www.menasoft.com).
    //
    
    #ifndef _INC_CIMAGEBITMAP_H
    #define _INC_CIMAGEBITMAP_H
    #if _MSC_VER >= 1000
    #pragma once
    #endif // _MSC_VER >= 1000
    
    #include "csurfacebase.h"
    #include "cfile.h"
    // #include "windowsx.h"
    
    #ifdef UNDER_CE // Some definitions not present under Windows CE:
    
    #define GlobalLock
    #define GlobalAllocPtr(flags, cb)  GlobalAlloc((flags), (cb))
    #define GHND  GPTR // (GMEM_MOVEABLE | GMEM_ZEROINIT)
    
    #endif  // UNDER_CE
    
    ////////////////////////////////////////////////////////////
    
    class CFile;
    
    class CImageDIB	: public CSurfaceBase	// The device independant bitmap.
    {
    	// May have pallette info etc.
    private:
    	LPBITMAPINFO m_pBMI;	// The base pointer. (we created this)
    private:
    	void Init();
    
    public:
    
    	// support functions
    	static int Bitmap_GetNumColors( LPBITMAPINFO pBMI );
    	static int Bitmap_GetWidthBytes( LPBITMAPINFO pBMI );
    	static int Bitmap_GetHeight( LPBITMAPINFO pBMI );
    	static int Bitmap_GetWidth( LPBITMAPINFO pBMI );
    	static long Bitmap_GetColorSize( LPBITMAPINFO pBMI );
    	static long Bitmap_GetImageSize( LPBITMAPINFO pBMI );
    	static long Bitmap_GetTotalSize( LPBITMAPINFO pBMI );
    	static RGBQUAD FAR * Bitmap_GetColorTablePtr( LPBITMAPINFO pBMI );
    	static BYTE FAR * Bitmap_GetPixelDataPtr( LPBITMAPINFO pBMI );
    
    	void Empty();
    
    	bool IsLoaded() const
    	{
    		return( this != NULL && m_pBMI != NULL );
    	}
    	LPBITMAPINFO GetInfo() const
    	{
    		return( m_pBMI );
    	}
    	RGBQUAD FAR * GetColors() const	// COLORREF
    	{
    		// Get the palette table.
    		return( Bitmap_GetColorTablePtr( m_pBMI ));
    	}
    	int GetBitCount( void ) const
    	{
    		ASSERT(m_pBMI);
    		return( m_pBMI->bmiHeader.biBitCount );
    	}
    	long GetHeaderSize( void ) const
    	{
    		ASSERT(m_pBMI);
    		return( m_pBMI->bmiHeader.biSize );
    	}
    	long GetImageSize( void ) const		// Bit Size
    	{
    		ASSERT(m_pBMI);
    		return( m_pBMI->bmiHeader.biSizeImage );
    	}
    	long GetTotalSize( void ) const 	// Total Size
    	{
    		return( Bitmap_GetTotalSize( m_pBMI ));
    	}
    	long GetColorSize( void ) const  // Size of the palette info.
    	{
    		return( Bitmap_GetColorSize( m_pBMI ));
    	}
    
    	bool CreateSurface( LPBITMAPINFO pBMI );
    	bool CreateSurface( int x, int y, int iBits = 8, int iColorMapSize = 0 );
    
    	virtual bool Read( CFile * pFile, bool fHeader = true );
    	virtual bool Write( CFile * pFile, bool fHeader = true );
    
    	~CImageDIB()
    	{
    		Empty();
    	}
    	CImageDIB()
    	{
    		Init();
    	}
    	CImageDIB( LPBITMAPINFO pBMI )
    	{
    		Init();
    		CreateSurface( pBMI );
    	}
    	CImageDIB( LPCTSTR pszFileName );
    };
    
    ////////////////////////////////////////////////////////////
    
    //#ifdef _AFXDLL	// Only need minimal sub-set if using MFC.
    
    // #define CImageBitmap CBitmap
    
    //#else
    
    class CImageBitmap	// Basic device dependant bitmap handling class. Similar to MFC
    {
    public:
    	HBITMAP m_hBitmap;
    	SIZE	m_Size;		// Keep this around as we use it alot.
    
    protected:
    	void Init();
    	bool Fill_DIB_Header( LPBITMAPINFO pBI, DWORD dwStyle, WORD wBits ) const;
    
    public:
    	static HBITMAP Bitmap_MakeCopy( HBITMAP hBitmap );
    
    	void DeleteObject();
    
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
    
    	CImageDIB * CreateDIB( DWORD dwStyle, WORD wBits, HPALETTE hPal ) const;
    
    	bool Attach( HBITMAP hBitmap );
    	void Detach()
    	{
    		// do not delete the HBITMAP when this is destroyed.
    		Init();		
    	}
    
    	bool CreateBitmap( LPBITMAPINFO pDIB, const void FAR * pBits );
    	bool CreateBitmap( CImageDIB * pDIB )
    	{
    		if ( ! pDIB->IsLoaded()) 
    			return( false );
    		return( CreateBitmap( pDIB->GetInfo(), pDIB->GetPixelData()));
    	}
    
    	void Copy( HBITMAP hBitmap );
    	HBITMAP MakeCopy() const;
    
    	bool Write( CFile * pFile, bool fHeader = true ) const;
    	bool Read( CFile * pFile, bool fHeader = true );
    
    	virtual bool Load( LPCTSTR pFileName );
    	virtual bool Save( LPCTSTR pFileName ) const;
    
    	bool Load( HINSTANCE hInst, int id )
    	{
    		// from resource.
    		return( Attach( LoadBitmap( hInst, MAKEINTRESOURCE( id ))));
    	}
    
    	COLORREF BltCopy( HDC hDC, int x = 0, int y = 0 ) const;
    	void BltStretch( HDC hDC, int cx, int cy, int x = 0, int y = 0 ) const;
    
    	HBITMAP CreateAnd( void ) const;
    	void Paint(
    		HDC hDC, HBITMAP hbmpAnd, int xDst = 0, int yDst = 0,
    		int cxDst = -1, int cyDst = -1,
    		int xSrc = 0, int ySrc = 0 ) const;
    
    	CImageBitmap( HBITMAP hBitmap )
    	{
    		Init();
    		Attach( hBitmap );
    	}
    	CImageBitmap( LPCTSTR pFileName )
    	{
    		Init();
    		Load( pFileName );
    	}
    	CImageBitmap( CImageDIB * pDIB )
    	{
    		Init();
    		CreateBitmap( pDIB );
    	}
    	CImageBitmap()
    	{
    		Init();
    	}
    	~CImageBitmap()
    	{
    		DeleteObject();
    	}
    };
    
    // #endif // _AFXDLL
    
    #endif // _INC_CIMAGEBITMAP_H