//
// CResourceDef.cpp
// Copyright Menace Software (www.menasoft.com).
// A variety of resource blocks.
//
#include "graysvr.h"	// predef header.
#include "../common/CAssoc.h"

//*******************************************
// -CLogIP = Blocked IP's

CLogIP::CLogIP( CSocketAddressIP dwIP ) :
	m_ip( dwIP )
{
	InitTimes();
	m_iConnecting	= 0;
	m_iConnected	= 0;
}

void CLogIP::InitTimes()
{
	m_fBlocked = false;
	m_timeFirst.Init();
	m_timeLast.Init();
	m_timeDecay.Init();
	m_iPings = 0;
	m_pAccount = NULL;	// disconnect from account

	/*if ( g_Cfg.m_fLocalIPAdmin && m_ip.IsLocalAddr())
	{
		m_pAccount = g_Accounts.Account_Find( "Administrator" );
	}*/
}

bool CLogIP::IsTimeDecay() const
{
	return( CServTime::GetCurrentTime() >= m_timeDecay );
}

void CLogIP::SetBlocked( bool fBlocked, int iTimeDecay )
{
	// ARGS:
	//  iTimeDecay = TICK_PER_SEC
	m_fBlocked = true;
	if ( iTimeDecay == -1 )
	{
		m_timeDecay.InitTime( INT_MAX );	// never decay
	}
	else
	{
		m_timeDecay = CServTime::GetCurrentTime() + iTimeDecay;
	}
}

bool CLogIP::CheckPingBlock( bool fPreAccept )
{
	// A ping has occured from this ip.
	// Have we had too many pings in a short time ?
	// ARGS:
	//  fPreAccept = is this the pre-accept stage?
	// NOTE:
	//  If the IP is blocked and this is the first ping then try to give a response.
	// RETURN:
	//  true = block this.
	//

	m_timeLast = CServTime::GetCurrentTime();
	if ( ! m_iPings )
	{
		// this is the first.
		m_timeFirst = m_timeLast;
	}
	else
	{
		// if we have had few pings in a long time then clear the start time.
		if ( m_timeLast - m_timeFirst > 15*TICK_PER_SEC &&
			m_iPings < 15 )
		{
			m_timeFirst = m_timeLast;
			m_iPings = 0;
		}
	}

	m_timeDecay = m_timeLast + (1*60*TICK_PER_SEC);
	m_iPings ++;

	if ( fPreAccept )	// always allow this thru just to shut up the normal clients.
	{
		if ( GetPings() <= 1 )
			return( false );
	}

	if ( GetPings() > 15 )
	{
		// block for now.
		if ( GetPings() == 16 && ! IsBlocked())
		{
			g_Log.Event( LOGM_CLIENTS_LOG|LOGL_ERROR, "Possible ping attack from %s" DEBUG_CR, (LPCTSTR) m_ip.GetAddrStr());
			if ( m_pAccount == NULL || m_pAccount->GetPrivLevel() < PLEVEL_GM )
			{
				SetBlocked( true, 1*60*TICK_PER_SEC );
			}
		}
		return( true );
	}

	// We are marked as always blocked?
	if ( IsBlocked())
	{
		// Look for it in the blockip list.
		if ( GetPings() <= 1 )
		{
			g_Log.Event( LOGM_CLIENTS_LOG|LOGL_ERROR, "BlockIP detected %s" DEBUG_CR, (LPCTSTR) m_ip.GetAddrStr());
		}
		return( true );
	}

	m_timeDecay = m_timeLast + (5*60*TICK_PER_SEC);
	return( false );	// looks ok to me.
}

//*******************************************
// -CValueRangeDef

int CValueRangeDef::GetRandomLinear( int iPercent ) const
{
	return( ( GetRandom() + GetLinear(iPercent) ) / 2 ); 
}

bool CValueRangeDef::Load( TCHAR * pszDef )
{
	// Typically in {lo# hi#} format. is hi#,lo# is ok too ???
	int piVal[2];
	int iQty = g_Exp.GetRangeVals( pszDef, piVal, COUNTOF(piVal));
	if ( iQty< 0 ) 
		return(false);

	m_iLo = piVal[0];
	if ( iQty > 1 )
	{
		m_iHi = piVal[1];
	}
	else
	{
		m_iHi = m_iLo;
	}
	return( true );
}

const TCHAR * CValueRangeDef::Write() const
{
	return( NULL );
}

//*******************************************
// -CValueCurveDef

const TCHAR * CValueCurveDef::Write() const
{
	TCHAR * pszOut = Str_GetTemp();
	int j=0;
	int iQty = m_aiValues.GetCount();
	for ( int i=0; i<iQty; i++ )
	{
		j += sprintf( pszOut+j, _TEXT("%d"), m_aiValues[i] );
		if ( i<iQty-1 )
		{
			pszOut[j++] = ',';
		}
	}
	pszOut[j] = '\0';
	return pszOut;
}

bool CValueCurveDef::Load( TCHAR * pszDef )
{
	// ADV_RATE = Chance at 0, to 100.0
	int Arg_piCmd[101];
	int iQty = Str_ParseCmds( pszDef, Arg_piCmd, COUNTOF(Arg_piCmd));
	m_aiValues.SetCount( iQty );
	if ( iQty == 0 )
	{
		return( false );
	}
	for ( int i=0; i<iQty; i++ )
	{
		m_aiValues[i] = Arg_piCmd[i];
	}
	return( true );
};

int CValueCurveDef::GetLinear( int iSkillPercent ) const
{
	//
	// ARGS:
	//  iSkillPercent = 0 - 1000 = 0 to 100.0 percent
	//  m_Rate[3] = the 3 advance rate control numbers, 100,50,0 skill levels
	//		Acts as line segments.
	// RETURN:
	//  raw chance value.

	int iSegSize;
	int iLoIdx;

	int iQty = m_aiValues.GetCount();
	switch (iQty)
	{
	case 0:
		return( 0 );	// no values defined !
	case 1:
		return( m_aiValues[0] );
	case 2:
		iLoIdx = 0;
		iSegSize = 1000;
		break;
	case 3:
		// Do this the fastest.
		if ( iSkillPercent >= 500 )
		{
			iLoIdx = 1;
			iSkillPercent -= 500;
		}
		else
		{
			iLoIdx = 0;
		}
		iSegSize = 500;
		break;
	default:
		// More
		iLoIdx = IMULDIV( iSkillPercent, iQty, 1000 );
		iQty--;
		if ( iLoIdx < 0 )
			iLoIdx = 0;
		if ( iLoIdx >= iQty )
			iLoIdx = iQty-1;
		iSegSize = 1000 / iQty;
		iSkillPercent -= ( iLoIdx * iSegSize );
		break;
	}

	int iLoVal = m_aiValues[iLoIdx];
	int iHiVal = m_aiValues[iLoIdx+1];
	int iChance = iLoVal + IMULDIV( iHiVal - iLoVal, iSkillPercent, iSegSize );

	if ( iChance <= 0 )
		return 0; // less than no chance ?

	return( iChance );
}

int CValueCurveDef::GetRandom( ) const
{
	return GetLinear( Calc_GetRandVal( 1000 ) );
}


int CValueCurveDef::GetRandomLinear( int iSkillPercent  ) const
{
	return ( GetLinear( iSkillPercent ) + GetRandom() ) / 2;
}

int CValueCurveDef::GetChancePercent( int iSkillPercent ) const
{
	// ARGS:
	//  iSkillPercent = 0 - 1000 = 0 to 100.0 percent
	//
	//  m_Rate[3] = the 3 advance rate control numbers, 0,50,100 skill levels
	//   (How many uses for a gain of .1 (div by 100))
	// RETURN:
	//  percent chance of success * 10 = 0 - 1000.

	// How many uses for a gain of .1 (div by 100)
	int iChance = GetLinear( iSkillPercent );
	if ( iChance <= 0 )
		return 0; // less than no chance ?
	// Express uses as a percentage * 10.
	return( 100000 / iChance );
}

//*******************************************
// -CSkillDef

LPCTSTR const CSkillDef::sm_szTrigName[SKTRIG_QTY+1] = 
{
	_TEXT("@AAAUNUSED"),
	_TEXT("@ABORT"),
	_TEXT("@FAIL"),	// we failed the skill check.
	_TEXT("@GAIN"),
	_TEXT("@SELECT"),	// just selecting params for the skill
	_TEXT("@START"),	// params for skill are done.
	_TEXT("@STROKE"),
	_TEXT("@SUCCESS"),
	NULL,	
};

enum SKC_TYPE
{
	SKC_ADV_RATE,
	SKC_BONUS_DEX,
	SKC_BONUS_INT,
	SKC_BONUS_STATS,
	SKC_BONUS_STR,
	SKC_COSTS,
	SKC_DEFNAME,
	SKC_DELAY,
	SKC_DESC,
	SKC_EFFECT,
	SKC_FLAGS,
	SKC_GROUP,
	SKC_KEY,
	SKC_NAME,
	SKC_PROMPT_MSG,
	SKC_STAT_DEX,
	SKC_STAT_INT,
	SKC_STAT_STR,
	SKC_TITLE,
	SKC_VALUES,
	SKC_QTY,
};

LPCTSTR const CSkillDef::sm_szLoadKeys[SKC_QTY+1] =
{
	"ADV_RATE",
	"BONUS_DEX",
	"BONUS_INT",
	"BONUS_STATS",
	"BONUS_STR",
	"COSTS",
	"DEFNAME",
	"DELAY",
	"DESC",
	"EFFECT",
	"FLAGS",
	"GROUP",
	"KEY",
	"NAME",
	"PROMPT_MSG",
	"STAT_DEX",
	"STAT_INT",
	"STAT_STR",
	"TITLE",
	"VALUES",
	NULL,
};

CSkillDef::CSkillDef( SKILL_TYPE skill ) :
	CResourceLink( RESOURCE_ID( RES_SKILL, skill ))
{
	if ( !CChar::IsSkillBase(skill) )
	// DEBUG_CHECK( CChar::IsSkillBase(skill) );
	m_StatPercent	= 0;
	m_dwFlags		= 0;
	m_dwGroup		= 0;
	m_Costs.Init();
	m_AdvRate.Init();
}

void CSkillDef::r_DumpLoadKeys( CTextConsole * pSrc )
{
	r_DumpKeys(pSrc,sm_szLoadKeys);
	CResourceDef::r_DumpLoadKeys(pSrc);
}

bool CSkillDef::r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc )
{
	EXC_TRY(("r_WriteVal('%s',,%x)", pszKey, pSrc));
	switch ( FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case SKC_ADV_RATE:	// ADV_RATE=Chance at 100, Chance at 50, chance at 0
		sVal = m_AdvRate.Write();
		break;
	case SKC_COSTS:
		sVal = m_Costs.Write();
		break;
	case SKC_BONUS_DEX: // "BONUS_DEX"
		sVal.FormatVal( m_StatBonus[STAT_DEX] );
		break;
	case SKC_BONUS_INT: // "BONUS_INT"
		sVal.FormatVal( m_StatBonus[STAT_INT] );
		break;
	case SKC_BONUS_STR: // "BONUS_STR"
		sVal.FormatVal( m_StatBonus[STAT_STR] );
		break;
	// case SKC_DEFNAME: // "DEFNAME"
	case SKC_DELAY:
		sVal = m_Delay.Write();
		break;
	case SKC_EFFECT:
		sVal = m_Effect.Write();
		break;
	case SKC_FLAGS:
		sVal.FormatHex( m_dwFlags );
		break;
	case SKC_GROUP:
		sVal.FormatHex( m_dwGroup );
		break;
	case SKC_KEY: // "KEY"
		sVal = m_sKey;
		break;
	case SKC_DESC:
		sVal = m_sDesc;
		break;
	case SKC_NAME: // "NAME"
		sVal = m_sName.IsEmpty() ? m_sKey : m_sName;
		break;
	case SKC_PROMPT_MSG: // "PROMPT_MSG"
		sVal = m_sTargetPrompt;
		break;
	case SKC_BONUS_STATS: // "BONUS_STATS"
		sVal.FormatVal( m_StatPercent );
		break;
	case SKC_STAT_DEX: // "STAT_DEX"
		sVal.FormatVal( m_Stat[STAT_DEX] );
		break;
	case SKC_STAT_INT: // "STAT_INT"
		sVal.FormatVal( m_Stat[STAT_INT] );
		break;
	case SKC_STAT_STR: // "STAT_STR"
		sVal.FormatVal( m_Stat[STAT_STR] );
		break;
	case SKC_TITLE: // "TITLE"
		sVal = m_sTitle;
		break;
	case SKC_VALUES: // VALUES = 100,50,0 price levels.
		sVal = m_Values.Write();
		break;
	default:
		return( CResourceDef::r_WriteVal( pszKey, sVal, pSrc ));
	}
	EXC_CATCH("CSkillDef");
	return( true );
}

bool CSkillDef::r_LoadVal( CScript &s )
{
	EXC_TRY(("r_LoadVal('%s %s')", s.GetKey(), s.GetArgStr()));
	switch ( FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case SKC_ADV_RATE:	// ADV_RATE=Chance at 100, Chance at 50, chance at 0
		m_AdvRate.Load( s.GetArgStr());
		break;
	case SKC_COSTS:
		m_Costs.Load( s.GetArgStr());
		break;
	case SKC_BONUS_DEX: // "BONUS_DEX"
		m_StatBonus[STAT_DEX] = s.GetArgVal();
		break;
	case SKC_BONUS_INT: // "BONUS_INT"
		m_StatBonus[STAT_INT] = s.GetArgVal();
		break;
	case SKC_BONUS_STR: // "BONUS_STR"
		m_StatBonus[STAT_STR] = s.GetArgVal();
		break;
	case SKC_DEFNAME: // "DEFNAME"
		return SetResourceName( s.GetArgStr());
	case SKC_DELAY:
		m_Delay.Load( s.GetArgStr());
		break;
	case SKC_FLAGS:
		m_dwFlags	= s.GetArgVal();
		break;
	case SKC_GROUP:
		m_dwGroup	= s.GetArgVal();
		break;
	case SKC_EFFECT:
		m_Effect.Load( s.GetArgStr());
		break;
	case SKC_KEY: // "KEY"
		m_sKey = s.GetArgStr();
		return SetResourceName( m_sKey );
	case SKC_DESC: // "KEY"
		m_sDesc = s.GetArgStr();
		break;
	case SKC_NAME: // "KEY"
		m_sName = s.GetArgStr();
		break;
	case SKC_PROMPT_MSG: // "PROMPT_MSG"
		m_sTargetPrompt = s.GetArgStr();
		break;
	case SKC_BONUS_STATS: // "BONUS_STATS"
		m_StatPercent = s.GetArgVal();
		break;
	case SKC_STAT_DEX: // "STAT_DEX"
		m_Stat[STAT_DEX]	=  s.GetArgVal();
		break;
	case SKC_STAT_INT: // "STAT_INT"
		m_Stat[STAT_INT]	=  s.GetArgVal();
		break;
	case SKC_STAT_STR: // "STAT_STR"
		m_Stat[STAT_STR]	=  s.GetArgVal();
		break;
	case SKC_TITLE: // "TITLE"
		m_sTitle = s.GetArgStr();
		break;
	case SKC_VALUES: // VALUES = 100,50,0 price levels.
		m_Values.Load( s.GetArgStr() );
		break;
	default:
		return( CResourceDef::r_LoadVal( s ));
	}
	EXC_CATCH("CSkillDef");
	return( true );
}



//*******************************************
// -CSkillClassDef

enum SCC_TYPE
{
	SCC_DEFNAME,
	SCC_NAME,
	SCC_SKILLSUM,
	SCC_STATSUM,
	SCC_QTY,
};

LPCTSTR const CSkillClassDef::sm_szLoadKeys[SCC_QTY+1] =
{
	"DEFNAME",
	"NAME",
	"SKILLSUM",
	"STATSUM",
	NULL,
};

void CSkillClassDef::Init()
{
	m_SkillSumMax = 10*1000;
	m_StatSumMax = 300;
	int i;
	for ( i=0; i<COUNTOF(m_SkillLevelMax); i++ )
	{
		m_SkillLevelMax[i] = 1000;
	}
	for ( i=0; i<COUNTOF(m_StatMax); i++ )
	{
		m_StatMax[i] = 100;
	}
}

void CSkillClassDef::r_DumpLoadKeys( CTextConsole * pSrc )
{
	r_DumpKeys(pSrc,sm_szLoadKeys);
	CResourceDef::r_DumpLoadKeys(pSrc);
}

bool CSkillClassDef::r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc )
{
	EXC_TRY(("r_WriteVal('%s',,%x)", pszKey, pSrc));
	switch ( FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case SCC_NAME: // "NAME"
		sVal = m_sName;
		break;
	case SCC_SKILLSUM:
		sVal.FormatVal( m_SkillSumMax );
		break;
	case SCC_STATSUM:
		sVal.FormatVal( m_StatSumMax );
		break;
	default:
		{
			int i = g_Cfg.FindSkillKey( pszKey);
			if ( i != SKILL_NONE )
			{
				ASSERT( i<COUNTOF(m_SkillLevelMax));
				sVal.FormatVal( m_SkillLevelMax[i] );
				break;
			}
			i = g_Cfg.FindStatKey( pszKey);
			if ( i >= 0 )
			{
				ASSERT( i<COUNTOF(m_StatMax));
				sVal.FormatVal( m_StatMax[i] );
				break;
			}
		}
		return( CResourceDef::r_WriteVal( pszKey, sVal, pSrc ));
	}
	EXC_CATCH("CSkillClassDef");
	return( true );
}


bool CSkillClassDef::r_LoadVal( CScript &s )
{
	EXC_TRY(("r_LoadVal('%s %s')", s.GetKey(), s.GetArgStr()));
	switch ( FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case SCC_DEFNAME:
		return SetResourceName( s.GetArgStr());
	case SCC_NAME: // "NAME"
		m_sName = s.GetArgStr();
		break;
	case SCC_SKILLSUM:
		m_SkillSumMax = s.GetArgVal();
		break;
	case SCC_STATSUM:
		m_StatSumMax = s.GetArgVal();
		break;
	default:
		{	/* TODO: Skillclass dynamic mod */
			int i = g_Cfg.FindSkillKey( s.GetKey());
			if ( i != SKILL_NONE )
			{
				ASSERT( i<COUNTOF(m_SkillLevelMax));
				m_SkillLevelMax[i] = s.GetArgVal();
				break;
			}
			i = g_Cfg.FindStatKey( s.GetKey());
			if ( i >= 0 )
			{
				ASSERT( i<COUNTOF(m_StatMax));
				m_StatMax[i] = s.GetArgVal();
				break;
			}
		}
		return( CResourceDef::r_LoadVal( s ));
	}
	EXC_CATCH("CSkillClassDef");
	return( true );
}


//*******************************************
// -CSpellDef

LPCTSTR const CSpellDef::sm_szTrigName[SPTRIG_QTY+1] = 
{
	_TEXT("@AAAUNUSED"),
	_TEXT("@EFFECT"),
	_TEXT("@FAIL"),
	_TEXT("@SELECT"),
	_TEXT("@START"),
	_TEXT("@SUCCESS"),
	NULL,	
};

enum SPC_TYPE
{
	SPC_CAST_TIME,
	SPC_DEFNAME,
	SPC_DESC,
	SPC_DURATION,
	SPC_EFFECT,
	SPC_EFFECT_ID,
	SPC_FLAGS,
	SPC_GROUP,
	SPC_INTERRUPT,
	SPC_MANAUSE,
	SPC_NAME,
	SPC_PROMPT_MSG,
	SPC_RESCAST,
	SPC_RESOURCES,
	SPC_RUNE_ITEM,
	SPC_RUNES,
	SPC_SCROLL_ITEM,
	SPC_SKILLREQ,
	SPC_SOUND,
	SPC_QTY,
};

LPCTSTR const CSpellDef::sm_szLoadKeys[SPC_QTY+1] =
{
	"CAST_TIME",
	"DEFNAME",
	"DESC",
	"DURATION",
	"EFFECT",
	"EFFECT_ID",
	"FLAGS",
	"GROUP",
	"INTERRUPT",
	"MANAUSE",
	"NAME",
	"PROMPT_MSG",
	"RESCAST",
	"RESOURCES",
	"RUNE_ITEM",
	"RUNES",
	"SCROLL_ITEM",
	"SKILLREQ",
	"SOUND",
	NULL,
};

CSpellDef::CSpellDef( SPELL_TYPE id ) :
	CResourceLink( RESOURCE_ID( RES_SPELL, id ))
{
	m_dwFlags = SPELLFLAG_DISABLED;
	m_dwGroup	= 0;
	m_idEffect = ITEMID_NOTHING;
	m_CastTime.Init();
	m_Interrupt.Init();
	m_Interrupt.m_aiValues.SetCount( 1 );
	m_Interrupt.m_aiValues[0]	= 1000;
}



bool CSpellDef::r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc )
{
	EXC_TRY(("r_WriteVal('%s',,%x)", pszKey, pSrc));
	switch ( FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case SPC_CAST_TIME:
		sVal = m_CastTime.Write();
		break;
	case SPC_DEFNAME:
		sVal = GetResourceName();
		break;
	case SPC_DURATION:
		sVal = m_Duration.Write();
		break;
	case SPC_EFFECT:
		sVal = m_Effect.Write();
		break;
	case SPC_EFFECT_ID:
		sVal.FormatVal( m_idEffect );
		break;
	case SPC_FLAGS:
		sVal.FormatVal( m_dwFlags );
		break;
	case SPC_GROUP:
		sVal.FormatVal( m_dwGroup );
		break;
	case SPC_INTERRUPT:
		sVal = m_Interrupt.Write();
		break;
	case SPC_MANAUSE:
		sVal.FormatVal( m_wManaUse );
		break;
	case SPC_DESC:
		sVal = m_sDesc;
		break;
	case SPC_NAME:
		sVal = m_sName;
		break;
	case SPC_PROMPT_MSG:
		sVal	= m_sTargetPrompt;
		break;
	case SPC_RESCAST:
		{
			TCHAR *pszTmp = Str_GetTemp();
			m_Reags.WriteNames( pszTmp );
			sVal = pszTmp;
		}
		break;
	case SPC_RESOURCES:
		{
			TCHAR *pszTmp = Str_GetTemp();
			m_Reags.WriteKeys( pszTmp );
			sVal = pszTmp;
		}
		break;
	case SPC_RUNE_ITEM:
		sVal.FormatVal( m_idSpell );
		break;
	case SPC_RUNES:
		// This may only be basic chars !
		sVal = m_sRunes;
		break;
	case SPC_SCROLL_ITEM:
		sVal.FormatVal( m_idScroll );
		break;
	case SPC_SKILLREQ:
		{
			TCHAR *pszTmp = Str_GetTemp();
			m_SkillReq.WriteKeys( pszTmp );
			sVal = pszTmp;
		}
		break;
	case SPC_SOUND:
		sVal.FormatVal(m_sound);
		break;
	default:
		return( CResourceDef::r_WriteVal( pszKey, sVal, pSrc ));
	}
	EXC_CATCH("CSpellDef");
	return( true );
}



bool CSpellDef::r_LoadVal( CScript &s )
{
	EXC_TRY(("r_LoadVal('%s %s')", s.GetKey(), s.GetArgStr()));
	// RES_SPELL
	switch ( FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case SPC_CAST_TIME:
		m_CastTime.Load( s.GetArgRaw());
		break;
	case SPC_DEFNAME:
		return SetResourceName( s.GetArgStr());
	case SPC_DURATION:
		m_Duration.Load( s.GetArgRaw());
		break;
	case SPC_EFFECT:
		m_Effect.Load( s.GetArgRaw());
		break;
	case SPC_EFFECT_ID:
		m_idEffect = (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, s.GetArgStr());
		break;
	case SPC_FLAGS:
		m_dwFlags = s.GetArgVal();
		break;
	case SPC_GROUP:
		m_dwGroup = s.GetArgVal();
		break;
	case SPC_INTERRUPT:
		m_Interrupt.Load( s.GetArgRaw());
		break;
	case SPC_MANAUSE:
		m_wManaUse = s.GetArgVal();
		break;
	case SPC_DESC:
		m_sDesc = s.GetArgStr();
		break;
	case SPC_NAME:
		m_sName = s.GetArgStr();
		break;
	case SPC_PROMPT_MSG:
		m_sTargetPrompt	= s.GetArgStr();
		break;
	case SPC_RESOURCES:
		m_Reags.Load( s.GetArgStr());
		break;
	case SPC_RUNE_ITEM:
		m_idSpell = (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, s.GetArgStr());
		break;
	case SPC_RUNES:
		// This may only be basic chars !
		m_sRunes = s.GetArgStr();
		break;
	case SPC_SCROLL_ITEM:
		m_idScroll = (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, s.GetArgStr());
		break;
	case SPC_SKILLREQ:
		m_SkillReq.Load( s.GetArgStr());
		break;
	case SPC_SOUND:
		m_sound = (SOUND_TYPE) s.GetArgVal();
		break;
	case SPC_RESCAST:	// unkown at load
	default:
		return( CResourceDef::r_LoadVal( s ) );
	}
	EXC_CATCH("CSpellDef");
	return( true );
}



bool	CSpellDef::GetPrimarySkill( int * iSkill, int * iQty ) const
{
	int i = m_SkillReq.FindResourceType( RES_SKILL );
	if ( i < 0 )
		return NULL;
	if ( iQty )
		*iQty	= m_SkillReq[i].GetResQty();
	if ( iSkill )
		*iSkill	= m_SkillReq[i].GetResIndex();
	return (g_Cfg.GetSkillDef( (SKILL_TYPE) m_SkillReq[i].GetResIndex() ) != NULL);
}

//*******************************************
// -CRandGroupDef

enum RGC_TYPE
{
	RGC_DEFNAME,
	RGC_ID,
	RGC_RESOURCES,
	RGC_WEIGHT,
	RGC_QTY,
};

LPCTSTR const CRandGroupDef::sm_szLoadKeys[RGC_QTY+1] =
{
	"DEFNAME",
	"ID",
	"RESOURCES",
	"WEIGHT",
	NULL,
};

int CRandGroupDef::CalcTotalWeight()
{
	int iTotal = 0;
	int iQty = m_Members.GetCount();
	for ( int i=0; i<iQty; i++ )
	{
		iTotal += m_Members[i].GetResQty();
	}
	return( m_iTotalWeight = iTotal );
}

bool CRandGroupDef::r_LoadVal( CScript &s )
{
	EXC_TRY(("r_LoadVal('%s %s')", s.GetKey(), s.GetArgStr()));
	// RES_SPAWN or RES_REGIONTYPE
	switch ( FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case RGC_DEFNAME: // "DEFNAME"
		return SetResourceName( s.GetArgStr());
	case RGC_ID:	// "ID"
		{
			TCHAR * ppCmd[2];
			int iArgs = Str_ParseCmds( s.GetArgStr(), ppCmd, COUNTOF(ppCmd));
			CResourceQty rec;
			rec.SetResourceID( 
				g_Cfg.ResourceGetID( RES_CHARDEF, (LPCTSTR&)ppCmd[0] ),
				( iArgs > 1 && ppCmd[1][0] ) ? Exp_GetVal( (LPCTSTR&)ppCmd[1] ) : 1 );
			m_iTotalWeight += rec.GetResQty();
			m_Members.Add(rec);
		}
		return( true );

	case RGC_RESOURCES:
		m_Members.Load( s.GetArgStr());
		CalcTotalWeight();
		return( true );

	case RGC_WEIGHT: // Modify the weight of the last item.
		if ( ! m_Members.GetCount())
			break;
		{
			int iWeight = s.GetArgVal();
			m_Members[ m_Members.GetCount()-1 ].SetResQty( iWeight );
			CalcTotalWeight();
		}
		return( true );

	default:
		return( CResourceDef::r_LoadVal( s ));
	}
	EXC_CATCH("CRandGroupDef");
	return( true );
}

int CRandGroupDef::GetRandMemberIndex( CChar * pCharSrc ) const
{
	int rid;
	int iCount = m_Members.GetCount();
	
	if ( ! iCount )
		return( -1 );

	int iWeight	= 0;
	int	i;
	if ( !pCharSrc )
	{
		iWeight	= Calc_GetRandVal( m_iTotalWeight ) + 1;

		for ( i = 0; iWeight > 0 && i<iCount; i++ )
		{
			iWeight -= m_Members[i].GetResQty();
		}
		if ( (i == iCount) && iWeight > 0 )
			return -1;
		return( i - 1);
	}

	CGPtrTypeArray <int>	members;

	// calculate weight only of items pCharSrc can get
	{
		int		iTotalWeight	= 0;
		for ( i = 0; i < iCount; i++ )
		{
			CRegionResourceDef *	pOreDef = dynamic_cast <CRegionResourceDef *>( g_Cfg.ResourceGetDef( m_Members[i].GetResourceID() ) );
			rid		= pOreDef->m_ReapItem;
			if ( rid != 0 )
			{
				// CItemBase * pItemDef = CItemBase::FindItemBase( (ITEMID_TYPE) rid );
				if ( !pCharSrc->Skill_MakeItem( (ITEMID_TYPE) rid, UID_CLEAR, SKTRIG_SELECT ) )
					continue;
				
				if ( pOreDef->OnTrigger( "@ResourceTest", pCharSrc, NULL ) == TRIGRET_RET_TRUE )
					continue;
			}
			members.Add( i );
			iTotalWeight	+= m_Members[i].GetResQty();
		}
		iWeight	= Calc_GetRandVal( iTotalWeight ) + 1;
		iCount	= members.GetCount();
	}

	for ( i = 0; iWeight > 0 && i<iCount; i++ )
	{
		iWeight -= m_Members[ members[i] ].GetResQty();
	}
	if ( (i == iCount) && iWeight > 0 )
		return -1;
	return( members[ i - 1 ]);
}

//*******************************************
// -CRegionResourceDef

enum RMC_TYPE
{
	RMC_AMOUNT,
	RMC_DEFNAME,
	RMC_REAP,
	RMC_REAPAMOUNT,
	RMC_REGEN,
	RMC_SKILL,
	RMC_QTY,
};

LPCTSTR const CRegionResourceDef::sm_szLoadKeys[RMC_QTY+1] =
{
	"AMOUNT",
	"DEFNAME",
	"REAP",
	"REAPAMOUNT",
	"REGEN",
	"SKILL",
	NULL,
};



TRIGRET_TYPE CRegionResourceDef::OnTrigger( LPCTSTR pszTrigName, CTextConsole * pSrc, CScriptTriggerArgs * pArgs )
{
	// Attach some trigger to the cchar. (PC or NPC)
	// RETURN: true = block further action.

	CResourceLock s;
	if ( ResourceLock( s ))
	{
		TRIGRET_TYPE iRet = CScriptObj::OnTriggerScript( s, pszTrigName, pSrc, pArgs );
		return iRet;
	}
	return TRIGRET_RET_DEFAULT;
}


bool CRegionResourceDef::r_LoadVal( CScript & s )
{
	EXC_TRY(("r_LoadVal('%s %s')", s.GetKey(), s.GetArgStr()));
	// RES_REGIONRESOURCE
	switch ( FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case RMC_AMOUNT: // AMOUNT
		m_Amount.Load( s.GetArgRaw() );
		break;
	case RMC_DEFNAME: // "DEFNAME",
		return SetResourceName( s.GetArgStr());
	case RMC_REAP: // "REAP",
		m_ReapItem = (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, s.GetArgStr());
		break;
	case RMC_REAPAMOUNT:
		m_ReapAmount.Load( s.GetArgRaw() );
		break;
	case RMC_REGEN:
		m_iRegenerateTime = s.GetArgVal();	// TICK_PER_SEC once found how long to regen this type.
		break;
	case RMC_SKILL:
		m_Skill.Load( s.GetArgRaw() );
		break;
	default:
		return( CResourceDef::r_LoadVal( s ));
	}
	EXC_CATCH("CRegionResourceDef");
	return( true );
}

void CRegionResourceDef::r_DumpLoadKeys( CTextConsole * pSrc )
{
	r_DumpKeys(pSrc,sm_szLoadKeys);
	CResourceDef::r_DumpLoadKeys(pSrc);
}

bool CRegionResourceDef::r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc )
{
	EXC_TRY(("r_WriteVal('%s',,%x)", pszKey, pSrc));
	// RES_REGIONRESOURCE
	switch ( FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case RMC_AMOUNT:
		sVal = m_Amount.Write();
		break;
	//case RMC_DEFNAME:
	/* case RMC_REAP:
		{
			CItemBase *	pItem	= m_ReapItem ...
		}
		*/
	case RMC_REAPAMOUNT:
		sVal = m_ReapAmount.Write();
		break;
	case RMC_REGEN:
		sVal.FormatVal( m_iRegenerateTime );
		break;
	case RMC_SKILL:
		sVal = m_Skill.Write();
		break;
	default:
		return( CResourceDef::r_WriteVal( pszKey, sVal, pSrc ));
	}
	EXC_CATCH("CRegionResourceDef");
	return( true );
}

CRegionResourceDef::CRegionResourceDef( RESOURCE_ID rid ) :
	CResourceLink( rid )
{
	// set defaults first.
	m_iRegenerateTime = 0;	// TICK_PER_SEC once found how long to regen this type.
}

CRegionResourceDef::~CRegionResourceDef()
{
}

//*******************************************
// -CRaceClassDef

CRaceClassDef g_BaseRaceClass( RES_RACECLASS );

LPCTSTR const CRaceClassDef::sm_szLoadKeys[] =
{
	"BLOODCOLOR",
	"BODYPARTS",
	"REGEN",
	NULL,
};

CRaceClassDef::CRaceClassDef( RESOURCE_ID id ) :
	CResourceDef( id )
{
	// RES_RACECLASS
	// Get rid of CAN_C_NONHUMANOID flag in favor of this !
	// Put blood wHue in here as well.

	m_dwBodyPartFlags = 0xFFFFFFFF;	// What body parts do we have ? BODYPART_TYPE

	m_iRegenRate[STAT_STR] = 6*TICK_PER_SEC;		// Seconds to heal ONE hp (before stam/food adjust)
	m_iRegenRate[STAT_INT] = 5*TICK_PER_SEC;		// Seconds to heal ONE mn
	m_iRegenRate[STAT_DEX] = 3*TICK_PER_SEC;		// Seconds to heal ONE stm
	m_iRegenRate[STAT_FOOD] = 30*60*TICK_PER_SEC;		// Food usage (1 time per 30 minutes)
	m_iRegenRate[STAT_KARMA] = 0;				// Karma doesn't drop
	m_iRegenRate[STAT_FAME] = 12*60*60*TICK_PER_SEC;	// Fame drop (1 time per x minutes)
}


void CRaceClassDef::r_DumpLoadKeys( CTextConsole * pSrc )
{
	r_DumpKeys(pSrc,sm_szLoadKeys);
	CResourceDef::r_DumpLoadKeys(pSrc);
}

bool CRaceClassDef::r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc )
{
	return( CResourceDef::r_WriteVal( pszKey, sVal, pSrc ));
}

bool CRaceClassDef::r_LoadVal( CScript &s )
{
	return( CResourceDef::r_LoadVal( s ));
}
