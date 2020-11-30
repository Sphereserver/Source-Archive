//
// CMemBlock.h
// Copyright Menace Software (www.menasoft.com).
//

#ifndef _INC_CMEMBLOCK_H
#define _INC_CMEMBLOCK_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

struct CMemBlock
{
#define CMEMBLOCK_SIGNATURE 0xBADD00D
private:
	BYTE * m_pData;	// the actual data bytes of the bitmap.
protected:
	// rather dangerous functions.
	void MemLink( BYTE * pData )
	{
		ASSERT( m_pData == NULL );
		m_pData = pData;
	}
	void MemUnlink()
	{
		ASSERT( m_pData != NULL );
		m_pData = NULL;
	}
public:
	CMemBlock()
	{
		m_pData = NULL;
	}
	void Alloc( DWORD dwSize )
	{
		Free();
		if ( dwSize )
		{
#ifdef _DEBUG
			dwSize += 3*4;
#endif
#ifdef _WIN32
			m_pData = (BYTE*) GlobalLock( GlobalAlloc( GHND, dwSize ));
#else
			m_pData = malloc( dwSize );
#endif
			ASSERT( m_pData );
#ifdef _DEBUG
			dwSize -= 4;
			*((DWORD*)(m_pData+0)) = dwSize;
			*((DWORD*)(m_pData+4)) = CMEMBLOCK_SIGNATURE;
			*((DWORD*)(m_pData+dwSize)) = CMEMBLOCK_SIGNATURE;
			m_pData += 8;
#endif
		}
	}
#ifdef _DEBUG
	virtual bool IsValidDebug() const
	{
		if ( m_pData == NULL ) 
			return( true );
		const BYTE * pData = m_pData - 8;
		DWORD dwSize = *((const DWORD*)(pData+0));
		if ( *((const DWORD*)(pData+4)) != CMEMBLOCK_SIGNATURE )
			return( false );
		if ( *((const DWORD*)(pData+dwSize)) != CMEMBLOCK_SIGNATURE )
			return( false );
		return( true );
	}
	virtual bool IsValidDebug( void const * pTest ) const
	{
		// Is this a valid pointer inside this ?
		if ( pTest == NULL ) 
			return( false );
		if ( m_pData == NULL )
			return( false );
		if ( ! IsValidDebug())
			return( false );
		const BYTE * pData = m_pData - 8;
		DWORD dwSize = *((const DWORD*)(pData+0));
		int iOffset = (const BYTE *) pTest - m_pData;
		return( iOffset >= 0 && iOffset < dwSize );
	}
#endif
	void Free()
	{
		if ( m_pData != NULL )
		{
#ifdef _DEBUG
			ASSERT( IsValidDebug());
			m_pData -= 8;
#endif
#ifdef _WIN32
			HGLOBAL hData = GlobalHandle(m_pData);
			GlobalUnlock(hData);
			GlobalFree(hData);
#else
			free( m_pData );
#endif
			m_pData = NULL;
		}
	}
	BYTE * GetData() const
	{
#ifdef _DEBUG
		ASSERT( IsValidDebug());
#endif
		return( m_pData );
	}
	bool IsValid() const
	{
		return( m_pData != NULL );
	}
	~CMemBlock()
	{
		Free();
	}
};

#endif	// _INC_CMEMBLOCK_H

