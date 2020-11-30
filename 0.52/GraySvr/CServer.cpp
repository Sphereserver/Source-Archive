//
// CServer.cpp
// Copyright Menace Software (www.menasoft.com).
//

#ifdef VISUAL_SPHERE
	#include "..\Visual Sphere\stdafx.h"
	#include "..\Visual Sphere\Visual Sphere.h"
	#include "..\Visual Sphere\ServerObject.h"
#else
	#include "graysvr.h"	// predef header.
#endif

#include <signal.h>

#ifdef _WIN32
#include "../common/cassoc.h"
#endif

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

const TCHAR * CProfileData::GetName( PROFILE_TYPE id ) const
{
	ASSERT( id < PROFILE_QTY );
	static const TCHAR * pszProfileName[PROFILE_QTY] =
	{
		"OVERHEAD",
		"IDLE",
		"NETWORK_RX",	// Just get client info and monitor new client requests. No processing.
		"CLIENTS",		// Client processing.
		"NETWORK_TX",
		"CHARS",
		"ITEMS",
		"NPC_AI",
		"DATA_TX",
		"DATA_RX",
	};
	return( pszProfileName[id] );
}

const TCHAR * CProfileData::GetDesc( PROFILE_TYPE id ) const
{
	ASSERT( id < PROFILE_QTY );
	TCHAR * pszTmp = GetTempStr();
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

////////////////////////////////////////////////////////
// -CTextConsole

CChar * CTextConsole::GetChar() const
{
	return((CChar *) dynamic_cast <const CChar *>( this ));
}

int CTextConsole::OnConsoleKey( CGString & sText, TCHAR nChar, bool fEcho )
{
	// RETURN:
	//  0 = dump this connection.
	//  1 = keep processing.
	//  2 = process this.

	if ( sText.GetLength() >= 80 )
	{
		SysMessage( "Command too long\n" );
		sText.Empty();
		return( 0 );
	}

	if ( nChar == '\r' || nChar == '\n' )
	{
		if ( fEcho )
		{
			SysMessage( "\n" );
		}
		return 2;
	}

	if ( fEcho )
	{
		// Echo
		TCHAR szTmp[2];
		szTmp[0] = nChar;
		szTmp[1] = '\0';
		SysMessage( szTmp );
	}

	if ( nChar == 8 )
	{
		if ( sText.GetLength())	// back key
		{
			sText.SetLength( sText.GetLength() - 1 );
		}
		return 1;
	}

	sText += nChar;
	return 1;
}

//////////////////////////////////////////////////////////

class CServersSortArray : public CGPtrTypeArray<CServRef *>
{
	int m_iType;	// SC_TYPE
private:
	int QCompare( int i, CServRef * pServ );
	void QSort( int left, int right );

public:
	void SortByType( int iType );
};

int CServersSortArray::QCompare( int left, CServRef * pServRef )
{
	// 1 = left > right
	ASSERT(pServRef);
	switch ( m_iType )
	{
	case 2:	// ClientsAvg
		return( pServRef->GetClientsAvg() - GetAt(left)->GetClientsAvg());

	case 3:	// Age
		return( pServRef->GetAgeHours() - GetAt(left)->GetAgeHours());

	case 4:	// Lang
		return( strcmpi( pServRef->m_sLang, GetAt(left)->m_sLang ));

	case 5:	// Ver
		return( strcmpi( pServRef->m_sVersion, GetAt(left)->m_sVersion ));

	case 6: // TZ
		return( pServRef->m_TimeZone - GetAt(left)->m_TimeZone );

	}
	return( 0 );
}

void CServersSortArray::QSort( int left, int right )
{
	int j = left;
	int i = right;

	CServRef * pServRef = GetAt( (left+right) / 2 );

	do 
	{
		while ( QCompare( j, pServRef ) < 0 ) j++;
		while ( QCompare( i, pServRef ) > 0 ) i--;

		if ( i >= j )
		{
			if ( i != j )
			{
				CServRef * pServTmp = GetAt(i);
				SetAt( i, GetAt(j));
				SetAt( j, pServTmp );
			}
			i--;
			j++;
		}

	} while (j <= i);

	if (left < i)  QSort(left,i);
	if (j < right) QSort(j,right);
}

void CServersSortArray::SortByType( int iType )
{
	// 2 = ClientsAvg
	// 3 = Age
	// 4 = Lang

	if ( iType < 1 || iType > 6 ) 
		return;

	int iSize = g_Serv.m_Servers.GetCount();
	SetCount( iSize );
	memcpy( GetBasePtr(), g_Serv.m_Servers.GetBasePtr(), sizeof(CServRef *) * iSize );

	if ( iType == 1 || iSize <= 1 )
		return;

	m_iType = iType;

	// Now quicksort the list to the needed format.
	QSort( 0, iSize-1 );
}

//////////////////////////////////////////////////////////

CWebPageDef::CWebPageDef( int id )
{
	// Web page m_sWebPageFilePath
	m_NextUpdateTime = 0;
	m_iUpdatePeriod = 2*60*TICK_PER_SEC;

	m_sClientListFormat = "<tr><td>%NAME%</td><td>%REGION.NAME%</td></tr>\n";
	m_sServListFormat = "<tr><td>%SERVNAME%</td><td>%STATUS%</td></tr>\n";

	if ( id )
	{
		m_sSrcFilePath.Format( GRAY_FILE "statusbase%d.htm", id + 1 );
	}
	else
	{
		m_sSrcFilePath = GRAY_FILE "statusbase.htm";
	}
}

const TCHAR * CWebPageDef::sm_KeyTable[] =
{
	"WEBCLIENTLISTFORM",	// m_sWebClientListFormat
	"WEBPAGEDST",
	"WEBPAGEFILE",
	"WEBPAGESRC",
	"WEBPAGEUPDATE",
	"WEBSERVERLISTFORM",	// m_sWebServListFormat
};

bool CWebPageDef::r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc )
{
	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0: // "WEBCLIENTLISTFORM",
		sVal = m_sClientListFormat;
		break;
	case 1:	// 	"WEBPAGEDST",
	case 2:	// "WEBPAGEFILE"
		sVal = m_sDstFilePath;
		break;
	case 3:	// "WEBPAGESRC"
		sVal = m_sSrcFilePath;
		break;
	case 4:    // "WEBPAGEUPDATE"
		sVal.FormatVal( m_iUpdatePeriod / TICK_PER_SEC );
		break;
	case 5: // "WEBSERVERLISTFORM"
		sVal = m_sServListFormat;
		break;
	default:
		return( g_Serv.r_WriteVal( pKey, sVal, pSrc ));
	}
	return( true );
}

bool CWebPageDef::r_LoadVal( CScript & s ) // Load an item Script
{
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0: // "WEBCLIENTLISTFORM",
		m_sClientListFormat = MakeFilteredStr( s.GetArgStr());
		break;
	case 1:	// 	"WEBPAGEDST",
	case 2:	// "WEBPAGEFILE"
		m_sDstFilePath = s.GetArgStr();
		break;
	case 3:   // "WEBPAGESRC"
		m_sSrcFilePath = s.GetArgStr();
		break;
	case 4:    // "WEBPAGEUPDATE"
		m_iUpdatePeriod = s.GetArgVal()*TICK_PER_SEC;
		break;
	case 5: // "WEBSERVERLISTFORM"
		m_sServListFormat = MakeFilteredStr( s.GetArgStr());
		break;
	default:
		return( CScriptObj::r_LoadVal( s ));
	}
	return( true );
}

int CWebPageDef::sm_iServListIndex;

void CWebPageDef::OutputServList( CFileText & FileOut, CServRef ** ppArray, int iCount )
{
	sm_iServListIndex = 0;
	for ( int i=0; i<iCount; i++ )
	{
		CServRef * pServ = ppArray[i];
		if ( pServ == NULL )
			continue;

		if ( pServ->GetTimeSinceLastValid() > ( TICK_PER_SEC * 60 * 60 * 8 ))
			continue;

		sm_iServListIndex ++;

		TCHAR szTmp[ MAX_SCRIPT_LINE_LEN ];
		strcpy( szTmp, m_sServListFormat );
		pServ->ParseText( szTmp, &g_Serv, '%', '%' );
		FileOut.WriteStr( szTmp );
	}
}

void CWebPageDef::WebPageUpdate( bool fNow )
{
	// Generate the status web pages.
	// Read in the Base status page "*STATUSBASE.HTM"
	// Filter the XML type tags.
	// Output the status page "*STATUS.HTM"
	// Server name
	// Server email
	// Number of clients, items, NPC's

//WEBPAGEUPDATE=100			// update web pages this often.
//WEBPAGESRC=d:\menace\graysvr\spheres1t.htm
//WEBPAGEDST=d:\menace\graysvr\spheres1.htm
//WEBSERVERLISTFORM=<tr %SERVLISTCOL%><td></td><td>%URLLINK%</td><td>%AGE% days</td><td>%CLIENTSAVG%</td><td>%ACCAPP%</td><td>%VER%</td><td>%LANG%</td><td>%NOTES%</td></tr>


	if ( ! fNow )
	{
		if ( m_iUpdatePeriod <= 0 )
			return;
		if ( g_World.GetTime() < m_NextUpdateTime )
			return;
	}

	m_NextUpdateTime = g_World.GetTime() + m_iUpdatePeriod;

	if ( m_sDstFilePath.IsEmpty())
		return;

	// DEBUG_MSG(( "Web Page Update '%s'\n", (const TCHAR*) m_sSrcFilePath ));

	CFileText FileRead;
	if ( ! FileRead.Open( m_sSrcFilePath ))
	{
		DEBUG_ERR(( "Can't open web page input '%s'\n", (const TCHAR*) m_sSrcFilePath ));
		return;
	}

	CFileText FileOut;
	if ( ! FileOut.Open( m_sDstFilePath, OF_WRITE|OF_TEXT ))
	{
		DEBUG_ERR(( "Can't open web page output '%s'\n", (const TCHAR*) m_sDstFilePath ));
		return;
	}

	TCHAR szTmp[ MAX_SCRIPT_LINE_LEN ];
	while ( FileRead.ReadLine( szTmp, sizeof( szTmp )))
	{
		// Look for <tr> </tr> to repeat clients list ?
		if ( ! strnicmp( szTmp, "%CLIENTLIST%", 12 ))
		{
			for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
			{
				CChar * pChar = pClient->GetChar();
				if ( pChar == NULL )
					continue;
				if ( pChar->IsDND())
					continue;
				DEBUG_CHECK( ! pChar->IsWeird());
				strcpy( szTmp, m_sClientListFormat );
				pChar->ParseText( szTmp, &g_Serv, '%', '%' );
				FileOut.WriteStr( szTmp );
			}
			continue;
		}

		if ( ! strnicmp( szTmp, "%SERVERLIST%", 12 ) ||
			! strnicmp( szTmp, "%SERVLIST1%", 11 ) ||
			! strnicmp( szTmp, "%SERVLIST%", 10 ))
		{
			// default alpha name sorted list.
			OutputServList( FileOut, g_Serv.m_Servers.GetBasePtr(), g_Serv.m_Servers.GetCount());
			continue;
		}

		if ( ! strnicmp( szTmp, "%SERVLIST", 9 ))
		{
			CServersSortArray Array;
			Array.SortByType( atoi( szTmp+9 ));
			OutputServList( FileOut, Array.GetBasePtr(), Array.GetCount());
			continue;
		}

		// Look for stuff we can displace here.
		ParseText( szTmp, &g_Serv, '%', '%' );
		FileOut.WriteStr( szTmp );
	}
}

//////////////////////////////////////////////////////////
// Def classes

void CAdvanceDef::Load( TCHAR * pszDef )
{
	int Arg_piCmd[3];
	int Arg_Qty = ParseCmds( pszDef, Arg_piCmd, COUNTOF(Arg_piCmd));
	m_Rate[0] = Arg_piCmd[0];
	m_Rate[1] = Arg_piCmd[1];
	m_Rate[2] = Arg_piCmd[2];
};

int CAdvanceDef::GetChance( int iSkillAdj ) const
{
	// iSkillAdj = 0 - 1000
	// RETURN: percent chance of success * 10 = 0 - 1000.

	int iHiIdx = ( iSkillAdj >= 500 ) ? 0 : 1;
	int iHiVal = m_Rate[iHiIdx];
	int iLoVal = m_Rate[iHiIdx+1];
	int iDiffVal = (( iHiIdx ) ? 500 : 1000 ) - iSkillAdj;

	// How many uses for a gain of .1 (div by 100)
	int iChance = iHiVal - IMULDIV( iHiVal - iLoVal, iDiffVal, 500 );
	if ( iChance <= 0 ) return 0; // less than no chance ?

	// Express uses as a percentage * 10.
	return( 100000 / iChance );
}

const TCHAR * CSkillDef::sm_SkillsTable[] =
{
	"ADV_RATE",
	"BONUS_DEX",
	"BONUS_INT",
	"BONUS_STR",
	"DELAY",
	"FAIL_MSG",
	"KEY",
	"PROMPT_MSG",
	"SKILL_STAT",
	"STAT_DEX",
	"STAT_INT",
	"STAT_STR",
	"TITLE",
};

CSkillDef::CSkillDef( SKILL_TYPE skill, CScript & s )
{
	DEBUG_CHECK( CChar::IsSkillBase(skill));
	r_Load(s);
	g_Serv.m_SkillDefs.SetAtGrow(skill,this);

	// Add it to the insertion sort list of keys.
	CValStr val;
	val.SetValues( skill, GetKey());

	for ( int j=0; j<g_Serv.m_SkillKeySort.GetCount(); j++ )
	{
		const TCHAR * pszName = g_Serv.m_SkillKeySort[j].m_pszName;
		int iCompare = strcmpi( GetKey(), pszName );
		if ( iCompare == 0 )
		{
			DEBUG_CHECK( 0 );
			return;
		}
		if ( iCompare < 0 )
			break;
	}
	g_Serv.m_SkillKeySort.InsertAt( j, val );
}

bool CSkillDef::r_LoadVal( CScript &s )
{
	switch ( FindTableSorted( s.GetKey(), sm_SkillsTable, COUNTOF( sm_SkillsTable )))
	{
	case 0: // "ADV_RATE"
		m_Adv.Load( s.GetArgStr());
		break;
	case 1: // "BONUS_DEX"
		m_StatBonus[2] = s.GetArgVal();
		break;
	case 2: // "BONUS_INT"
		m_StatBonus[1] = s.GetArgVal();
		break;
	case 3: // "BONUS_STR"
		m_StatBonus[0] = s.GetArgVal();
		break;
	case 4:	// "DELAY"
		m_wDelay = s.GetArgVal();
		break;
	case 5: // "FAIL_MSG"
		m_sFailMsg = s.GetArgStr();
		break;
	case 6: // "KEY"
		m_sKey = s.GetArgStr();
		break;
	case 7: // "PROMPT_MSG"
		m_sTargetPrompt = s.GetArgStr();
		break;
	case 8: // "SKILL_STAT"
		m_SkillStat = s.GetArgVal();
		break;
	case 9: // "STAT_DEX"
		m_Stat[2] = s.GetArgVal();
		break;
	case 10: // "STAT_INT"
		m_Stat[1] = s.GetArgVal();
		break;
	case 11: // "STAT_STR"
		m_Stat[0] = s.GetArgVal();
		break;
	case 12: // "TITLE"
		m_sTitle = s.GetArgStr();
		break;
	default:
		return( false );
	}
	return( true );
}

const TCHAR * CSkillClassDef::sm_Table[] =
{
	"NAME",
	"PLOTFLAGS",
};

bool CSkillClassDef::r_LoadVal( CScript &s )
{
	switch ( FindTableSorted( s.GetKey(), sm_Table, COUNTOF( sm_Table )))
	{
	case 0: // "NAME"
		m_sName = s.GetArgStr();
		break;
	case 1: // "PLOTFLAGS"
		m_dwPlotFlags = s.GetArgVal();
		break;
	default:
		{
			int i = g_Serv.FindSkillKey( s.GetKey());
			if ( i != SKILL_NONE )
			{
				ASSERT( i<COUNTOF(m_SkillLevelMax));
				m_SkillLevelMax[i] = s.GetArgVal();
				break;
			}
		}
		return( false );
	}
	return( true );
}

const TCHAR * CPotionDef::sm_PotionTable[] =
{
	"COLOR",
	"ITEM",
	"NAME",
	"REAGENT",
	"REAGENT_AMT",
	"SKILL_MIN",
};

bool CPotionDef::r_LoadVal( CScript &s )
{
	switch ( FindTableSorted( s.GetKey(), sm_PotionTable, COUNTOF( sm_PotionTable )))
	{
	case 0: // "COLOR"
		m_color = (COLOR_TYPE) s.GetArgVal();
		break;
	case 1: // "ITEM"
		m_potionID = (ITEMID_TYPE) s.GetArgVal();
		break;
	case 2: // "NAME"
		m_sName = s.GetArgStr();
		break;
	case 3: // "REAGENT"
		m_RegID = GetResourceItem( s.m_pArg );
		break;
	case 4: // "REAGENT_AMT"
		m_RegReq = s.GetArgVal();
		break;
	case 5: // "SKILL_MIN"
		m_SkillReq = s.GetArgVal();
		break;
	default:
		return( false );
	}
	return( true );
}

const TCHAR * CSpellDef::sm_SpellsTable[] =
{
	"CAST_TIME",
	"DURATION_HI",
	"DURATION_LO",
	"EFFECT_HI",
	"EFFECT_ID",
	"EFFECT_LO",
	"FLAGS",
	"NAME",
	"REAGENTS",
	"RUNE_ITEM",
	"RUNES",
	"SCROLL_ITEM",
	"SOUND",
};

bool CSpellDef::r_LoadVal( CScript &s )
{
	switch ( FindTableSorted( s.GetKey(), sm_SpellsTable, COUNTOF( sm_SpellsTable )))
	{
	case 0:	// "CAST_TIME"
		m_wCastTime = s.GetArgVal();	// In tenths.
		break;
	case 1:	// "DURATION_HI"
		m_wDurationTimeHi = s.GetArgVal();
		break;
	case 2:	// "DURATION_LO"
		m_wDurationTimeLo = s.GetArgVal();
		break;
	case 3: // "EFFECT_HI"
		m_wEffectHi = s.GetArgVal();
		break;
	case 4: // "EFFECT_ID"
		m_wEffectID = (ITEMID_TYPE) s.GetArgVal();
		break;
	case 5: // "EFFECT_LO"
		m_wEffectLo = s.GetArgVal();
		break;
	case 6: // "FLAGS"
		m_wFlags = s.GetArgVal();
		break;
	case 7: // "NAME"
		m_sName = s.GetArgStr();
		break;
	case 8: // "REAGENTS"
		m_sReags = s.GetArgStr();
		break;
	case 9: // "RUNE_ITEM"
		m_SpellID = (ITEMID_TYPE) s.GetArgVal();
		break;
	case 10: // "RUNES"
		// This may only be basic chars !
		m_sRunes = s.GetArgStr();
		break;
	case 11: // "SCROLL_ITEM"
		m_ScrollID = (ITEMID_TYPE) s.GetArgVal();
		break;
	case 12: // "SOUND"
		m_sound = s.GetArgVal();
		break;
	default:
		return( false );
	}
	return( true );
}

const TCHAR * CRandGroupDef::sm_Table[] =
{
	"ID",
	"NAME",
	"WEIGHT",
};

bool CRandGroupDef::r_LoadVal( CScript &s )
{
	switch ( FindTableSorted( s.GetKey(), sm_Table, COUNTOF( sm_Table )))
	{
	case 0:	// "ID"
		{
			TCHAR * ppCmd[2];
			int iArgs = ParseCmds( s.GetArgStr(), ppCmd, COUNTOF(ppCmd));
			CRandGroupRec rec;
			rec.m_dwVal = Exp_GetHex( ppCmd[0] );
			rec.m_iWeight = ( iArgs > 1 && ppCmd[1][0] ) ? Exp_GetVal( ppCmd[1] ) : 1;
			m_iTotalWeight += rec.m_iWeight;
			m_Members.Add(rec);
		}
		return( true );
	case 1: // "NAME"
		m_sName = s.GetArgStr();
		return( true );
	case 2: // "WEIGHT"
		if ( ! m_Members.GetCount())
			break;
		{
			m_iTotalWeight -= m_Members[ m_Members.GetCount() -1 ].m_iWeight;
			int iWeight = s.GetArgVal();
			m_Members[ m_Members.GetCount() -1 ].m_iWeight = iWeight;
			m_iTotalWeight += iWeight;
		}
		return( true );
	}
	return( false );
}

COreDef::COreDef( CScript & s )
{
	// set defaults first.
	// Merge this into the MATERIAL_TYPE stuff ???

	m_wColor = COLOR_DEFAULT;
	m_iVeinChance = 0;
	m_iMinSkill = INT_MAX;
	m_iMinAmount = 0;
	m_iMaxAmount = 0;
	m_wIngotItem = ITEMID_INGOT_IRON;

	TCHAR * pszDef;
	TCHAR * ppCmd[10];
	int iCmds = ParseCmds( s.GetArgStr(), ppCmd, COUNTOF(ppCmd), "," );
	if ( iCmds < 5 )
	{
badscript:
		DEBUG_ERR(( "Bad Ore definition '%s'\n", s.GetArgStr()));
		return;
	}

	// ??? is this a dupe ?

	if ( ppCmd[0][0] == '\0' )
		goto badscript;

	// This is the name
	m_sName = ppCmd[0];

	// This is the color
	m_wColor = Exp_GetVal( ppCmd[1] );

	// This is the min skill
	m_iMinSkill = Exp_GetVal( ppCmd[2] );

	// This is the Vein Chance
	m_iVeinChance = Exp_GetVal( ppCmd[3] );

	pszDef = ppCmd[4];
	m_iMinAmount = Exp_GetSingle( pszDef );
	if ( pszDef[0] == '-' )
		pszDef++;
	m_iMaxAmount = Exp_GetSingle( pszDef );
	if ( m_iMaxAmount < m_iMinAmount)
	{
		int iTmp = m_iMaxAmount;
		m_iMaxAmount = m_iMinAmount;
		m_iMinAmount = iTmp;
	}

	if ( iCmds >= 6 )
	{
		pszDef = ppCmd[5];
		if ( pszDef[0] != 0x00 )
			m_wIngotItem = ( ITEMID_TYPE ) Exp_GetHex( pszDef );
	}
}

/////////////////////////////////////////////////
// Blocked IP's

void CLogIP::SetPingTime()
{
	if ( ! m_iPings )
	{
		m_Time = g_World.GetTime();
	}
	m_iPings ++;
}

time_t CLogIP::GetTimeAge() const
{
	return( g_World.GetTime() - m_Time );
}

#define LOGIP_RECYCLE_TIME (10 * TICK_PER_SEC)

CLogIP * CServer::FindLogIP( in_addr IP, bool fCreate )
{
	if ( IP.s_addr == 0xFFFFFFFF || IP.s_addr == 0 )
	{
		// this address is always blocked.
		return( NULL );
	}

	// Will create if not found.
	for ( int i=0; i<m_LogIP.GetCount(); i++ )
	{
		CLogIP * pLogIP = m_LogIP[i];
		if ( pLogIP->GetTimeAge() >= LOGIP_RECYCLE_TIME )
		{
			pLogIP->ResetTime();
		}

		if ( pLogIP->IsSameIP( IP ))
			return( pLogIP );

		// Is this ip > 60 seconds old ?
		if ( ! pLogIP->GetPings())
		{
			if ( ! pLogIP->IsBlocked())
			{
				// remove it.
				m_LogIP.DeleteAt( i );
				i--;
			}
		}
	}

	// Create a new 1.
	if ( ! fCreate )
		return( NULL );

	CLogIP * pLogIP = new CLogIP( IP );
	m_LogIP.Add( pLogIP );
	return( pLogIP );
}

////////////////////////////////////////////////////////////////////////////////////////
// -CServer

CServer::CServer() : CServRef( GRAY_TITLE, SOCKET_LOCAL_ADDRESS )
{
	m_wExitFlag = 0;
	m_iExitCode = 1;	// we are in start up mode. // IsLoading()

	m_Clock_Shutdown = 0;
	m_Clock_Periodic = 0;

	m_iAdminClients = 0;
	m_fResyncPause = false;
	m_fUseNTService = false;
	m_iPollServers = 0;
	m_sRegisterServer = GRAY_MAIN_SERVER;
	m_fMainLogServer = false;
	m_iMapCacheTime = 2 * 60 * TICK_PER_SEC;
	m_iSectorSleepMask = 0x1ff;

	m_wDebugFlags = 0; //DEBUGF_NPC_EMOTE
	m_fSecure = true;
	m_iFreezeRestartTime = 10*TICK_PER_SEC;

	//Magic
	m_fReagentsRequired = true;
	m_fReagentLossFail = true;
	m_fWordsOfPowerPlayer = true;
	m_fWordsOfPowerStaff = true;
	m_fEquippedCast = true;
	m_iMagicUnlockDoor = 1000;

	m_iSpell_Teleport_Effect_Staff = ITEMID_FX_FLAMESTRIKE;	// drama
	m_iSpell_Teleport_Sound_Staff = 0x1f3;
	//m_iSpell_Teleport_Effect_Player = ;
	//m_iSpell_Teleport_Sound_Player;

	// Decay
	m_iDecay_Item = 30*60*TICK_PER_SEC;
	m_iDecay_CorpsePlayer = 45*60*TICK_PER_SEC;
	m_iDecay_CorpseNPC = 15*60*TICK_PER_SEC;

	// Accounts
	m_nClientsMax = FD_SETSIZE-1;
	m_fRequireEmail = false;
	m_nGuestsMax = 0;
	m_nGuestsCur = 0;
	m_fArriveDepartMsg = true;
	m_iClientLingerTime = 60 * TICK_PER_SEC;
	m_iDeadSocketTimeMin = 10*60*TICK_PER_SEC;
	m_iMinCharDeleteTime = 3*24*60*60*TICK_PER_SEC;
	m_iMaxCharsPerAccount = MAX_CHARS_PER_ACCT;

	// Save
	m_iSaveBackupLevels = 3;
	m_iSaveBackgroundTime = 5* 60 * TICK_PER_SEC;	// Use the new background save.
	m_fSaveGarbageCollect = false;	// Always force a full garbage collection.
	m_iSavePeriod = 15*60*TICK_PER_SEC;
	m_fSaveBinary = false;

	// In game effects.
	m_fMonsterFight = true;
	m_fMonsterFear = true;
	m_iLightDungeon = 17;
	m_iLightNight = 17;	// dark before t2a.
	m_iLightDay = LIGHT_BRIGHT;
	m_iBankIMax = 1000;
	m_iBankWMax = 400 * WEIGHT_UNITS;
	m_fGuardsInstantKill = false;
	m_iSnoopCriminal = 500;
	m_iTrainSkillPercent = 50;
	m_iTrainSkillMax = 500;
	m_fCharTags = true;
	m_iVendorMaxSell = 30;
	m_iGameMinuteLength = 8 * TICK_PER_SEC;
	m_sSpeechBaseDir = "speech";
	m_fNoWeather = false;
	m_fFlipDroppedItems = true;
	m_iAvgSumOfStats = 300;
	m_iMaxSumOfStats = 350;
	m_iMurderMinCount = 5;
	m_iMurderDecayTime = 8*60*60* TICK_PER_SEC;
	m_iMaxComplexity = 16;
	m_iPlayerKarmaNeutral = -2000; // How much bad karma makes a player neutral?
	m_iPlayerKarmaEvil = -8000;
	m_iGuardLingerTime = 1*60*TICK_PER_SEC; // "GUARDLINGER",
	m_iCriminalTimer = 3*60*TICK_PER_SEC;
	m_iHitpointPercentOnRez = 10;
	m_fLootingIsACrime = true;
	m_fHelpingCriminalsIsACrime = true;
	m_fPlayerGhostSounds = true;
	m_fAutoNewbieKeys = true;
	m_iMaxBaseSkill = 250;
	m_iStamRunningPenalty = 50;
	m_iStaminaLossAtWeight = 100;

	m_ClientVersion.SetClientVersion( GRAY_CLIENT_VER );
}

CServer::~CServer()
{
	Unload();
	m_ItemBase.RemoveAll();
	m_CharBase.RemoveAll();
}

void CServer::LoadChangedFiles()
{
	// Look for files in the changed directory.
	// m_Scripts
	// *ACCT.SCP
	// m_FragFiles
	// m_WebPages

	if ( m_sChangedBaseDir.IsEmpty())
		return;

	// If there is a file here and i can open it exlusively then do a resync.
#ifdef _WIN32
	CFileList filelist;
	int iRet = filelist.ReadDir( m_sChangedBaseDir );
	if ( iRet < 0 )
	{
		DEBUG_MSG(( "DirList=%d for '%s'\n", iRet, (const TCHAR*) m_sChangedBaseDir ));
		return;
	}
	if ( iRet <= 0 )	// no files here.
		return;

	bool fSetResync = false;
	TCHAR szTmpDir[ _MAX_PATH ];
	strcpy( szTmpDir, m_sChangedBaseDir );
	GetStrippedDirName( szTmpDir );

	CGStringListRec * psFile = filelist.GetHead();
	for ( ; psFile; psFile = psFile->GetNext())
	{
		CGString sPathName = GetMergedFileName( szTmpDir, (const TCHAR*) *psFile );
		DEBUG_MSG(( "LoadChange to '%s'='%s'\n", (const TCHAR*) *psFile, (const TCHAR*) sPathName ));

		// Is the file matching one of my system files ?


#if 0
		if ( ! m_fResyncPause )
		{
			fSetResync = true;
			SetResyncPause( true, this );
		}

		// now delete the file. we are done with it.
		// remove the full path.
#endif

	}

	if ( fSetResync )
	{
		SetResyncPause( false, this );
	}

#endif

}

void CServer::UpdatePeriodic( bool fNow )
{
	// Give a tick to the less critical stuff.

	if ( ! fNow )
	{
		if ( m_fResyncPause )
			return;
		if ( m_Clock_Periodic > g_World.GetTime())
			return;
	}

	for ( int i=0; i<m_WebPages.GetCount(); i++ )
	{
		if ( m_WebPages[i] == NULL )
			continue;
		m_WebPages[i]->WebPageUpdate( fNow );
	}

	LoadChangedFiles();

	m_Clock_Periodic = g_World.GetTime() + ( 60 * TICK_PER_SEC );
}

void CServer::SetResyncPause( bool fPause, CTextConsole * pSrc )
{
	ASSERT(pSrc);
	m_fResyncPause = fPause;
	if ( fPause )
	{
		pSrc->SysMessage( "Server is PAUSED for Resync.\n" );
		g_World.Broadcast( "Server is being PAUSED for Resync." );

		// Unlock all the SCP files.
		Unload();
		g_World.UnLoadRegions();
	}
	else
	{
		pSrc->SysMessage( "Resync Restart\n" );

		// Set all the SCP files to re-index.
		// Relock the SCP files.
		m_iExitCode = 1;	// IsLoading()
		g_World.LoadRegions();
		if ( ! Load())
		{
			pSrc->SysMessage( "Resync FAILED!\n" );
			g_World.Broadcast( "Resync FAILED!" );
		}
		else
		{
			pSrc->SysMessage( "Resync Complete!\n" );
			g_World.Broadcast( "Resync Complete!" );
		}
		m_iExitCode = 0;	// ready to go. ! IsLoading()
	}
}

bool CServer::OnConsoleCmd( CGString & sText, CTextConsole * pSrc )
{
	// RETURN: false = boot the client.

	if ( sText.GetLength() <= 0 )
		return( true );

	if ( sText.GetLength() > 1 )
	{
		TCHAR Cmd_Buffer[256];
		if ( IsConsoleCmd( sText[0] ))
			strncpy( Cmd_Buffer, sText+1, sizeof(Cmd_Buffer));
		else
			strncpy( Cmd_Buffer, sText, sizeof(Cmd_Buffer));
		Cmd_Buffer[sizeof(Cmd_Buffer)-1] = '\0';

		if ( ! r_Verb( CScript( Cmd_Buffer ), pSrc ))
		{
			TCHAR * Cmd_pArgs;
			Parse( Cmd_Buffer, &Cmd_pArgs );

			if ( ! OnServCmd( Cmd_Buffer, Cmd_pArgs, pSrc ))
			{
				pSrc->SysMessagef( "unknown command '%s'\n", sText );
			}
		}
		sText.Empty();
		return( true );
	}

	switch ( toupper( sText[0] ))
	{
	case '?':
		pSrc->SysMessagef(
			"Available Commands:\n"
			"# = Immediate Save world\n"
			"A = Accounts file update\n"
			"B message = Broadcast a message\n"
			"C = Clients List (%d)\n"
			"G = Garbage collection\n"
			"H = Hear all that is said (%s)\n"
			"I = Information\n"
			"L = Toggle log file (%s)\n"
			"P = Profile Info (%s)\n"
			"R = Resync Pause\n"
			"S = Secure mode toggle (%s)\n"
			"V = Verbose Mode (%s)\n"
			"X = immediate exit of the server\n"
			,
			m_Clients.GetCount(),
			g_Log.IsLoggedMask( LOGM_PLAYER_SPEAK ) ? "ON" : "OFF",
			g_Log.IsFileOpen() ? "OPEN" : "CLOSED",
			m_Profile.IsActive() ? "ON" : "OFF",
			m_fSecure ? "ON" : "OFF",
			g_Log.IsLogged( LOGL_TRACE ) ? "ON" : "OFF"
		);
		break;

	case 'H':	// Hear all said.
		r_Verb( CScript( "HEARALL" ), pSrc );
		break;
	case 'S':
		r_Verb( CScript( "SECURE" ), pSrc );
		break;
	case 'L': // Turn the log file on or off.
		r_Verb( CScript( "LOG" ), pSrc );
		break;
	case 'V':
		r_Verb( CScript( "VERBOSE" ), pSrc );
		break;

	case 'X':
		if (m_fSecure)
		{
			pSrc->SysMessage( "NOTE: Secure mode prevents keyboard exit!\n" );
		}
		else
		{
			g_Log.Event( LOGL_FATAL, "Immediate Shutdown initialized!\n");
			m_wExitFlag = 0xDEAD;
		}
		break;
	case 'A':
		// Force periodic stuff
		if ( m_fResyncPause )
		{
do_resync:
			pSrc->SysMessage( "Not allowed during resync pause. Use 'R' to restart.\n");
			break;
		}
		if ( g_World.IsSaving())
		{
do_saving:
			// Is this really true ???
			pSrc->SysMessage( "Not allowed during background worldsave. Use '#' to finish.\n");
			break;
		}
		g_World.SaveAccounts();
		UpdatePeriodic(true);
		break;
	case 'G':
		if ( m_fResyncPause ) goto do_resync;
		if ( g_World.IsSaving()) goto do_saving;
		g_World.GarbageCollection();
		break;
	case '#':
		// Start a syncronous save or finish a background save synchronously
		if ( m_fResyncPause ) goto do_resync;
		g_World.Save( true );
		break;
	case 'I':
		pSrc->SysMessage( GetStatusString( 0x22 ));
		pSrc->SysMessage( GetStatusString( 0x24 ));
		break;
	case 'C':
	case 'W':
		// List all clients on line.
		ListClients( pSrc );
		break;
	case 'R':
		// resync Pause mode. Allows resync of things in files.
		if ( g_World.IsSaving()) goto do_saving;
		SetResyncPause( !m_fResyncPause, pSrc );
		break;

#ifdef COMMENT
	case 'B':
		// Binary Save. (for test only)
		m_fSaveBinary = !m_fSaveBinary;
		pSrc->SysMessagef( "Binary Save = %s\n", m_fSaveBinary ? "ON" : "OFF" );
		break;
#endif

	case 'P':
		// Display profile information.
		// ? Peer status. Show status of other servers we list.
		{
			pSrc->SysMessagef( "Profiles %s: (%d sec total)\n", m_Profile.IsActive() ? "ON" : "OFF", m_Profile.GetActiveWindow());
			for ( int i=0; i < PROFILE_QTY; i++ )
			{
				pSrc->SysMessagef( "'%s' = %s\n", m_Profile.GetName((PROFILE_TYPE) i), m_Profile.GetDesc((PROFILE_TYPE) i ));
			}
		}
		break;

#ifdef _DEBUG
	case '^':	// hang forever.
		while (true)
		{
		}
		break;
#endif

	default:
		pSrc->SysMessagef( "unknown command '%c'\n", sText[0] );
		break;
	}

	sText.Empty();
	return( true );
}

bool CServer::OnServCmd( TCHAR * pszCmd, TCHAR * pszArgs, CTextConsole * pSrc )
{
	static const TCHAR * szCmds[] =	// Designer only commands
	{
		"ACCOUNT",
		"BLOCKIP",
		"EXPORT",
		"IMPORT",
		"RESTORE",
		"SAVE",
		"SERVLIST",
		"UNBLOCKIP",
	};

	switch ( FindTableSorted( pszCmd, szCmds, COUNTOF(szCmds)))
	{
	case 0: // "ACCOUNT"
		// Modify the accounts from on line.
		return g_World.OnAccountCmd( pszArgs, pSrc );

	case 1:	// "BLOCKIP"
		if ( SetLogIPBlock( pszArgs, true ))
		{
			pSrc->SysMessage( "IP Blocked\n" );
		}
		else
		{
			pSrc->SysMessage( "IP Already Blocked\n" );
		}
		break;
	case 2: // "EXPORT" name [chars] [area distance]
		if (pszArgs[0])
		{
			TCHAR * Arg_ppCmd[5];
			int Arg_Qty = ParseCmds( pszArgs, Arg_ppCmd, COUNTOF( Arg_ppCmd ));
			if ( ! Arg_Qty )
			{
				break;
			}
			if ( ! g_World.Export( Arg_ppCmd[0], pSrc->GetChar(),
				(Arg_Qty>=2)? atoi(Arg_ppCmd[1]) : 1,
				(Arg_Qty>=3)? atoi(Arg_ppCmd[2]) : 0xffff ))
			{
				pSrc->SysMessage( "Export failed\n" );
			}
		}
		break;
	case 3: // "IMPORT" name [flags] [area distance]
		if (pszArgs[0])
		{
			TCHAR * Arg_ppCmd[5];
			int Arg_Qty = ParseCmds( pszArgs, Arg_ppCmd, COUNTOF( Arg_ppCmd ));
			if ( ! Arg_Qty )
			{
				break;
			}
			if ( ! g_World.Import( Arg_ppCmd[0], pSrc->GetChar(),
				(Arg_Qty>=2)?atoi(Arg_ppCmd[1]) : 1,
				(Arg_Qty>=3)?atoi(Arg_ppCmd[2]) : 0xffff ))
			{
				pSrc->SysMessage( "Import failed\n" );
			}
			// addReSync();
		}
		break;
	case 4:	// "RESTORE" backupfile.SCP Account CharName
		if (pszArgs[0])
		{
			TCHAR * Arg_ppCmd[4];
			int Arg_Qty = ParseCmds( pszArgs, Arg_ppCmd, COUNTOF( Arg_ppCmd ));
			if ( ! Arg_Qty )
			{
				break;
			}
			if ( ! g_World.Import( Arg_ppCmd[0], pSrc->GetChar(),
				0x23, 0xffff, Arg_ppCmd[1], Arg_ppCmd[2] ))
			{
				pSrc->SysMessage( "Restore failed\n" );
			}
			else
			{
				pSrc->SysMessage( "Restore success\n" );
			}
		}
		break;
	case 5: // "SAVE" x
		g_World.Save( Exp_GetVal(pszArgs));
		break;
	case 6:	// "SERVLIST",
		ListServers( pSrc );
		break;
	case 7:	// "UNBLOCKIP"
		if ( SetLogIPBlock( pszArgs, false ))
		{
			pSrc->SysMessage( "IP UN-Blocked\n" );
		}
		else
		{
			pSrc->SysMessage( "IP Was NOT Blocked\n" );
		}
		break;
#if 0 // def _DEBUG
	case 8: // "GENOCIDE"
		// Loop thru the whole world and kill all thse creatures.
		if ( pszArgs[0] )
		{
			WORD id = Exp_GetVal(pszArgs);
		}
		break;
#endif
	default:
		return( false );
	}

	return( true );
}

#ifndef _WIN32
void _cdecl Signal_Terminate(int x=0) // If shutdown is initialized
{
	// LINUX specific stuff.
	g_Log.Event( LOGL_FATAL, "Signal:Server should terminate.\n" );
	throw CGrayError( LOGL_FATAL, x, "Signal_Terminate" );
}

void _cdecl Signal_Illegal_Instruction(int x=0)
{
	g_Log.Event( LOGL_FATAL, "Signal:Illegal Instruction caught.\n" );
	throw CGrayError( LOGL_FATAL, x, "Signal_Illegal_Instruction" );
}
#endif

void CServer::SetSignals()
{
#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN); // This appears when we try to write to a broken network connection

	if ( m_fSecure )
	{
		signal(SIGQUIT, &Signal_Terminate);
		signal(SIGTERM, &Signal_Terminate);
		signal(SIGINT, &Signal_Terminate);
		signal(SIGILL, &Signal_Illegal_Instruction);
	}
	else
	{
		signal(SIGTERM, SIG_DFL );
		signal(SIGQUIT, SIG_DFL );
		signal(SIGINT, SIG_DFL );
		signal(SIGILL, SIG_DFL);
	}
#endif
}

bool CServer::SetLogIPBlock( const TCHAR * szIP, bool fBlock )
{
	// Block or unblock an IP.
	// RETURN: true = set, false = already that state.
	in_addr dwIP;
	dwIP.s_addr = inet_addr( szIP );

	CLogIP * pLogIP = FindLogIP( dwIP, fBlock );
	if ( ! pLogIP )
		return( false );

	bool fPrevBlock = pLogIP->IsBlocked();
	if ( ! fBlock )
	{
		if ( ! fPrevBlock )
			return( false );	// not here.
		m_LogIP.DeleteOb( pLogIP );
	}
	else
	{
		if ( fPrevBlock )
			return( false );	// not here.
		pLogIP->SetBlocked( true );
	}

	// Write change to *.INI
	if ( ! IsLoading())
	{
		CScript s;
		if ( ! s.OpenFind( GRAY_FILE ".ini" )) // Open script file
			return( true );

		s.WriteProfileString( "BLOCKIP", szIP, fBlock ? "" : NULL );
	}

	return( true );
}

bool CServer::CheckLogIPBlocked( in_addr IP, bool fPreAccept )
{
	// ARGS:
	//  fPreAccept = is this the pre-accept stage?
	// NOTE:
	//  If the IP is blocked and this is the first ping then try to give a response.
	//

	CLogIP * pLogIP = FindLogIP( IP, true );
	if ( pLogIP == NULL )
		return( true );

	pLogIP->SetPingTime();

	if ( fPreAccept )	// always allow this thru just to shut up the normal clients.
	{
		if ( pLogIP->GetPings() <= 1 )
			return( false );
	}

	if ( pLogIP->GetTimeAge() < LOGIP_RECYCLE_TIME &&
		pLogIP->GetPings() > 4 )
	{
		// block for now.
		if ( pLogIP->GetPings() == 10 && ! pLogIP->IsBlocked())
		{
			g_Log.Event( LOGM_CLIENTS_LOG | LOGL_ERROR, "Possible ping attack from %s\n", inet_ntoa( IP ));
		}
		return( true );
	}

	// We are marked as always blocked?
	if ( pLogIP->IsBlocked())
	{
		// Look for it in the blockip list.
		if ( pLogIP->GetPings() <= 1 )
		{
			g_Log.Event( LOGM_CLIENTS_LOG | LOGL_ERROR, "BlockIP detected %s\n", inet_ntoa( IP ));
		}
		return( true );
	}

	return( false );
}

void CServer::Shutdown( int iMinutes ) // If shutdown is initialized
{
	if ( iMinutes == 0 )
	{
		if ( ! m_Clock_Shutdown ) 
			return;
		m_Clock_Shutdown=0;
		g_World.Broadcast("Shutdown has been interrupted.");
		return;
	}

	m_Clock_Shutdown = g_World.GetTime() + ( iMinutes * 60 * TICK_PER_SEC );

	CGString sTmp;
	sTmp.Format( "Server going down in %i minute(s).", iMinutes );
	g_World.Broadcast( sTmp );
}

bool CServer::r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc )
{
	if ( isdigit( pszKey[0] ))
	{
		int i=1;
		while ( isdigit( pszKey[i] )) i++;
		if ( pszKey[i] == '.' )
		{
			int index = atoi( pszKey );	// must use this to stop at .
			if ( ! m_Servers.IsValidIndex( index ))
				pRef = NULL;
			else
				pRef = m_Servers[index];
			pszKey += i + 1;
			return( true );
		}
	}
	return( CScriptObj::r_GetRef( pszKey, pRef, pSrc ));
}

enum SC_TYPE
{
	SC_ACCTFILES,			// m_sAcctBaseDir
	SC_ARRIVEDEPARTMSG, 
	SC_AUTONEWBIEKEYS,		// m_fAutoNewbieKeys
	SC_BACKUPLEVELS,				// m_iSaveBackupLevels
	SC_BANKMAXITEMS,
	SC_BANKMAXWEIGHT,
	SC_CHANGEFILES,			// m_sChangedBaseDir
	SC_CHARTAGS,					// m_fCharTags
	SC_CLIENTLINGER,
	SC_CLIENTMAX,			// m_nClientsMax
	SC_CLIENTS,
	SC_CLIENTVERSION,
	SC_CORPSENPCDECAY,
	SC_CORPSEPLAYERDECAY,
	SC_CRIMINALTIMER,		// m_iCriminalTimer
	SC_DEADSOCKETTIME,
	SC_DEBUGFLAGS,
	SC_DECAYTIMER,
	SC_DUNGEONLIGHT,
	SC_EQUIPPEDCAST,				// m_fEquippedCast
	SC_FILES,	//
	SC_FLIPDROPPEDITEMS,			// m_fFlipDroppedItems
	SC_FORCEGARBAGECOLLECT,		// m_fSaveGarbageCollect
	SC_FREESERVER,
	SC_FREESHARD,
	SC_FREEZERESTARTTIME,		// m_iFreezeRestartTime
	SC_GAMEMINUTELENGTH,			// m_iGameMinuteLength
	SC_GUARDLINGER,			// m_iGuardLingerTime
	SC_GUARDSINSTANTKILL,
	SC_GUESTS,
	SC_HEARALL,
	SC_HELPINGCRIMINALSISACRIME,	// m_fHelpingCriminalsIsACrime
	SC_HITPOINTPERCENTONREZ, // m_iHitpointPercentOnRez
	SC_LIGHTDAY,			// m_iLightDay
	SC_LIGHTNIGHT,			// m_iLightNight
	SC_LOG,
	SC_LOGMASK,				// GetLogMask
	SC_LOOTINGISACRIME,		// m_fLootingIsACrime
	SC_MAGICUNLOCKDOOR,		// m_iMagicUnlockDoor
	SC_MAINLOGSERVER,		// m_fMainLogServer
	SC_MAPCACHETIME,
	SC_MAXBASESKILL,			// m_iMaxBaseSkill
	SC_MAXCHARSPERACCOUNT,	// m_iMaxCharsPerAccount
	SC_MAXCOMPLEXITY,			// m_iMaxComplexity
	SC_MINCHARDELETETIME,
	SC_MONSTERFEAR,			// m_fMonsterFear
	SC_MONSTERFIGHT,
	SC_MULFILES,
	SC_MURDERDECAYTIME,		// m_iMurderDecayTime;
	SC_MURDERMINCOUNT,		// m_iMurderMinCount;		// amount of murders before we get title.
	SC_NOWEATHER,				// m_fNoWeather
	SC_NPCTRAINMAX,			// m_iTrainSkillMax
	SC_NPCTRAINPERCENT,			// m_iTrainSkillPercent
	SC_NTSERVICE,				// m_fUseNTService
	SC_PLAYERGHOSTSOUNDS,	// m_fPlayerGhostSounds
	SC_PLAYERNEUTRAL,		// m_iPlayerKarmaNeutral
	SC_POLLSERVERS,				// m_iPollServers
	SC_PROFILE,
	SC_REAGENTLOSSFAIL,			// m_fReagentLossFail
	SC_REAGENTSREQUIRED,
	SC_REGISTERSERVER,			// m_sRegisterServer
	SC_REQUIREEMAIL,		// m_fRequireEmail
	SC_RUNNINGPENALTY,		// m_iStamRunningPenalty
	SC_SAVEBACKGROUND,			// m_iSaveBackgroundTime
	SC_SAVEPERIOD,
	SC_SCPFILES,
	SC_SECTORSLEEP,				// m_iSectorSleepMask
	SC_SECURE,
	SC_SNOOPCRIMINAL,
	SC_SPEECHFILES,
	SC_STAMINALOSSATWEIGHT,	// m_iStaminaLossAtWeight
	SC_STATAVG,					// m_iAvgSumOfStats
	SC_STATCAP,					// m_iMaxSumOfStats
	SC_VENDORMAXSELL,			// m_iVendorMaxSell
	SC_VERBOSE,
	SC_WOPPLAYER,
	SC_WOPSTAFF,
	SC_WORLDSAVE,
	SC_QTY,
};

const TCHAR * CServer::sm_KeyTable[SC_QTY] =
{
	"ACCTFILES",			// m_sAcctBaseDir
	"ARRIVEDEPARTMSG", 
	"AUTONEWBIEKEYS",		// m_fAutoNewbieKeys
	"BACKUPLEVELS",				// m_iSaveBackupLevels
	"BANKMAXITEMS",
	"BANKMAXWEIGHT",
	"CHANGEFILES",			// m_sChangedBaseDir
	"CHARTAGS",					// m_fCharTags
	"CLIENTLINGER",
	"CLIENTMAX",			// m_nClientsMax
	"CLIENTS",
	"CLIENTVERSION",
	"CORPSENPCDECAY",
	"CORPSEPLAYERDECAY",
	"CRIMINALTIMER",  // m_iCriminalTimer
	"DEADSOCKETTIME",
	"DEBUGFLAGS",
	"DECAYTIMER",
	"DUNGEONLIGHT",
	"EQUIPPEDCAST",				// m_fEquippedCast
	"FILES",	//
	"FLIPDROPPEDITEMS",			// m_fFlipDroppedItems
	"FORCEGARBAGECOLLECT",		// m_fSaveGarbageCollect
	"FREESERVER",
	"FREESHARD",
	"FREEZERESTARTTIME",		// m_iFreezeRestartTime
	"GAMEMINUTELENGTH",			// m_iGameMinuteLength
	"GUARDLINGER",			// m_iGuardLingerTime
	"GUARDSINSTANTKILL",
	"GUESTS",
	"HEARALL",
	"HELPINGCRIMINALSISACRIME",	// m_fHelpingCriminalsIsACrime
	"HITPOINTPERCENTONREZ", // m_iHitpointPercentOnRez
	"LIGHTDAY",			// m_iLightDay
	"LIGHTNIGHT",			// m_iLightNight
	"LOG",
	"LOGMASK",				// GetLogMask
	"LOOTINGISACRIME",		// m_fLootingIsACrime
	"MAGICUNLOCKDOOR",		// m_iMagicUnlockDoor
	"MAINLOGSERVER",		// m_fMainLogServer
	"MAPCACHETIME",
	"MAXBASESKILL",			// m_iMaxBaseSkill
	"MAXCHARSPERACCOUNT",	// m_iMaxCharsPerAccount
	"MAXCOMPLEXITY",			// m_iMaxComplexity
	"MINCHARDELETETIME",
	"MONSTERFEAR",			// m_fMonsterFear
	"MONSTERFIGHT",
	"MULFILES",
	"MURDERDECAYTIME",		// m_iMurderDecayTime;
	"MURDERMINCOUNT",		// m_iMurderMinCount;		// amount of murders before we get title.
	"NOWEATHER",				// m_fNoWeather
	"NPCTRAINMAX",			// m_iTrainSkillMax
	"NPCTRAINPERCENT",			// m_iTrainSkillPercent
	"NTSERVICE",				// m_fUseNTService
	"PLAYERGHOSTSOUNDS",	// m_fPlayerGhostSounds
	"PLAYERNEUTRAL",		// m_iPlayerKarmaNeutral
	"POLLSERVERS",				// m_iPollServers
	"PROFILE",
	"REAGENTLOSSFAIL",			// m_fReagentLossFail
	"REAGENTSREQUIRED",
	"REGISTERSERVER",			// m_sRegisterServer
	"REQUIREEMAIL",			// m_fRequireEmail
	"RUNNINGPENALTY",		// m_iStamRunningPenalty
	"SAVEBACKGROUND",			// m_iSaveBackgroundTime
	"SAVEPERIOD",
	"SCPFILES",
	"SECTORSLEEP",				// m_iSectorSleepMask
	"SECURE",
	"SNOOPCRIMINAL",
	"SPEECHFILES",
	"STAMINALOSSATWEIGHT",	// m_iStaminaLossAtWeight
	"STATAVG",					// m_iAvgSumOfStats
	"STATCAP",					// m_iMaxSumOfStats
	"VENDORMAXSELL",			// m_iVendorMaxSell
	"VERBOSE",
	"WOPPLAYER",
	"WOPSTAFF",
	"WORLDSAVE",
};

bool CServer::r_LoadVal( CScript &s )
{
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case SC_ACCTFILES:	// Put acct files here.
		m_sAcctBaseDir = GetMergedFileName( s.GetArgStr(), "" );
		break;
	case SC_ARRIVEDEPARTMSG:
		m_fArriveDepartMsg = s.GetArgVal();
		break;
	case SC_AUTONEWBIEKEYS:
		m_fAutoNewbieKeys = s.GetArgVal();
		break;
	case SC_BACKUPLEVELS:
		m_iSaveBackupLevels = s.GetArgVal();
		break;
	case SC_BANKMAXITEMS:
		m_iBankIMax = s.GetArgVal();
		break;
	case SC_BANKMAXWEIGHT:
		m_iBankWMax = s.GetArgVal() * WEIGHT_UNITS;
		break;
	case SC_CHANGEFILES:
		m_sChangedBaseDir = s.GetArgStr();
		break;
	case SC_CHARTAGS:
		m_fCharTags = s.GetArgVal();
		break;
	case SC_CLIENTLINGER:
		m_iClientLingerTime = s.GetArgVal() * TICK_PER_SEC;
		break;
	case SC_CLIENTMAX:
	case SC_CLIENTS:
		m_nClientsMax = s.GetArgVal();
		break;
	case SC_CLIENTVERSION:
		m_ClientVersion.SetClientVersion( s.GetArgVal());
		break;
	case SC_CORPSENPCDECAY:
		m_iDecay_CorpseNPC = s.GetArgVal()*60*TICK_PER_SEC;
		break;
	case SC_CORPSEPLAYERDECAY:
		m_iDecay_CorpsePlayer = s.GetArgVal()*60*TICK_PER_SEC ;
		break;
	case SC_CRIMINALTIMER:
		m_iCriminalTimer = s.GetArgVal() * 60 * TICK_PER_SEC;
		break;
	case SC_DEADSOCKETTIME:
		m_iDeadSocketTimeMin = s.GetArgVal()*60*TICK_PER_SEC;
		break;
	case SC_DEBUGFLAGS:
		m_wDebugFlags = s.GetArgHex();
		break;
	case SC_DECAYTIMER:
		m_iDecay_Item = s.GetArgVal() *60*TICK_PER_SEC;
		break;
	case SC_DUNGEONLIGHT:
		m_iLightDungeon = s.GetArgVal();
		break;
	case SC_EQUIPPEDCAST:
		m_fEquippedCast = s.GetArgVal();
		break;
	case SC_FILES: // Get data files from here.
		goto do_mulfiles;
	case SC_FREESERVER:
	case SC_FREESHARD:
		if ( s.GetArgVal()) 
		{
			m_eAccApp = ACCAPP_Free;
		}
		break;
	case SC_FREEZERESTARTTIME:
		m_iFreezeRestartTime = s.GetArgVal() * TICK_PER_SEC;
		break;
	case SC_FORCEGARBAGECOLLECT:
		m_fSaveGarbageCollect = s.GetArgVal();
		break;
	case SC_FLIPDROPPEDITEMS:
		m_fFlipDroppedItems = s.GetArgVal();
		break;
	case SC_GUESTS:
		m_nGuestsMax = s.GetArgVal();
		break;
	case SC_GUARDSINSTANTKILL:
		m_fGuardsInstantKill = s.GetArgVal();
		break;
	case SC_GAMEMINUTELENGTH:
		m_iGameMinuteLength = s.GetArgVal();	// TICK_PER_SEC
		break;
	case SC_GUARDLINGER:
		m_iGuardLingerTime = s.GetArgVal() * 60 * TICK_PER_SEC;
		break;
	case SC_HEARALL:
		{
			WORD wMask = g_Log.GetLogMask();
			if ( ! s.HasArgs())
				wMask ^= LOGM_PLAYER_SPEAK;
			else if ( s.GetArgVal())
				wMask |= LOGM_PLAYER_SPEAK;
			else
				wMask &= ~LOGM_PLAYER_SPEAK;
			g_Log.SetLogMask( wMask );
		}
		break;
	case SC_HITPOINTPERCENTONREZ:
		m_iHitpointPercentOnRez = s.GetArgVal();
		break;
	case SC_HELPINGCRIMINALSISACRIME:
		m_fHelpingCriminalsIsACrime = s.GetArgVal();
		break;
	case SC_LOG:
		g_Log.Open( s.GetArgStr());
		break;
	case SC_LOGMASK:
		g_Log.SetLogMask( s.GetArgVal());
		break;
	case SC_LIGHTDAY:
		m_iLightDay = s.GetArgVal();
		break;
	case SC_LIGHTNIGHT:
		m_iLightNight = s.GetArgVal();
		break;
	case SC_LOOTINGISACRIME:
		m_fLootingIsACrime = s.GetArgVal();
		break;
	case SC_MULFILES:
do_mulfiles:
		g_Install.SetPreferPath( GetMergedFileName( s.GetArgStr(), "" ));
		break;
	case SC_MAPCACHETIME:
		m_iMapCacheTime = s.GetArgVal() * TICK_PER_SEC;
		break;
	case SC_MAXBASESKILL:
		m_iMaxBaseSkill = s.GetArgVal();
		break;
	case SC_MAXCHARSPERACCOUNT:
		m_iMaxCharsPerAccount = s.GetArgVal();
		break;
	case SC_MAXCOMPLEXITY:
		m_iMaxComplexity = s.GetArgVal();
		break;
	case SC_MAGICUNLOCKDOOR:
		m_iMagicUnlockDoor = s.GetArgVal();
		break;
	case SC_MAINLOGSERVER:
		m_fMainLogServer = s.GetArgVal();
		break;
	case SC_MONSTERFEAR:
		m_fMonsterFear= s.GetArgVal();
		break;
	case SC_MONSTERFIGHT:
		m_fMonsterFight = s.GetArgVal();
		break;
	case SC_MINCHARDELETETIME:
		m_iMinCharDeleteTime = s.GetArgVal()*60*TICK_PER_SEC;
		break;
	case SC_MURDERMINCOUNT:
		m_iMurderMinCount = s.GetArgVal();		// amount of murders before we get title.
		break;
	case SC_MURDERDECAYTIME:
		m_iMurderDecayTime = s.GetArgVal() * TICK_PER_SEC;
		break;
	case SC_NPCTRAINPERCENT:
		m_iTrainSkillPercent = s.GetArgVal();
		break;
	case SC_NOWEATHER:
		m_fNoWeather = s.GetArgVal();
		break;
	case SC_NTSERVICE:
		m_fUseNTService = s.GetArgVal();
		break;
	case SC_NPCTRAINMAX:
		m_iTrainSkillMax = s.GetArgVal();
		break;
	case SC_PROFILE:
		m_Profile.SetActive( s.GetArgVal());
		break;
	case SC_POLLSERVERS:
		m_iPollServers = s.GetArgVal() *60*TICK_PER_SEC;
		m_BackTask.CreateThread();
		break;
	case SC_PLAYERNEUTRAL:	// How much bad karma makes a player neutral?
		m_iPlayerKarmaNeutral = s.GetArgVal();
		if ( m_iPlayerKarmaEvil > m_iPlayerKarmaNeutral )
			m_iPlayerKarmaEvil = m_iPlayerKarmaNeutral;
		break;
	case SC_PLAYERGHOSTSOUNDS:
		m_fPlayerGhostSounds = s.GetArgVal();
		break;
	case SC_REAGENTSREQUIRED:
		m_fReagentsRequired = s.GetArgVal();
		break;
	case SC_REGISTERSERVER:
		m_sRegisterServer = s.GetArgStr();
		m_BackTask.CreateThread();
		break;
	case SC_REAGENTLOSSFAIL:
		m_fReagentLossFail = s.GetArgVal();
		break;
	case SC_REQUIREEMAIL:
		m_fRequireEmail = s.GetArgVal();
		break;
	case SC_RUNNINGPENALTY:
		m_iStamRunningPenalty = s.GetArgVal();
		break;

	case SC_SCPFILES: // Get SCP files from here.
		m_sSCPBaseDir = GetMergedFileName( s.GetArgStr(), "" );
		break;
	case SC_SECURE:
		m_fSecure = s.GetArgVal();
		SetSignals();
		break;
	case SC_SAVEPERIOD:
		m_iSavePeriod = s.GetArgVal()*60*TICK_PER_SEC;
		break;
	case SC_SPEECHFILES:
		m_sSpeechBaseDir = s.GetArgStr();
		break;
	case SC_STAMINALOSSATWEIGHT:
		m_iStaminaLossAtWeight = s.GetArgVal();
		break;
	case SC_SNOOPCRIMINAL:
		m_iSnoopCriminal = s.GetArgVal();
		break;
	case SC_SECTORSLEEP:
		m_iSectorSleepMask = ( 1 << s.GetArgVal()) - 1;
		break;
	case SC_SAVEBACKGROUND:
		m_iSaveBackgroundTime = s.GetArgVal() * 60 * TICK_PER_SEC;
		break;
	case SC_STATCAP:
		m_iMaxSumOfStats = s.GetArgVal();
		break;
	case SC_STATAVG:
		m_iAvgSumOfStats = s.GetArgVal();
		break;

	case SC_VENDORMAXSELL:
		m_iVendorMaxSell = s.GetArgVal();
		break;
	case SC_VERBOSE:
		g_Log.SetLogLevel( s.GetArgVal() ? LOGL_TRACE : LOGL_EVENT );
		break;

	case SC_WOPPLAYER:
		m_fWordsOfPowerPlayer = s.GetArgVal();
		break;
	case SC_WOPSTAFF:
		m_fWordsOfPowerStaff = s.GetArgVal();
		break;
	case SC_WORLDSAVE: // Put save files here.
		m_sWorldBaseDir = GetMergedFileName( s.GetArgStr(), "" );
		break;

	default:
		if ( s.IsKeyHead( "MULFILE", 7 ))
		{
			return g_Install.SetMulFile((VERFILE_TYPE)( atoi(s.GetKey()+7)), s.GetArgStr());
		}
		if ( s.IsKeyHead( "SCPFILE", 7 ))
		{
			return SetScpFile((SCPFILE_TYPE)( atoi(s.GetKey()+7)), s.GetArgStr());
		}
		if ( s.IsKeyHead( "REGEN", 5 ))
		{
			int index = atoi(s.GetKey()+5);
			if ( index < COUNTOF( CChar::sm_Stat_Val_Regen ))
			{
				CChar::sm_Stat_Val_Regen[index] = s.GetArgVal();
				return( true );
			}
		}
		return( CServRef::r_LoadVal( s ));
	}

	return( true );
}

const TCHAR * CServer::sm_KeyStatsTable[] =
{
	"ACCOUNTS",
	"GUILDS",
	"RCLOCK",
	"RTIME",
	"RTIMETEXT",
	"STATACCOUNTS",
	"STATGUILDS",
	"TIME",
	"TIMEUP",
	"TOTALPOLLEDCLIENTS",
	"TOTALPOLLEDSERVERS",
	"VER",
	"VERSION",
};

bool CServer::r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc )
{
	// Just do stats values for now.

	switch ( FindTableSorted( pKey, sm_KeyStatsTable, COUNTOF( sm_KeyStatsTable )))
	{
	case 0:	// "ACCOUNTS",
		goto do_accounts;
	case 1: // "GUILDS"
		goto do_guilds;
	case 2: // "RCLOCK"
		sVal.Format( "0%lx / 0%lx",	(long) getclock(), (long) CLOCKS_PER_SEC );
		return( true );
	case 3: // "RTIME"	= the current real world time.
	case 4: // "RTIMETEXT"	= the current real world time.
		{
			CRealTime thetime;
			thetime.FillRealTime();
			sVal = thetime.Write();
		}
		return( true );
	case 5:	// "STATACCOUNTS",
do_accounts:
		sVal.FormatVal( g_World.m_Accounts.GetCount());
		return( true );
	case 6: // "STATGUILDS"
do_guilds:
		sVal.FormatVal( g_World.m_Stones.GetCount());
		return( true );
	case 7: // "TIME"
		sVal.FormatHex( g_World.GetTime());
		return( true );
	case 8:	// "TIMEUP"
		sVal.FormatVal(( g_World.GetTime() - g_World.m_Clock_Startup ) / TICK_PER_SEC );
		return( true );
	case 9: // "TOTALPOLLEDCLIENTS"
		sVal.FormatVal( m_BackTask.m_iTotalPolledClients );
		return( true );
	case 10: // "TOTALPOLLEDSERVERS"
		sVal.FormatVal( m_Servers.GetCount() + 1 );
		return( true );
	case 11:	// "VER"
	case 12:	// "VERSION"
		sVal = g_szServerDescription;
		return( true );
	}

	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case SC_ACCTFILES:	// Put acct files here.
		sVal = m_sAcctBaseDir;
		break;
	case SC_ARRIVEDEPARTMSG:
		sVal.FormatVal( m_fArriveDepartMsg );
		break;
	case SC_AUTONEWBIEKEYS:
		sVal.FormatVal( m_fAutoNewbieKeys );
		break;
	case SC_BACKUPLEVELS:
		sVal.FormatVal( m_iSaveBackupLevels );
		break;
	case SC_BANKMAXITEMS:
		sVal.FormatVal( m_iBankIMax );
		break;
	case SC_BANKMAXWEIGHT:
		sVal.FormatVal( m_iBankWMax / WEIGHT_UNITS );
		break;
	case SC_CHANGEFILES:
		sVal = m_sChangedBaseDir;
		break;
	case SC_CHARTAGS:
		sVal.FormatVal( m_fCharTags );
		break;
	case SC_CLIENTLINGER:
		sVal.FormatVal( m_iClientLingerTime / TICK_PER_SEC );
		break;
	case SC_CLIENTMAX:
	case SC_CLIENTS:
		sVal.FormatVal( m_nClientsMax );
		break;
	case SC_CLIENTVERSION:
		{
			TCHAR szVersion[ 128 ];
			m_ClientVersion.WriteClientVersion( szVersion );
			sVal = szVersion;
		}
		break;
	case SC_CORPSENPCDECAY:
		sVal.FormatVal( m_iDecay_CorpseNPC / (60*TICK_PER_SEC));
		break;
	case SC_CORPSEPLAYERDECAY:
		sVal.FormatVal( m_iDecay_CorpsePlayer / (60*TICK_PER_SEC));
		break;
	case SC_CRIMINALTIMER:
		sVal.FormatVal( m_iCriminalTimer / (60*TICK_PER_SEC));
		break;
	case SC_DEADSOCKETTIME:
		sVal.FormatVal( m_iDeadSocketTimeMin / (60*TICK_PER_SEC));
		break;
	case SC_DEBUGFLAGS:
		sVal.FormatHex( m_wDebugFlags );
		break;
	case SC_DECAYTIMER:
		sVal.FormatVal( m_iDecay_Item / (60*TICK_PER_SEC));
		break;
	case SC_DUNGEONLIGHT:
		sVal.FormatVal( m_iLightDungeon );
		break;
	case SC_EQUIPPEDCAST:
		sVal.FormatVal( m_fEquippedCast );
		break;
	case SC_FILES: // Get data files from here.
		goto do_mulfiles;
	case SC_FREESERVER:
do_freeserver:
		sVal.FormatVal(( m_eAccApp == ACCAPP_Free ) ? 1 : 0 );
		break;
	case SC_FREESHARD:
		goto do_freeserver;
		break;
	case SC_FREEZERESTARTTIME:
		sVal.FormatVal( m_iFreezeRestartTime / TICK_PER_SEC );
		break;
	case SC_FORCEGARBAGECOLLECT:
		sVal.FormatVal( m_fSaveGarbageCollect );
		break;
	case SC_FLIPDROPPEDITEMS:
		sVal.FormatVal( m_fFlipDroppedItems );
		break;
	case SC_GUESTS:
		sVal.FormatVal( m_nGuestsMax );
		break;
	case SC_GUARDSINSTANTKILL:
		sVal.FormatVal( m_fGuardsInstantKill );
		break;
	case SC_GAMEMINUTELENGTH:
		sVal.FormatVal( m_iGameMinuteLength );	// TICK_PER_SEC
		break;
	case SC_GUARDLINGER:
		sVal.FormatVal( m_iGuardLingerTime / (60*TICK_PER_SEC));
		break;
	case SC_HEARALL:
		sVal.FormatVal( g_Log.GetLogMask() & LOGM_PLAYER_SPEAK );
		break;
	case SC_HELPINGCRIMINALSISACRIME:
		sVal.FormatVal( m_fHelpingCriminalsIsACrime );
		break;
	case SC_HITPOINTPERCENTONREZ:
		sVal.FormatVal( m_iHitpointPercentOnRez );
		break;
	case SC_LOG:
		return( false );
	case SC_LOGMASK:
		sVal.FormatHex( g_Log.GetLogMask());
		break;
	case SC_LIGHTDAY:
		sVal.FormatVal( m_iLightDay );
		break;
	case SC_LIGHTNIGHT:
		sVal.FormatVal( m_iLightNight );
		break;
	case SC_LOOTINGISACRIME:
		sVal.FormatVal( m_fLootingIsACrime );
		break;
	case SC_MULFILES:
do_mulfiles:
		sVal = g_Install.GetPreferPath( NULL );
		break;
	case SC_MAPCACHETIME:
		sVal.FormatVal( m_iMapCacheTime / TICK_PER_SEC );
		break;
	case SC_MONSTERFIGHT:
		sVal.FormatVal( m_fMonsterFight );
		break;
	case SC_MINCHARDELETETIME:
		sVal.FormatVal( m_iMinCharDeleteTime /( 60*TICK_PER_SEC ));
		break;
	case SC_MAGICUNLOCKDOOR:
		sVal.FormatVal( m_iMagicUnlockDoor );
		break;
	case SC_MAINLOGSERVER:
		sVal.FormatVal( m_fMainLogServer );
		break;
	case SC_MURDERMINCOUNT:
		sVal.FormatVal( m_iMurderMinCount );
		break;
	case SC_MURDERDECAYTIME:
		sVal.FormatVal( m_iMurderDecayTime / (TICK_PER_SEC ));
		break;
	case SC_MAXCHARSPERACCOUNT:
		sVal.FormatVal( m_iMaxCharsPerAccount );
		break;
	case SC_MAXCOMPLEXITY:
		sVal.FormatVal( m_iMaxComplexity );
		break;
	case SC_MONSTERFEAR:
		sVal.FormatVal( m_fMonsterFear );
		break;
	case SC_MAXBASESKILL:
		sVal.FormatVal( m_iMaxBaseSkill );
		break;
	case SC_NPCTRAINPERCENT:
		sVal.FormatVal( m_iTrainSkillPercent );
		break;
	case SC_NOWEATHER:
		sVal.FormatVal( m_fNoWeather );
		break;
	case SC_NTSERVICE:
		sVal.FormatVal( m_fUseNTService );
		break;
	case SC_NPCTRAINMAX:
		sVal.FormatVal( m_iTrainSkillMax );
		break;
	case SC_PROFILE:
		return( false );
	case SC_POLLSERVERS:
		sVal.FormatVal( m_iPollServers / (60*TICK_PER_SEC));
		break;
	case SC_PLAYERNEUTRAL:	// How much bad karma makes a player neutral?
		sVal.FormatVal( m_iPlayerKarmaNeutral );
		break;
	case SC_PLAYERGHOSTSOUNDS:
		sVal.FormatVal( m_fPlayerGhostSounds );
		break;
	case SC_REAGENTSREQUIRED:
		sVal.FormatVal( m_fReagentsRequired );
		break;
	case SC_REGISTERSERVER:
		sVal = m_sRegisterServer;
		break;
	case SC_REAGENTLOSSFAIL:
		sVal.FormatVal( m_fReagentLossFail );
		break;
	case SC_REQUIREEMAIL:
		sVal.FormatVal( m_fRequireEmail );
		break;
	case SC_RUNNINGPENALTY:
		sVal.FormatVal( m_iStamRunningPenalty );
		break;
	case SC_SCPFILES:	// Get SCP files from here.
		sVal = m_sSCPBaseDir;
		break;
	case SC_SECURE:
		sVal.FormatVal( m_fSecure );
		break;
	case SC_SAVEPERIOD:
		sVal.FormatVal( m_iSavePeriod / (60*TICK_PER_SEC));
		break;
	case SC_SPEECHFILES:
		sVal = m_sSpeechBaseDir;
		break;
	case SC_SNOOPCRIMINAL:
		sVal.FormatVal( m_iSnoopCriminal );
		break;
	case SC_SECTORSLEEP:
		sVal.FormatVal( GetLog2( m_iSectorSleepMask+1 )-1 );
		break;
	case SC_SAVEBACKGROUND:
		sVal.FormatVal( m_iSaveBackgroundTime / (60 * TICK_PER_SEC));
		break;
	case SC_STATCAP:
		sVal.FormatVal( m_iMaxSumOfStats );
		break;
	case SC_STATAVG:
		sVal.FormatVal( m_iAvgSumOfStats );
		break;
	case SC_STAMINALOSSATWEIGHT:
		sVal.FormatVal( m_iStaminaLossAtWeight );
		break;
	case SC_VENDORMAXSELL:
		sVal.FormatVal( m_iVendorMaxSell );
		break;
	case SC_VERBOSE:
		return( false );
	case SC_WOPPLAYER:
		sVal.FormatVal( m_fWordsOfPowerPlayer );
		break;
	case SC_WOPSTAFF:
		sVal.FormatVal( m_fWordsOfPowerStaff );
		break;
	case SC_WORLDSAVE:	// Put save files here.
		sVal = m_sWorldBaseDir;
		break;
	default:
		return( CServRef::r_WriteVal( pKey, sVal, pSrc ));
	}

	return( true );
}

bool CServer::CanRunBackTask() const
{
	// Run the background task or not ?

	if ( IsLoading())
		return( false );
	if ( m_iPollServers && m_Servers.GetCount())
		return( true );
	if ( m_sRegisterServer.GetLength())
		return( true );

	return( false );
}

int CServer::GetAgeHours() const
{
	return( g_World.GetTime() / (60*60*TICK_PER_SEC));
}

const TCHAR * CServer::GetStatusStringRegister( bool fFirst ) const
{
	// we are registering ourselves.
	TCHAR * pTemp = GetTempStr();

	if ( fFirst )
	{
		sprintf( pTemp, GRAY_TITLE ", Name=%s, RegPass=%s, Port=%d, Ver=%s, TZ=%d, EMail=%s, URL=%s, Lang=%s, AccApp=%d\n",
			GetName(),
			(const TCHAR*) m_sRegisterPassword,
			m_ip.GetPort(),
			GRAY_VERSION,
			m_TimeZone,
			(const TCHAR*) m_sEMail,
			(const TCHAR*) m_sURL,
			(const TCHAR*) m_sLang,
			m_eAccApp
			);
	}
	else
	{
		sprintf( pTemp, GRAY_TITLE ", Name=%s, RegPass=%s, Age=%i, Accounts=%d, Clients=%i, Items=%i, Chars=%i, Mem=%iK, Notes=%s\n",
			GetName(),
			(const TCHAR*) m_sRegisterPassword,
			GetAgeHours()/24,
			g_World.m_Accounts.GetCount(),
			StatGet(SERV_STAT_CLIENTS), 
			StatGet(SERV_STAT_ITEMS), 
			StatGet(SERV_STAT_CHARS),
			StatGet(SERV_STAT_MEM)/1024, 
			(const TCHAR*) m_sNotes 
			);
	}
	return( pTemp );
}

const TCHAR * CServer::GetStatusString( BYTE iIndex ) const
{
	// NOTE: The key names should match those in CServRef::r_LoadVal
	// A ping will return this as well.
	// 0 or 0x21 = main status.

	TCHAR * pTemp = GetTempStr();

	switch ( iIndex )
	{
	case 0x21:	// '!' 
		// typical (first time) poll response.
		sprintf( pTemp, GRAY_TITLE ", Name=%s, Port=%d, Ver=%s, TZ=%d, EMail=%s, URL=%s, Lang=%s\n",
			GetName(),
			m_ip.GetPort(),
			GRAY_VERSION,
			m_TimeZone,
			(const TCHAR*) m_sEMail,
			(const TCHAR*) m_sURL,
			(const TCHAR*) m_sLang 
			);
		break;
	case 0x22: // '"'
		// shown in the INFO page in game.
		sprintf( pTemp, GRAY_TITLE ", Name=%s, Age=%i, Clients=%i, Items=%i, Chars=%i, Mem=%iK\n",
			GetName(),
			GetAgeHours()/24,
			StatGet(SERV_STAT_CLIENTS), 
			StatGet(SERV_STAT_ITEMS), 
			StatGet(SERV_STAT_CHARS),
			StatGet(SERV_STAT_MEM)/1024 
			);
		break;
	case 0x23:
	default:	// default response to ping.
		sprintf( pTemp, GRAY_TITLE ", Name=%s, Age=%i, Ver=%s, TZ=%d, EMail=%s, URL=%s, Clients=%i\n",
			GetName(),
			GetAgeHours()/24,
			GRAY_VERSION,
			m_TimeZone,
			(const TCHAR*) m_sEMail,
			(const TCHAR*) m_sURL,
			StatGet(SERV_STAT_CLIENTS)
			);
		break;
	case 0x24: // '$'
		// show at startup.
		sprintf( pTemp, "Admin=%s, URL=%s, Lang=%s, TZ=%d\n",
			(const TCHAR*) m_sEMail,
			(const TCHAR*) m_sURL,
			(const TCHAR*) m_sLang,
			m_TimeZone
			);
		break;
	}

	return( pTemp );
}

bool CServer::r_Verb( CScript &s, CTextConsole * pSrc )
{
	static const TCHAR * table[] =
	{
		"ALLCLIENTS",
		"B",
		"HEARALL",
		"LOG",
		"SAFE",
		"SECURE",
		"SHUTDOWN",
		"VERBOSE",
	};

	CGString sMsg;

	switch ( FindTableSorted( s.GetKey(), table, COUNTOF( table )))
	{
	case 0:	// "ALLCLIENTS"
		{
			// Send a verb to all clients
			for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
			{
				if ( pClient->GetChar() == NULL )
					continue;
				pClient->GetChar()->r_Verb( CScript( s.GetArgStr()), pSrc );
			}
		}
		break;

	case 1: // "B"
		g_World.Broadcast( s.GetArgStr());
		break;

	case 2:	// "HEARALL" = Hear all said.
		{
		g_Log.SetLogMask( g_Log.GetLogMask() ^ LOGM_PLAYER_SPEAK );
		sMsg.Format( "Hear All %s.\n", g_Log.IsLoggedMask(LOGM_PLAYER_SPEAK) ? "Enabled" : "Disabled" );
		}
		break;

	case 3:	// "LOG" = Turn the log file on or off.
		if ( g_Log.IsFileOpen())
		{
			g_Log.Close();
		}
		else
		{
			g_Log.Open();
		}
		sMsg.Format( "Log file %s.\n", g_Log.IsFileOpen() ? "opened" : "closed" );
		break;

	case 4: // "SAFE"
		goto scp_secure;

	case 5: // "SECURE"
scp_secure:
		m_fSecure = ! m_fSecure;
		SetSignals();
		g_World.Broadcast( m_fSecure ?
			"The world is now running in SECURE MODE" :
			"WARNING: The world is NOT running in SECURE MODE" );

		sMsg.Format( "Secure mode %s.\n", m_fSecure ? "re-enabled" : "disabled" );
		break;

	case 6: // "SHUTDOWN"
		Shutdown(( s.HasArgs()) ? s.GetArgVal() : 15 );
		break;

	case 7: // "VERBOSE"
		g_Log.SetLogLevel(( g_Log.GetLogLevel() >= LOGL_TRACE ) ? LOGL_EVENT : LOGL_TRACE );
		sMsg.Format( "Verbose display %s.\n", g_Log.IsLogged(LOGL_TRACE) ? "Enabled" : "Disabled" );
		break;

	default:
		return( CScriptObj::r_Verb( s, pSrc ));
	}

	if ( ! sMsg.IsEmpty())
	{
		pSrc->SysMessage( sMsg );
	}
	return( true );
}

bool CServer::LoadIni()
{
	CScriptLock s;
	if ( ! s.OpenFind( GRAY_FILE ".ini" )) // Open script file
	{
		g_Log.Event( LOGL_FATAL|LOGM_INIT, "Can't open " GRAY_FILE ".ini\n" );
		return( false );
	}

	while (s.FindNextSection())
	{
		if ( s.IsSectionType(GRAY_FILE))
		{
			CScriptObj::r_Load(s);
			continue;
		}

		else if ( s.IsSectionType("SERVERS"))
		{
			// Don't re-read these if we already have them. (Resync command)
			if ( m_Servers.GetCount())
				continue;

			bool fReadSelf = false;

			while ( s.ReadKey())
			{
				CServRef * pServ;
				if ( ! fReadSelf )
				{
					SetName( s.GetKey()); // this is me!
					pServ = this;
				}
				else
				{
					pServ = new CServRef( s.GetKey(), SOCKET_LOCAL_ADDRESS );
				}
				if ( s.ReadKey())
				{
					pServ->m_ip.SetHostPortStr( s.GetKey());
					if ( s.ReadKey())
					{
						pServ->m_ip.SetPort( s.GetArgVal());
					}
				}
				if ( ! fReadSelf )
				{
					fReadSelf = true;
				}
				else
				{
					m_Servers.AddSortKey( pServ, pServ->GetName());
				}
			}
			continue;
		}

		else if ( s.IsSectionType("BLOCKIP"))
		{
			m_LogIP.Empty();
			while ( s.ReadKeyParse())
			{
				SetLogIPBlock( s.GetKey(), true );
			}
			continue;
		}

		else if ( s.IsSectionType("WEBPAGE"))
		{
			// Read a web page entry.
			int id = s.GetArgVal() - 1;
			if ( id < 0 )
				continue;
			CWebPageDef * pWeb;
			if ( id >= m_WebPages.GetCount())
			{
				pWeb = new CWebPageDef( id );
				m_WebPages.SetAtGrow( id, pWeb );
			}
			else
			{
				pWeb = m_WebPages[id];
			}
			pWeb->r_Load(s);
			continue;
		}
	}

	if ( GetName()[0] == '\0' )	// make sure we have a set name
	{
		TCHAR szName[ MAX_SERVER_NAME_SIZE ];
		int iRet = gethostname( szName, sizeof( szName ));
		if ( ! iRet )
			SetName( szName );
	}

	return( true );
}

void CServer::SysMessage( const TCHAR * pStr ) const
{
	// Print just to the console.
#ifdef _WIN32
	fputs( pStr, stdout );	// print out locally as well.
#ifdef VISUAL_SPHERE
	if ( g_pServerObject )
	{
		CComBSTR msg(pStr);
		g_pServerObject->Fire_OnSysMessage(msg);
	}
#endif // VISUAL_SPHERE
#endif
}

void CServer::PrintStr( const TCHAR * pStr ) const
{
	// print to all consoles.
	SysMessage( pStr );

	if ( m_iAdminClients )
	{
		ASSERT( m_iAdminClients );
		for ( CClient * pClient = GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
		{
			if ( pClient->GetTargMode() != TARGMODE_CONSOLE ) 
				continue;
			pClient->SysMessage( pStr );
		}
	}
}

int CServer::PrintPercent( long iCount, long iTotal )
{
	// These vals can get very large. so use MulDiv to prevent overflow. (not IMULDIV)
	DEBUG_CHECK( iCount >= 0 );
	DEBUG_CHECK( iTotal >= 0 );
	if ( iTotal <= 0 )
		return( 100 );
    int iPercent = MulDiv( iCount, 100, iTotal );
	CGString sProgress;
	int len = sProgress.Format( "%d%%", iPercent );
	PrintStr( sProgress );
	while ( len-- ) PrintStr( "\x08" );
	return( iPercent );
}

const COreDef * CServer::GetOreColor( COLOR_TYPE color ) const
{
	for ( int i = 0; i < g_Serv.m_OreDefs.GetCount(); i++ )
	{
		const COreDef * pOre = g_Serv.m_OreDefs.GetAt(i);
		if ( pOre->m_wColor == color )
			return( pOre );
	}
	return( NULL );
}

const COreDef * CServer::GetOreEntry( int iMiningSkill ) const
{
	if ( g_Serv.m_OreDefs.GetCount() == 0 )
		return NULL;

	int iChanceBase = 0;	// We can't assume that the average scripter will have the chances add up to 100.0
	int i = 0;
	for ( ; i < g_Serv.m_OreDefs.GetCount(); i++ )
	{
		const COreDef * pOre = g_Serv.m_OreDefs.GetAt( i );
		iChanceBase += pOre->m_iVeinChance;
	}

	int iVeinIndex = GetRandVal( iChanceBase );
	int iRunningChance = 0;
	const COreDef * pOre = g_Serv.m_OreDefs.GetAt( 0 );			// This should be the default ore
	for ( i = 0; i < g_Serv.m_OreDefs.GetCount(); i++)
	{
		const COreDef * pTestOre = g_Serv.m_OreDefs.GetAt(i);
		if ( pTestOre->m_iMinSkill > iMiningSkill )
			break;
		iRunningChance += pTestOre->m_iVeinChance;
		if ( iVeinIndex < iRunningChance )
		{
			pOre = pTestOre;
			break;
		}
	}
	return pOre;
}

//////////////////////////////////////////////////////////

void CServer::ListServers( CTextConsole * pConsole ) const
{
	ASSERT( pConsole );

	for ( int i=0; i<m_Servers.GetCount(); i++ )
	{
		CServRef * pServ = m_Servers[i];
		if ( pServ == NULL )
			continue;
		CGString sMsg;
		sMsg.Format( "%d:NAME=%s, STATUS=%s\n", i, pServ->GetName(), pServ->GetStatus());
		pConsole->SysMessage( sMsg );
	}
}

void CServer::ListClients( CTextConsole * pConsole ) const
{
	ASSERT( pConsole );
	CChar * pCharCmd = pConsole->GetChar();

	for ( CClient * pClient = GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		struct sockaddr_in Name;
		pClient->GetPeerName( &Name );

		CGString sMsg;
		CChar * pChar = pClient->GetChar();
		if ( pChar )
		{
			if ( pCharCmd &&
				! pCharCmd->CanDisturb( pChar ))
			{
				continue;
			}

			TCHAR chRank = '=';
			if ( pClient->IsPriv(PRIV_GM) || pClient->GetPrivLevel() >= PLEVEL_Counsel )
			{
				chRank = ( pChar && pChar->IsDND()) ? '*' : '+';
			}

			sMsg.Format( "%x:Acc%c'%s', (%s) Char='%s',(%s)\n",
				pClient->GetSocket(),
				chRank,
				pClient->GetAccount()->GetName(),
				inet_ntoa( Name.sin_addr ),
				pChar->GetName(),
				pChar->GetTopPoint().Write());
		}
		else
		{
			if ( pConsole->GetPrivLevel() < pClient->GetPrivLevel())
			{
				continue;
			}
			sMsg.Format( "%x:Acc='%s', (%s) %s\n",
				pClient->GetSocket(),
				pClient->GetAccount() ? pClient->GetAccount()->GetName() : "<NA>",
				inet_ntoa( Name.sin_addr ),
				pClient->IsConsole() ? "Remote Console" : "NOT LOGGED IN" );
		}

		pConsole->SysMessage( sMsg );
	}
}

void CServer::SocketsReceive() // Check for messages from the clients
{
	// What sockets do I want to look at ?
	struct fd_set readfds;
	FD_ZERO(&readfds);

	FD_SET(GetSocket(), &readfds);
	int nfds = GetSocket();

	CClient * pClientNext;
	CClient * pClient = GetClientHead();
	for ( ; pClient!=NULL; pClient = pClientNext )
	{
		pClientNext = pClient->GetNext();
		SOCKET hSocket = pClient->GetSocket();
		FD_SET(hSocket,&readfds);
		if ( hSocket > nfds )
			nfds = hSocket;
	}

	// we task sleep in here. NOTE: this is where we give time back to the OS.

	m_Profile.Start( PROFILE_IDLE );

	timeval Timeout;	// time to wait for data.
	Timeout.tv_sec=0;
	Timeout.tv_usec=100;	// micro seconds = 1/1000000
	int ret = select( nfds+1, &readfds, NULL, NULL, &Timeout );
	if ( ret <= 0 )
		return;

	m_Profile.Start( PROFILE_NETWORK_RX );

	// Any events from clients ?
	for ( pClient = GetClientHead(); pClient!=NULL; pClient = pClientNext )
	{
		pClientNext = pClient->GetNext();
		if ( FD_ISSET( pClient->GetSocket(), &readfds ))
		{
			if ( ! pClient->xRecvData())
			{
				delete pClient;
				continue;
			}
		}
		else
		{
			if ( m_iDeadSocketTimeMin &&
				( pClient->m_Time_LastEvent + m_iDeadSocketTimeMin ) < g_World.GetTime() &&
				! pClient->IsConsole())
			{
				// We have not talked in several minutes.
				DEBUG_ERR(( "%x:Dead Socket Timeout\n", pClient->GetSocket()));
				delete pClient;
				continue;
			}
		}

		// On a timer allow the client to walk.
		// catch up with real time !
		// while ( time blah blah )
		pClient->addWalkCode( EXTDATA_WalkCode_Add, 1 );
	}

	// Any new connections ? what if there are several ???
	if ( FD_ISSET( GetSocket(), &readfds))
	{
		int len = sizeof( struct sockaddr_in );
		struct sockaddr_in client_addr;

		SOCKET hSocketClient = Accept( &client_addr, &len );
		if ( hSocketClient < 0 || hSocketClient == INVALID_SOCKET )	// LINUX case is signed ?
		{
			// NOTE: Client_addr might be invalid.
			g_Log.Event( LOGL_FATAL, "Failed at client connection to '%s'(?)\n", inet_ntoa( client_addr.sin_addr ));
			return;
		}

		if ( CheckLogIPBlocked( client_addr.sin_addr, true ))
		{
			// kill it by allowing it to go out of scope.
			CGSocket sock( hSocketClient );
			return;
		}

#ifdef _WIN32
		DWORD lVal = 1;	// 0 =  block
		int iRet = ioctlsocket( hSocketClient, FIONBIO, &lVal );
#endif

		new CClient( hSocketClient );
	}

	m_Profile.Start( PROFILE_OVERHEAD );
}

void CServer::SocketsFlush() // Sends ALL buffered data
{
	for ( CClient * pClient = GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		pClient->addPause( false );	// always turn off pause here if it is on.
		pClient->xFlush();
	}
}

void CServer::OnTick()
{
#ifdef _WIN32
	if ( _kbhit())
	{
		int iRet = OnConsoleKey( m_sConsoleText, _getch(), true );
		if ( iRet == 2 )
		{
			OnConsoleCmd( m_sConsoleText, this );
		}
	}
#endif

	// Check clients for incoming packets.
	SocketsReceive();

	if ( ! m_fResyncPause )
	{
		m_Profile.Start( PROFILE_CLIENTS );

		for ( CClient * pClient = GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
		{
			if ( ! pClient->xHasData())
				continue;

			if ( m_fSecure )	// enable the try code.
			{
				try
				{
					pClient->xProcess( pClient->xDispatchMsg());
				}
				catch (...)	// catch all
				{
					// clear this clients messages. so it won't do the same bad thing next time.
					pClient->xProcess( false );
					// throw( 1 );
				}
			}
			else
			{
				pClient->xProcess( pClient->xDispatchMsg());
			}
		}
	}

	m_Profile.Start( PROFILE_NETWORK_TX );
	SocketsFlush();
	m_Profile.Start( PROFILE_OVERHEAD );

	if ( m_Clock_Shutdown )
	{
		if ( m_Clock_Shutdown <= g_World.GetTime())
			m_wExitFlag=0xBEEF;
	}

	UpdatePeriodic(false);
}

bool CServer::SocketsInit() // Initialize sockets
{
	if ( ! Create())
	{
		g_Log.Event( LOGL_FATAL|LOGM_INIT, "Unable to create socket\n");
		return( false );
	}

	linger lval;
	lval.l_onoff = 0;
	lval.l_linger = 10;
	int iRet = setsockopt(GetSocket(), SOL_SOCKET, SO_LINGER, (const char*) &lval, sizeof(lval));
	if ( iRet )
	{
		DEBUG_ERR(( "setsockopt linger FAIL\n" ));
	}
#ifdef _WIN32
	DWORD lVal = 1;	// 0 =  block
	iRet = ioctlsocket( GetSocket(), FIONBIO, &lVal );
#else
	int on=1;
	int off=0;
	iRet = setsockopt(GetSocket(), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#endif

	int bcode = Bind( m_ip.GetPort());
	if (bcode<0)
	{
		// Probably already a server running.
		g_Log.Event( LOGL_FATAL|LOGM_INIT, "Unable to bind listen socket port %d - Error code: %i\n", m_ip.GetPort(), bcode );
		return( false );
	}

	// Max number we can deal with. compile time thing.
	if ( m_nClientsMax > FD_SETSIZE-1 )
		m_nClientsMax = FD_SETSIZE-1;

	Listen();

	// What are we listing our port as to the world.
	// Tell the admin what we know.

	TCHAR szName[ _MAX_PATH ];
	struct hostent * pHost = NULL;
	iRet = gethostname( szName, sizeof( szName ));
	if ( iRet )
	{
		strcpy( szName, inet_ntoa( m_ip ));
	}
	else
	{
		pHost = gethostbyname( szName );
		if ( pHost != NULL &&
			pHost->h_addr != NULL &&
			pHost->h_name &&
			pHost->h_name[0] )
		{
			strcpy( szName, pHost->h_name );
		}
	}

	g_Log.Event( LOGM_INIT, "Server started on '%s' port %d.\n", szName, m_ip.GetPort());

	if ( ! iRet )
	{
		if ( pHost == NULL || pHost->h_addr == NULL )	// can't resolve the address.
		{
			g_Log.Event( LOGL_CRIT|LOGM_INIT, "gethostbyname does not resolve the address.\n" );
		}
		else
		{
			for ( int j=0; pHost->h_aliases[j]; j++ )
			{
				g_Log.Event( LOGM_INIT, "Alias '%s'.\n", pHost->h_aliases[j] );
			}
			// h_addrtype == 2
			// h_length = 4
			for ( int i=0; pHost->h_addr_list[i] != NULL; i++ )
			{
				struct in_addr ip;
				ip.s_addr = *((DWORD*)( pHost->h_addr_list[i] )); // 0.1.2.3
				g_Log.Event( LOGM_INIT, "Monitoring IP '%s'.\n", inet_ntoa( ip ));
			}
		}
	}

	m_iExitCode = 0;	// ready to go. ! IsLoading()
	m_BackTask.CreateThread();
	return( true );
}

void CServer::SocketsClose()
{
	m_Clients.DeleteAll();
#ifdef _WIN32
	if ( m_wExitFlag )
	{
		// Wait for this to close ?
		m_BackTask.TerminateThread(-1);
	}
#endif
	Close();
}

bool CServer::LoadTables()
{
	CScriptLock s;
	if ( ! s.OpenFind( GRAY_FILE "tables" ))
	{
		g_Log.Event( LOGL_FATAL|LOGM_INIT, "Error opening table definitions file.\n" );
		return false;
	}

	static const TCHAR * pszSections[] =
	{
		"ADVANCE",
   		"EVENTS",
		"MOTIVES",
		"NOTOTITLES",
		"OBSCENE",
		"ORE",
		"PLEVEL",
		"POTION",
		"RUNES",
		"SKILL",
		"SKILLCLASS",
		"SPAWN",
		"SPEECH",
		"SPELL",
		"STARTS",
	};

	while ( s.FindNextSection())
	{
		switch ( FindTableSorted( s.GetData(), pszSections, COUNTOF( pszSections )))
		{
		case 0:	// "ADVANCE"
			// Stat advance rates.
			while ( s.ReadKeyParse())
			{
				int i = FindStatKey( s.GetKey());
				if ( i >= STAT_BASE_QTY )
					continue;
				m_StatAdv[i].Load( s.GetArgStr());
			}
			continue;
		case 1:	// "EVENTS"
		case 2: // "MOTIVES"
do_events:
			// predefine a bunch of frag sections. (Not really necessary)
			while ( s.ReadKey())
			{
				CFragmentDef::FindFragName( s.GetKey(), true );
			}
			continue;

		case 3: // "NOTOTITLES"
			while ( s.ReadKey())
			{
				TCHAR * pName = s.GetKey();
				if ( * pName == '<' ) pName = "";
				TCHAR * pNew = new TCHAR [ strlen( pName ) + 1 ];
				strcpy( pNew, pName );
				m_NotoTitles.Add( pNew );
			}
			continue;

		case 4: // "OBSCENE"
			while ( s.ReadKey())
			{
				m_Obscene.AddSortString( s.GetKey());
			}
			continue;

		case 5:	// "ORE"
			while ( s.ReadKey())
			{
				m_OreDefs.Add( new COreDef( s ));
			}
			continue;

		case 6: // "PLEVEL"
			{
				int i = s.GetArgVal();
				DEBUG_CHECK( i >= 0 && i <= PLEVEL_QTY );
				while ( s.ReadKey())
				{
					m_PrivCommands[i].AddSortString( s.GetKey());
				}
			}
			continue;

		case 7:	// "POTION"
			{
				int i = s.GetArgVal();
				DEBUG_CHECK( ! m_PotionDefs.IsValidIndex(i));
				// DEBUG_CHECK( i < CChar::IsSkillBase( iskill ));
				CPotionDef * pPotion = new CPotionDef(s);
				m_PotionDefs.SetAtGrow(i,pPotion);
#ifdef _DEBUG
				m_ppPotionDefs[i] = pPotion;
#endif
			}
			continue;
		case 8:	// "RUNES"
			while ( s.ReadKey())
			{
				TCHAR * pNew = new TCHAR [ strlen( s.GetKey()) + 1 ];
				strcpy( pNew, s.GetKey());
				m_Runes.Add( pNew );
			}
			continue;
		case 9:	// "SKILL"
			{
				SKILL_TYPE iskill = (SKILL_TYPE) s.GetArgVal();
				CSkillDef * pSkill = new CSkillDef( iskill, s );
#ifdef _DEBUG
				m_ppSkillDefs[iskill] = pSkill;
#endif
			}
			continue;
		case 10:	// "SKILLCLASS"
			{
				int i = s.GetArgVal();
				CSkillClassDef * pSkillClass = new CSkillClassDef(s);
				m_SkillClassDefs.SetAtGrow(i,pSkillClass);
			}
			continue;
		case 11:	// "SPAWN" the spawn tables
			{
				int i = s.GetArgHex() - SPAWNTYPE_START;
				DEBUG_CHECK( i >= 0 );
				if ( i < 0 ) continue;
				CRandGroupDef * pSpawnGroup = new CRandGroupDef(s);
				m_SpawnGroupDefs.SetAtGrow(i,pSpawnGroup);
			}
			continue;
		case 12: // "SPEECH"
			goto do_events;

		case 13: // "SPELL"
			{
				SPELL_TYPE i = (SPELL_TYPE) s.GetArgVal();
				CSpellDef * pSpell = new CSpellDef( s );
				m_SpellDefs.SetAtGrow(i,pSpell);
#ifdef _DEBUG
				m_ppSpellDefs[i] = pSpell;
#endif
			}
			continue;

		case 14: // "STARTS"
			while ( s.ReadKey())
			{
				CStartLoc * pStart = new CStartLoc( s.GetKey());
				if ( s.ReadKey())
				{
					pStart->m_sName = s.GetKey();
					if ( s.ReadKey())
					{
						pStart->m_p.Read( s.GetKey());
					}
				}
				m_StartDefs.Add( pStart );
			}
			continue;

		default:
			g_Log.Event( LOGL_WARN|LOGM_INIT, "Unknown section '%s' in " GRAY_FILE "TABLES.SCP\n", s.GetKey());
			continue;
		}
	}

	if ( ! m_SkillClassDefs.GetCount())
	{
		CSkillClassDef * pSkillClass = new CSkillClassDef();
		m_SkillClassDefs.SetAtGrow(0,pSkillClass);
	}

	if ( ! m_StartDefs.GetCount())	// must have 1 start location
	{
		CStartLoc * pStart = new CStartLoc( GRAY_TITLE );
		pStart->m_sName = "The Throne Room";
		pStart->m_p = g_pntLBThrone;
		m_StartDefs.Add( pStart );
	}

	return true;
}

STAT_TYPE CServer::FindStatKey( const TCHAR * pszKey ) // static
{
	return (STAT_TYPE) FindTable( pszKey, g_Stat_Name, COUNTOF( g_Stat_Name ));
}

SKILL_TYPE CServer::FindSkillKey( const TCHAR * pszKey ) const
{
	// Find the skill name in the alpha sorted list.
	// RETURN: SKILL_NONE = error.
	int i = FindTableSorted( pszKey,
		(const TCHAR * const *) m_SkillKeySort.GetBasePtr(),
		m_SkillKeySort.GetCount(),
		sizeof(CValStr));
	if ( i < 0 )
		return( SKILL_NONE );

	return( (SKILL_TYPE)( m_SkillKeySort[i].GetValue()));
}

bool CServer::LoadDefs()
{
	CScriptLock s;
	if ( ! s.OpenFind( GRAY_FILE "defs" ))
	{
		g_pLog->Event( LOGL_FATAL, "opening symbol definition file.\n" );
		return false;
	}
	while ( s.ReadKeyParse())
	{
		g_Exp.SetVarDef( s.GetKey(), s.GetArgStr());
	}
	if ( s.OpenFind( GRAY_FILE "defs2", OF_READ|OF_NONCRIT ))
	{
		while ( s.ReadKeyParse())
		{
			g_Exp.SetVarDef( s.GetKey(), s.GetArgStr());
		}
	}
	return true;
}

static const TCHAR * sm_pszScpFileNames[SCPFILE_QTY+1] =	// my required data files.
{
	GRAY_FILE "char",
	GRAY_FILE "char2",
	GRAY_FILE "item",
	GRAY_FILE "item2",
	GRAY_FILE "trig",
	GRAY_FILE "trig2",
	GRAY_FILE "book",
	GRAY_FILE "book2",
	GRAY_FILE "menu",
	GRAY_FILE "menu2",
	GRAY_FILE "gump",
	GRAY_FILE "gump2",
	GRAY_FILE "skill",
	GRAY_FILE "skill2",
	GRAY_FILE "template",
	GRAY_FILE "template2",
	GRAY_FILE "name",
	GRAY_FILE "name2",
	GRAY_FILE "spee",
	GRAY_FILE "spee2",
	NULL,
};

CScript * CServer::ScriptLock( CScriptLock & s, SCPFILE_TYPE nfile, const TCHAR * pszSection, WORD wModeFlags )
{
	// Open the script file. (get already open version if we have it)
	// Get a *2.scp version if we can.
	// NOTE: lPosStart (if set) is only good for one file !
	// RETURN:
	//   the base nfile index pointer if success.
	//   NULL = fail

	ASSERT( nfile < SCPFILE_QTY );

	while(true)
	{
		if ( m_Scripts[nfile].IsFileOpen())
		{
			if ( s.OpenCopy( m_Scripts[nfile]))
			{
				if ( ! pszSection )	// just open the file, do not search.
				{
					return( &m_Scripts[nfile] );
				}
				if ( s.FindSection( pszSection, wModeFlags | (( nfile&1 ) ? OF_NONCRIT : 0 )))
				{
					return( &m_Scripts[nfile] );
				}
			}
		}

		if ( ! ( nfile&1 ) ||	// Is it not a *2.SCP type file ?
			! pszSection )		// Only look in the one file specified.
		{
			// Failed to find the section.
			return( NULL );
		}

		nfile = (SCPFILE_TYPE)(nfile-1);
	}
}

bool CServer::ScriptLockPlot( CScriptLock & s, int index )
{
	// pre-index the plot bits.

	int i = m_PlotScpLinks.FindKey( index );
	if ( i >= 0 )
	{
		return( m_PlotScpLinks[i]->OpenLinkLock( s ));
	}

	CGString sSection;
	sSection.Format( "PLOTITEM %d", index );

	// just go searching for it.
	CScript * pScript = ScriptLock( s, SCPFILE_TRIG_2, sSection );
	if ( pScript == NULL )
		return( false );

	m_PlotScpLinks.AddSortKey( new CScriptIndexLink( index, pScript, s ), index );
	return( true );
}

bool CServer::ScriptLockTrig( CScriptLock & s, ITEM_TYPE index )
{
	// pre-index the triggers.

	int i = m_TrigScpLinks.FindKey( index );
	if ( i >= 0 )
	{
		return( m_TrigScpLinks[i]->OpenLinkLock( s ));
	}

	CGString sSection;
	sSection.Format( "TRIG %d", index );

	// just go searching for it.
	CScript * pScript = ScriptLock( s, SCPFILE_TRIG_2, sSection );
	if ( pScript == NULL )
		return( false );

	m_TrigScpLinks.AddSortKey( new CScriptIndexLink( index, pScript, s ), index );
	return( true );
}

bool CServer::SetScpFile( SCPFILE_TYPE i, const TCHAR * pszName )
{
	CScript * pFile = GetScpFile(i);
	if ( pFile == NULL )
		return false;
	pFile->SetFilePath( pszName );
	return true;
}

bool CServer::LoadScripts()
{
	// open all my script files i'm going to use.

	VERFILE_TYPE i = g_Install.OpenFiles(
		(1<<VERFILE_MAP)|
		(1<<VERFILE_STAIDX )|
		(1<<VERFILE_STATICS )|
		(1<<VERFILE_TILEDATA )|
		(1<<VERFILE_MULTIIDX )|
		(1<<VERFILE_MULTI )|
		(1<<VERFILE_VERDATA )
		);
	if ( i != VERFILE_QTY )
	{
		g_Log.Event( LOGL_FATAL|LOGM_INIT, "MUL File '%s' not found...\n", g_Install.GetBaseFileName(i));
		return( false );
	}

	for ( int j=0; j<SCPFILE_QTY; j++ )
	{
		CScript * pScript = GetScpFile((SCPFILE_TYPE)j);
		if ( pScript == NULL )
			continue;
		CGString sFileName = pScript->GetFilePath();	// name may already be set.
		if ( sFileName.IsEmpty())
		{
			if ( sm_pszScpFileNames[j] == NULL )
				continue;
			sFileName = sm_pszScpFileNames[j];
		}
		if ( ! pScript->OpenFind( sFileName, OF_READ | ((j & 1) ? OF_NONCRIT : 0 )))
		{
			if ( j & 1 )
				continue;
			g_Log.Event( LOGL_FATAL|LOGM_INIT, "Script '%s' not found...\n", pScript->GetFilePath());
			if ( j > 2 )
				continue;
			return( false );
		}
	}

	return( true );
}

bool CServer::Load()
{
	DEBUG_CHECK( IsLoading());

#ifdef _WIN32
	if ( ! IsOpen())
	{
		WSADATA wsaData;
		int err = WSAStartup( 0x0101, &wsaData );
		if (err)
		{
			g_Log.Event( LOGL_FATAL|LOGM_INIT, "Winsock 1.1 not found!\n" );
			return( false );
		}
		//if ( m_nClientsMax > wsaData.iMaxSockets-1 )
		//	m_nClientsMax = wsaData.iMaxSockets-1;
	}
#endif

#ifdef _DEBUG
	srand( 0 ); // regular randomizer.
#else
	srand( getclock()); // Perform randomize
#endif

	SetSignals();
	g_Install.FindInstall();

	if ( ! LoadIni())
		return( false );
	if ( ! LoadDefs())
		return( false );
	if ( ! LoadTables())
		return( false );
	if ( ! LoadScripts())
		return( false );

	TCHAR szVersion[ 256 ];
	m_ClientVersion.WriteClientVersion( szVersion );
	g_Log.Event( LOGM_INIT, _TEXT("Client Version: '%s'\n"), szVersion );
	if ( ! m_ClientVersion.IsValid())
	{
		g_Log.Event( LOGL_FATAL|LOGM_INIT, "Bad Client Version '%s'\n", szVersion );
		return( false );
	}

	// Load the verdata cache.
	try
	{
		g_VerData.Load();
	}
	catch(...)
	{
		return( false );
	}

	// If this is a resync. We need to reload the CBaseBase stuff we unloaded. (If it is in use)
	g_World.ReLoadBases();

#ifdef _WIN32
	CGString sTitle;
	sTitle.Format( GRAY_TITLE " V" GRAY_VERSION " - %s", GetName());
	SetConsoleTitle( sTitle );
#endif

	return( true );
}

void CServer::Unload()
{
	// Close all the files we opened.

	g_Install.CloseFiles();
	for ( int j=0; j<SCPFILE_QTY; j++ )
	{
		CScript * pFile = GetScpFile((SCPFILE_TYPE)j);
		if ( pFile == NULL )
			continue;
		pFile->Close();
	}

	g_Exp.m_VarDefs.RemoveAll();	 // Defined variables.

	// login server stuff. 0 = us.
	m_StartDefs.RemoveAll(); // Start points list
	m_PotionDefs.RemoveAll();	 // Defined potions
	m_SpellDefs.RemoveAll();	// Defined Spells
	m_SkillDefs.RemoveAll();	// Defined Skills
	m_SkillClassDefs.RemoveAll();
	m_SkillKeySort.RemoveAll();
	m_SpawnGroupDefs.RemoveAll();
	m_Runes.RemoveAll();	// Words of power. (A-Z)
	m_Obscene.RemoveAll();
	m_NotoTitles.RemoveAll();
	m_OreDefs.RemoveAll();

	// resync all the frag file pointers.
	for ( j=0; j<m_FragDefs.GetCount(); j++ )
	{
		m_FragDefs[j]->ClearFragLink();
	}

	m_PlotScpLinks.RemoveAll();
	m_TrigScpLinks.RemoveAll();

	for ( j=0; j<PLEVEL_QTY; j++ )
	{
		m_PrivCommands[j].RemoveAll();
	}

	// Type defintion information. Set all indexes to 0.
	// Can't delete these becase pointers rely on them.
	for ( j=0; j<m_ItemBase.GetCount(); j++ )
	{
		m_ItemBase[j]->UnLoad();
	}
	for ( j=0; j<m_CharBase.GetCount(); j++ )
	{
		m_CharBase[j]->UnLoad();
	}
}

bool CServer::IsValidEmailAddressFormat( const TCHAR * pszEmail ) // static
{
	// what are the invalid email name chars ?
	// Valid characters are, numbers, letters, underscore "_", dash "-" and the dot ".").

	int len1 = strlen( pszEmail );
	if ( len1 <= 0 || len1 > 128 )
		return( false );

	TCHAR szEmailStrip[256];
	int len2 = GetBareText( szEmailStrip, pszEmail,
		sizeof(szEmailStrip),
		" !\"#%&()*,/:;<=>?[\\]^{|}'`+" );
	if ( len2 != len1 )
		return( false );

	TCHAR * pszAt = strchr( pszEmail, '@' );
	if ( ! pszAt )
		return( false );
	if ( pszAt == pszEmail )
		return( false );
	if ( ! strchr( pszAt, '.' ))
		return( false );
	if ( strstr( pszAt, "@here.com" ))	// don't allow this domain.
		return( false );
	return( true );
}

bool CServer::IsObscene( const TCHAR * pszText ) const
{
	// does this text contain obscene content?
	// NOTE: allow partial match syntax *fuck* or ass (alone)

	for ( int i=0; i<m_Obscene.GetCount(); i++ )
	{
		MATCH_TYPE ematch = Text_Match( m_Obscene[i], pszText );
		if ( ematch == MATCH_VALID )
			return( true );
	}
	return( false );
}

bool CServer::CmdScriptCheck( const TCHAR *pszName )
{
	// look for script entries out of order.

	CScript s;
	if ( ! s.OpenFind( pszName ))
	{
		return( false );
	}

	int iSize = s.GetLength();
	int iErrors = 0;
	int iCount = 0;
	int iPrvVal = 0;
	while ( s.FindNextSection())
	{
		iCount++;
		TCHAR * pszKey = s.GetKey();
		int iVal = Exp_GetHex( pszKey );
		if ( iVal < iPrvVal )
		{
			DEBUG_WARN(( "Script Value [%s] is out of order\n", s.GetKey()));
			if ( ++iErrors >= 32 )
				return( false );
		}

		if ( ! ( iCount & 0x1f ))
		{
			PrintPercent( s.GetPosition(), iSize );
		}

		iPrvVal = iVal;
	}

	DEBUG_MSG(( "Script '%s' has %d order errors.\n", s.GetFilePath(), iErrors ));
	return( true );
}

bool CServer::CommandLine( int argc, TCHAR * argv[] )
{
	// RETURN:
	//  true = keep running after this.
	//
	for ( int argn=1; argn<argc; argn++ )
	{
		TCHAR * pArg = argv[argn];
		if ( ! _IS_SWITCH( pArg[0] ))
			continue;

		pArg ++;
		TCHAR ch = toupper( pArg[0] );

		if ( ch == 'B' )
		{
			// Compile to binary form this file.

		}
		if ( ch == 'P' )
		{
			// Set the port.
			m_ip.SetPort( atoi( pArg+1 ));
			continue;
		}
		if ( ch == 'N' )
		{
			// Set the system name.
			SetName( pArg+1 );
			continue;
		}
		if ( ch == 'C' )
		{
			// Compile this script. NOT USED.
			CmdScriptCheck( pArg+1 );
			continue;
		}
		if ( ch == '1' )
		{
			// dump the items database. as just a text doc.
			// Argument = hex bitmask to search thru.

			DWORD dwMask = 0xFFFFFFFF;	// UFLAG4_ANIM
			if ( argn+1<argc && argv[argn+1][0] == '0' )
			{
				dwMask = ahextoi( argv[argn+1] );
				argn++;
			}

			CFileText File;
			if ( ! File.Open( "items.txt", OF_WRITE|OF_TEXT ))
				return( false );
			int i = 0;
			for ( ; i < ITEMID_MULTI; i++ )
			{
				if ( !( i%128 )) PrintPercent( i, ITEMID_MULTI );

				CUOItemTypeRec item;
				if ( ! CItemBase::GetItemData((ITEMID_TYPE) i, &item ))
					continue;
				if ( ! ( item.m_flags & dwMask ))
					continue;

				File.Printf( "%04x: %08x,W%02x,L%02x,?%08x,A%08x,?%04x,H%02x,'%s'\n",
					i,
					item.m_flags,
					item.m_weight,
					item.m_layer,
					item.m_dwUnk6,
					item.m_dwAnim,
					item.m_wUnk14,
					item.m_height,
					item.m_name );
			}
			return( false );
		}

		if ( ch == '2' )
		{
			// dump the ground tiles database.
			CFileText File;
			if ( ! File.Open( "terrain.txt", OF_WRITE|OF_TEXT ))
				return( false );

			for ( int i=0; i<TERRAIN_QTY; i++ )
			{
				CGrayTerrainInfo block( (TERRAIN_TYPE) i);

				if ( ! block.m_flags &&
					! block.m_index &&	// just counts up.  0 = unused.
					! block.m_name[0] )
					continue;

				File.Printf( "%04x: %08x,%04x,'%s'\n",
					i,
					block.m_flags,
					block.m_index,	// just counts up.  0 = unused.
					block.m_name );
			}
			return( false );
		}

		if ( ch == '3' )
		{
			// dump a list of npc's and chars from GrayChar.scp

			CFileText File;
			if ( ! File.Open( "npcs.txt", OF_WRITE|OF_TEXT ))
				return( false );

			for ( int i=0; i<NPCID_Qty; i++ )
			{
				CCharBase * pDef = CCharBase::FindCharBase((CREID_TYPE) i );
				if ( pDef == NULL )
					continue;
				File.Printf( "[%04x] '%s'\n", i, pDef->GetTypeName());
			}

			// ??? Check for out of order stuff in GRAYCHAR.SCP

			return( false );
		}

		if ( ch == '4' )
		{
			// Xref the GRAYMENU.SCP file. "ITEMMENU %i"

			CScript s;
			if ( ! s.OpenFind( GRAY_FILE "menu" ))
				return( false );
			CFileText File;
			if ( ! File.Open( "menus.txt", OF_WRITE|OF_TEXT ))
				return( false );

			struct CMenuLink
			{
				CGString m_sName;	// Menu title.
				int m_iItems;		// How many menu items ?
				int m_iLinksIn;		// refs from other menus to this.
				int m_iLinksOut;	// Links to other menus.
			};

			CMenuLink Menus[ 255 ];	// Assume a max of this many menus.

			// 1st pass = just collect stats.
			int iMax = 0;
			int iQty = 0;
			int i=0;
			for ( ; i<COUNTOF(Menus); i++ )
			{
				Menus[i].m_iLinksIn = 0; // refs from other menus to this.
				Menus[i].m_iLinksOut = 0;	// Links to other menus.
				Menus[i].m_iItems = 0;

				CGString sSection;
				sSection.Format( "ITEMMENU %i", i );
				if ( ! s.FindSection( sSection ))
					continue;
				if ( ! s.ReadKey())
					continue;

				if ( i > iMax ) iMax = i;
				iQty ++;
			}

			// 2nd pass calculate the links.
			for ( i=0; i<iMax; i++ )
			{
				CGString sSection;
				sSection.Format( "ITEMMENU %i", i );
				if ( ! s.FindSection( sSection ))
					continue;
				if ( ! s.ReadKey())
					continue;
				Menus[i].m_sName = s.GetKey();

				while ( s.ReadKeyParse())
				{
					Menus[i].m_iItems ++;

					// ??? count this stuff.
				}
			}

			// 3rd pass print everything out.

			File.Printf(
				"Menus:\n"
				"Quantity = %d\n"
				"Max Index = %d\n\n",
				iQty, iMax );

			for ( i=0; i<iMax; i++ )
			{
				if ( Menus[i].m_sName.IsEmpty() && ! Menus[i].m_iItems && ! Menus[i].m_iLinksIn )
					continue;

				File.Printf(
					"[ITEMMENU %d]\n"
					"Items     = %d\n"
					"Links In  = %d\n"
					"Links Out = %d\n\n",
					Menus[i].m_iItems,
					Menus[i].m_iLinksIn,
					Menus[i].m_iLinksOut
					);
			}

			return( false );
		}

		if ( ch == '5' )
		{
			// ? Check for out of order stuff in ITEM.SCP

			// xref the DUPEITEM= stuff to match DUPELIST=aa,bb,cc,etc stuff
			m_iExitCode = 5;	// special mode.

			// read them all in first.
			int i=1;
			for ( ; i<ITEMID_MULTI; i++ )	// ITEMID_MULTI
			{
				if ( !( i%256 )) PrintPercent( i, 2*ITEMID_MULTI );
				CItemBase * pDef = CItemBase::FindItemBase((ITEMID_TYPE) i );
				if ( pDef == NULL || ! pDef->IsLoaded())
				{
					// Is it a defined item ?
					CUOItemTypeRec item;
					if ( ! CItemBase::GetItemData((ITEMID_TYPE) i, &item ))
						continue;

					// Seems we have a new item on our hands.
					g_Log.Event( LOGL_EVENT, "[%04x] // %s //New\n", i, item.m_name );
					continue;
				}
			}

			// Now do XREF stuff. write out the differences.
			for ( i=1; i<ITEMID_MULTI; i++ )
			{
				if ( !( i%256 )) PrintPercent( i+ITEMID_MULTI, 2*ITEMID_MULTI );

				CItemBase * pDef = CItemBase::FindItemBase((ITEMID_TYPE) i );
				if ( pDef == NULL || ! pDef->IsLoaded())
					continue;
				if ( pDef->GetBaseID() != i )
					continue;

				CGString sSec;
				sSec.Format( "%04X", i );

				CGString sVal;
				if ( ! pDef->r_WriteVal( "DUPELIST", sVal ))
					continue;

				if ( sVal.IsEmpty())
					continue;
				GetScpFile(SCPFILE_ITEM)->WriteProfileString( sSec, "DUPELIST", sVal );
			}

			return( false );
		}

		if ( ch == '6' )
		{
			// xref the ANIM= lines in xCHAR.SCP with ANIM.IDX

			if ( ! g_Install.OpenFile( VERFILE_ANIMIDX ))
			{
				return( false );
			}

			for ( int id = 0; id < CREID_QTY; id ++ )
			{
				if ( id >= CREID_MAN )
				{
					// must already have an entry in the CHAR.SCP file. (don't get the clothing etc)

					continue;
				}

				// Does it have an entry in "ANIM.IDX" ?

				int index;
				int animqty;
				if ( id < CREID_HORSE1 )
				{
					// Monsters and high detail critters.
					// 22 actions per char.
					index = ( id * ANIM_QTY_MON * 5 );
					animqty = ANIM_QTY_MON;
				}
				else if ( id < CREID_MAN )
				{
					// Animals and low detail critters.
					// 13 actions per char.
					index = 22000 + (( id - CREID_HORSE1 ) * ANIM_QTY_ANI * 5 );
					animqty = ANIM_QTY_ANI;
				}
				else
				{
					// Human and equip
					// 35 actions per char.
					index = 35000 + (( id - CREID_MAN ) * ANIM_QTY_MAN * 5 );
					animqty = ANIM_QTY_MAN;
				}

				// It MUST have a walk entry !

				DWORD dwAnim = 0;	// mask of valid anims.
				for ( int anim = 0; anim < animqty; anim ++, index += 5 )
				{
					CUOIndexRec Index;
					if ( ! g_Install.ReadMulIndex( VERFILE_ANIMIDX, VERFILE_ANIM, index, Index ))
					{
						if ( anim == 0 ) break;	// skip this.
						continue;
					}

					dwAnim |= ( 1L<<anim );
				}

				if ( dwAnim )
				{
					// report the valid animations.
					CGString sVal;
					sVal.FormatHex( dwAnim );
					CGString sSec;
					sSec.Format( "%04X", id );
					GetScpFile(SCPFILE_CHAR)->WriteProfileString( sSec, "ANIM", sVal );
				}
			}
			return( false );
		}

		if ( ch == '7' )
		{
			// resolve all the defs for the item names.
			// Match up defs.SCP with the items.SCP;

			int i=1;
			for ( ; i<ITEMID_MULTI; i++ )	// ITEMID_MULTI
			{
				if ( !( i%256 )) PrintPercent( i, 2*ITEMID_MULTI );
				CItemBase * pDef = CItemBase::FindItemBase((ITEMID_TYPE) i );
				if ( pDef == NULL ) continue;

				if ( pDef->GetBaseID() != i )
					continue;

				CGString sSec;
				sSec.Format( "%04X", i );

				int j = g_Exp.m_VarDefs.FindValNum( i );
				if ( j<0 )
				{
					for ( int k=0; pDef->GetFlippedID(k); k++ )
					{
						j = g_Exp.m_VarDefs.FindValNum( pDef->GetFlippedID(k));
						if ( j>=0) break;
					}
				}
				if ( j>= 0 )
				{
					GetScpFile(SCPFILE_ITEM)->WriteProfileString( sSec, "DEF", g_Exp.m_VarDefs[j]->GetName());
					g_Exp.m_VarDefs.DeleteAt(j);
				}

				j = g_Exp.m_VarDefs.FindValNum( i );
				if ( j<0 )
				{
					for ( int k=0; pDef->GetFlippedID(k); k++ )
					{
						j = g_Exp.m_VarDefs.FindValNum( pDef->GetFlippedID(k));
						if ( j>=0) break;
					}
				}
				if ( j>= 0 )
				{
					GetScpFile(SCPFILE_ITEM)->WriteProfileString( sSec, "DEF2", g_Exp.m_VarDefs[j]->GetName());
					g_Exp.m_VarDefs.DeleteAt(j);
				}

			}

			// What DEFS are left ?
			for ( i=0; i< g_Exp.m_VarDefs.GetCount(); i++ )
			{
				g_Log.Event( LOGL_EVENT, "Unused DEF = %s\n", g_Exp.m_VarDefs[i]->GetName());
			}

			return( false );
		}

		g_Log.Event( LOGM_INIT|LOGL_CRIT, "Don't recognize command line data '%s'\n", argv[argn] );
	}

	return( true );
}

