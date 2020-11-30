//
// CImageGif.h
//

#ifndef _INC_CIMAGEGIF_H
#define _INC_CIMAGEGIF_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "cImageBitmap.h"

#pragma pack(1)
struct CImageGifHeader		// size = 6+7=13
{
	BYTE m_GIF[3];		// "GIF"
	BYTE m_ID[3];		// "87a" or "89a"
	WORD m_width;
	WORD m_height;
	BYTE m_bFlag;		// 0x80 = color table used.
	BYTE m_bBackgroundColor;  // background color
	BYTE m_bZero;		//  float aspect_ratio = (float)((q[6] + 15) / 64.0);
};
struct CImageGifFrameHeader	// size=9
{
	WORD m_leftOffset;
	WORD m_topOffset;
	WORD m_width;		// For this frame.
	WORD m_height;		// For this frame.
	BYTE m_bFlag;		// skip 2nd gif flag, 0x40 = interlaced.
};
#pragma pack()

struct CImageGifParm
{
	// Persistent info for the GIF and all it's frames.
	int m_iFrameCur;		// How many frames so far.
	int m_iFrameCount;
	int m_iLoopCur;			// How many loops so far.
	int m_iLoopCount;		// -1 = loop forever.
	bool m_fTransparent;	// Background color is not valid.

	CImageGifHeader m_GIF;	// Info about the full GIF.

	// Per frame
	CImageGifFrameHeader m_Frame;	// The current frame.
	int m_iTimeDelay;	// How long til next frame in (1/100ths of sec), -1=forever.
public:
	CImageGifParm();
};

class CImageGif : public CImageBitmap
{
	// CImageGif can act like a CImageBitmap and more.
	// Has transparent elements and can be animated.
private:
	CGFile m_File;				// animations must have access to the file. (multiple frames)
	CImageGifParm m_Params;

private:
	bool SetGIF( bool fHeader );

public:
	CImageGif();
	~CImageGif();
	int GetTimeToNextFrame() const;	// In MSec
	bool SelectFirstFrame();
	bool SelectNextFrame();
	bool Load( LPCTSTR lpszFilename );
};

#endif
