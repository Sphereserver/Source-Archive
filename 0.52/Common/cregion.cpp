//
// Cregion.cpp
// Copyright Menace Software (www.menasoft.com).
// Common for client and server.
//

#ifdef GRAY_SVR
#include "../graysvr/graysvr.h"
#elif defined(GRAY_MAP)
#include "../graymap/graymap.h"
#else
#include "graycom.h"
#include "graymul.h"
#include "cregion.h"
#endif

const TCHAR * Dirs[DIR_QTY+1] =
{
	_TEXT("North"),
	_TEXT("North East"),
	_TEXT("East"),
	_TEXT("South East"),
	_TEXT("South"),
	_TEXT("South West"),
	_TEXT("West"),
	_TEXT("North West"),
	_TEXT("Here"),
};

//////////////////////////////////////////////////////////////////
// -CGRect

#if 0
int CGRect::Read( TCHAR * pszBuffer )
{
	const TCHAR * pSeps = _TEXT(" ,\t\n");
	TCHAR *pToken = strtok( pszBuffer, pSeps );
	for ( int i=0; i<4 && pToken != NULL; i++ )
	{
		int iVal = atoi( pToken );
		switch ( i )
		{
		case 0: m_left = iVal; break;
		case 1: m_top = iVal; break;
		case 2: m_right = iVal; break;
		case 3: m_bottom = iVal; break;
		}
		pToken = strtok( NULL, pSeps );
	}
	NormalizeRect();
	return( i );
}
#endif

int CGRect::Read( const TCHAR * pVal )
{
	// parse reading the rectangle
	TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
	strcpy( szTemp, pVal );
	TCHAR * ppVal[4];
	int i = ParseCmds( szTemp, ppVal, COUNTOF( ppVal ), _TEXT(" ,\t\n"));
	switch (i)
	{
	case 4: m_bottom = atoi(ppVal[3]);
	case 3: m_right = atoi(ppVal[2]);
	case 2: m_top =	atoi(ppVal[1]);
	case 1: m_left = atoi(ppVal[0]);
	}
	NormalizeRect();
	return( i );
}

const TCHAR * CGRect::Write( void ) const
{
	TCHAR * pszTmp = GetTempStr();
	sprintf( pszTmp, _TEXT("%d,%d,%d,%d"),
		m_left, 
		m_top, 
		m_right, 
		m_bottom );
	return( pszTmp );
}

CPointBase CGRect::GetDirCorner( DIR_TYPE dir ) const
{
	// Get the point if a directional corner of the CRectMap.
	CPointBase pt;
	pt.m_z = 0;	// NOTE: remember this is a nonsense value.
	switch ( dir )
	{
	case DIR_N:
		pt.m_x = ( m_left + m_right ) / 2;
		pt.m_y = m_top;
		break;
	case DIR_NE:
		pt.m_x = m_right; 
		pt.m_y = m_top; 
		break;
	case DIR_E:
		pt.m_x = m_right;
		pt.m_y = ( m_top + m_bottom ) / 2;
		break;
	case DIR_SE:
		pt.m_x = m_right; 
		pt.m_y = m_bottom; 
		break;
	case DIR_S:
		pt.m_x = ( m_left + m_right ) / 2;
		pt.m_y = m_bottom;
		break;
	case DIR_SW:
		pt.m_x = m_left; 
		pt.m_y = m_bottom; 
		break;
	case DIR_W: 
		pt.m_x = m_left;
		pt.m_y = ( m_top + m_bottom ) / 2;
		break;
	case DIR_NW:
		pt.m_x = m_left; 
		pt.m_y = m_top; 
		break;
	case DIR_QTY:
		pt = GetCenter();
		break;
	}
	return( pt );
}

//////////////////////////////////////////////////////////////////
// -CRectMap

void CRectMap::NormalizeRect()
{
	CGRect::NormalizeRect();

	if ( m_top < 0 ) m_top = 0;
	if ( m_bottom > UO_SIZE_Y ) m_bottom = UO_SIZE_Y;
	if ( m_left < 0 ) m_left = 0;
	if ( m_right > UO_SIZE_X ) m_right = UO_SIZE_X;
}

///////////////////////////////////////////////////////////////
// -CPointBase

int CPointBase::GetDist3D( const CPointBase pt ) const // Distance between points
{
	// OK, 1 unit of Z is not the same (real life) distance as 1
	// unit of X (or Y)

	// Get the deltas and correct the Z for height first
	int dx = abs(m_x-pt.m_x);
	int dy = abs(m_y-pt.m_y);
	int dist = max( dx, dy );
	int dz = abs(m_z-pt.m_z) / (PLAYER_HEIGHT/2); // Take player height into consideration
	return( max( dist, dz ));
	// Return the real distance return((int) sqrt(dx*dx+dy*dy+dz*dz));
}

DIR_TYPE CPointBase::GetDir( const CPointBase pt, DIR_TYPE DirDefault ) const // Direction to point pt
{
	int dx = (m_x-pt.m_x);
	int dy = (m_y-pt.m_y);

	int ax = abs(dx);
	int ay = abs(dy);

	if ( ay > ax )
	{
		if ( ! ax )
		{
			return(( dy > 0 ) ? DIR_N : DIR_S );
		}
		int slope = ay / ax;
		if ( slope > 2 )
			return(( dy > 0 ) ? DIR_N : DIR_S );
		if ( dx > 0 )	// westish
		{
			return(( dy > 0 ) ? DIR_NW : DIR_SW );
		}
		return(( dy > 0 ) ? DIR_NE : DIR_SE );
	}
	else 
	{
		if ( ! ay )
		{
			if ( ! dx )
				return( DirDefault );	// here ?
			return(( dx > 0 ) ? DIR_W : DIR_E );
		}
		int slope = ax / ay;
		if ( slope > 2 ) 
			return(( dx > 0 ) ? DIR_W : DIR_E );
		if ( dy > 0 )
		{
			return(( dx > 0 ) ? DIR_NW : DIR_NE );
		}
		return(( dx > 0 ) ? DIR_SW : DIR_SE );
	}
}

void CPointBase::CalcPath( CPointBase pSrc, int iSteps )
{
	// Take x steps toward this point.
	int dx = m_x - pSrc.m_x;
	int dy = m_y - pSrc.m_y;
	int iDist = GetDist( pSrc );
	if ( ! iDist )
		return;

	m_x = pSrc.m_x + IMULDIV( iSteps, dx, iDist );
	m_y = pSrc.m_y + IMULDIV( iSteps, dy, iDist );
}

void CPointBase::Move( DIR_TYPE dir, int amount )
{
	// Move a point in a direction.
	switch ( dir )
	{
	case DIR_N:	 m_y -= amount; break;
	case DIR_NE: m_x += amount; m_y -= amount; break;
	case DIR_E:	 m_x += amount; break;
	case DIR_SE: m_x += amount; m_y += amount; break;
	case DIR_S:	 m_y += amount; break;
	case DIR_SW: m_x -= amount; m_y += amount; break;
	case DIR_W:	 m_x -= amount; break;
	case DIR_NW: m_x -= amount; m_y -= amount; break;
	}
}

bool CPointBase::IsInDungeon() const
{
	// ??? in the future make this part of the region info !
	// What part of the maps are filled with dungeons.
	int x1=(m_x-UO_SIZE_X_REAL);
	if ( x1 < 0 )
		return( false );

	x1 /= 256;
	switch ( m_y / 256 )
	{
	case 0:
	case 5:
		return( true );
	case 1:
		if (x1!=0)
			return( true );
		break;
	case 2:
	case 3:
		if (x1<3) 
			return( true );
		break;
	case 4:
	case 6:
		if (x1<1) 
			return( true );
		break;
	case 7:
		if (x1<2) 
			return( true );
		break;
	}
	return( false );
}

const TCHAR * CPointBase::Write( void ) const
{
	TCHAR * pszTmp = GetTempStr();
	sprintf( pszTmp, _TEXT("%d,%d,%d"), m_x, m_y, m_z );
	return( pszTmp );
}

#ifdef GRAY_SVR	// server only.

int CPointBase::Read( TCHAR * pVal )
{
	// parse reading the point. It might be invalid intentionally.
	int iArgs = 0;
	if ( pVal[0] )
	{
		iArgs++;
		m_x = Exp_GetSingle(pVal);
	}
	if ( pVal[0] )
	{
		iArgs++;
		pVal++;
		m_y = Exp_GetSingle(pVal);
	}
	if ( pVal[0] )
	{
		iArgs++;
		pVal++;
		m_z = Exp_GetSingle(pVal);
	}
	return( iArgs );
}

#else

int CPointBase ::Read( TCHAR * pVal )
{
	// parse reading the point
	TCHAR * ppVal[3];
	int iArgs = ParseCmds( pVal, ppVal, COUNTOF( ppVal ));
	switch ( iArgs )
	{
	default:
	case 3: m_z = atoi(ppVal[2]);
	case 2: m_y = atoi(ppVal[1]);
	case 1: m_x = atoi(ppVal[0]);
	case 0: break;
	}
	return( iArgs );
}

#endif

#if defined(GRAY_SVR) || defined(GRAY_MAP)	// server only.

CSector * CPointBase::GetSector() const
{
	// Get the world Sector we are in.
	ASSERT(	m_x >= 0 && m_x < UO_SIZE_X );
	ASSERT(	m_y >= 0 && m_y < UO_SIZE_Y );
	return( & ( g_World.m_Sectors[ (( m_y / SECTOR_SIZE_Y ) * SECTOR_COLS ) + ( m_x / SECTOR_SIZE_X ) ] ));
}

CRegionBase * CPointBase::GetRegion( DWORD dwType ) const
{
	// What region in the current CSector am i in ?
	// We only need to update this every 8 or so steps ?
	// REGION_TYPE_AREA

	if ( ! IsValid()) 
	{
		return NULL;
	}

	CSector * pSector = GetSector();
	ASSERT(pSector);
	CRegionBase * pRegion = STATIC_CAST <CRegionBase*> (pSector->m_Regions.GetHead());
	for ( ; pRegion!=NULL; pRegion = pRegion->GetNext())
	{
		if ( ! pRegion->IsInside( *this )) 
			continue;
		if ( pRegion->IsMatchType( dwType ))
			return( pRegion );
	}

	// Look at the world region if no CSector regions apply.
	pRegion = STATIC_CAST <CRegionBase*> (g_World.m_Regions.GetHead());
	for ( ; pRegion!=NULL; pRegion = pRegion->GetNext())
	{
		if ( ! pRegion->IsInside( *this )) 
			continue;
		if ( pRegion->IsMatchType( dwType ))
			return( pRegion );
	}
	return( NULL );
}

//////////////////////////////////////////////////////////////////
// -CGRegion

bool CGRegion::RealizeRegion()
{
	// Make sure all the rectangles are proper.
	// 1. None are pure children of each other.
	// ? should we Make sure the rectangle is not a dupe ?
	return( ! IsRegionEmpty());
}

//////////////////////////////////////////////////////////////////
// -CRegionBase

bool CRegionBase::RealizeRegion()
{
	// Link the region to the world.
	// RETURN: false = not valid.

	RemoveSelf();
	CPointMap pt;
	if ( IsRegionEmpty())
	{
		if ( ! m_p.IsValid())
			return( false );
		pt = m_p;
	}
	else
	{
		pt = GetDirCorner();
		if ( ! m_p.IsValid())
		{
			m_p = pt;
		}
	}

#if defined(GRAY_SVR) || defined(GRAY_MAP)

	CGObList * pList;
	if ( IsFlag(REGION_FLAG_GLOBALNAME)) 
	{
		// Jail is like this.
globalregion:
		pList = &(g_World.m_Regions);
	}
	else
	{
		// This may overlap multi sectors ?
		// If so then put in the non-sector based.

		static const DIR_TYPE DirCorner[] = 
		{ 
			DIR_NE,
			DIR_SE,
			DIR_SW,
			DIR_NW,
		};
	
		// Check to see if it's partly in other sectors.
		CSector * pSector = pt.GetSector();

		for ( int i=0; i<GetRegionRectCount(); i++ )
		{
			for ( int j=0; j<COUNTOF(DirCorner); j++ )
			{
				CPointMap pt = GetRegionRect(i).GetDirCorner( DirCorner[j] );
				if ( ! pt.IsValid() || pSector != pt.GetSector())
				{
					// Must b checked on the world level then.
					goto globalregion;
				}
			}
		}

		// Was all in the same CSector.
		pList = &(pSector->m_Regions);
	}

	// Add it to the list. But make sure it is after dynamics.

	CRegionBase * pRegion = STATIC_CAST <CRegionBase*>( pList->GetHead());
	while (true)
	{
		if (!pRegion)
		{
			pList->InsertTail( this );
			break;
		}
		if ( ! pRegion->IsMatchType(REGION_TYPE_MULTI))
		{
			pList->InsertBefore( this, pRegion );
			break;
		}
		pRegion = pRegion->GetNext();
	}
#endif

	return( true );
}

bool CRegionBase::CheckRealized()
{
	// Make sure the last one got filed correctly.
	if ( GetParent()) 
		return( true );
	DEBUG_ERR(( _TEXT("CRegionBase:Bad Region '%s'\n"), GetName() ));
	delete this;
	return( false );
}

bool CRegionBase::AddRegionRect( const CRectMap & rect )
{
	// Add an included rectangle to this region.
	if ( ! rect.IsValid())
		return( false );
	bool fEmpty = IsRegionEmpty();
	CGRegion::AddRegionRect( rect );
	if ( fEmpty )
	{
		RealizeRegion();
	}
	return( true );
}

void CRegionBase::SetName( const TCHAR * pszName )
{
#if defined(GRAY_SVR)
	if ( pszName == NULL || pszName[0] == '%' )
	{
		m_sName = g_Serv.GetName();
	}
	else
#endif
	{
		m_sName = pszName;
	}
}

const TCHAR * CRegionBase::sm_KeyTable[] =	// static (Sorted)
{
	_TEXT("ANNOUNCE"),
	_TEXT("BUILDABLE"),
	_TEXT("CLASS"),
	_TEXT("COLDCHANCE"),
	_TEXT("FLAGS"),
	_TEXT("GATE"),		
	_TEXT("GLOBAL"),
	_TEXT("GUARDED"),
	_TEXT("MAGIC"),
	_TEXT("MAGICDAMAGE"),
	_TEXT("MARK"),		// recall in teleport as well.
	_TEXT("NAME"),
	_TEXT("NOBUILD"),	
	_TEXT("NODECAY"),
	_TEXT("P"),
	_TEXT("RAINCHANCE"),
	_TEXT("RECALL"),	// recall out
	_TEXT("RECALLIN"),
	_TEXT("RECALLOUT"),
	_TEXT("RECT"),
	_TEXT("SAFE"),
	_TEXT("TYPE"),
	_TEXT("UNDERGROUND"),
};

bool CRegionBase::r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc )
{
	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0: // "ANNOUNCE"
		sVal.FormatVal( IsFlag(REGION_FLAG_ANNOUNCE));
		break;
	case 1: // "BUILDABLE"
		sVal.FormatVal( ! IsFlag(REGION_FLAG_NOBUILDING));
		break;
	case 2:	// "CLASS"
		sVal.FormatVal( m_RegionClass );
		break;
	case 3:	// "COLDCHANCE"
		return( false );
	case 4: // "FLAGS",
		sVal.FormatHex( GetFlags() );
		break;
	case 5: // "GATE",
		sVal.FormatVal( ! IsFlag(REGION_ANTIMAGIC_GATE));
		break;
	case 6: // "GLOBAL"
		sVal.FormatVal( IsFlag(REGION_FLAG_GLOBALNAME));
		break;
	case 7: // "GUARDED",
		sVal.FormatVal( IsFlag(REGION_FLAG_GUARDED));
		break;
	case 8: // "MAGIC",
		sVal.FormatVal( ! IsFlag(REGION_ANTIMAGIC_ALL));
		break;
	case 9: // "MAGICDAMAGE",
		sVal.FormatVal( ! IsFlag(REGION_ANTIMAGIC_DAMAGE));
		break;
	case 10: // "MARK",
recallin:
		sVal.FormatVal( ! IsFlag(REGION_ANTIMAGIC_RECALL_IN));
		break;
	case 11: //"NAME",
		sVal = GetName();
		break;
	case 12: // "NOBUILD"
		sVal.FormatVal( IsFlag(REGION_FLAG_NOBUILDING));
		break;
	case 13: // "NODECAY"
		sVal.FormatVal( IsFlag(REGION_FLAG_NODECAY));
		break;
	case 14: //"P",
		sVal = m_p.Write();
		break;
	case 15: // "RAINCHANCE
		return( false );
	case 16: // "RECALL",
recallout:
		sVal.FormatVal( ! IsFlag(REGION_ANTIMAGIC_RECALL_OUT));
		break;
	case 17: // "RECALLIN",
		goto recallin;
	case 18: //"RECT",
		return( false );
	case 19: // "RECALLOUT"
		goto recallout;
	case 20: // "SAFE"
		sVal.FormatVal( IsFlag(REGION_FLAG_SAFE));
		break;
	case 21: // "TYPE"
		sVal.FormatHex( m_RegionType );
		break;
	case 22: // "UNDERGROUND"
		sVal.FormatVal( IsFlag(REGION_FLAG_UNDERGROUND));
		break;
	default: 
		return( CScriptObj::r_WriteVal( pKey, sVal, pSrc ));
	}

	return( true );
}

bool CRegionBase::r_LoadVal( CScript & s )
{
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0: // "ANNOUNCE"
		ModFlags( REGION_FLAG_ANNOUNCE, s.GetArgVal());
		break;
	case 1: // "BUILDABLE"
		ModFlags( REGION_FLAG_NOBUILDING, ! s.GetArgVal());
		break;
	case 2:	// "CLASS"
		m_RegionClass = (RegionClass_Type) s.GetArgVal();
		break;
#ifdef GRAY_SRV
	case 3: // "COLDCHANCE",
		SendSectorsVerb( s.GetKey(), s.GetArgStr(), &g_Serv );
		break;
#endif
	case 4: // "FLAGS",
		m_Flags = ( s.GetArgVal() &~REGION_FLAG_SHIP ) | ( m_Flags & REGION_FLAG_SHIP );
		break;
	case 5: // "GATE",
		ModFlags( REGION_ANTIMAGIC_GATE, ! s.GetArgVal());
		break;
	case 6: // "GLOBAL"
		ModFlags( REGION_FLAG_GLOBALNAME, s.GetArgVal());
		break;
	case 7: // "GUARDED",
		ModFlags( REGION_FLAG_GUARDED, s.GetArgVal());
		break;
	case 8: // "MAGIC",
		ModFlags( REGION_ANTIMAGIC_ALL, ! s.GetArgVal());
		break;
	case 9: // "MAGICDAMAGE",
		ModFlags( REGION_ANTIMAGIC_DAMAGE, ! s.GetArgVal());
		break;
	case 10: // "MARK",
recallin:
		ModFlags( REGION_ANTIMAGIC_RECALL_IN, ! s.GetArgVal());
		break;
	case 11: //"NAME",
		SetName( s.GetArgStr());
		break;
	case 12: // "NODECAY"
		ModFlags( REGION_FLAG_NODECAY, s.GetArgVal());
		break;
	case 13: // "NOBUILD"
		ModFlags( REGION_FLAG_NOBUILDING, s.GetArgVal());
		break;
	case 14: //"P",
		m_p.Read(s.GetArgStr());
		if ( IsRegionEmpty())
		{
			RealizeRegion();
		}
		break;
#ifdef GRAY_SRV
	case 15: // "RAINCHANCE"
		SendSectorsVerb( s.GetKey(), s.GetArgStr(), &g_Serv );
		break;
#endif
	case 16: // "RECALL",
recallout:
		ModFlags( REGION_ANTIMAGIC_RECALL_OUT, ! s.GetArgVal());
		break;
	case 17: // "RECALLIN",
		goto recallin;
	case 18: // "RECALLOUT"
		goto recallout;
	case 19: //"RECT",
		{
			CRectMap rect;
			rect.Read(s.GetArgStr());
			return( AddRegionRect( rect ));
		}
	case 20: // "SAFE"
		ModFlags( REGION_FLAG_SAFE, s.GetArgVal());
		break;
	case 21: // "TYPE"
		return( false );
	case 22: // "UNDERGROUND"
		ModFlags( REGION_FLAG_UNDERGROUND, s.GetArgVal());
		break;
	default:
		return( false );
	}
	return( true );
}

bool CRegionBase::CheckAntiMagic( SPELL_TYPE spell ) const
{
	// return: true = blocked.
	if ( ! IsFlag( REGION_FLAG_SHIP |
		REGION_ANTIMAGIC_ALL | 
		REGION_ANTIMAGIC_RECALL_IN | 
		REGION_ANTIMAGIC_RECALL_OUT |
		REGION_ANTIMAGIC_GATE |
		REGION_ANTIMAGIC_TELEPORT |
		REGION_ANTIMAGIC_DAMAGE ))	// no effects on magic anyhow.
		return( false );

	if ( IsFlag( REGION_ANTIMAGIC_ALL ))
		return( true );

	if ( IsFlag( REGION_ANTIMAGIC_RECALL_IN | REGION_FLAG_SHIP ))
	{
		if ( spell == SPELL_Mark || spell == SPELL_Gate_Travel )
			return( true );
	}
	if ( IsFlag( REGION_ANTIMAGIC_RECALL_OUT ))
	{
		if ( spell == SPELL_Recall || spell == SPELL_Gate_Travel || spell == SPELL_Mark )
			return( true );
	}
	if ( IsFlag( REGION_ANTIMAGIC_GATE ))
	{
		if ( spell == SPELL_Gate_Travel )
			return( true );
	}
	if ( IsFlag( REGION_ANTIMAGIC_TELEPORT ))
	{
		if ( spell == SPELL_Teleport ) 
			return( true );
	}
#ifdef GRAY_SVR
	if ( IsFlag( REGION_ANTIMAGIC_DAMAGE ))
	{
		if ( g_Serv.m_SpellDefs[spell]->IsSpellType( SPELLFLAG_HARM ))
			return( true );
	}
#endif
	return( false );
}

bool CRegionBase::r_Verb( CScript & s, CTextConsole * pSrc ) // Execute command from script
{
#ifdef GRAY_SVR
	if ( s.IsKey( _TEXT("SECTORS") ))
	{
		TCHAR * pszArg;
		Parse( s.GetArgStr(), &pszArg );
		return SendSectorsVerb( s.GetArgStr(), pszArg, pSrc );
	}
	if ( s.IsKey( _TEXT("ALLCLIENTS")))
	{
		for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
		{
			CChar * pChar = pClient->GetChar();
			if ( pChar == NULL ) continue;
			if ( pChar->m_pArea != this ) continue;
			pChar->r_Verb( CScript( s.GetArgStr()), pSrc );
		}
	}
#endif
	return( CScriptObj::r_Verb( s, pSrc ));
}

#endif

#ifdef GRAY_SVR	// server only.

bool CRegionBase::SendSectorsVerb( const TCHAR * pszVerb, const TCHAR * pszArgs, CTextConsole * pSrc )
{
	// Send a command to all the CSectors in this region.

	bool fRet = false;
	for ( int i=0; i<SECTOR_QTY; i++ )
	{
		// Does the rect overlap ?
		if ( IsOverlapped( g_World.m_Sectors[i].GetRect()))
		{
			fRet |= g_World.m_Sectors[i].r_Verb( CScript( pszVerb, pszArgs ), pSrc );
		}
	}
	return( fRet );
}

bool CRegionWorld::OnRegionTrigger( CTextConsole * pSrc, bool fEnter, bool fPeriodic )
{
	// RETURN: true = halt prodcessing (don't allow in this region
	// [REGION 0]
	// ONTRIGGER=ENTER
	// ONTRIGGER=EXIT

	int id;
	const TCHAR * pszAction;
	if ( fPeriodic ) 
	{
		id = m_TrigPeriodic;
		if ( fEnter && m_TrigPeriodic == m_TrigEntry )
			return( true );	// don't dupe this,
		pszAction = (fEnter)? _TEXT("ENTER") : _TEXT("PERIODIC");
	}
	else
	{
		id = m_TrigEntry;
		pszAction = (fEnter)? _TEXT("ENTER") : _TEXT("EXIT");
	}
	if ( ! id ) 
		return( false );

	CGString sSection;
	sSection.Format( _TEXT("REGION %d"), id );

	CScriptLock s;
	if ( ! g_Serv.ScriptLock( s, SCPFILE_TRIG_2, sSection ))
		return( false );

	return( CScriptObj::OnTriggerScript( s, pszAction, pSrc ) == TRIGRET_RET_TRUE );
}

#endif

/////////////////////////////////////////////////////////////////////////////////////
// -CRegionWorld

#if defined(GRAY_SVR) || defined(GRAY_MAP)

const TCHAR * CRegionWorld::sm_KeyTable[] =	// static
{
	_TEXT("ANNOUNCEMENT"),
	_TEXT("GUARDOWNER"),
	_TEXT("JAILBANISH"),
	_TEXT("JAILBREAK"),
	_TEXT("JAILPOINT"),
	_TEXT("JAILRELEASE"),
	_TEXT("LOCAL"),
	_TEXT("TRIGGER"),
	_TEXT("TRIGPERIODIC"),
	NULL,
};

bool CRegionWorld::r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc )
{
	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )-1 ))
	{
	case 0:	// "ANNOUNCEMENT"
		if ( ! IsFlag(REGION_FLAG_ANNOUNCE))
			goto takedefault;
		sVal = m_sGuardAnnounce;
		break;
	case 1:	// "GUARDOWNER"
		if ( ! IsFlag(REGION_FLAG_GUARDED) || IsFlag(REGION_FLAG_ANNOUNCE))
			goto takedefault;
		sVal = m_sGuardAnnounce;
		break;
	case 2: // "LOCAL" = for the speech system.
		sVal = m_sLocal;
		break;
	case 3:	// "TRIGGER"
		if ( ! m_TrigEntry ) 
			goto takedefault;
		sVal.FormatVal( m_TrigEntry );
		break;
	case 4:	// "TRIGPERIODIC"
		if ( ! m_TrigPeriodic ) 
		{
takedefault:
			sVal = "";
			break;
		}
		sVal.FormatVal( m_TrigPeriodic );
		break;
	default:
		return( CRegionBase::r_WriteVal( pKey, sVal, pSrc ));
	}
	return( true );
}

bool CRegionWorld::r_LoadVal( CScript &s )
{
	// Load the values for the region from script.
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )-1 ))
	{
	case 0: // "ANNOUNCEMENT"
		ModFlags( REGION_FLAG_ANNOUNCE, true );
		m_sGuardAnnounce = s.GetArgStr();
		break;
	case 1: // "GUARDOWNER",
		ModFlags( REGION_FLAG_GUARDED, true );
		m_sGuardAnnounce = s.GetArgStr();
		break;
	case 2: // "LOCAL" = for the speech system.
		m_sLocal = s.GetArgStr();
		break;
	case 3:	// "TRIGGER" = m_iTriggerID
		m_TrigEntry = s.GetArgVal();
		break;
	case 4: // "TRIGPERIODIC",
		m_TrigPeriodic = s.GetArgVal();
		break;
	default:
		return( CRegionBase::r_LoadVal( s ));
	}
	return( true );
}

void CRegionBase::r_Write( CScript &s )
{
	if ( IsMatchType(REGION_TYPE_ROOM))
	{
		s.WriteSection( "ROOM %s", GetName());
	}
	else
	{
		ASSERT( IsMatchType(REGION_TYPE_AREA));
		s.WriteSection( "AREA %s", GetName());
	}
	if ( m_RegionClass )
	{
		s.WriteKeyVal( _TEXT("CLASS"), m_RegionClass);
	}
	if ( GetFlags())
	{
		s.WriteKeyHex( _TEXT("FLAGS"), GetFlags());
	}
	for ( int i=0; i<GetRegionRectCount(); i++ )
	{
		s.WriteKey( _TEXT("RECT"), GetRegionRect(i).Write());	
	}
	if ( m_p.IsValid())
	{
		s.WriteKey( _TEXT("P"), m_p.Write());	
	}
}

void CRegionWorld::r_Write( CScript &s )
{
	CRegionBase::r_Write(s);
	if ( IsFlag(REGION_FLAG_ANNOUNCE))
	{
		s.WriteKey( _TEXT("ANNOUNCEMENT"), m_sGuardAnnounce );
	}
	else if ( IsFlag(REGION_FLAG_GUARDED))
	{
		s.WriteKey( _TEXT("GUARDOWNER"), m_sGuardAnnounce );
	}
	if ( ! m_sLocal.IsEmpty())
	{
		s.WriteKey( _TEXT("LOCAL"), m_sLocal );
	}
	if ( m_TrigEntry )
	{
		s.WriteKeyVal( _TEXT("TRIGGER"), m_TrigEntry );
	}
	if ( m_TrigPeriodic )
	{
		s.WriteKeyVal( _TEXT("TRIGPERIODIC"), m_TrigPeriodic );
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// -CRegionJail

const TCHAR * CRegionJail::sm_KeyTable[] =	// static
{
	_TEXT("JAILBANISH"),
	_TEXT("JAILBREAK"),
	_TEXT("JAILPOINT"),
	_TEXT("JAILRELEASE"),
	NULL,
};

bool CRegionJail::r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc )
{
	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )-1 ))
	{
	case 0: // "JAILBANISH"
		sVal = m_ptJailBanish.Write();
		break;
	case 1: // "JAILBREAK"
		sVal = m_ptJailBreak.Write();
		break;
	case 2: // "JAILPOINT"
		sVal = m_ptJailPoint.Write();
		break;
	case 3: // "JAILRELEASE"
		sVal = m_ptJailRelease.Write();
		break;
	default:
		return( CRegionWorld::r_WriteVal( pKey, sVal, pSrc ));
	}
	return( true );
}

bool CRegionJail::r_LoadVal( CScript &s )
{
	// Load the values for the region from script.
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )-1 ))
	{
	case 0: // "JAILBANISH"
		m_ptJailBanish.Read(s.GetArgStr());
		break;
	case 1: // "JAILBREAK"
		m_ptJailBreak.Read(s.GetArgStr());
		break;
	case 2: // "JAILPOINT"
		m_ptJailPoint.Read(s.GetArgStr());
		break;
	case 3: // "JAILRELEASE"
		m_ptJailRelease.Read(s.GetArgStr());
		break;
	default:
		return( CRegionWorld::r_LoadVal( s ));
	}
	return( true );
}

#endif
