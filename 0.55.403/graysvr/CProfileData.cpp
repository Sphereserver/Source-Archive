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
	memset( m_AvgTimes, 0, sizeof( m_AvgTimes ));
	memset( m_CurTimes, 0, sizeof( m_CurTimes ));
	memset( m_PrvTimes, 0, sizeof( m_PrvTimes ));
	m_iAvgCount		= 1;

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
	m_Frequency		= 1000;
	m_CurTime		= GetTickCount();	// our own function
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
		for ( int i = 0; i < PROFILE_QTY; i++ )
		{
			if ( m_iAvgCount < 4 )
				memcpy( m_AvgTimes, m_CurTimes, sizeof( m_AvgTimes ));
			else
			{
			if ( m_PrvTimes[i].m_Time > m_Frequency )
				m_PrvTimes[i].m_Time	= m_Frequency ;
			m_AvgTimes[i].m_Time	= (((m_AvgTimes[i].m_Time * 90)
									+ (m_PrvTimes[i].m_Time*10))/100);
			m_AvgTimes[i].m_iCount	= (((m_AvgTimes[i].m_iCount * 95)
									+ (m_PrvTimes[i].m_iCount*10))/100);
			}
		}

		++m_iAvgCount;

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
	CurTime		= GetTickCount();	// our own function
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




PROFILE_TYPE	CProfileData::GetCurrentTask()
{
	return m_CurTask;
}


LPCTSTR CProfileData::GetName( PROFILE_TYPE id ) const
{
	ASSERT( id < PROFILE_QTY );
	static LPCTSTR const sm_pszProfileName[PROFILE_QTY] =
	{
		"IDLE",
		"OVERHEAD",		// PROFILE_OVERHEAD
#ifndef BUILD_WITHOUT_IRCSERVER
		"IRC",	// PROFILE_IRC
#endif
		"NETWORK_RX",	// Just get client info and monitor new client requests. No processing.
		"CLIENTS",		// Client processing.
		"NETWORK_TX",
		"CHARS",
		"ITEMS",
		"NPC_AI",
		"SCRIPTS",
		"DATA_TX",
		"DATA_RX",
	};
	return( sm_pszProfileName[id] );
}

LPCTSTR CProfileData::GetDesc( PROFILE_TYPE id ) const
{
	ASSERT( id < PROFILE_QTY );
	TCHAR * pszTmp = Str_GetTemp();
	int iCount		= m_PrvTimes[id].m_iCount;


	if ( id >= PROFILE_TIME_QTY )
	{
		sprintf( pszTmp, "%i (avg: %i) bytes", (int) m_PrvTimes[id].m_Time, m_AvgTimes[id].m_Time );
	}
	else if ( m_Frequency )
	{
		sprintf( pszTmp, "%i.%04is   avg: %i.%04is     [samples:%5i  avg:%5i ]  runtime: %is",
			(int)( m_PrvTimes[id].m_Time / ( m_Frequency )),
			(int)((( m_PrvTimes[id].m_Time * 10000 ) / ( m_Frequency )) % 10000 ),
			(int) ( m_AvgTimes[id].m_Time / ( m_Frequency )),
			(int) ((( m_AvgTimes[id].m_Time * 10000 ) / ( m_Frequency )) % 10000 ),
			iCount,
			(int) m_AvgTimes[id].m_iCount,
			m_iAvgCount );
	}
	else
	{
		sprintf( pszTmp, "%i (%i avg) ticks [%i (%i avg) samples]",
			(int) m_PrvTimes[id].m_Time,
			(int) m_AvgTimes[id].m_Time,
			iCount,
			(int) m_AvgTimes[id].m_iCount );
	}
	return( pszTmp );
}



