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
    		m_pData = NULL;
    	}
    	BYTE * AllocBase( DWORD dwSize )
    	{
    		ASSERT(dwSize);
    #if 0 // def _DEBUG
    		dwSize += 3*4;
    #endif
    		BYTE * pData = new BYTE ;
    		ASSERT( pData );
    #if 0 // def _DEBUG
    #define CMEMBLOCK_SIGNATURE 0xBADD00D
    		dwSize -= 4;
    		*((DWORD*)(pData+0)) = dwSize;
    		*((DWORD*)(pData+4)) = CMEMBLOCK_SIGNATURE;
    		*((DWORD*)(pData+dwSize)) = CMEMBLOCK_SIGNATURE;
    		pData += 2*4;
    #endif
    		return( pData );
    	}
    public:
    	void Alloc( DWORD dwSize )
    	{
    		Free();
    		if ( dwSize )
    		{
    			m_pData = AllocBase(dwSize);
    		}
    	}
    #if 0 // def _DEBUG
    	virtual bool IsValidDebug() const
    	{
    		if ( m_pData == NULL )
    			return( true );
    		const BYTE * pData = m_pData - (2*4);
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
    		const BYTE * pData = m_pData - (2*4);
    		DWORD dwSize = *((const DWORD*)(pData+0));
    		int iOffset = (const BYTE *) pTest - m_pData;
    		return( iOffset >= 0 && ((DWORD) iOffset < dwSize ));
    	}
    #endif
    	void Free()
    	{
    		if ( m_pData != NULL )
    		{
    #if 0 // def _DEBUG
    			ASSERT( IsValidDebug());
    			m_pData -= (2*4);
    #endif
    			delete  m_pData;
    			m_pData = NULL;
    		}
    	}
    	BYTE * GetData() const
    	{
    		return( m_pData );
    	}
    	bool IsValid() const
    	{
    		// Is this valid to use now ?
    		if ( m_pData == NULL )
    			return( false );
    		return( true );
    	}
    	CMemBlock()
    	{
    		m_pData = NULL;
    	}
    	~CMemBlock()
    	{
    		Free();
    	}
    };
    
    struct CMemLenBlock : public CMemBlock
    {
    private:
    	DWORD m_dwLength;
    public:
    	CMemLenBlock()
    	{
    		m_dwLength = 0;
    	}
    	DWORD GetDataLength() const
    	{
    		return( m_dwLength );
    	}
    	void Alloc( DWORD dwSize )
    	{
    		m_dwLength = dwSize;
    		CMemBlock::Alloc(dwSize);
    	}
    	void Free()
    	{
    		m_dwLength = 0;
    		CMemBlock::Free();
    	}
    	void Resize( DWORD dwSizeNew )
    	{
    		ASSERT( dwSizeNew != m_dwLength );
    		BYTE * pDataNew = AllocBase( dwSizeNew );
    		ASSERT(pDataNew);
    		if ( GetData())
    		{
    			// move any existing data.
    			ASSERT(m_dwLength);
    			memcpy( pDataNew, GetData(), min( dwSizeNew, m_dwLength ));
    			CMemBlock::Free();
    		}
    		else
    		{
    			ASSERT(!m_dwLength);
    		}
    		m_dwLength = dwSizeNew;
    		MemLink( pDataNew );
    	}
    };
    
    
    #endif	// _INC_CMEMBLOCK_H