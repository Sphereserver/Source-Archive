//
// CProfileData.cpp
// Copyright Menace Software (www.menasoft.com).
//

#include "graysvr.h"	// predef header.

//////////////////////////////////////////////////////////
// -CProfileData

void CProfileData::SetActive( int iSampleSec )
{
	m_iActiveWindowSec = iSampleSec;
	memset( m_CurTimes, 0, sizeof( m_CurTimes ));
	memset( m_PrvTimes, 0, sizeof( m_PrvTimes ));
	if ( ! m_iActiveWindowSec )
		return;

#ifdef _WIN32
	if ( ! QueryPerformanceFrequency((LARGE_INTEGER *) &m_Frequency ))
	{
		m_Frequency = 1000;	// milliSec freq = default.
	}
	if ( ! QueryPerformanceCounter((LARGE_INTEGER *) &m_CurTime ))
	{
		m_CurTime = GetTickCount();
	}
#else
	// ??? Find a high precision timer for LINUX.
	m_Frequency = 0;
	m_iActiveWindowSec = 0;
#endif
	m_CurTask = PROFILE_OVERHEAD;
	m_TimeTotal = 0;
}

void CProfileData::Start( PROFILE_TYPE id )
{
	// Stop the previous task and start a new one.
	ASSERT( id < PROFILE_TIME_QTY );
	if ( ! m_iActiveWindowSec )
		return;

	// Stop prev task.
	ASSERT( m_CurTask < PROFILE_TIME_QTY );

	if ( m_TimeTotal >= m_Frequency * m_iActiveWindowSec )
	{
		memcpy( m_PrvTimes, m_CurTimes, sizeof( m_PrvTimes ));
		memset( m_CurTimes, 0, sizeof( m_CurTimes ));
		m_TimeTotal = 0;

		// ??? If NT we can push these values out to the registry !
	}

	// Get the current precise time.
	LONGLONG CurTime;
#ifdef _WIN32
	if ( ! QueryPerformanceCounter((LARGE_INTEGER *) &CurTime ))
	{
		CurTime = GetTickCount();
	}
#else

#endif
	// accumulate the time for this task.
	LONGLONG Diff = ( CurTime - m_CurTime );
	m_TimeTotal += Diff;
	m_CurTimes[m_CurTask].m_Time += Diff;
	m_CurTimes[m_CurTask].m_iCount ++;

	// We are now on to the new task.
	m_CurTime = CurTime;
	m_CurTask = id;
}

LPCTSTR CProfileData::GetName( PROFILE_TYPE id ) const
{
	ASSERT( id < PROFILE_QTY );
	static LPCTSTR const sm_pszProfileName[PROFILE_QTY] =
	{
		"IDLE",
		"OVERHEAD",		// PROFILE_OVERHEAD
		"IRC",	// PROFILE_IRC
		"NETWORK_RX",	// Just get client info and monitor new client requests. No processing.
		"CLIENTS",		// Client processing.
		"NETWORK_TX",
		"CHARS",
		"ITEMS",
		"NPC_AI",
		"DATA_TX",
		"DATA_RX",
	};
	return( sm_pszProfileName[id] );
}

LPCTSTR CProfileData::GetDesc( PROFILE_TYPE id ) const
{
	ASSERT( id < PROFILE_QTY );
	TCHAR * pszTmp = Str_GetTemp();
	int iCount = m_PrvTimes[id].m_iCount;

	if ( id >= PROFILE_TIME_QTY )
	{
		sprintf( pszTmp, "%i bytes", (int) m_PrvTimes[id].m_Time );
	}
	else if ( m_Frequency )
	{
		sprintf( pszTmp, "%i.%04i sec [%i samples]",
			(int)( m_PrvTimes[id].m_Time / ( m_Frequency )),
			(int)((( m_PrvTimes[id].m_Time * 10000 ) / ( m_Frequency )) % 10000 ),
			iCount );
	}
	else
	{
		sprintf( pszTmp, "%i ticks [%i samples]",
			(int)( m_PrvTimes[id].m_Time ),
			iCount );
	}
	return( pszTmp );
}



