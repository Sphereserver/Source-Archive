//
// CQueue.cpp
//

#ifdef GRAY_SVR
#include "../graysvr/graysvr.h"
#else
#include "graycom.h"
#endif
#include "CQueue.h"

CQueueBytes::CQueueBytes()
{
	m_iDataQty = 0;
}

CQueueBytes::~CQueueBytes()
{
}

void CQueueBytes::Empty()
{
	m_iDataQty = 0;
}

const BYTE * CQueueBytes::RemoveDataLock() const
{
	return m_Mem.GetData();
}

void CQueueBytes::RemoveDataAmount( int iSize )
{
	// use up this read data. (from the start)
	ASSERT(iSize<= m_iDataQty);
	ASSERT(m_iDataQty <= m_Mem.GetDataLength());
	m_iDataQty -= iSize;
	memmove( m_Mem.GetData(), m_Mem.GetData()+iSize, m_iDataQty );
}

BYTE * CQueueBytes::AddNewDataLock( int iLen )
{
	// lock the queue to place this data in it.
	ASSERT(iLen>=0);
	int iLenNew = m_iDataQty + iLen;
	if ( iLenNew > m_Mem.GetDataLength() )
	{
		// re-alloc a bigger buffer. as needed.

		ASSERT(m_iDataQty<=m_Mem.GetDataLength());
		m_Mem.Resize( ( iLenNew + 0x1000 ) &~ 0xFFF );
	}

	return( m_Mem.GetData() + m_iDataQty );
}

void CQueueBytes::AddNewDataFinish( int iLen )
{
	// The data is now ready.
	m_iDataQty += iLen;
}

void CQueueBytes::AddNewData( const BYTE * pData, int iLen )
{
	// Add new data to the end of the queue.
	memcpy( AddNewDataLock(iLen), pData, iLen );
	AddNewDataFinish( iLen );
}
