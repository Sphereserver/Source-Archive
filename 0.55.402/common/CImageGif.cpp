//
// CImageGif.cpp
//

#if defined(GRAY_MAP)
#include "../graymap/stdafx.h"
#include "../graymap/graymap.h"
#elif defined(GRAY_MAKER)
#include "../graymaker/stdafx.h"
#include "../graymaker/graymaker.h"
#else
#include "stdafx.h"
#endif

#include "cImageGif.h"

#if 1	// til we have a license from UINISYS

class CImageGIFBuilder : public CImageDIB
{
	CGFile * m_pFile;		// GIF file source.

	// GIF params.
	bool	m_fGIF89a;		// GIF file version.
	int		m_iInterlaced;	// Interlace pass.

	// Build up the current line.
	BYTE *  m_pLineBuf;		// temporary working buffer.
	short   m_height_cur;	// Current working line. for OutLine()

	// Just use these as shortcuts.
	short	m_width;		// CImageDIB.GetWidth()
	short	m_height;		// CImageDIB.GetHeight()
	short	m_widthbytes;	// CImageDIB.GetWidthBytes()
	short   m_wBitCount;	// CImageDIB.GetBitCount()

	// The following are used for seperating out codes
	BYTE	m_bBytesLeft;   // # bytes left in block
	short	m_wBitsLeft;    // # bits left in current byte
	short	m_wSizeCur;     // The current code size

public:
	// GIF frame info.
	int		m_iTimeDelay;	// Delay to the next frame. (1/100ths of sec)
	int		m_iLoopCount;
    BYTE	m_bTransPixel;
	bool	m_fTransparent;

private:
	void  ParseHeader();
	void  ParseFrame();
	void  ReadBytes( void * pByte, UINT iLen );
	short GetNextCode(void);
	void  OutLine( int iLineWidth );
	void  Decoder();
	void  Reset();
	WORD  GetFlagBitCount( WORD wBitCount ) const;

public:
	bool ReadGIF( CGFile * pFile, bool fHeader = true );
	CImageGIFBuilder();
	~CImageGIFBuilder();
};

//************************************************************************
// CImageGIFBuilder

CImageGIFBuilder::CImageGIFBuilder()
{
	m_pFile = NULL;
	m_pLineBuf = NULL;
	m_iLoopCount = 0;	// just stop at the end.
	m_fTransparent = false;
	m_fGIF89a = false;
	m_iInterlaced = 0;
}

CImageGIFBuilder::~CImageGIFBuilder()
{
	Reset();
}

void CImageGIFBuilder::Reset()
{
	if ( m_pLineBuf != NULL )
	{
		delete [] m_pLineBuf;
		m_pLineBuf = NULL;
	}
}

void CImageGIFBuilder::ReadBytes( void * pByte, UINT iLen )
{
	// return the next byte(s) from the GIF file, or a negative number.
	if ( ! iLen ) 
		return;
	if ( m_pFile->Read( pByte, iLen ) != iLen )
	{
		AfxThrowUserException();
	}
}

WORD CImageGIFBuilder::GetFlagBitCount( WORD wBitCount ) const
{
	wBitCount = (WORD)(( wBitCount & 0x07) + 1 );
	if ( wBitCount <= 2)
		wBitCount = 2;
	else if( wBitCount <= 4)
		wBitCount = 4;
	else if( wBitCount <= 8)
		wBitCount = 8;
	else
	{
		AfxThrowUserException();
	}
#if 0
	else if( wBitCount <= 16)
		wBitCount = 16;
	else if( wBitCount <= 32)
		wBitCount = 32;
#endif
	return( wBitCount );
}

void CImageGIFBuilder::ParseHeader()
{
	// parses the gif header.
	// false = is not GIF87a.

	Reset();

	CImageGifHeader Header;
	ReadBytes( &Header, sizeof(Header));

	if ( memcmp( Header.m_GIF, "GIF", 3 ))
		AfxThrowUserException();
	if ( memcmp( Header.m_ID, "87a", 3 ))
	{
		if ( memcmp( Header.m_ID, "89a", 3 )) 
			AfxThrowUserException();
		m_fGIF89a = true;
	}

	WORD wBitCount = GetFlagBitCount( Header.m_bFlag );

	// This is the number of colors in the color table
	WORD wColorMapSize;
	if ( Header.m_bFlag & 0x80 )	// Global color map.
		wColorMapSize = (WORD)( 2 << (Header.m_bFlag & 0x07));
	else
		wColorMapSize = 0;

	if ( ! CreateSurface( Header.m_width, Header.m_height, wBitCount, wColorMapSize ))
	{
		AfxThrowUserException();
	}

	// Initialize DIB color table
	RGBQUAD * rgbPtr = GetColors();
	RGBQUAD * tempPtr = rgbPtr;
	for ( int i = 0; i < wColorMapSize; i++ )
	{
		BYTE colors[3];
		ReadBytes( colors, 3 );
		rgbPtr->rgbRed   = colors[0];
		rgbPtr->rgbGreen = colors[1];
		rgbPtr->rgbBlue  = colors[2];
		rgbPtr->rgbReserved = 0;
		rgbPtr++;
	}
}

void CImageGIFBuilder::ParseFrame()
{
	CImageGifFrameHeader FrameHead;
	// Get the next frame in a (possibly) multi frame GIF.

	while ( 1 )
	{
		char command;
		ReadBytes( &command, 1 );

		switch ( command )
		{
		case 0: continue;	// Seperator between frames.

		case '!':
			// Extension of some type.
			break;
		case ',':
			{
				// Normal Image header junk,
				ReadBytes( &FrameHead, sizeof( FrameHead ));
				m_iInterlaced = ( FrameHead.m_bFlag & 0x40 ) ? 1 : 0;
				if ( ! IsLoaded())
				{
					if ( ! CreateSurface( FrameHead.m_width, FrameHead.m_height, 8, 0 ))	// ???
					{
						AfxThrowUserException();
					}
				}
			}
			return;
		case ';': // terminator 
			AfxThrowUserException();
		default:
			// Invalid extension char.
			AfxThrowUserException();
		}

		BYTE data[257];
		ReadBytes( data, 2 );
		if ( ! data[1] ) continue;

		switch ( data[0] )
		{
		case 0x01:	// text comment
		default:
			// Just skip it.
			ReadBytes( data, data[1] );
consume_block:
			do 
			{
				ReadBytes( data, 1 );
			} while ( data[0] );
			continue;

	    case 0xf9:	// gif_control_extension;
			{
			ReadBytes( data, data[1] );
			if ( data[0] & 1 )
			{
                m_bTransPixel = data[3];
                m_fTransparent = true;
			}
	        int m_disposal_method = ((data[0] >> 2) & 0x7);
            m_iTimeDelay = *((WORD*)( data + 1));	// get 1/100 ths of sec
			goto consume_block;
			}

	    case 0xfe:	// gif_consume_comment;
			ReadBytes( data, data[1] );
			while ( 1 )
			{
				ReadBytes( data, 1 );
				if ( ! data[0] ) break;
				ReadBytes( data, data[0] );
			} 
			continue;

		case 0xff:	// gif_application_extension;
			// Probably an animated GIF of some sort.
			ReadBytes( data, data[1] );
			if ( memcmp(data, "NETSCAPE2.0", 11) &&
		        memcmp(data, "ANIMEXTS1.0", 11 ))
			{
				goto consume_block;
			}

			while ( true )
			{
				ReadBytes( data, 1 );
				if ( ! data[0] ) 
					break;
				ReadBytes( data, data[0] );

	            int netscape_extension = ( data[0] & 7 );
	            if (netscape_extension == 1)
				{
					// Zero loop count is infinite animation loop request 
					// NOTE: This will be on the first frame header only.
					m_iLoopCount = *((WORD*)( data + 1 ));
					if ( ! m_iLoopCount ) m_iLoopCount = -1;	// infinite loop.
	            }

	            // Wait for specified # of bytes to enter buffer 
	            else if (netscape_extension == 2)
		        {
			        DWORD requested_buffer_fullness = *((DWORD*)( data+1 ));
					ASSERT( requested_buffer_fullness <= 255 );
				    ReadBytes( data, requested_buffer_fullness );
				}
			}
			continue;
		}
	}
}
											// 0, 1, 5, 3, 0 or 0, 0, 4, 2, 1
static const short GIF_iInterlaceStart[]	= { 0, 1, 5, 3, 0 };
static const short GIF_iInterlaceIncrement[]= { 1, 8, 8, 4, 2 };

void CImageGIFBuilder::OutLine( int iLineWidth )
{
	// Notes:	This function takes a full line of pixels (one byte per pixel) and
	// displays them (or does whatever your program wants with them...). 
	// Note that the iLineWidth
	// passed will almost always be equal to the line length passed to the
	// decoder function, with the sole exception occurring when an ending code
	// occurs in an odd place in the GIF file...  In any case, linelen will be
	// equal to the number of pixels passed...

	while ( m_height_cur >= m_height )
	{
		if ( m_iInterlaced < 1 || m_iInterlaced >= 4 )
		{
			AfxThrowUserException();
		}
		m_iInterlaced ++;
		m_height_cur = GIF_iInterlaceStart[ m_iInterlaced ];
	}

	BYTE * pBits = ((BYTE*)GetPixelData()) + ((long) m_widthbytes * (long) m_height_cur );

	if ( m_iInterlaced )	// What interlace pass is this.
		m_height_cur += GIF_iInterlaceIncrement[ m_iInterlaced ];
	else
		m_height_cur ++;

	int i;
	switch (m_wBitCount)
	{
	case 1:
		for (i=0; i<iLineWidth; i+=8)
		{
			BYTE cByte = 0;
			for (int j=0; j<8; j++)
			{
				if (m_pLineBuf[i+j] > 0)
					cByte += (BYTE)(1<<(7-j));
			}
			pBits[0] = cByte;
			pBits++;
		}
		break;

	case 4:
		for (i=0; i<iLineWidth; i+=2)
		{
			pBits[0] = (BYTE)((m_pLineBuf[i] << 4) + m_pLineBuf[i + 1]);
			pBits++;
		}
		break;

	case 8:
		memcpy( pBits, m_pLineBuf, iLineWidth );
		break;
	}
}

short CImageGIFBuilder::GetNextCode(void)
{
	// Gets the next code from the GIF file.
	// Returns the code,
	// throw in case of file errors...

static const short code_mask[13] =
{
	0,
	0x0001, 0x0003,
	0x0007, 0x000F,
	0x001F, 0x003F,
	0x007F, 0x00FF,
	0x01FF, 0x03FF,
	0x07FF, 0x0FFF
};

	static BYTE byte_buff[257];        // Current block
	static BYTE *pBytes;               // Pointer to next byte in block
	static BYTE b1;                    // Current byte

	if (m_wBitsLeft == 0)
	{
		if (m_bBytesLeft <= 0)
		{
			// Out of bytes in current block, so read next block
			pBytes = byte_buff;
			ReadBytes( &m_bBytesLeft, 1 );
			ReadBytes( byte_buff, m_bBytesLeft );
		}

		b1 = *pBytes++;
		m_wBitsLeft = 8;
		--m_bBytesLeft;
	}

	DWORD dwRet = b1 >> (8 - m_wBitsLeft);
	while (m_wSizeCur > m_wBitsLeft)
	{
		if (m_bBytesLeft <= 0)
		{
			// Out of bytes in current block, so read next block
			pBytes = byte_buff;
			ReadBytes( &m_bBytesLeft, 1 );
			ReadBytes( byte_buff, m_bBytesLeft );
		}

		b1 = *pBytes++;
		dwRet |= ((DWORD) b1 ) << m_wBitsLeft;
		m_wBitsLeft += 8;
		--m_bBytesLeft;
	}

	m_wBitsLeft -= m_wSizeCur;
	dwRet &= code_mask[m_wSizeCur];
	return( ( short ) dwRet );
}

void CImageGIFBuilder::Decoder()
{
	// decodes an LZW image, according to the method used
	// in the GIF spec.  Every *m_width* "characters" (ie. pixels) decoded
	// will generate a call to OutLine(), which is a user specific function
	// to display a line of pixels.  The function gets it's codes from
	// GetNextCode() which is responsible for reading blocks of data and
	// seperating them into the proper size codes.  Finally, GetNextByte() is
	// the global routine to read the next byte from the GIF file.
	//

	// Initialize for decoding a new image...
	BYTE bSize;
	ReadBytes( &bSize, 1 );
	if ( bSize < 2 || bSize > 9 )
	{
		AfxThrowUserException();
	}

	// Get the shortcuts.
	m_widthbytes = (WORD) GetWidthBytes();
	m_height = (WORD) GetHeight();
	m_width  = (WORD) GetWidth();
	m_wBitCount = (WORD) GetBitCount();

	// Allocate space for the decode buffer
	m_pLineBuf = new BYTE[m_width + 1];

	// initializes the decoder for reading a new image.
	m_wSizeCur = (WORD) ( bSize + 1 );
	m_bBytesLeft = 0;
	m_wBitsLeft = 0;
	m_height_cur = GIF_iInterlaceStart[ m_iInterlaced ];

	short wCodeTop = (short)( 1 << m_wSizeCur );  // Highest code for current size
	short wCodeClear = (short)( 1 << bSize );         // Value for a clear code
	short wCodeEnd = (short)(wCodeClear + 1);       // Value for a ending code
	short wCodeNew = (short)(wCodeEnd + 1);    // First available code
	short wCodeSlot = (short)(wCodeEnd + 1);         // Last read code
	short wCodeO = 0;
	short wCodeF = 0;

	int bad_code_count = 0;	// statistical.

	//
	// Notes:	The reason we have these seperated like this instead of using
	// a structure like the original Wilhite code did, is because this
	// stuff generally produces significantly faster code when compiled...
	// This code is full of similar speedups...  (For a good book on writing
	// C for speed or for space optomisation, see Efficient C by Tom Plum,
	// published by Plum-Hall Associates...)
#define MAX_CODES   4095
	BYTE stack[ MAX_CODES + 1 ];            // Stack for storing pixels
	BYTE suffix[ MAX_CODES + 1 ];           // Suffix table
	WORD prefix[ MAX_CODES + 1 ];           // Prefix linked list

	register BYTE * pStack = stack;
	register BYTE * pBufCur = m_pLineBuf;
	short iBufCnt = m_width;
	
	//
	// Notes:	This is the main loop.  For each code we get we pass through the
	// linked list of prefix codes, pushing the corresponding "character" for
	// each code onto the stack.  When the list reaches a single "character"
	// we push that on the stack too, and then start unstacking each
	// character for output in the correct order.  Special handling is
	// included for the clear code, and the whole thing ends when we get
	// an ending code.

	short wCode;
	while (( wCode = GetNextCode()) != wCodeEnd )
	{
		// If we had a file error, return without completing the decode

		// If the code is a clear code, reinitialize all necessary items.
		if (wCode == wCodeClear)
		{
			m_wSizeCur = (short)(bSize + 1);
			wCodeTop = (short)(1 << m_wSizeCur);
			wCodeSlot = wCodeNew;

			// Continue reading codes until we get a non-clear code
			while ((wCode = GetNextCode()) == wCodeClear)
				;

			// If we get an ending code immediately after a clear code
			if (wCode == wCodeEnd)
				break;
		
			//
			// Notes:	Finally, if the code is beyond the range of already set codes,
			//(This one had better NOT happen...  I have no idea what will
			// result from this, but I doubt it will look good...) then set it
			// to color zero.
			
			if (wCode >= wCodeSlot)
				wCode = 0;
			wCodeO = wCodeF = wCode;
			
			//
			// Notes:	And let us not forget to put the char into the buffer... And
			// if, on the off chance, we were exactly one pixel from the end
			// of the line, we have to send the buffer to the OutLine()
			// routine...

			*pBufCur++ = (BYTE) wCode;
			if (--iBufCnt <= 0)
			{
				OutLine( m_width );
				pBufCur = m_pLineBuf;
				iBufCnt = m_width;
			}
		}
		else
		{
			//
			// Notes:	In this case, it's not a clear code or an ending code, so
			// it must be a code code...  So we can now decode the code into
			// a stack of character codes. (Clear as mud, right?)

			short wCodeX = wCode;
			
			//
			// Notes:	Here we go again with one of those off chances...  If, on the
			// off chance, the code we got is beyond the range of those already
			// set up (Another thing which had better NOT happen...) we trick
			// the decoder into thinking it actually got the last code read.
			//(Hmmn... I'm not sure why this works...  But it does...)

			if (wCodeX >= wCodeSlot)
			{
				if (wCodeX > wCodeSlot)
					++bad_code_count;

				wCodeX = wCodeO;
				*pStack++ = (BYTE) wCodeF;
			}
			
			//
			// Notes:	Here we scan back along the linked list of prefixes, pushing
			// helpless characters (ie. suffixes) onto the stack as we do so.

			while (wCodeX >= wCodeNew)
			{
				*pStack++ = suffix[wCodeX];
				wCodeX = prefix[wCodeX];
			}
			
			//
			// Notes:	Push the last character on the stack, and set up the new
			// prefix and suffix, and if the required slot number is greater
			// than that allowed by the current bit size, increase the bit
			// size.  (NOTE - If we are all full, we *don't* save the new
			// suffix and prefix...  I'm not certain if this is correct...
			// it might be more proper to overwrite the last code...

			*pStack++ = (BYTE) wCodeX;
			if (wCodeSlot < wCodeTop)
			{
				wCodeF = wCodeX;
				suffix[wCodeSlot] = (BYTE) wCodeF;
				prefix[wCodeSlot++] = wCodeO;
				wCodeO = wCode;
			}

			if (wCodeSlot >= wCodeTop)
			{
				if (m_wSizeCur < 12)
				{
					wCodeTop <<= 1;
					++m_wSizeCur;
				}
			}
			
			//
			// Notes:	Now that we've pushed the decoded string (in reverse order)
			// onto the stack, lets pop it off and put it into our decode
			// buffer...  And when the decode buffer is full, write another
			// line...

			while (pStack > stack)
			{
				*pBufCur++ = *(--pStack);

				if (--iBufCnt == 0)
				{
					OutLine(m_width);
					pBufCur = m_pLineBuf;
					iBufCnt = m_width;
				}
			}
		}
	}

	if (iBufCnt != m_width)
	{
		OutLine( m_width - iBufCnt );
	}
}

bool CImageGIFBuilder::ReadGIF( CGFile * pFile, bool fHeader )
{
	m_pFile = pFile;
	// decode GIF image. throw exception if failed.
	// Make sure no stack unwind needed.
	TRY
	{
		if ( fHeader )
		{
			ParseHeader();
		}
		ParseFrame();
		Decoder();
		return( true );
	}
	CATCH_ALL (pException)
	{
		return( false );
	}
    END_CATCH_ALL
}

////////////////////////////////////////////////////////////////////
// -CImageGifParm

CImageGifParm::CImageGifParm()
{
	m_iFrameCur = 0;		// How many frames so far.
	m_iFrameCount = 0;
	m_iLoopCur = 0;			// How many loops so far.
	m_iLoopCount = 0;		// -1 = loop forever.
	m_fTransparent = false;	// Background color is not valid.
	m_iTimeDelay = 0;	// How long til next frame in (1/100ths of sec), -1=forever.
}

////////////////////////////////////////////////////////////////////
// -CImageGif

CImageGif::CImageGif()
{
}

CImageGif::~CImageGif()
{
}

int CImageGif::GetTimeToNextFrame() const
{
	if ( m_Params.m_iTimeDelay == -1 ) 
		return( 0 );
	return( m_Params.m_iTimeDelay * 10 );
}

bool CImageGif::SetGIF( bool fHeader )
{
	CImageGIFBuilder Gif;
	if ( ! Gif.ReadGIF( &m_File, fHeader ))
	{
		m_Params.m_iFrameCount = m_Params.m_iFrameCur;
		return( false );
	}

	m_Params.m_iLoopCount = Gif.m_iLoopCount;
	m_Params.m_fTransparent = Gif.m_fTransparent;
	m_Params.m_iTimeDelay = Gif.m_iTimeDelay;

	return( CreateBitmap( &Gif ));
}

bool CImageGif::SelectNextFrame()
{
	// After the correct amount of wait.
	// Move to the next frame.
	m_Params.m_iFrameCur ++;
	return( SetGIF( false ));
}

bool CImageGif::SelectFirstFrame()
{
	// restart the loop.
	if ( m_Params.m_iLoopCur >= m_Params.m_iLoopCount ) 
		return( false );
	m_Params.m_iLoopCur++;
	m_File.Seek(0,SEEK_SET );	// Seek back to start.
	return( SetGIF( true ));
}

bool CImageGif::Load( LPCTSTR lpszFilename )
{
	// CFileException e;
	if ( ! m_File.Open( lpszFilename, OF_READ|OF_BINARY ))
		return( false );
	return( SetGIF( true ));
}

#endif
