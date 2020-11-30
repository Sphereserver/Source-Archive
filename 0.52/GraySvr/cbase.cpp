//
// CBase.cpp
// Copyright Menace Software (www.menasoft.com).
// base classes.
//
#include "graysvr.h"	// predef header.

extern bool World_fDeleteCycle;

#if defined(_DEBUG) && defined(GRAY_SVR)	//
bool CObjBaseTemplate::IsValidContainer() const	// am i really in the container i say i am ?
{
	if ( IsDisconnected())	// must also be at top level !
	{
		// There are several places a disconnected item can be.
		if ( GetParent() == NULL ) 
			return( true );
		if ( IsChar())
		{
			return( dynamic_cast <CCharsDisconnectList*>( GetParent()) != NULL );
		}
		else
		{
			return( dynamic_cast <CItemsDisconnectList*>( GetParent()) != NULL );
		}
	}
	if ( IsItem())
	{
		if ( dynamic_cast <const CItem*>(this) == NULL )
			return( false );
	}
	else
	{
		if ( dynamic_cast <const CChar*>(this) == NULL )
			return( false );
	}
	if ( IsTopLevel())
	{
		if ( IsChar())
		{
			return( dynamic_cast <CCharsActiveList*>( GetParent()) != NULL );
		}
		else
		{
			return( dynamic_cast <CItemsList*>( GetParent()) != NULL );
		}
	}
	if ( IsEquipped())
	{
		if ( IsChar())
		{
			return( false );
		}
		else
		{
			return( dynamic_cast <CChar*>( GetParent()) != NULL );
		}
	}
	if ( IsInContainer())
	{
		if ( IsChar())
		{
			return( false );
		}
		else
		{
			return( dynamic_cast <CItemContainer*>( GetParent()) != NULL );
		}
	}
	return( false );	// no flags !?
}
#endif

///////////////////////////////////////////////////////////////
// -CBaseBase

const TCHAR * CBaseBase::sm_KeyTable[] =
{
	"ARMOR",
	"ATT",
	"BASEID",
	"CAN",
	"DAM",
	"DISPID",
	"INSTANCES",
	"NAME",
};

bool CBaseBase::r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pChar )
{
	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0:  // "ARMOR"
armor:
		sVal.Format( "%d-%d", m_attackBase, m_attackBase+m_attackRange );
		return( true );
	case 1:	// "ATT" from script
		goto armor;
	case 2: // "BASEID"
		sVal.FormatHex( GetBaseID());
		return( true );
	case 3:	// "CAN"
		sVal.FormatHex( m_Can );
		return( true );
	case 4:  // "DAM",
		goto armor;
	case 5:	// "DISPID"
		sVal.FormatHex( m_wDispIndex );
		return( true );
	case 6: // "INSTANCES"
		sVal.FormatVal( GetInstances());
		return( true );
	case 7: // "NAME"
		sVal = m_sName;
		return( true );
	}
	return( false );
}

bool CBaseBase::r_LoadVal( CScript & s )
{
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0:  // "ARMOR"
armor:
		m_attackBase = Exp_GetSingle( s.m_pArg );
		if ( s.m_pArg[0] )
		{
			s.m_pArg ++;
			m_attackRange = Exp_GetSingle( s.m_pArg ) - m_attackBase;
		}
		else
		{
			m_attackRange = 0;
		}
		return( true );
	case 1:	// "ATT" from script
		goto armor;
	case 2: // "BASEID"
		return( false );
	case 3:	// "CAN"
		m_Can = s.GetArgHex() | ( m_Can & ( CAN_C_INDOORS|CAN_C_EQUIP|CAN_C_USEHANDS|CAN_C_NONHUMANOID|CAN_C_MOVE_ITEMS ));
		return( true );
	case 4:  // "DAM",
		goto armor;
	case 5: // "DISPID"
	case 6: // "INSTANCES"
		return( false );
	case 7: // "NAME"
		SetTypeName( s.GetArgStr());
		return( true );
	}
	return( CScriptObj::r_LoadVal(s));
}

void CBaseBase::DupeCopy( const CBaseBase * pBase )
{
	m_wDispIndex = pBase->m_wDispIndex;
	m_lInstances = pBase->m_lInstances;
	m_ScriptLink = pBase->m_ScriptLink;
	m_sName = pBase->m_sName;
	m_Height = pBase->m_Height;
	m_Can = pBase->m_Can;
	m_attackBase = pBase->m_attackBase;
	m_attackRange = pBase->m_attackRange;
}

CScriptLink * CBaseBase::FindScpPrevLoad( CScript * pScript ) const
{
	// Find the previously loaded CBaseBase (if there is one) in this pScript file.
	ASSERT( ! IsLoaded());

	CBaseBaseArray * pList = GetBaseArray();
	int iCompare;
	int iPrevKnown = pList->FindKeyNear( GetBaseID(), iCompare );
	if ( iCompare < 0 )
	{
		iPrevKnown--;
	}

	CCharBase * pPrevLoad = NULL;
	for ( int i=iPrevKnown; i>=0; i-- )
	{
		CBaseBase * pBaseTmp = dynamic_cast <CBaseBase *> ( pList->GetAt(i) );
		if ( pBaseTmp == NULL ) // just a CBaseStub ?
			continue;
		if ( ! pBaseTmp->IsLoaded())
			continue;
		DEBUG_CHECK( pBaseTmp->GetBaseID() < GetBaseID());	// not useful
		if ( pBaseTmp->m_ScriptLink.GetLinkScript() != pScript )
			continue;
		return( & pBaseTmp->m_ScriptLink );
	}
	return( NULL );
}

bool CBaseBase::ScriptLockBase( CScriptLock &s )
{
	// Find the defintion of this item in the scripts.
	// Possibly in the 2.SCP sections.
	//
	// ARGS: pPrev = the last known previous offset (in base script).
	//

	if ( m_ScriptLink.IsLinked())	// has already been loaded previously.
	{
		if ( m_ScriptLink.OpenLinkLock( s ))
		{
			return( true );
		}
		DEBUG_ERR(( "SeekScript '%s' offset=0%x base=0%x FAILED\n", s.GetFilePath(), m_ScriptLink.GetLinkOffset(), GetBaseID()));
		m_ScriptLink.InitLink();
	}

	CGString sSection;
	sSection.Format( "%04X", GetBaseID() );

	// Go looking for it.

	SCPFILE_TYPE nfile = GetScpFileIndex(true);
	CScript * pScript = g_Serv.ScriptLock( s, nfile );
	if ( pScript != NULL )
	{
		if ( s.FindSection( sSection, OF_NONCRIT|OF_SORTED, FindScpPrevLoad( pScript )))
		{
			m_ScriptLink.SetLink( pScript, s );
			return( true );
		}
	}

	// Look in the Base SCP file.
	nfile = GetScpFileIndex(false);
	pScript = g_Serv.ScriptLock( s, nfile );
	if ( pScript != NULL )
	{
		if ( s.FindSection( sSection, OF_SORTED, FindScpPrevLoad( pScript )))
		{
			m_ScriptLink.SetLink( pScript, s );
			return( true );
		}
	}

	// didn't find it.
	return( false );
}

#if 0

CBaseBase * CBaseBase::FindName( const TCHAR * pszName ) // static
{
	// NOT USED !
	// Try to find an item by name. (only those that are loaded !)

	for ( int i=0; i<g_Serv.m_ItemBase.GetCount(); i++ )
	{
		if ( ! g_Serv.m_ItemBase[i]->m_sName.CompareNoCase( pszName ))
			return( g_Serv.m_ItemBase[i] );
	}

	return( NULL );
}

#endif

/////////////////////////////////////////////////////////////////
// -CObjBase stuff
// Either a player, npc or item.

CObjBase::CObjBase( bool fItem )
{
	sm_iCount ++;
	m_color=COLOR_DEFAULT;
	m_timeout=0;

	if ( g_Serv.IsLoading())
	{
		// Don't do this yet if we are loading. UID will be set later.
		// Just say if this is an item or not.
		CObjBaseTemplate::SetPrivateUID( fItem ? ( UID_ITEM | UID_MASK ) : ( UID_MASK ));
	}
	else
	{
		// Find a free UID slot for this.
		SetPrivateUID( 0, fItem );
		ASSERT(IsValidUID());
	}
	SetContainerFlags(UID_SPEC);	// it is no place for now
}

CObjBase::~CObjBase()
{
	sm_iCount --;
	ASSERT( IsDisconnected());
	if ( GetParent() != NULL )
	{
		DEBUG_CHECK( GetParent() == NULL );
	}

	// free up the UID slot.
	SetPrivateUID( UID_UNUSED, false );
}

bool CObjBase::IsContainer() const
{
	// Simple test if object is a container.
	return( dynamic_cast <const CContainer*>(this) != NULL );
}

int CObjBase::IsWeird() const
{
	int iResultCode = CObjBaseTemplate::IsWeird();
	if ( iResultCode )
	{
		return( iResultCode );
	}
	if ( ! g_Serv.IsLoading())
	{
		if ( GetUID().ObjFind() != this )	// make sure it's linked both ways correctly.
		{
			return( 0x3201 );
		}
	}
	return( 0 );
}

void CObjBase::SetPrivateUID( DWORD dwIndex, bool fItem )
{
	// Move the serial number,
	// This is possibly dangerous if conflict arrises.

	dwIndex &= UID_MASK;	// Make sure no flags in here.
	if ( IsValidUID())	// i already have a uid ?
	{
		if ( ! dwIndex )
			return;	// The point was just to make sure it was located.
		// remove the old UID.
		g_World.FreeUID( ((DWORD)GetUID()) & UID_MASK );
	}

	if ( dwIndex != UID_MASK )	// just wanted to remove it
	{
		dwIndex = g_World.AllocUID( dwIndex, this );
	}

	if ( fItem ) dwIndex |= UID_ITEM;
	CObjBaseTemplate::SetPrivateUID( dwIndex ); 
}

bool CObjBase::SetNamePool( const TCHAR * pszName )
{
	ASSERT(pszName);

	// Parse out the name from the name pool ?
	if ( pszName[0] == '#' )
	{
		pszName ++;
		TCHAR szTmp[ MAX_SCRIPT_LINE_LEN ];
		strcpy( szTmp, pszName );

		TCHAR * ppTitles[2];
		ParseCmds( szTmp, ppTitles, COUNTOF(ppTitles));

		CScriptLock s;
		if ( g_Serv.ScriptLock( s, SCPFILE_NAME_2, ppTitles[0] ) == NULL )
		{
failout:
			DEBUG_ERR(( "Name pool '%s' could not be found\n", ppTitles[0] ));
			CObjBase::SetName( ppTitles[0] );
			return false;
		}

		// Pick a random name.
		if ( ! s.ReadKey())
			goto failout;
		int iCount = GetRandVal( atoi( s.GetKey())) + 1;
		while ( iCount-- )
		{
			if ( ! s.ReadKey())
				goto failout;
		}

		return CObjBaseTemplate::SetName( s.GetKey());
	}

	// NOTE: Name must be <= MAX_NAME_SIZE
	TCHAR szTmp[ MAX_NAME_SIZE + 1 ];
	int len = strlen( pszName );
	if ( len >= MAX_NAME_SIZE )
	{
		strncpy( szTmp, pszName, MAX_NAME_SIZE );
		szTmp[ MAX_NAME_SIZE ] = '\0';
		pszName = szTmp;
	}

	// Can't be a dupe name with type ?
	const TCHAR * pszTypeName = GetBase()->GetTypeName();
	if ( ! strcmpi( pszTypeName, pszName ))
		pszName = "";

	return CObjBaseTemplate::SetName( pszName );
}

void CObjBase::WriteTry( CScript & s )
{
	if ( g_Serv.m_fSecure )	// enable the try code.
	{
		try
		{
			if ( ! g_Serv.m_fSaveGarbageCollect )
			{
				if ( g_World.FixObj( this )) 
					return;
			}
			r_Write(s);
		}
		catch (...)	// catch all
		{
			g_Log.Event( LOGL_CRIT|LOGM_SAVE, "Write Object Exception!\n" );
		}
	}
	else
	{
		if ( ! g_Serv.m_fSaveGarbageCollect )
		{
			if ( g_World.FixObj( this )) 
				return;
		}
		r_Write(s);
	}
}

void CObjBase::SetTimeout( int iDelayInTicks )
{
	// Set delay in TICK_PER_SEC of a sec.
	// -1 = never.

#if 0 // def _DEBUG
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		// DEBUG_MSG(( "SetTimeout(%d) for 0%lx\n", iDelay, GetUID()));
		//if ( iDelay > 100*TICK_PER_SEC )
		//{
		//	DEBUG_MSG(( "SetTimeout(%d) for 0%lx LARGE\n", iDelay, GetUID()));
		//}
	}
#endif

	if ( iDelayInTicks < 0 )
	{
		m_timeout = 0;
	}
	else
	{
		m_timeout = g_World.GetTime() + iDelayInTicks;
	}
}

void CObjBase::Sound( SOUND_TYPE id, int iOnce ) const // Play sound effect for player
{
	// play for everyone near by.

	if ( id <= 0 ) 
		return;

	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( ! pClient->CanSee( this )) continue;
		pClient->addSound( id, this, iOnce );
	}
}

void CObjBase::Effect( EFFECT_TYPE motion, ITEMID_TYPE id, const CObjBase * pSource, BYTE speed, BYTE loop, bool explode ) const
{
	// show for everyone near by.

	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( ! pClient->CanSee( this ))
			continue;
		pClient->addEffect( motion, id, this, pSource, speed, loop, explode );
	}
}

void CObjBase::Speak( const TCHAR * pText, COLOR_TYPE color, TALKMODE_TYPE mode, FONT_TYPE font )
{
	g_World.Speak( this, pText, color, mode, font );
}

void CObjBase::SpeakUTF8( const TCHAR * pText, COLOR_TYPE color, TALKMODE_TYPE mode, FONT_TYPE font, const char * pszLanguage )
{
	// convert to UNICODE.
	NCHAR szBuffer[ MAX_TALK_BUFFER ];
	int iLen = CvtSystemToNUNICODE( szBuffer, pText, COUNTOF(szBuffer));
	g_World.SpeakUNICODE( this, szBuffer, color, mode, font, pszLanguage );
}

void CObjBase::SpeakUNICODE( const NCHAR * pText, COLOR_TYPE color, TALKMODE_TYPE mode, FONT_TYPE font, const char * pszLanguage )
{
	g_World.SpeakUNICODE( this, pText, color, mode, font, pszLanguage );
}

void CObjBase::MoveNear( CPointMap pt, int iSteps, WORD wCan )
{
	// Move to nearby this other object.
	// Actually move it within +/- iSteps

	DIR_TYPE dir = (DIR_TYPE) GetRandVal( DIR_QTY );
	for (; iSteps > 0 ; iSteps-- )
	{
		// Move to the right or left?
		CPointBase pTest = pt;	// Save this so we can go back to it if we hit a blocking object.
		pt.Move( dir );
		dir = GetDirTurn( dir, GetRandVal(3)-1 );	// stagger ?
		// Put the item at the correct Z point
		WORD wBlockRet = wCan;
		pt.m_z = g_World.GetHeight( pt, wBlockRet, pt.GetRegion(REGION_TYPE_MULTI));
		if ( wBlockRet &~ wCan )
		{
			// Hit a block, so go back to the previous valid position
			pt = pTest;
			break;	// stopped
		}
	}

	MoveTo( pt );
}

void CObjBase::UpdateCanSee( const CCommand * pCmd, int iLen, CClient * pClientExclude ) const
{
	// Send this update message to everyone who can see this.
	// NOTE: Need not be a top level object. CanSee() will calc that.
	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == pClientExclude )
			continue;
		if ( ! pClient->CanSee( this )) 
			continue;
		pClient->xSendPkt( pCmd, iLen );
	}
}

bool CObjBase::r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc )
{
	if ( ! strnicmp( pszKey, "SECTOR.", 7 ))
	{
		pszKey += 7;
		pRef = GetTopLevelObj()->GetTopSector();
		return( true );
	}
	if ( ! strnicmp( pszKey, "TYPEDEF", 7 ) && pszKey[7] )
	{
		pszKey += 8;
		pRef = GetBase();
		return( true );
	}
	return( CScriptObj::r_GetRef( pszKey, pRef, pSrc ));
}

const TCHAR * CObjBase::sm_KeyTable[] =
{
	"COLOR",
	"DISTANCE",
	"NAME",
	"P",
	"SERIAL",
	"TIMER",
	"UID",
	"Z",
};

bool CObjBase::r_WriteVal( const TCHAR *pKey, CGString &sVal, CTextConsole * pSrc )
{
	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0:	// "COLOR"
		sVal.FormatHex( GetColor()); 
		break;
	case 1: // "DISTANCE" 
		{
			// Distance from source to this.
			CChar * pChar = pSrc->GetChar();
			if ( pChar == NULL ) 
				return( false );
			sVal.FormatVal( GetDist( pChar ));
		}
		break;
	case 2: // "NAME"
		sVal = GetName(); 
		break;
	case 3: // "P"
		sVal = GetUnkPoint().Write(); 
		break;
	case 4:	// "SERIAL"
		goto scp_uid;
	case 5: // "TIMER"
		sVal.FormatVal( GetTimerAdjusted()); 
		break;
	case 6: // "UID"
scp_uid:
		sVal.FormatHex( GetUID()); 
		break;
	case 7: // "Z"
		sVal.FormatVal( GetUnkZ()); 
		break;
	default:
		if ( GetTopLevelObj()->GetTopSector()->r_WriteVal( pKey, sVal, pSrc ))
			return( true );	// SECTOR. ?
		if ( g_World.r_WriteVal( pKey, sVal, pSrc ))
			return( true );	// WORLD. ?
		if ( GetBase()->r_WriteVal( pKey, sVal, pSrc ))
			return( true );	// TYPEDEF. ?
		return(	CScriptObj::r_WriteVal( pKey, sVal, pSrc ));
	}
	return( true );
}

bool CObjBase::r_LoadVal( CScript & s )
{
	// load the basic stuff.

	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0:	// "COLOR"

		if ( ! strcmpi( s.GetArgStr(), "match_hair" ))
		{
			CChar * pChar = dynamic_cast <CChar*>(GetTopLevelObj());
			if ( pChar )
			{
				CItem * pHair = pChar->LayerFind(LAYER_HAIR);
				if ( pHair )
				{
					m_color = pHair->GetColor();
					break;
				}
			}
			m_color = COLOR_GRAY;
			break;
		}
		m_color = (COLOR_TYPE) s.GetArgHex();
		break;
	case 1: // "DISTANCE"
		return( false );
	case 2:	// "NAME"
		SetName( s.GetArgStr());
		break;
	case 3:	// "P"
		return( false );
	case 4: // "SERIAL"
		goto scp_uid;
	case 5:	// "TIMER"
		if ( g_World.m_iSaveVersion < 34 )
		{
			m_timeout = s.GetArgHex();
		}
		else
		{
			SetTimeout( s.GetArgVal() * TICK_PER_SEC );
		}
		break;
	case 6: // "UID"
scp_uid:
		DEBUG_CHECK( IsDisconnected());
		DEBUG_CHECK( g_Serv.IsLoading());
		DEBUG_CHECK( ! IsValidUID());
		SetPrivateUID( s.GetArgHex(), (dynamic_cast <CItem*>(this)) ? true : false );
		if ( GetUID() == (DWORD) 0x038a72 )
		{
			DEBUG_CHECK( ! IsValidUID());
		}
		break;
	default:
		return( CScriptObj::r_LoadVal(s));
	}

	return( true );
}

void CObjBase::r_Write( CScript & s )
{
	s.WriteKeyHex( "SERIAL", GetUID());
	if ( IsIndividualName())
		s.WriteKey( "NAME", GetIndividualName());
	if ( m_color != COLOR_DEFAULT )
		s.WriteKeyHex( "COLOR", GetColor());
	if ( m_timeout )
		s.WriteKeyVal( "TIMER", GetTimerAdjusted());
}

enum OV_TYPE
{
	OV_DAMAGE,
	OV_DCLICK,
	OV_EDIT,
	OV_EFFECT,
	OV_FIX,
	OV_FIXWEIGHT,
	OV_FLIP,
	OV_FOLLOW,
	OV_FOLLOWED,
	OV_GUMPDLG,
	OV_INPDLG,
	OV_M,
	OV_MESSAGE,
	OV_MOVE,
	OV_MOVETO,
	OV_MSG,
	OV_NUDGEDOWN,
	OV_NUDGEUP,
	OV_P,	// protect this from use.
	OV_REMOVE,
	OV_SAY,
	OV_SAYU,
	OV_SFX,
	OV_SOUND,
	OV_SPEAK,
	OV_SPEAKU,
	OV_SPELLEFFECT,
	OV_TARGET,
	OV_TOME,
	OV_TRY,
	OV_TRYP,
	OV_UPDATE,
	OV_USEITEM,
	OV_Z,
};

bool CObjBase::r_Verb( CScript & s, CTextConsole * pSrc ) // Execute command from script
{
	static const TCHAR * table[] =
	{
		"DAMAGE",
		"DCLICK",
		"EDIT",
		"EFFECT",
		"FIX",
		"FIXWEIGHT",
		"FLIP",
		"FOLLOW",
		"FOLLOWED",	// get rid of this
		"GUMPDLG",
		"INPDLG",
		"M",
		"MESSAGE",
		"MOVE",
		"MOVETO",
		"MSG",
		"NUDGEDOWN",
		"NUDGEUP",
		"P",	// protect this from use.
		"REMOVE",
		"SAY",
		"SAYU",
		"SFX",
		"SOUND",
		"SPEAK",
		"SPEAKU",
		"SPELLEFFECT",
		"TARGET",
		"TOME",
		"TRY",
		"TRYP",
		"UPDATE",
		"USEITEM",
		"Z",
	};

	ASSERT(pSrc);
	CChar * pCharSrc = pSrc->GetChar();
	CClient * pClientSrc = (pCharSrc && pCharSrc->IsClient()) ? (pCharSrc->GetClient()) : NULL ;

	switch ( FindTableSorted( s.GetKey(), table, COUNTOF(table)))
	{
	case OV_DAMAGE:	//	"Amount,SourceFlags,SourceCharUid" = do me some damage.
		{
			int piCmd[3];
			int iArgQty = ParseCmds( s.GetArgStr(), piCmd, COUNTOF(piCmd));
			if ( iArgQty < 1 ) 
				return( false );
			if ( iArgQty > 2 )	// Give it a new source char UID
			{
				CObjBaseTemplate * pObj = CObjUID( piCmd[2] ).ObjFind();
				if ( pObj )
				{
					pObj = pObj->GetTopLevelObj();
				}
				pCharSrc = dynamic_cast <CChar*>(pObj);
			}
			OnTakeDamage( piCmd[0], 
				pCharSrc,
				( iArgQty > 1 ) ? piCmd[1] : ( DAMAGE_HIT | DAMAGE_GENERAL ));
		}
		break;

	case OV_DCLICK:
		if ( ! pCharSrc ) 
			return( false );
		return pCharSrc->Use_Obj( this, true );

	case OV_EDIT:
		{
			// Put up a list of items in the container. (if it is a container)
			if ( pClientSrc == NULL )
				return( false );
			pClientSrc->m_Targ_Text = s.GetArgStr();
			pClientSrc->Cmd_EditItem( this, -1 );
		}
		break;
	case OV_EFFECT: // some visual effect.
		{
			int piCmd[5];
			int iArgQty = ParseCmds( s.GetArgStr(), piCmd, COUNTOF(piCmd));
			if ( iArgQty < 2 )
				return( false );

			Effect( (EFFECT_TYPE) piCmd[0], (ITEMID_TYPE) piCmd[1],
				pCharSrc,
				(iArgQty>=3)? piCmd[2] : 5, //BYTE speed = 5,
				(iArgQty>=4)? piCmd[3] : 1, //BYTE loop = 1,
				(iArgQty>=5)? piCmd[4] : false //bool explode = false
				);
		}
		break;
	case OV_FIX:
		s.m_pArg[0] = '\0';
		goto do_fixz;
	case OV_FIXWEIGHT:
		return( FixWeirdness() == 0 );
	case OV_FLIP:
		Flip( s.GetArgStr());
		break;
	case OV_FOLLOW:
	case OV_FOLLOWED:
		// Follow this item.
		if ( pCharSrc )
		{
			// I want to follow this person about.
			pCharSrc->Memory_AddObjTypes( this, MEMORY_FOLLOW );
		}
		break;
	case OV_GUMPDLG:
		goto do_gumpdlg;
	case OV_INPDLG:
		// "INPDLG" verb maxchars
		// else assume it was a property button.
		// m_Targ_PrvMode = parent dialog.
		{
			if ( pClientSrc == NULL )
				return( false );

			TCHAR *Arg_ppCmd[2];		// Maximum parameters in one line
			int iQty = ParseCmds( s.GetArgStr(), Arg_ppCmd, COUNTOF( Arg_ppCmd ));

			CGString sOrgValue;
			if ( ! r_WriteVal( Arg_ppCmd[0], sOrgValue, pSrc ))
				sOrgValue = ".";

			pClientSrc->m_Targ_Text = Arg_ppCmd[0];
			pClientSrc->m_Prop_UID = GetUID();

			int iMaxLength = atoi(Arg_ppCmd[1]);
			CGString sPrompt;
			sPrompt.Format( "%s (# = default)", Arg_ppCmd[0] );
			pClientSrc->addGumpInputBox( TARGMODE_GUMP_VAL, 
				pClientSrc->m_Targ_PrvMode, 0,
				true, STYLE_TEXTEDIT,
				iMaxLength,	sPrompt, sOrgValue );
		}
		break;
	case OV_M:
		goto do_move;
	case OV_MESSAGE:	//put info message (for pSrc client only) over item.
do_message:
		{
			if ( pCharSrc == NULL ) 
				return( false );
			pCharSrc->ObjMessage( s.GetArgStr(), this );
		}
		break;
	case OV_MOVE:
do_move:
		// move without restriction. east,west,etc. (?up,down,)
		if ( IsTopLevel())
		{
			TCHAR * ppCmd[3];
			int iQty = ParseCmds( s.GetArgStr(), ppCmd, COUNTOF(ppCmd));
			TCHAR chDir = toupper( ppCmd[0][0] );
			int iTmp = Exp_GetVal( ppCmd[1] );

			CPointMap pt = GetTopPoint();
			if ( isdigit( chDir ) || chDir == '-' )
			{
				pt.m_x += Exp_GetVal( ppCmd[0] );
				pt.m_y += iTmp;
				pt.m_z += Exp_GetVal( ppCmd[2] );
			}
			else	// a direction by name.
			{
				if ( iTmp == 0 ) 
					iTmp = 1;
				DIR_TYPE eDir = GetDirStr( ppCmd[0] );
				if ( eDir >= DIR_QTY ) 
					return( false );
				pt.Move( eDir, iTmp );
			}
			MoveTo( pt );
			Update();
		}
		break;
	case OV_MOVETO:
		goto do_movetop;
	case OV_MSG:
		goto do_message;
	case OV_NUDGEDOWN:
		if ( IsTopLevel())
		{
			int zdiff = s.GetArgVal();
			SetTopZ( GetTopZ() - ( zdiff ? zdiff : 1 ));
			Update();
		}
		break;
	case OV_NUDGEUP:
		if ( IsTopLevel())
		{
			int zdiff = s.GetArgVal();
			SetTopZ( GetTopZ() + ( zdiff ? zdiff : 1 ));
			Update();
		}
		break;
	case OV_P:
do_movetop:
		MoveTo( g_World.GetRegionPoint( s.GetArgStr()));
		break;
	case OV_REMOVE:	//remove this object now.
		Delete();
		return( true );
	case OV_SAY:
do_speak:
		Speak( s.GetArgStr());
		break;
	case OV_SAYU:
do_speaku:
		// Speak in unicode from the UTF8 system format.
		if ( s.GetArgStr()[3] == ',' )
		{
			SpeakUTF8( s.GetArgStr()+4, COLOR_TEXT_DEF, TALKMODE_SAY, FONT_NORMAL, s.GetArgStr());
		}
		else
		{
			SpeakUTF8( s.GetArgStr());
		}
		break;
	case OV_SFX:
do_sound:
		{
			int piCmd[2];
			int iArgQty = ParseCmds( s.GetArgStr(), piCmd, COUNTOF(piCmd));
			Sound( piCmd[0], ( iArgQty > 1 ) ? piCmd[1] : 1 );
		}
		break;
	case OV_SOUND:
		goto do_sound;
	case OV_SPEAK:	//speak so everyone can here
		goto do_speak;
	case OV_SPEAKU:
		goto do_speaku;
	case OV_SPELLEFFECT:	// a b
		{
			int piCmd[2];
			int iArgs = ParseCmds( s.GetArgStr(), piCmd, COUNTOF(piCmd));
			OnSpellEffect( (SPELL_TYPE) piCmd[0], pCharSrc, piCmd[1] );
		}
		break;

	case OV_TARGET:
		{
			if ( pClientSrc == NULL )
				return( false );
			bool fGroundOnly = true;
			if ( s.HasArgs())
			{
				if ( isdigit( s.GetArgStr()[0] ))
				{
					fGroundOnly = s.GetArgVal();
				}
			}
			pClientSrc->m_Targ_UID = GetUID();
			pClientSrc->m_tmUseItem.m_pParent = GetParent();	// Cheat Verify 
			pClientSrc->addTarget( TARGMODE_USE_ITEM, s.GetArgStr(), fGroundOnly );
		}
		break;

	case OV_TOME:
do_gumpdlg:
		{
			if ( pClientSrc == NULL )
				return( false );
			pClientSrc->Gump_Setup( (TARGMODE_TYPE)( (s.HasArgs())?s.GetArgVal() : 0 ), this );
		}
		break;
	case OV_TRY:
do_try:
		// do this verb only if we can touch it.
		{
			const TCHAR * pszVerb = s.GetArgStr();
			if ( pSrc->GetPrivLevel() <= PLEVEL_Counsel )
			{
				if ( pCharSrc == NULL || ! pCharSrc->CanTouch( this ))
				{
					goto failit;
				}
			}

			if ( ! r_Verb( CScript( pszVerb ), pSrc ))
			{
failit:
				pSrc->SysMessagef( "Can't %s object %s", pszVerb, GetName());
				return( false );
			}

			pSrc->SysMessagef( "%s object %s success", pszVerb, GetName());
		}
		return( true );
	case OV_TRYP:
		{
			int iMinPriv = s.GetArgVal();

			if ( iMinPriv >= PLEVEL_QTY )
			{
				pSrc->SysMessagef("The %s property can't be changed.", s.GetArgStr());
				return( false );
			}
			if ( pSrc->GetPrivLevel() < iMinPriv )
			{
				pSrc->SysMessagef( "You lack the privelege to change the %s property.", s.GetArgStr());
				return( false );
			}
		}
		goto do_try;

	case OV_UPDATE:
		Update();
		break;
	case OV_USEITEM:
		if ( ! pCharSrc ) 
			return( false );
		return pCharSrc->Use_Obj( this, false );
	case OV_Z:	//	ussually in "SETZ" form
do_fixz:
		if ( IsEquipped())
			return( false );
		if ( s.HasArgs())
		{
			SetUnkZ( s.GetArgVal());
		}
		else if ( IsTopLevel())
		{
			WORD wBlockFlags = CAN_C_WALK;
			SetTopZ( g_World.GetHeight( GetTopPoint(), wBlockFlags, GetTopPoint().GetRegion(REGION_TYPE_MULTI)));
		}
		Update();
		break;

	default:
		return( CScriptObj::r_Verb( s, pSrc ));
	}
	return( true );
}

void CObjBase::RemoveFromView( CClient * pClientExclude )
{
	// Remove this item from all clients.
	// In a destructor this can do funny things.

	if ( IsDisconnected()) 
		return;	// not in the world.

	CObjBaseTemplate * pObjTop = GetTopLevelObj();

	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClientExclude == pClient ) 
			continue;
		CChar * pChar = pClient->GetChar();
		if ( pChar == NULL ) 
			continue;
		if ( pChar->GetTopDist( pObjTop ) > UO_MAP_VIEW_RADAR ) 
			continue;
		pClient->addObjectRemove( this );
	}
}

void CObjBase::DeletePrepare()
{
	// Prepare to delete.
	RemoveFromView();
	RemoveSelf();	// Must remove early or else virtuals will fail.
	ASSERT( GetParent() == NULL );
	ASSERT( IsDisconnected());	// It is no place in the world.
}

void CObjBase::Delete()
{
	DeletePrepare();
	ASSERT( IsDisconnected());	// It is no place in the world.
	g_World.m_ObjDelete.InsertAfter(this);
}

