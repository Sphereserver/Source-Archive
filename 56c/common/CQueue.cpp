/**
* @file CQueue.cpp
*/

#include "../graysvr/graysvr.h"

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

void CQueueBytes::RemoveDataAmount( size_t iSize )
{
	ADDTOCALLSTACK("CQueueBytes::RemoveDataAmount");
	// use up this read data. (from the start)
	if ( iSize > m_iDataQty )
		iSize = m_iDataQty;
	m_iDataQty -= iSize;
	memmove( m_Mem.GetData(), m_Mem.GetData()+iSize, m_iDataQty );
}

BYTE * CQueueBytes::AddNewDataLock( size_t iLen )
{
	ADDTOCALLSTACK("CQueueBytes::AddNewDataLock");
	// lock the queue to place this data in it.

	size_t iLenNew = m_iDataQty + iLen;
	if ( iLenNew > m_Mem.GetDataLength() )
	{
		// re-alloc a bigger buffer. as needed.

		ASSERT(m_iDataQty<=m_Mem.GetDataLength());
		m_Mem.Resize( ( iLenNew + 0x1000 ) &~ 0xFFF );
	}

	return( m_Mem.GetData() + m_iDataQty );
}

void CQueueBytes::AddNewDataFinish( size_t iLen )
{
	// The data is now ready.
	m_iDataQty += iLen;
}

void CQueueBytes::AddNewData( const BYTE * pData, size_t iLen )
{
	ADDTOCALLSTACK("CQueueBytes::AddNewData");
	// Add new data to the end of the queue.
	memcpy( AddNewDataLock(iLen), pData, iLen );
	AddNewDataFinish( iLen );
}
