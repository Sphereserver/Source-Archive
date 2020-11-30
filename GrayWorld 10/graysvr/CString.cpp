//
// CString.CPP
//

#include "graysvr.h"	// predef header.

#ifndef _WIN32
void strupr( char * pText )
{
	// No portable UNIX equiv to this.
	for ( ;pText[0] != '\0'; pText++ )
	{
		pText[0] = toupper( pText[0] );
	}
}
#endif

const char CString :: sm_Nil = '\0'; // Use this instead of null.

void CString :: Empty()
{
    if ( m_pchData != &sm_Nil )
    {
        delete [] m_pchData;
        Init();
    }
}

int CString :: SetLength( int iLength )
{
	Empty();
    if ( iLength )
    {
        m_pchData = new char [ iLength + 1 ];
        if ( m_pchData == NULL )
        	Init();
       	else
	        m_iLength = iLength;
    }
    return( m_iLength );
}

const CString& CString :: Copy( const char * pStr )
{
    //DEBUG_MSG(( "CString:Copy" ));
    if ( pStr != m_pchData )    // Same string.
    {
        Empty();
        if ( pStr != NULL )
            if ( SetLength( strlen( pStr )))
                strcpy( m_pchData, pStr );
    }
    return *this;
}

int CString :: Format( const char * pStr, ... )
{
    va_list args;
    va_start( args, pStr );
	char * pTemp = GetTempStr();
    vsprintf( pTemp, pStr, args );
	Copy( pTemp );
	return( GetLength());
}

const CString CString :: operator+=(const char* psz)
{
	CString sNew;
	sNew.SetLength( GetLength() + strlen( psz ));
	strcpy( sNew.m_pchData, m_pchData );
	strcpy( sNew.m_pchData + GetLength(), psz );
	return( Copy( sNew ));
}

//***************************************************************************
// -CObList 

void CObListRec  :: RemoveSelf( void )
{
	// Remove myself from my parent list (if i have one)
	if ( m_pParent == NULL ) return;
	m_pParent->RemoveAt( this );
}

void CObList  :: DeleteAll( void )
{
	// delete all entries.
	// Don't bother removing from the list as they are all to die anyhow.
	CObListRec * pRec = GetHead();
	while ( 1 )	// iterate the list.
	{
		if ( pRec == NULL ) break;
		CObListRec * pNext = pRec->GetNext();
		delete pRec;
		pRec = pNext;
	}
	m_iCount = 0;
	m_pHead = NULL;
	m_pTail = NULL;
}

void CObList :: RemoveAt( CObListRec * pRec )
{
	// just remove from list. DON'T delete !
	if ( pRec == NULL ) return;
	ASSERT( pRec->m_pParent == this );

	CObListRec * pNext = pRec->GetNext();
	CObListRec * pPrev = pRec->GetPrev();

	if ( pNext != NULL )
		pNext->m_pPrev = pPrev;
	else
		m_pTail = pPrev;
	if ( pPrev != NULL )
		pPrev->m_pNext = pNext;
	else
		m_pHead = pNext;
	pRec->m_pParent = NULL;	// We are now unlinked.
	m_iCount --;
}

void CObList  :: InsertAfter( CObListRec * pNewRec, CObListRec * pPrev )
{
	// Add after pPrev.
	// pPrev = NULL == add to the start.

	ASSERT( pNewRec != NULL );
	pNewRec->RemoveSelf();	// Get out of previous list first.

	pNewRec->m_pParent = this;
#ifdef _DEBUG
	if ( pPrev != NULL ) { ASSERT( pPrev->m_pParent == this ); }
#endif
	pNewRec->m_pPrev = pPrev;

	CObListRec * pNext;
	if ( pPrev == NULL )		// Its the first.
	{
		pNext = GetHead();
		m_pHead = pNewRec;
	}
	else
	{
		ASSERT( pPrev->m_pParent == this );
		pNext = pPrev->GetNext();
		pPrev->m_pNext = pNewRec;
	}

	pNewRec->m_pNext = pNext;
	if ( pNext != NULL )
		pNext->m_pPrev = pNewRec;
	else
		m_pTail = pNewRec;
	m_iCount ++;
}

void CObList :: DeleteAt( CObListRec * pElement )
{
	RemoveAt( pElement );
	delete pElement;
}

