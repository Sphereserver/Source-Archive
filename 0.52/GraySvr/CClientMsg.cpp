//
// CClientMsg.cpp
// Copyright Menace Software (www.menasoft.com).
//
// Game server messages. (No login stuff)
//

#ifdef VISUAL_SPHERE
	#include "..\Visual Sphere\stdafx.h"
	#include "..\Visual Sphere\Visual Sphere.h"
	#include "..\Visual Sphere\ServerObject.h"
#else
	#include "graysvr.h"	// predef header.
#endif

/////////////////////////////////////////////////////////////////
// -CClient stuff.

void CClient::addPause( bool fPause )
{
	// 0 = restart. 1=pause
	DEBUG_CHECK( this );
	if ( this == NULL ) 
		return;
	if ( m_pChar == NULL )
		return;
	if ( m_fPaused == fPause )
		return;

	if ( ! fPause )
	{
		if ( m_fUpdateStats )
		{
			// update all my own stats.
			addCharStatWindow( m_pChar->GetUID());
		}
	}

	m_fPaused = fPause;
	CCommand cmd;
	cmd.Pause.m_Cmd = XCMD_Pause;
	cmd.Pause.m_Arg = fPause;
	xSendPkt( &cmd, sizeof( cmd.Pause ));
}

void CClient ::	addOptions()
{
	CCommand cmd;

#if 0
	// set options ? // ??? not needed ?
	for ( int i=1; i<=3; i++ )
	{
		cmd.Options.m_Cmd = XCMD_Options;
		cmd.Options.m_len = sizeof( cmd.Options );
		cmd.Options.m_index = i;
		xSendPkt( &cmd, sizeof( cmd.Options ));
	}
#endif

	cmd.ReDrawAll.m_Cmd = XCMD_ReDrawAll;	// ???
	xSendPkt( &cmd, sizeof( cmd.ReDrawAll ));

	// Send time. (real or game time ??? why ?)
	cmd.Time.m_Cmd = XCMD_Time;
	cmd.Time.m_hours = ( g_World.GetTime() / ( 60*60*TICK_PER_SEC )) % 24;
	cmd.Time.m_min   = ( g_World.GetTime() / ( 60*TICK_PER_SEC )) % 60;
	cmd.Time.m_sec   = ( g_World.GetTime()	/ ( TICK_PER_SEC )) % 60;
	xSendPkt( &cmd, sizeof( cmd.Time ));
}

void CClient::addObjectRemoveCantSee( CObjUID uid, const TCHAR * pszName )
{
	// Seems this object got out of sync some how.
	if ( pszName == NULL ) pszName = "it";
	SysMessagef( "You can't see %s", pszName );
	addObjectRemove( uid );
}

void CClient::addObjectRemove( CObjUID uid )
{
	// Tell the client to remove the item or char
	DEBUG_CHECK( m_pChar );
	addPause();
	CCommand cmd;
	cmd.Remove.m_Cmd = XCMD_Remove;
	cmd.Remove.m_UID = uid;
	xSendPkt( &cmd, sizeof( cmd.Remove ));
}

void CClient::addRemoveAll( bool fItems, bool fChars )
{
	addPause();
	if ( fItems )
	{
		// Remove any multi objects first ! or client will hang
		CWorldSearch AreaItems( GetChar()->GetTopPoint(), UO_MAP_VIEW_RADAR );
		while (true)
		{
			CItem * pItem = AreaItems.GetItem();
			if ( pItem == NULL ) 
				break;
			addObjectRemove( pItem );
		}
	}
	if ( fChars )
	{
		// Remove any multi objects first ! or client will hang
		CWorldSearch AreaChars( GetChar()->GetTopPoint(), UO_MAP_VIEW_SIZE );
		AreaChars.SetInertView( IsPriv( PRIV_ALLSHOW ));	// show logged out chars?
		while (true)
		{
			CChar * pChar = AreaChars.GetChar();
			if ( pChar == NULL ) 
				break;
			addObjectRemove( pChar );
		}
	}
}

void CClient::addObjectLight( const CObjBase * pObj, LIGHT_PATTERN iLightPattern )		// Object light level.
{
	// ??? this does not seem to work !
	if ( pObj == NULL )
		return;
	CCommand cmd;
	cmd.LightPoint.m_Cmd = XCMD_LightPoint;
	cmd.LightPoint.m_UID = pObj->GetUID();
	cmd.LightPoint.m_level = iLightPattern;	// light level/pattern.
	xSendPkt( &cmd, sizeof( cmd.LightPoint ));
}

void CClient::addItem_OnGround( CItem * pItem ) // Send items (on ground)
{
	ASSERT(pItem);
	// Get base values.
	DWORD dwUID = pItem->GetUID();
	WORD wID = pItem->GetDispID();
	COLOR_TYPE wColor = pItem->GetColor();
	WORD wAmount = ( pItem->GetAmount() > 1 ) ? pItem->GetAmount() : 0;
	CPointMap pt = pItem->GetTopPoint();
	BYTE bFlags = 0;
	BYTE bDir = DIR_N;

	// Modify the values for the specific client/item.
	bool fHumanCorpse = ( wID == ITEMID_CORPSE && CCharBase::IsHuman( pItem->GetCorpseType() ));

	if ( m_pChar->IsStat( STATF_Hallucinating ))
	{
		// cmd.Put.m_id = pItem->GetDispID();	// ??? random displayable item ?
		wColor = GetRandVal( COLOR_DYE_HIGH );	// restrict colors
	}
	else if ( ! fHumanCorpse )
	{
		if ( wColor > COLOR_QTY ) wColor &= COLOR_MASK;	// restrict colors
	}
	else
	{
		// allow transparency colors ?
		if (( wColor &~ COLOR_UNDERWEAR ) > COLOR_TRANSLUCENT ) wColor &= COLOR_MASK | COLOR_UNDERWEAR;
	}

	if ( ! fHumanCorpse && m_pChar->CanMove( pItem, false ))
	{
		bFlags |= ITEMF_MOVABLE;
	}

	if ( IsPriv(PRIV_DEBUG))
	{
		// just use safe numbers
		wID = ITEMID_WorldGem;	// bigger item ???
		pt.m_z = m_pChar->GetTopZ();
		wAmount = 0;
		bFlags |= ITEMF_MOVABLE;
	}
	else
	{
		if ( pItem->IsAttr(ATTR_INVIS) && ! IsPriv(PRIV_GM))
		{
			bFlags |= ITEMF_INVIS;
		}
		if ( pItem->m_pDef->Can( CAN_I_LIGHT ) ||	// ??? Gate or other ?
			pItem->m_pDef->m_type == ITEM_LIGHT_LIT )
		{
			bDir = pItem->m_itLight.m_pattern;	
		}
		else if ( pItem->GetID() == ITEMID_CORPSE )
		{
			// If this item has direction facing use it
			bDir = pItem->m_itCorpse.m_facing_dir;	// DIR_N
		}
	}

	// Pack values in our strange format.
	CCommand cmd;
	BYTE * pData = cmd.m_Raw + 9;

	cmd.Put.m_Cmd = XCMD_Put;

	if ( wAmount ) dwUID |= UID_SPEC;	// Enable amount feild
	cmd.Put.m_UID = dwUID;

	cmd.Put.m_id = wID;
	if ( wAmount )
	{
		PACKWORD(pData,wAmount);
		pData += 2;
	}

	if ( bDir ) pt.m_x |= 0x8000;
	PACKWORD(pData,pt.m_x);
	pData += 2;

	if ( wColor ) pt.m_y |= 0x8000;	 // Enable m_color and m_movable
	if ( bFlags ) pt.m_y |= 0x4000;
	PACKWORD(pData,pt.m_y);
	pData += 2;

	if ( bDir )
	{
		pData[0] = bDir;
		pData++;
	}

	pData[0] = pt.m_z;
	pData++;

	if ( wColor )
	{
		PACKWORD(pData,wColor);
		pData += 2;
	}
	if ( bFlags )
	{
		pData[0] = bFlags;
		pData++;
	}

	int iLen = pData - (cmd.m_Raw);
	ASSERT( iLen );
	cmd.Put.m_len = iLen;

	xSendPkt( &cmd, iLen );

	if ( pItem->m_type == ITEM_SOUND )
	{
		addSound( (SOUND_TYPE) pItem->m_itSound.m_Sound, pItem, pItem->m_itSound.m_Repeat );
	}

	if ( ! IsPriv(PRIV_DEBUG) && fHumanCorpse )	// cloths on corpse
	{
		CItemCorpse * pCorpse = dynamic_cast <CItemCorpse*> (pItem);
		ASSERT(pCorpse);

		// send all the items on the corpse.
		addContents( pCorpse, false, true, false );
		// equip the proper items on the corpse.
		addContents( pCorpse, true, true, false );
	}
}

void CClient::addItem_Equipped( const CItem * pItem )
{
	ASSERT(pItem);
	// Equip a single item on a CChar.
	CChar * pChar = dynamic_cast <CChar*> (pItem->GetParent());
	ASSERT( pChar != NULL );
	ASSERT( pItem->GetEquipLayer() < LAYER_SPECIAL );

	CCommand cmd;
	cmd.ItemEquip.m_Cmd = XCMD_ItemEquip;
	cmd.ItemEquip.m_UID = pItem->GetUID();
	cmd.ItemEquip.m_id = ( pItem->GetEquipLayer() == LAYER_BANKBOX ) ? ITEMID_CHEST_SILVER : pItem->GetDispID();
	cmd.ItemEquip.m_zero7 = 0;
	cmd.ItemEquip.m_layer = pItem->GetEquipLayer();
	cmd.ItemEquip.m_UIDchar = pChar->GetUID();

	COLOR_TYPE wColor;
	GetAdjustedItemID( pChar, pItem, wColor );
	cmd.ItemEquip.m_color = wColor;

	xSendPkt( &cmd, sizeof( cmd.ItemEquip ));
}

void CClient::addItem_InContainer( const CItem * pItem )
{
	ASSERT(pItem);
	CItemContainer * pCont = dynamic_cast <CItemContainer*> (pItem->GetParent());
	if ( pCont == NULL )
		return;

	CPointBase pt = pItem->GetContainedPoint();

	// Add a single item in a container.
	CCommand cmd;
	cmd.ContAdd.m_Cmd = XCMD_ContAdd;
	cmd.ContAdd.m_UID = pItem->GetUID();
	cmd.ContAdd.m_id = pItem->GetDispID();
	cmd.ContAdd.m_zero7 = 0;
	cmd.ContAdd.m_amount = pItem->GetAmount();
	cmd.ContAdd.m_x = pt.m_x;
	cmd.ContAdd.m_y = pt.m_y;
	cmd.ContAdd.m_UIDCont = pCont->GetUID();

	COLOR_TYPE wColor = pItem->GetColor();
	if ( m_pChar->IsStat( STATF_Hallucinating ))
	{
		wColor = GetRandVal( COLOR_DYE_HIGH );	// restrict colors
	}
	else
	{
		if ( wColor > COLOR_QTY ) wColor &= COLOR_MASK;
	}
	cmd.ContAdd.m_color = wColor;

	xSendPkt( &cmd, sizeof( cmd.ContAdd ));
}

void CClient::addItem( CItem * pItem )
{
	addPause();
	if ( pItem->IsTopLevel())
	{
		addItem_OnGround( pItem );
	}
	else if ( pItem->IsEquipped())
	{
		addItem_Equipped( pItem );
	}
	else if ( pItem->IsInContainer())
	{
		addItem_InContainer( pItem );
	}
}

int CClient::addContents( const CItemContainer * pContainer, bool fCorpseEquip, bool fCorpseFilter, bool fShop ) // Send Backpack (with items)
{
	// NOTE: We needed to send the header for this FIRST !!!
	// 1 = equip a corpse
	// 0 = contents.

	addPause();

	bool fLayer[LAYER_HORSE];
	memset( fLayer, 0, sizeof(fLayer));

	CCommand cmd;

	// send all the items in the container.
	int count = 0;
	for ( CItem* pItem=pContainer->GetContentHead(); pItem!=NULL; pItem=pItem->GetNext())
	{
		LAYER_TYPE layer;
		if ( fCorpseFilter )	// dressing a corpse is different from opening the coffin!
		{
			layer = (LAYER_TYPE) pItem->GetContainedLayer();
			ASSERT( layer < LAYER_HORSE );
			switch ( layer )	// don't put these on a corpse.
			{
			case LAYER_NONE:
			case LAYER_PACK:	// these display strange.
				continue;
			case LAYER_PACK2:
			case LAYER_UNUSED9:
				ASSERT(0);
				continue;
			}
			// Make certain that no more than one of each layer goes out to client....crashes otherwise!!
			if ( fLayer[layer] )
			{
				DEBUG_CHECK( !fLayer[layer]);
				continue;
			}
			fLayer[layer] = true;
		}
		if ( fCorpseEquip )	// list equipped items on a corpse
		{
			ASSERT( fCorpseFilter );
			cmd.CorpEquip.items[count].m_layer	= pItem->GetContainedLayer();
			cmd.CorpEquip.items[count].m_UID	= pItem->GetUID();
		}
		else	// Content items
		{
			cmd.Content.items[count].m_UID		= pItem->GetUID();
			cmd.Content.items[count].m_id		= pItem->GetDispID();
			cmd.Content.items[count].m_zero6	= 0;
			cmd.Content.items[count].m_amount	= pItem->GetAmount();
			if ( fShop )
			{
				CItemVendable * pVendItem = dynamic_cast <CItemVendable *> (pItem);
				if ( ! pVendItem )
					continue;
				if ( ! pVendItem->GetAmount()) 
					continue;
				if ( pVendItem->GetID() == ITEMID_GOLD_C1 )
					continue;
				if ( pVendItem->GetAmount() > g_Serv.m_iVendorMaxSell )
				{
					cmd.Content.items[count].m_amount = g_Serv.m_iVendorMaxSell;
				}
				cmd.Content.items[count].m_x	= count;
				cmd.Content.items[count].m_y	= count;
			}
			else
			{
				CPointBase pt = pItem->GetContainedPoint();
				cmd.Content.items[count].m_x	= pt.m_x;
				cmd.Content.items[count].m_y	= pt.m_y;
			}
			cmd.Content.items[count].m_UIDCont	= pContainer->GetUID();

			COLOR_TYPE wColor = pItem->GetColor();
			if ( m_pChar->IsStat( STATF_Hallucinating ))
			{
				wColor = GetRandVal( COLOR_DYE_HIGH );
			}
			else
			{
				if ( wColor > COLOR_QTY ) wColor &= COLOR_MASK;	// restrict colors
			}
			cmd.Content.items[count].m_color = wColor;
		}
		count ++;
	}

	if ( ! count )
	{
		return 0;
	}
	int len;
	if ( fCorpseEquip )
	{
		cmd.CorpEquip.items[count].m_layer = LAYER_NONE;	// terminator.
		len = sizeof( cmd.CorpEquip ) - sizeof(cmd.CorpEquip.items) + ( count * sizeof(cmd.CorpEquip.items[0])) + 1;
		cmd.CorpEquip.m_Cmd = XCMD_CorpEquip;
		cmd.CorpEquip.m_len = len;
		cmd.CorpEquip.m_UID = pContainer->GetUID();
	}
	else
	{
		len = sizeof( cmd.Content ) - sizeof(cmd.Content.items) + ( count * sizeof(cmd.Content.items[0]));
		cmd.Content.m_Cmd = XCMD_Content;
		cmd.Content.m_len = len;
		cmd.Content.m_count = count;
	}

	xSendPkt( &cmd, len );
	return( count );
}

void CClient::addOpenGump( const CObjBase * pContainer, GUMP_TYPE gump )
{
	// NOTE: if pContainer has not already been sent to the client
	//  this will crash client.
	CCommand cmd;
	cmd.ContOpen.m_Cmd = XCMD_ContOpen;
	cmd.ContOpen.m_UID = pContainer->GetUID();
	cmd.ContOpen.m_gump = gump;

	// we automatically get open sound for this,.
	xSendPkt( &cmd, sizeof( cmd.ContOpen ));
}

bool CClient::addContainerSetup( const CItemContainer * pContainer ) // Send Backpack (with items)
{
	ASSERT( ! pContainer->IsWeird());
	ASSERT( pContainer->IsItem());

	// open the conatiner with the proper GUMP.
	GUMP_TYPE gump = CItemBase::IsContainerID( pContainer->GetDispID());
	if ( gump <= GUMP_RESERVED )
		return false;

	addOpenGump( pContainer, gump );
	addContents( pContainer, false, false, false );
	return( true );
}

void CClient::addWeather( WEATHER_TYPE weather ) // Send new weather to player
{
	ASSERT( m_pChar );
	if ( m_pChar->m_pArea && m_pChar->m_pArea->IsFlag(REGION_FLAG_UNDERGROUND))
	{
		weather = WEATHER_DRY;
	}
	else if ( m_pChar->IsStat( STATF_InDoors ))
	{
		// If there is a roof over our head at the moment then stop rain.
		weather = WEATHER_DRY;
	}
	else if ( weather == WEATHER_DEFAULT )
	{
		weather = m_pChar->GetTopSector()->GetWeather();
	}

	CCommand cmd;
	cmd.Weather.m_Cmd = XCMD_Weather;
	cmd.Weather.m_type = weather;

	// m_ext = seems to control a very gradual fade in and out ?
	switch ( weather )
	{
	case WEATHER_RAIN:	// rain
		cmd.Weather.m_ext = 0x4600;
		break;
	case WEATHER_SNOW:	// snow
		cmd.Weather.m_ext = 0x46EC;
		break;
	default:	// dry or cloudy
		cmd.Weather.m_type = WEATHER_DRY;
		cmd.Weather.m_ext = 0;
		break;
	}
	xSendPkt( &cmd, sizeof(cmd.Weather));
}

void CClient::addLight( int iLight )
{
	// NOTE: This could just be a flash of light.
	// Global light level.
	if ( iLight < LIGHT_BRIGHT ) 
	{
		iLight = m_pChar->GetLightLevel();
	}

	// Scale light level for non-t2a.
	if ( iLight < LIGHT_BRIGHT ) 
		iLight = LIGHT_BRIGHT;
	if ( iLight > LIGHT_DARK )
		iLight = LIGHT_DARK;

	if ( ! IsPriv( PRIV_T2A ))
	{
		iLight = IMULDIV( iLight, LIGHT_DARK_OLD, LIGHT_DARK );
	}

	CCommand cmd;
	cmd.Light.m_Cmd = XCMD_Light;
	cmd.Light.m_level = iLight;
	xSendPkt( &cmd, sizeof( cmd.Light ));
}

void CClient::addArrowQuest( int x, int y )
{
	CCommand cmd;
	cmd.Arrow.m_Cmd = XCMD_Arrow;
	cmd.Arrow.m_Active = ( x && y ) ? 1 : 0;	// 1/0
	cmd.Arrow.m_x = x;
	cmd.Arrow.m_y = y;
	xSendPkt( &cmd, sizeof( cmd.Arrow ));
}

void CClient::addMusic( WORD id )
{
	// Music is ussually appropriate for the region.
	CCommand cmd;
	cmd.PlayMusic.m_Cmd = XCMD_PlayMusic;
	cmd.PlayMusic.m_musicid = id;
	xSendPkt( &cmd, sizeof( cmd.PlayMusic ));
}

bool CClient::addKick( CTextConsole * pSrc, bool fBlock )
{
	// Kick me out.
	ASSERT( pSrc );
	if ( m_pAccount == NULL )
		return( false );

	if ( GetPrivLevel() >= pSrc->GetPrivLevel())
		return( false );

	if ( fBlock )
	{
		m_pAccount->r_SetVal( "BLOCK", "1" );
	}
	const TCHAR * pszAction = fBlock ? "KICK" : "DISCONNECTED";

	SysMessagef( "You have been %sed by '%s'", pszAction, pSrc->GetName());
	g_Log.Event( LOGL_EVENT, "'%s' was %sed by '%s'\n", GetName(), pszAction, pSrc->GetName());

	CCommand cmd;
	cmd.Kick.m_Cmd = XCMD_Kick;
	cmd.Kick.m_unk1 = 0;
	xSendPkt( &cmd, sizeof( cmd.Kick ));

	delete this;
	return( true );
}

void CClient::addSound( SOUND_TYPE id, const CObjBaseTemplate * pBase, int iOnce )
{
	// if ( id <= 0 ) return;

	CPointMap pt;
	if ( pBase)
	{
		pBase = pBase->GetTopLevelObj();
		pt = pBase->GetTopPoint();
	}
	else
	{
		pt = m_pChar->GetTopPoint();
	}

	if ( id > 0 && id != SOUND_CLEAR )
	{
		if ( id > SOUND_QTY ) return;
		if ( ! iOnce )
		{
			// This sound should repeat.
			if ( ! pBase ) return;
			if ( m_pntSoundRecur.IsValid())
			{
				addSound( SOUND_CLEAR );
			}
			m_pntSoundRecur = pt;
		}
	}
	else
	{
		// iOnce = 1;
		m_pntSoundRecur.Init();
	}

	CCommand cmd;
	cmd.Sound.m_Cmd = XCMD_Sound;
	cmd.Sound.m_flags = iOnce;
	cmd.Sound.m_id = id;
	cmd.Sound.m_speed = 0;
	cmd.Sound.m_x = pt.m_x;
	cmd.Sound.m_y = pt.m_y;
	cmd.Sound.m_z = pt.m_z;

	xSendPkt( &cmd, sizeof(cmd.Sound));
}

void CClient::addItemDragCancel( BYTE type )
{
	// Sound seems to be automatic ???
	// addSound( 0x051 );
	CCommand cmd;
	cmd.DragCancel.m_Cmd = XCMD_DragCancel;
	cmd.DragCancel.m_type = type;
	xSendPkt( &cmd, sizeof( cmd.DragCancel ));
}

void CClient::addBarkUNICODE( const NCHAR * pText, const CObjBaseTemplate * pSrc, COLOR_TYPE color, TALKMODE_TYPE mode, FONT_TYPE font, const char * pszLanguage )
{
	if ( pText == NULL ) return;

	CCommand cmd;
	cmd.SpeakUNICODE.m_Cmd = XCMD_SpeakUNICODE;

	if ( mode == TALKMODE_BROADCAST )
	{
		mode = TALKMODE_SYSTEM;
		pSrc = NULL;
	}

	int i=0;
	for ( ; pText[i] && i < MAX_TALK_BUFFER; i++ )
	{
		cmd.SpeakUNICODE.m_utext[i] = pText[i];
	}
	cmd.SpeakUNICODE.m_utext[i++] = '\0';	// add for the null

	int len = sizeof(cmd.SpeakUNICODE) + (i*sizeof(NCHAR));
	cmd.SpeakUNICODE.m_len = len;
	cmd.SpeakUNICODE.m_mode = mode;		// mode = range.
	cmd.SpeakUNICODE.m_color = color;
	cmd.SpeakUNICODE.m_font = font;		// font. 3 = system message just to you !

	if ( pszLanguage == NULL ) pszLanguage = "ENU";
	strncpy( cmd.SpeakUNICODE.m_lang, pszLanguage, sizeof(cmd.SpeakUNICODE.m_lang));
	cmd.SpeakUNICODE.m_lang[sizeof(cmd.SpeakUNICODE.m_lang)-1] = '\0';

	if ( pSrc == NULL )
	{
		cmd.SpeakUNICODE.m_UID = 0x01010101;
		strcpy( cmd.SpeakUNICODE.m_name, "System" );
	}
	else
	{
		cmd.SpeakUNICODE.m_UID = pSrc->GetUID();
		strncpy( cmd.SpeakUNICODE.m_name, pSrc->GetName(), sizeof(cmd.SpeakUNICODE.m_name));
		cmd.SpeakUNICODE.m_name[ sizeof(cmd.SpeakUNICODE.m_name)-1 ] = '\0';
	}
	if ( pSrc == NULL || pSrc->IsItem())
	{
		cmd.SpeakUNICODE.m_id = 0x0101;
	}
	else	// char id only.
	{
		cmd.SpeakUNICODE.m_id = (dynamic_cast <const CChar*>(pSrc))->GetDispID();
	}
	xSendPkt( &cmd, len );
}

void CClient::addBark( const TCHAR * pText, const CObjBaseTemplate * pSrc, COLOR_TYPE color, TALKMODE_TYPE mode, FONT_TYPE font )
{
	if ( pText == NULL ) 
		return;

	CCommand cmd;
	cmd.Speak.m_Cmd = XCMD_Speak;

	if ( mode == TALKMODE_BROADCAST )
	{
		mode = TALKMODE_SYSTEM;
		pSrc = NULL;
	}

	int len = strlen(pText);
	DEBUG_CHECK( len < MAX_TALK_BUFFER );
	len += sizeof(cmd.Speak);
	cmd.Speak.m_len = len;
	cmd.Speak.m_mode = mode;		// mode = range.
	cmd.Speak.m_color = color;
	cmd.Speak.m_font = font;		// font. 3 = system message just to you !

	if ( pSrc == NULL )
	{
		cmd.Speak.m_UID = 0x01010101;
		strcpy( cmd.Speak.m_name, "System" );
	}
	else
	{
		cmd.Speak.m_UID = pSrc->GetUID();
		strncpy( cmd.Speak.m_name, pSrc->GetName(), sizeof(cmd.Speak.m_name));
		cmd.Speak.m_name[ sizeof(cmd.Speak.m_name)-1 ] = '\0';
	}
	if ( pSrc == NULL || pSrc->IsItem())
	{
		cmd.Speak.m_id = 0x0101;
	}
	else	// char id only.
	{
		cmd.Speak.m_id = (dynamic_cast <const CChar*>(pSrc))->GetDispID();
	}
	strncpy( cmd.Speak.m_text, pText, MAX_TALK_BUFFER );
	xSendPkt( &cmd, len );
}

void CClient::addObjMessage( const TCHAR *pMsg, const CObjBaseTemplate * pSrc, COLOR_TYPE color ) // The message when an item is clicked
{
	addBark( pMsg, pSrc, color, ( pSrc == m_pChar ) ? TALKMODE_4 : TALKMODE_ITEM, FONT_NORMAL );
}

void CClient::addEffect( EFFECT_TYPE motion, ITEMID_TYPE id, const CObjBaseTemplate * pDst, const CObjBaseTemplate * pSrc, BYTE speed, BYTE loop, bool explode )
{
	ASSERT( pDst );
	pDst = pDst->GetTopLevelObj();
	CPointMap ptDst = pDst->GetTopPoint();

	// speed = 0=very fast, 7=slow.
	CCommand cmd;
	cmd.Effect.m_Cmd = XCMD_Effect;
	cmd.Effect.m_motion = motion;
	cmd.Effect.m_id = id;
	cmd.Effect.m_UID = pDst->GetUID();

	CPointMap ptSrc;
	if ( pSrc != NULL )
	{
		pSrc = pSrc->GetTopLevelObj();
		ptSrc = pSrc->GetTopPoint();
	}
	else
	{
		ptSrc = ptDst;
	}

	cmd.Effect.m_speed = speed;		// 22= 0=very fast, 7=slow.
	cmd.Effect.m_loop = loop;		// 23= 0 is really long.  1 is the shortest., 6 = longer
	cmd.Effect.m_color = 0; // 0x300;		// 24=0 unknown
	cmd.Effect.m_OneDir = true;		// 26=1=point in single dir else turn.
	cmd.Effect.m_explode = explode;	// 27=effects that explode on impact.

	cmd.Effect.m_srcx = ptSrc.m_x;
	cmd.Effect.m_srcy = ptSrc.m_y;
	cmd.Effect.m_srcz = ptSrc.m_z;
	cmd.Effect.m_dstx = ptDst.m_x;
	cmd.Effect.m_dsty = ptDst.m_y;
	cmd.Effect.m_dstz = ptDst.m_z;

	switch ( motion )
	{
	case EFFECT_BOLT:	// a targetted bolt
		if ( ! pSrc ) return;
		cmd.Effect.m_targUID = pDst->GetUID();
		cmd.Effect.m_UID = pSrc->GetUID();	// source
		cmd.Effect.m_OneDir = false;
		cmd.Effect.m_loop = 0;	// Does not apply.
		break;

	case EFFECT_LIGHTNING:	// lightning bolt.
		break;

	case EFFECT_XYZ:	// Stay at current xyz ??? not sure about this.
		break;

	case EFFECT_OBJ:	// effect at single Object.
		break;
	}

	xSendPkt( &cmd, sizeof( cmd.Effect ));
}

void CClient::GetAdjustedItemID( const CChar * pChar, const CItem * pItem, COLOR_TYPE & wColor )
{
	// An equipped item.
	ASSERT( pChar );
			
	if ( m_pChar->IsStat( STATF_Hallucinating ))
	{
		wColor = GetRandVal( COLOR_DYE_HIGH );
	}
	else if ( pChar->IsStat(STATF_Stone))
	{
		wColor = COLOR_STONE;
	}
	else
	{
		wColor = pItem->GetColor();
		if ( wColor > COLOR_QTY )
			wColor &= COLOR_MASK;
	}
}

void CClient::GetAdjustedCharID( const CChar * pChar, CREID_TYPE & id, COLOR_TYPE & wColor )
{
	ASSERT( m_pAccount );
	ASSERT( pChar );

	if ( m_pChar->IsStat( STATF_Hallucinating )) // viewer is halucinating.
	{
		id = CCharBaseDisp::GetRandBase();
		wColor = GetRandVal( COLOR_DYE_HIGH );
	}
	else
	{
		id = pChar->GetDispID();
		if ( pChar->IsStat(STATF_Stone))	// turned to stone.
		{
			wColor = COLOR_STONE;
		}
		else
		{
			wColor = pChar->GetColor();
			if ( ! pChar->IsHuman())
			{
				if ( wColor > COLOR_TRANSLUCENT )
					wColor &= COLOR_MASK;
			}
			else
			{
				// allow transparency colors ?
				if (( wColor&~COLOR_UNDERWEAR ) > COLOR_TRANSLUCENT )
					wColor &= COLOR_MASK | COLOR_UNDERWEAR;
			}
		}
	}

	// IF they do not have t2a.
	if ( ! m_pAccount->IsPriv( PRIV_T2A ))
	{
		switch ( id )
		{
		case CREID_Tera_Warrior:
		case CREID_Tera_Drone:
		case CREID_Tera_Matriarch:
			id = CREID_GIANT_SNAKE;
			break;
		case CREID_Titan:
		case CREID_Cyclops:
			id = CREID_OGRE;
			break;
		case CREID_Giant_Toad:
		case CREID_Bull_Frog:
			id = CREID_GiantRat;
			break;
		case CREID_Ophid_Mage:
		case CREID_Ophid_Warrior:
		case CREID_Ophid_Queen:
			id = CREID_GIANT_SPIDER;
			break;
		case CREID_SEA_Creature:
			id = CREID_SEA_SERP;
			break;
		case CREID_LavaLizard:
			id = CREID_Alligator;
			break;
		case CREID_Ostard_Desert:
		case CREID_Ostard_Frenz:
		case CREID_Ostard_Forest:
			id = CREID_HORSE1;
			break;
		case CREID_VORTEX:
			id = CREID_AIR_ELEM;
			break;
		}
	}
}

void CClient::addCharMove( const CChar * pChar )
{
	// This char has just moved on screen.
	// or changed in a subtle way like "hidden"
	//

	addPause();

	CCommand cmd;
	cmd.CharMove.m_Cmd = XCMD_CharMove;
	cmd.CharMove.m_UID = pChar->GetUID();

	CREID_TYPE id;
	COLOR_TYPE color;
	GetAdjustedCharID( pChar, id, color );
	cmd.CharMove.m_id = id;
	cmd.CharMove.m_color = color;

	CPointMap pt = pChar->GetTopPoint();
	cmd.CharMove.m_x  = pt.m_x;
	cmd.CharMove.m_y  = pt.m_y;
	cmd.CharMove.m_z = pt.m_z;
	cmd.CharMove.m_dir = pChar->GetDirFlag();
	cmd.CharMove.m_mode = pChar->GetModeFlag( pChar->CanSeeTrue( m_pChar ));
	cmd.CharMove.m_noto = pChar->GetNotoFlag( m_pChar );

	if ( IsPriv(PRIV_DEBUG))
	{
		cmd.CharMove.m_id = CREID_MAN;
		cmd.CharMove.m_z = m_pChar->GetTopZ();
	}

	xSendPkt( &cmd, sizeof(cmd.CharMove));
}

void CClient::addChar( const CChar * pChar )
{
	// Full update about a char.

	addPause();
	// if ( pChar == m_pChar ) m_WalkCount = -1;

	CCommand cmd;
	cmd.Char.m_Cmd = XCMD_Char;
	cmd.Char.m_UID = pChar->GetUID();

	CREID_TYPE id;
	COLOR_TYPE color;
	GetAdjustedCharID( pChar, id, color );
	cmd.Char.m_id = id;
	cmd.Char.m_color = color;

	CPointMap pt = pChar->GetTopPoint();
	cmd.Char.m_x = pt.m_x;
	cmd.Char.m_y = pt.m_y;
	cmd.Char.m_z = pt.m_z;
	cmd.Char.m_dir = pChar->GetDirFlag();
	cmd.Char.m_mode = pChar->GetModeFlag( pChar->CanSeeTrue( m_pChar ));
	cmd.Char.m_noto = pChar->GetNotoFlag( m_pChar );

	if ( IsPriv(PRIV_DEBUG))
	{
		cmd.Char.m_id = CREID_MAN;
		cmd.Char.m_z = m_pChar->GetTopZ();
	}

	int len = ( sizeof( cmd.Char ) - sizeof( cmd.Char.equip ));
	CCommand * pCmd = &cmd;

//#ifdef _DEBUG
	bool fLayer[LAYER_HORSE+1];
	memset( fLayer, 0, sizeof(fLayer));
//#endif

	if ( ! pChar->IsStat( STATF_Sleeping ))
	{
		// extend the current struct for all the equipped items.
		for ( CItem* pItem=pChar->GetContentHead(); pItem!=NULL; pItem=pItem->GetNext())
		{
			LAYER_TYPE layer = pItem->GetEquipLayer();
			if ( ! CItemBase::IsVisibleLayer( layer )) 
				continue;

//#ifdef _DEBUG
			// Make certain that no more than one of each layer goes out to client....crashes otherwise!!
			if ( fLayer[layer] )
			{
				DEBUG_CHECK( !fLayer[layer]);
				continue;
			}
			fLayer[layer] = true;
//#endif

			pCmd->Char.equip[0].m_UID = pItem->GetUID();
			pCmd->Char.equip[0].m_layer = layer;

			COLOR_TYPE wColor;
			GetAdjustedItemID( pChar, pItem, wColor );
			if ( wColor )
			{
				pCmd->Char.equip[0].m_id = pItem->GetDispID() | 0x8000;	// include color.
				pCmd->Char.equip[0].m_color = wColor;
				len += sizeof(pCmd->Char.equip[0]);
				pCmd = (CCommand*)(((BYTE*)pCmd)+sizeof(pCmd->Char.equip[0]));
			}
			else
			{
				pCmd->Char.equip[0].m_id = pItem->GetDispID();
				len += sizeof(pCmd->Char.equip[0]) - sizeof(pCmd->Char.equip[0].m_color);
				pCmd = (CCommand*)(((BYTE*)pCmd)+sizeof(pCmd->Char.equip[0])-sizeof(pCmd->Char.equip[0].m_color));
			}
		}
	}
	pCmd->Char.equip[0].m_UID = 0;	// terminator.
	len += sizeof( DWORD );

	cmd.Char.m_len = len;
	xSendPkt( &cmd, len );
}

void CClient::addItemName( const CItem * pItem )
{
	if ( ! IsPriv(PRIV_GM) &&
		m_pChar->GetDist( pItem ) > UO_MAP_VIEW_SIZE )
	{
		DEBUG_ERR(( "%x:Event_SingleClick distance cheat\n", GetSocket()));
		return;
	}

	TCHAR szName[ MAX_NAME_SIZE + 256 ];
	int len = strcpylen( szName, pItem->GetNameFull( IsPriv(PRIV_GM) || pItem->IsAttr( ATTR_IDENTIFIED )));

	const CContainer* pCont = dynamic_cast<const CContainer*>(pItem);
	if ( pCont != NULL )
	{
		// ??? Corpses show hair as an item !!
		len += sprintf( szName+len, " (%d items)", pCont->GetCount());
	}

	// obviously damaged ?
	else if ( pItem->IsArmorWeapon())
	{
		int iPercent = pItem->Armor_GetRepairPercent();
		if ( iPercent < 50 && 
			( m_pChar->Skill_GetAdjusted( SKILL_ARMSLORE ) / 10 > iPercent ))
		{
			len += sprintf( szName+len, " (%s)", pItem->Armor_GetRepairDesc());
		}
	}

	// Show the priced value
	CItemContainer * pMyCont = dynamic_cast <CItemContainer *>( pItem->GetParent());
	if ( pMyCont != NULL && pMyCont->GetID() == ITEMID_VENDOR_BOX )
	{
		const CItemVendable * pVendItem = dynamic_cast <const CItemVendable *> (pItem);
		len += sprintf( szName+len, " (%d gp)",
			//pItem->GetVendorPrice( pMyCont->GetEquipLayer() != LAYER_VENDOR_BUYS ));
			pVendItem->GetSellPrice());
	}

	COLOR_TYPE color = COLOR_TEXT_DEF;
	const CItemCorpse * pCorpseItem = dynamic_cast <const CItemCorpse *>(pItem);
	if ( pCorpseItem )
	{
		CChar * pCharCorpse = pCorpseItem->m_uidLink.CharFind();
		if ( pCharCorpse )
		{
			switch ( pCharCorpse->GetNotoFlag( m_pChar, true ))
			{
			case NOTO_GOOD:			color = 0x0063;	break;	// Blue
			case NOTO_GUILD_SAME:	color = 0x0044;	break;	// Green (same guild)
			case NOTO_NEUTRAL:		color = 0x03b2;	break;	// Grey 1 (someone that can be attacked)
			case NOTO_CRIMINAL:		color = 0x03b2;	break;	// Grey 2 (criminal)
			case NOTO_GUILD_WAR:	color = 0x002b;	break;	// Orange (enemy guild)
			case NOTO_EVIL:			color = 0x0026;	break;	// Red
			default: color = COLOR_TEXT_DEF;	break;	// ?Grey
			}
		}
	}

	if ( IsPriv( PRIV_GM ))
	{
		if ( pItem->IsAttr(ATTR_INVIS ))
		{
			len += strcpylen( szName+len, " (invis)" );
		}
		// Show the restock count
		if ( pMyCont != NULL && pMyCont->GetID() == ITEMID_VENDOR_BOX )
		{
			len += sprintf( szName+len, " (%d restock)", pItem->GetContainedLayer());
		}
		switch ( pItem->m_type )
		{
		case ITEM_ADVANCE_GATE:
			if ( pItem->m_itAdvanceGate.m_Type )
			{
				CCharBase * pDef = CCharBase::FindCharBase( pItem->m_itAdvanceGate.m_Type );
				len += sprintf( szName+len, " (%x=%s)", pItem->m_itAdvanceGate.m_Type, (pDef==NULL)?"?": pDef->GetTradeName());
			}
			break;
		case ITEM_SPAWN_CHAR:
		case ITEM_SPAWN_ITEM:
			{
				len += pItem->Spawn_GetName( szName + len );
			}
			break;
		}
	}
	if ( IsPriv( PRIV_DEBUG ))
	{
		// Show UID
		len += sprintf( szName+len, " [%lx]", pItem->GetUID());
	}

	addObjMessage( szName, pItem, color );
}

void CClient::addCharName( const CChar * pChar ) // Singleclick text for a character
{
	// Karma color codes ?
	ASSERT( pChar );

	WORD color;
	switch ( pChar->GetNotoFlag( m_pChar, true ))
	{
	case NOTO_GOOD:			color = 0x0063;	break;	// Blue
	case NOTO_GUILD_SAME:	color = 0x0044;	break;	// Green (same guild)
	case NOTO_NEUTRAL:		color = 0x03b2;	break;	// Grey 1 (someone that can be attacked)
	case NOTO_CRIMINAL:		color = 0x03b2;	break;	// Grey 2 (criminal)
	case NOTO_GUILD_WAR:	color = 0x002b;	break;	// Orange (enemy guild)
	case NOTO_EVIL:			color = 0x0026;	break;	// Red
	default: color = COLOR_TEXT_DEF;	break;	// ?Grey
	}

	TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];

	strcpy( szTemp, pChar->GetFameTitle());
	strcat( szTemp, pChar->GetName());

	// Guild abbrev.
	const TCHAR * pAbbrev = pChar->Guild_AbbrevBracket(MEMORY_TOWN);
	if ( pAbbrev )
	{
		strcat( szTemp, pAbbrev );
	}

	pAbbrev = pChar->Guild_AbbrevBracket(MEMORY_GUILD);
	if ( pAbbrev )
	{
		strcat( szTemp, pAbbrev );
	}

	if ( g_Serv.m_fCharTags )
	{
		if ( pChar->m_pArea && pChar->m_pArea->IsGuarded() && pChar->m_pNPC )
		{
			if ( pChar->IsStat( STATF_Pet ))
				strcat( szTemp, " [tame]" );
			else
				strcat( szTemp, " [NPC]" );
		}
		if ( pChar->IsStat( STATF_INVUL ) && ! pChar->IsStat( STATF_Incognito ) && ! pChar->IsPriv( PRIV_PRIV_NOSHOW ))
			strcat( szTemp, " [invulnerable]" );
		if ( pChar->IsStat( STATF_Stone ))
			strcat( szTemp, " [stone]" );
		else if ( pChar->IsStat( STATF_Freeze ))
			strcat( szTemp, " [frozen]" );
		if ( pChar->IsStat( STATF_Insubstantial | STATF_Invisible | STATF_Hidden ))
			strcat( szTemp, " [hidden]" );
		if ( pChar->IsStat( STATF_Sleeping ))
			strcat( szTemp, " [sleeping]" );
		if ( pChar->IsStat( STATF_Hallucinating ))
			strcat( szTemp, " [hallu]" );
	}
	if ( pChar->GetPrivLevel() <= PLEVEL_Guest )
	{
		strcat( szTemp, " [guest]" );
	}
	if ( pChar->IsPriv( PRIV_JAILED ))
	{
		strcat( szTemp, " [jailed]" );
	}
	if ( pChar->IsDisconnected())
	{
		strcat( szTemp, " [logout]" );
	}
	if ( IsPriv(PRIV_DEBUG))
	{
		sprintf( szTemp+strlen(szTemp), " [%lx]", pChar->GetUID());
	}
	if (( IsPriv(PRIV_DEBUG) || pChar == m_pChar ) && pChar->IsStat( STATF_Criminal ))
	{
		strcat( szTemp, " [criminal]" );
	}

#ifdef _DEBUG
	if ( IsPriv(PRIV_GM) && ( g_Serv.m_wDebugFlags & DEBUGF_NPC_EMOTE ))
	{
		strcat( szTemp, " [" );
		strcat( szTemp, pChar->Skill_GetName());
		strcat( szTemp, "]" );
	}
#endif

	addObjMessage( szTemp, pChar, color );
}

void CClient::addPlayerWalkCancel( void )
{
	// Resync CChar client back to a previous move.
	CCommand cmd;
	cmd.WalkCancel.m_Cmd = XCMD_WalkCancel;
	cmd.WalkCancel.m_count = m_WalkCount;	// sequence #

	CPointMap pt = m_pChar->GetTopPoint();

	cmd.WalkCancel.m_x = pt.m_x;
	cmd.WalkCancel.m_y = pt.m_y;
	cmd.WalkCancel.m_dir = m_pChar->GetDirFlag();
	cmd.WalkCancel.m_z = pt.m_z;
	xSendPkt( &cmd, sizeof( cmd.WalkCancel ));
	m_WalkCount = -1;
}

void CClient::addPlayerStart( CChar * pChar )
{
	m_pChar = pChar;
	m_pChar->ClientAttach( this );
	ASSERT( pChar->m_pPlayer && pChar->m_pNPC == NULL );

	CItem * pItemChange = m_pChar->ContentFindType( ITEM_EQ_CLIENT_LINGER );
	if ( pItemChange != NULL )
	{
		pItemChange->Delete();
	}

	static BYTE Pkt_Start1[20] =	// no idea what this stuff does.
		"\x00"
		"\x00\x00\x7F\x00"
		"\x00\x00"
		"\x00\x00"
		"\x07\x80"
		"\x09\x60"
		"\x00\x00\x00\x00\x00\x00";

	CCommand cmd;
	cmd.Start.m_Cmd = XCMD_Start;
	cmd.Start.m_UID = m_pChar->GetUID();
	cmd.Start.m_unk_5_8 = 0;
	cmd.Start.m_id = m_pChar->GetDispID();

	CPointMap pt = m_pChar->GetTopPoint();
	cmd.Start.m_x = pt.m_x;
	cmd.Start.m_y = pt.m_y;
	cmd.Start.m_z = pt.m_z;
	cmd.Start.m_dir = m_pChar->GetDirFlag();
	memcpy( &cmd.Start.m_unk_18, Pkt_Start1, sizeof( Pkt_Start1 ));
	cmd.Start.m_mode = m_pChar->GetModeFlag();
	xSendPkt( &cmd, sizeof( cmd.Start ));

	ClearTargMode();	// clear death menu mode. etc. ready to walk about. cancel any previos modes.

	addPause();	// OSI seems to do this.
	addPlayerWarMode();
	m_pChar->MoveTo( pt );	// Make sure we are in active list.
	m_pChar->Update();
	addOptions();
	addSound( SOUND_CLEAR );
	addWeather( WEATHER_DEFAULT );
	addLight();		// Current light level where I am.
	addEnableChatButton();
}

void CClient::addPlayerWarMode()
{
	CCommand cmd;
	cmd.War.m_Cmd = XCMD_War;
	cmd.War.m_warmode = ( m_pChar->IsStat( STATF_War )) ? 1 : 0;
	cmd.War.m_unk2[0] = 0;
	cmd.War.m_unk2[1] = 0x32;
	cmd.War.m_unk2[2] = 0;
	xSendPkt( &cmd, sizeof( cmd.War ));
}

void CClient::addToolTip( const CObjBase * pObj, const TCHAR* pszText )
{
	if ( pObj == NULL )
		return;
	if ( pObj->IsChar())
		return; // no tips on chars.

	addPause(); // OSI does this, don't know if needed

	CCommand cmd;
	int i = CvtSystemToNUNICODE( cmd.ToolTip.m_utext, pszText, COUNTOF(cmd.ToolTip.m_utext));
	int len = ((i + 1) * sizeof(NCHAR)) + ( sizeof(cmd.ToolTip) - sizeof(cmd.ToolTip.m_utext));

	cmd.ToolTip.m_Cmd = XCMD_ToolTip;
	cmd.ToolTip.m_len = len;
	cmd.ToolTip.m_UID = pObj->GetUID();

	xSendPkt( &cmd, len );
}

bool CClient::addBookOpen( CItem * pBook )
{
	// word wrap is done when typed in the client. (it has a var size font)

	int iPagesNow = 0;
	bool fWritable;
	WORD nPages;
	CGString sTitle;
	CGString sAuthor;

	if ( pBook->IsBookSystem())
	{
		fWritable = false;

		CGString sSec;
		sSec.Format( "BOOK %li", pBook->m_itBook.m_TimeID );

		CScriptLock s;
		if ( g_Serv.ScriptLock( s, SCPFILE_BOOK_2, sSec ) == NULL )
			return false;

		if ( s.FindKey( "TITLE" ))
		{
			sTitle = s.GetArgStr();
			pBook->SetName( s.GetArgStr());	// Make sure the book is named.
		}
		else
		{
			sTitle = pBook->GetName();
		}

		if ( ! s.FindKey( "PAGES" ))
			return( false );

		nPages = s.GetArgVal();
		sAuthor = ( s.FindKey( "AUTHOR" )) ? s.GetArgStr() : "";
	}
	else
	{
		// User written book.
		CItemMessage * pText = dynamic_cast <CItemMessage *> (pBook);
		if ( pText == NULL ) 
			return false;

		fWritable = pText->IsBookWritable() ? true : false;	// Not sealed
		nPages = fWritable ? ( IsPriv(PRIV_GM) ? 64 : 16 ) : ( pText->GetPageCount());	// Max pages.
		sTitle = pText->GetName();
		sAuthor = (pText->m_sAuthor.IsEmpty()) ? "unknown" : (const TCHAR*) pText->m_sAuthor;

		if ( fWritable )	// For some reason we must send them now.
		{
			iPagesNow = pText->GetPageCount();
		}
	}

	CCommand cmd;
	if ( m_Crypt.GetClientVersion() >= 12600 )
	{
		cmd.BookOpen_v26.m_Cmd = XCMD_BookOpen;
		cmd.BookOpen_v26.m_UID = pBook->GetUID();
		cmd.BookOpen_v26.m_writable = fWritable ? 1 : 0;
		cmd.BookOpen_v26.m_NEWunk1 = fWritable ? 1 : 0;
		cmd.BookOpen_v26.m_pages = nPages;
		strcpy( cmd.BookOpen_v26.m_title, sTitle );
		strcpy( cmd.BookOpen_v26.m_author, sAuthor );
		xSendPkt( &cmd, sizeof( cmd.BookOpen_v26 ));
	}
	else
	{
		cmd.BookOpen_v25.m_Cmd = XCMD_BookOpen;
		cmd.BookOpen_v25.m_UID = pBook->GetUID();
		cmd.BookOpen_v25.m_writable = fWritable ? 1 : 0;
		cmd.BookOpen_v25.m_pages = nPages;
		strcpy( cmd.BookOpen_v25.m_title, sTitle );
		strcpy( cmd.BookOpen_v25.m_author, sAuthor );
		xSendPkt( &cmd, sizeof( cmd.BookOpen_v25 ));
	}

	// We could just send all the pages now if we want.
	if ( iPagesNow )
	{
		for ( int i=0; i<iPagesNow; i++ )
		{
			addBookPage( pBook, i+1 );
		}
	}

	return( true );
}

void CClient::addBookPage( const CItem * pBook, int iPage )
{
	DEBUG_CHECK( iPage > 0 );
	if ( iPage <= 0 ) 
		return;

	CCommand cmd;
	cmd.BookPage.m_Cmd = XCMD_BookPage;
	cmd.BookPage.m_UID = pBook->GetUID();
	cmd.BookPage.m_pages = 1;	// we can send multiple pages if we wish.
	cmd.BookPage.m_page[0].m_pagenum = iPage;	// 1 based page.

	int lines=0;
	int length=0;

	if ( pBook->IsBookSystem())
	{
		CGString sSec;
		sSec.Format( "BOOK %d %d", pBook->m_itBook.m_TimeID, iPage );

		CScriptLock s;
		if ( g_Serv.ScriptLock( s, SCPFILE_BOOK_2, sSec ) == NULL )
			return;

		while (s.ReadKey(false))
		{
			lines++;
			length += strcpylen( cmd.BookPage.m_page[0].m_text+length, s.GetKey()) + 1;
		}
	}
	else
	{
		// User written book pages.
		const CItemMessage * pText = dynamic_cast <const CItemMessage *> (pBook);
		if ( pText == NULL ) 
			return;
		iPage --;
		if ( iPage < pText->GetPageCount())
		{
			// Copy the pages to the book
			const TCHAR * pszText = pText->GetPageText(iPage);
			if ( pszText )
			{
				while (true)
				{
					TCHAR ch = pszText[ length ];
					if ( ch == '\t' )
					{
						ch = '\0';
						lines++;
					}
					cmd.BookPage.m_page[0].m_text[ length ] = ch;
					if ( pszText[ length ++ ] == '\0' )
					{
						if ( length > 1 ) lines ++;
						break;
					}
				}
			}
		}
	}

	length += sizeof( cmd.BookPage ) - sizeof( cmd.BookPage.m_page[0].m_text );
	cmd.BookPage.m_len = length;
	cmd.BookPage.m_page[0].m_lines = lines;

	xSendPkt( &cmd, length );
}

int CClient::Setup_FillCharList( CEventCharDef * pCharList, const CChar * pCharFirst )
{
	// list available chars for your account that are idle.
	ASSERT( m_pAccount );
	int j=0;

	if ( pCharFirst && m_pAccount->IsMyAccountChar( pCharFirst ))
	{
		m_tmSetupCharList[0] = pCharFirst->GetUID();
		strncpy( pCharList[0].m_name, pCharFirst->GetName(), sizeof( pCharList[0].m_name ));
		pCharList[0].m_pass[0] = '\0';
		j++;
	}

	for ( int k=0; k<SECTOR_QTY; k++ )
	{
		CChar * pChar = STATIC_CAST <CChar*> ( g_World.m_Sectors[k].m_Chars_Disconnect.GetHead());
		for ( ; pChar != NULL; pChar = pChar->GetNext())
		{
			if ( pCharFirst == pChar )
				continue;
			if ( ! m_pAccount->IsMyAccountChar( pChar ))
				continue;
			if ( j >= MAX_CHARS_PER_ACCT )
				break;

			m_tmSetupCharList[j] = pChar->GetUID();
			strncpy( pCharList[j].m_name, pChar->GetName(), sizeof( pCharList[0].m_name ));
			pCharList[j].m_pass[0] = '\0';

			if ( ++j >= g_Serv.m_iMaxCharsPerAccount )
				break;
		}
	}

	// always show max count for some stupid reason. (client bug)
	// pad out the rest of the chars.
	for ( ;j<MAX_CHARS_PER_ACCT;j++)
	{
		m_tmSetupCharList[j].ClearUID();
		pCharList[j].m_name[0] = '\0';
		pCharList[j].m_pass[0] = '\0';
	}

	return( MAX_CHARS_PER_ACCT );
}

void CClient::addCharList3()
{
	// just pop up a list of chars for this account.
	// GM's may switch chars on the fly.

	ASSERT( m_pAccount );

	CCommand cmd;
	cmd.CharList3.m_Cmd = XCMD_CharList3;
	cmd.CharList3.m_len = sizeof( cmd.CharList3 );
	cmd.CharList3.m_count = Setup_FillCharList( cmd.CharList3.m_char, m_pChar );
	cmd.CharList3.m_unk = 0;

	xSendPkt( &cmd, sizeof( cmd.CharList3 ));
	SetTargMode( TARGMODE_SETUP_CHARLIST );
}

void CClient::SetTargMode( TARGMODE_TYPE targmode, const TCHAR *pPrompt )
{
	// ??? Get rid of menu stuff if previous targ mode.
	// Can i close a menu ?
	// Cancel a cursor input.

	if ( GetTargMode() == targmode ) 
		return;

	if ( GetTargMode() != TARGMODE_NONE && targmode != TARGMODE_NONE )
	{
		// Just clear the old target mode
		addSysMessage( "Previous Targeting Cancelled" );
	}

	m_Targ_Mode = targmode;
	addSysMessage( (targmode == TARGMODE_NONE ) ? "Targeting Cancelled" : pPrompt );
}

void CClient::addPromptConsole( TARGMODE_TYPE targmode, const TCHAR *pPrompt )
{
	SetTargMode( targmode, pPrompt );

	CCommand cmd;
	cmd.Prompt.m_Cmd = XCMD_Prompt;
	cmd.Prompt.m_len = sizeof( cmd.Prompt );
	memset( cmd.Prompt.m_unk3, 0, sizeof(cmd.Prompt.m_unk3));
	cmd.Prompt.m_text[0] = '\0';

	xSendPkt( &cmd, cmd.Prompt.m_len );
}

void CClient::addTarget( TARGMODE_TYPE targmode, const TCHAR *pPrompt, bool fAllowGround ) // Send targetting cursor to client
{
	// a = 0, b = 0 = target any object static or dynamic - no terrain.
	// a = 0, b = 1 = target any object static or dynamic - no terrain.
	// a = 1, b = 0 = anything
	// a = 1, b = 1 = anything
	// Expect XCMD_Target back.

	SetTargMode( targmode, pPrompt );

	CCommand cmd;
	cmd.Target.m_Cmd = XCMD_Target;
	cmd.Target.m_fAllowGround = fAllowGround; // fAllowGround;	// 1=allow xyz, 0=objects only.
	cmd.Target.m_code = targmode ;	// 5=my id code for action.

	// ??? will this be selective for us ? objects only or chars only ? not on the ground (statics) ?
	cmd.Target.m_unk6 = 1; // // Not sure what this is.

	xSendPkt( &cmd, sizeof( cmd.Target ));
}

void CClient::addTargetDeed( const CItem * pDeed )
{
	// Place an item from a deed.

	ASSERT( m_Targ_UID == pDeed->GetUID());

	ITEMID_TYPE iddef = pDeed->m_itDeed.m_Type;

	if ( iddef == ITEMID_GUILDSTONE1 ||
		iddef == ITEMID_GUILDSTONE2 )
	{
		// Check if they are already in a guild first
		CItemStone * pStone = m_pChar->Guild_Find(MEMORY_GUILD);
		if (pStone)
		{
			addSysMessage( "You are already a member of a guild. Resign first!");
			return;
		}
	}

	m_tmUseItem.m_pParent = pDeed->GetParent();	// Cheat Verify.
	addTargetItems( TARGMODE_USE_ITEM, iddef );
}

bool CClient::addTargetItems( TARGMODE_TYPE targmode, ITEMID_TYPE id )
{
	// Add a list of items to place at target.

	const TCHAR * pszName;
	CItemBase * pBase;
	if ( id < ITEMID_TEMPLATE )
	{
		pBase = CItemBase::FindItemBase( id );
		if ( pBase == NULL ) 
			return false;
		pszName = CItemBase::GetNamePluralize( pBase->GetTypeName(), false );
	}
	else
	{
		pBase = NULL;
		pszName = "template";
	}

	CGString sPrompt;
	sPrompt.Format( "Where would you like to place the %s?", pszName );

	if ( CItemBase::IsMultiID( id ))	// a multi we get from Multi.mul
	{
		SetTargMode( targmode, sPrompt );

		CCommand cmd;
		cmd.TargetMulti.m_Cmd = XCMD_TargetMulti;
		cmd.TargetMulti.m_fAllowGround = 1;
		cmd.TargetMulti.m_code = targmode ;	// 5=my id code for action.
		memset( cmd.TargetMulti.m_zero6, 0, sizeof(cmd.TargetMulti.m_zero6));
		cmd.TargetMulti.m_id = id - ITEMID_MULTI;
		memset( cmd.TargetMulti.m_zero20, 0, sizeof(cmd.TargetMulti.m_zero20));

		xSendPkt( &cmd, sizeof( cmd.TargetMulti ));
		return true;
	}

	if ( ! m_Crypt.GetClientVersion() ||
		m_Crypt.GetClientVersion() >= 12603 )	
	{
		// preview not supported by this ver?
		addTarget( targmode, sPrompt, true );
		return true;
	}

	CCommand cmd;
	cmd.TargetItems.m_Cmd = XCMD_TargetItems;
	cmd.TargetItems.m_fAllowGround = 1;
	cmd.TargetItems.m_code = targmode;
	cmd.TargetItems.m_xOffset = 0;
	cmd.TargetItems.m_yOffset = 0;
	cmd.TargetItems.m_zOffset = 0;

	int iCount;
	CItemBaseMulti * pBaseMulti = dynamic_cast <CItemBaseMulti *>(pBase);
	if ( pBaseMulti )
	{
		iCount = pBaseMulti->m_Components.GetCount();
		for ( int i=0; i<iCount; i++ )
		{
			ASSERT( pBaseMulti->m_Components[i].m_id < ITEMID_MULTI );
			cmd.TargetItems.m_item[i].m_id = pBaseMulti->m_Components[i].m_id;
			cmd.TargetItems.m_item[i].m_dx = pBaseMulti->m_Components[i].m_dx;
			cmd.TargetItems.m_item[i].m_dy = pBaseMulti->m_Components[i].m_dy;
			cmd.TargetItems.m_item[i].m_dz = pBaseMulti->m_Components[i].m_dz;
			cmd.TargetItems.m_item[i].m_unk = 0;
		}
	}
	else
	{
		iCount = 1;
		cmd.TargetItems.m_item[0].m_id = id;
		cmd.TargetItems.m_item[0].m_dx = 0;
		cmd.TargetItems.m_item[0].m_dy = 0;
		cmd.TargetItems.m_item[0].m_dz = 0;
		cmd.TargetItems.m_item[0].m_unk = 0;
	}

	cmd.TargetItems.m_count = iCount;
	int len = ( sizeof(cmd.TargetItems) - sizeof(cmd.TargetItems.m_item)) + ( iCount * sizeof(cmd.TargetItems.m_item[0]));
	cmd.TargetItems.m_len = len;

	SetTargMode( targmode, sPrompt );
	xSendPkt( &cmd, len);

	return( true );
}

void CClient::addDyeOption( const CObjBase * pObj )
{
	// Put up the color chart for the client.
	// This results in a Event_Item_Dye message.
	ITEMID_TYPE id;
	if ( pObj->IsItem())
	{
		id = (dynamic_cast <const CItem*> (pObj))->GetDispID();
	}
	else
	{
		// Get the item equiv for the creature.
		id = (dynamic_cast <const CChar*> (pObj))->m_pDef->m_trackID;
	}

	CCommand cmd;
	cmd.DyeVat.m_Cmd = XCMD_DyeVat;
	cmd.DyeVat.m_UID = pObj->GetUID();
	cmd.DyeVat.m_zero5 = 0;
	cmd.DyeVat.m_id = id;
	xSendPkt( &cmd, sizeof( cmd.DyeVat ));
	SetTargMode( TARGMODE_DYE );
}

void CClient::addSkillWindow( SKILL_TYPE skill ) // Opens the skills list
{
	CCommand cmd;
	cmd.Skill.m_Cmd = XCMD_Skill;

	bool fVer12602 = (! m_Crypt.GetClientVersion() || m_Crypt.GetClientVersion() >= 12602);

	int len = ( fVer12602 ) ?
		sizeof(cmd.Skill) : sizeof(cmd.Skill_v261);

	CChar * pChar = m_Prop_UID.CharFind();
	if ( pChar == NULL ) pChar = m_pChar;

	if ( skill >= SKILL_QTY )
	{	// all skills
		cmd.Skill.m_single = 0;
		int i=0;
		for ( ; i<SKILL_QTY; i++ )
		{
			int iskillval = pChar->Skill_GetBase( (SKILL_TYPE) i);
			if ( fVer12602 )
			{
				cmd.Skill.skills[i].m_index = i+1;
				cmd.Skill.skills[i].m_val = pChar->Skill_GetAdjusted( (SKILL_TYPE) i);
				cmd.Skill.skills[i].m_valraw = iskillval;
				cmd.Skill.skills[i].m_lock = pChar->Skill_GetLock( (SKILL_TYPE) i);
			}
			else
			{
				cmd.Skill_v261.skills[i].m_index = i+1;
				cmd.Skill_v261.skills[i].m_val = iskillval;
			}
		}
		if ( fVer12602 )
		{
			cmd.Skill.skills[i].m_index = 0;	// terminator.
			len += ((SKILL_QTY-1) * sizeof(cmd.Skill.skills[0])) + sizeof(NWORD);
		}
		else
		{
			cmd.Skill_v261.skills[i].m_index = 0;	// terminator.
			len += ((SKILL_QTY-1) * sizeof(cmd.Skill_v261.skills[0])) + sizeof(NWORD);
		}
	}
	else
	{	// Just one skill update.
		cmd.Skill.m_single = 0xff;
		int iskillval = pChar->Skill_GetBase( skill );
		if ( fVer12602 )
		{
			cmd.Skill.skills[0].m_index = skill;
			cmd.Skill.skills[0].m_val = pChar->Skill_GetAdjusted( skill );
			cmd.Skill.skills[0].m_valraw = iskillval;
			cmd.Skill.skills[0].m_lock = pChar->Skill_GetLock( skill );
		}
		else
		{
			cmd.Skill_v261.skills[0].m_index = skill;
			cmd.Skill_v261.skills[0].m_val = iskillval;
		}
	}

	cmd.Skill.m_len = len;
	xSendPkt( &cmd, len );
}

void CClient::addPlayerSee( const CPointMap & ptold )
{
	// adjust to my new location.
	// What do I now see here ?

	// What new things do i see (on the ground) ?
	CWorldSearch AreaItems( m_pChar->GetTopPoint(), UO_MAP_VIEW_RADAR );
	while (true)
	{
		CItem * pItem = AreaItems.GetItem();
		if ( pItem == NULL ) 
			break;
		if ( ! CanSee( pItem )) 
			continue;

		// I didn't see it before
		if ( ptold.GetDist( pItem->GetTopPoint()) > UO_MAP_VIEW_SIZE )
		{
			addItem( pItem );
		}
	}

	// What new people do i see ?
	CWorldSearch AreaChars( m_pChar->GetTopPoint(), UO_MAP_VIEW_SIZE );
	AreaChars.SetInertView( IsPriv( PRIV_ALLSHOW ));	// show logged out chars?
	while (true)
	{
		CChar * pChar = AreaChars.GetChar();
		if ( pChar == NULL ) 
			break;
		if ( m_pChar == pChar ) 
			continue;	// I saw myself before.
		if ( ! CanSee( pChar )) 
			continue;

		if ( ptold.GetDist( pChar->GetTopPoint()) > UO_MAP_VIEW_SIZE )
		{
			addChar( pChar );
		}
	}
}

void CClient::addPlayerView( const CPointMap & ptold )
{
	// I moved = Change my point of view. Teleport etc..

	addPause();

	CCommand cmd;
	cmd.View.m_Cmd = XCMD_View;
	cmd.View.m_UID = m_pChar->GetUID();

	CREID_TYPE id;
	COLOR_TYPE color;
	GetAdjustedCharID( m_pChar, id, color );
	cmd.View.m_id = id;
	cmd.View.m_zero7 = 0;
	cmd.View.m_color = color;

	cmd.View.m_mode = m_pChar->GetModeFlag();

	CPointMap pt = m_pChar->GetTopPoint();
	cmd.View.m_x = pt.m_x;
	cmd.View.m_y = pt.m_y;
	cmd.View.m_zero15 = 0;
	cmd.View.m_dir = m_pChar->GetDirFlag();
	cmd.View.m_z = pt.m_z;
	xSendPkt( &cmd, sizeof( cmd.View ));

	m_WalkCount = -1;

	if ( ptold == pt ) 
		return;	// not a move.

	// What can i see here ?
	addPlayerSee( ptold );
}

void CClient::addReSync()
{
	// Reloads the client with all it needs.
	CPointMap ptold( 0xFFFF, 0xFFFF, 0 );
	addPlayerView( ptold );
	addChar( m_pChar );
}

void CClient::addCharStatWindow( CObjUID uid ) // Opens the status window
{
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL ) 
		return;

	CCommand cmd;
	cmd.Status.m_Cmd = XCMD_Status;
	cmd.Status.m_UID = pChar->GetUID();

	// WARNING: Char names must be <= 30 !!!
	// "kellen the magincia counsel me" > 30 char names !!
	strncpy( cmd.Status.m_name, pChar->GetName(), sizeof( cmd.Status.m_name ));
	cmd.Status.m_name[ sizeof( cmd.Status.m_name )-1 ] = '\0';

	cmd.Status.m_health = pChar->m_StatHealth;
	cmd.Status.m_maxhealth = pChar->Stat_Get(STAT_STR);

	// renamable ?
	if ( m_pChar != pChar &&	// can't rename self. it looks weird.
		pChar->NPC_IsOwnedBy( m_pChar ) && // my pet.
		! pChar->m_pDef->GetHireDayWage())	// can't rename hirelings
	{
		cmd.Status.m_perm = 0xFF;	// I can rename them
	}
	else
	{
		cmd.Status.m_perm = 0x00;
	}

	// Dont bother sending the rest of this if not my self.
	cmd.Status.m_ValidStats = 0;

	if ( pChar == m_pChar )
	{
		m_fUpdateStats = false;

		cmd.Status.m_len = sizeof( cmd.Status );
		cmd.Status.m_ValidStats |= 1;	// valid stats
		cmd.Status.m_sex = ( CCharBase::IsFemaleID( pChar->GetDispID())) ? 1 : 0;
		cmd.Status.m_str = pChar->Stat_Get(STAT_STR);
		cmd.Status.m_dex = pChar->Stat_Get(STAT_DEX);
		cmd.Status.m_int = pChar->Stat_Get(STAT_INT);
		cmd.Status.m_stam =	pChar->m_StatStam ;
		cmd.Status.m_maxstam = pChar->Stat_Get(STAT_DEX);
		cmd.Status.m_mana =	pChar->m_StatMana ;
		cmd.Status.m_maxmana = pChar->Stat_Get(STAT_INT);
		cmd.Status.m_gold = pChar->ContentCount( ITEMID_GOLD_C1 );	/// ??? optimize this count is too often.
		cmd.Status.m_armor = pChar->m_defense + pChar->m_pDef->m_defense;
		cmd.Status.m_weight = pChar->GetTotalWeight() / WEIGHT_UNITS;
	}
	else
	{
		cmd.Status.m_len = ((BYTE*)&(cmd.Status.m_sex)) - ((BYTE*)&(cmd.Status));
	}
	xSendPkt( &cmd, cmd.Status.m_len );
}

void CClient::addSpellbookOpen( CItem * pBook )
{
	// NOTE: if the spellbook item is not present on the client it will crash.
	// count what spells I have.

	if ( pBook->GetDispID() == ITEMID_SPELLBOOK2 )
	{
		// weird client bug.
		pBook->SetDispID( ITEMID_SPELLBOOK );
		pBook->Update();
		return;
	}

	int count=0;
	int i=SPELL_Clumsy;
	for ( ;i<SPELL_BOOK_QTY;i++ )
	{
		if ( pBook->IsSpellInBook( (SPELL_TYPE) i ))
		{
			count++;
		}
	}

	addPause();	// try to get rid of the double open.
	addOpenGump( pBook, GUMP_OPEN_SPELLBOOK );
	if (!count) 
		return;

	CCommand cmd;
	int len = sizeof( cmd.Content ) - sizeof(cmd.Content.items) + ( count * sizeof(cmd.Content.items[0]));
	cmd.Content.m_Cmd = XCMD_Content;
	cmd.Content.m_len = len;
	cmd.Content.m_count = count;

	int j=0;
	for ( i=SPELL_Clumsy; i<SPELL_BOOK_QTY; i++ )
		if ( pBook->IsSpellInBook( (SPELL_TYPE) i ))
	{
		cmd.Content.items[j].m_UID = UID_ITEM + UID_FREE + i; // just some unused uid.
		cmd.Content.items[j].m_id = g_Serv.m_SpellDefs[i]->m_ScrollID;	// scroll id. (0x1F2E)
		cmd.Content.items[j].m_zero6 = 0;
		cmd.Content.items[j].m_amount = i;
		cmd.Content.items[j].m_x = 0x48;	// may not mean anything ?
		cmd.Content.items[j].m_y = 0x7D;
		cmd.Content.items[j].m_UIDCont = pBook->GetUID();
		cmd.Content.items[j].m_color = COLOR_DEFAULT;
		j++;
	}

	xSendPkt( &cmd, len );
}

void CClient::addScroll( const TCHAR * pszText )
{
	// ??? We need to do word wrap here. But it has a var size font !
	CCommand cmd;
	cmd.Scroll.m_Cmd = XCMD_Scroll;
	cmd.Scroll.m_type = SCROLL_TYPE_NOTICE;
	cmd.Scroll.m_scrollID = 0;

	int length = strcpylen( cmd.Scroll.m_text, pszText );
	cmd.Scroll.m_lentext = length;
	length += sizeof( cmd.Scroll ) - sizeof( cmd.Scroll.m_text );
	cmd.Scroll.m_len = length;

	xSendPkt( &cmd, length );
}

void CClient::addScrollFile( CScript &s, SCROLL_TYPE type, DWORD scrollID, const TCHAR * pszHeader )
{
	CCommand cmd;
	cmd.Scroll.m_Cmd = XCMD_Scroll;
	cmd.Scroll.m_type = type;
	cmd.Scroll.m_scrollID = scrollID;	// for ScrollClose ?

	int length=0;

	if ( pszHeader )
	{
		length = strcpylen( &cmd.Scroll.m_text[0], pszHeader );
		cmd.Scroll.m_text[length++] = 0x0d;
		length += strcpylen( &cmd.Scroll.m_text[length], "  " );
		cmd.Scroll.m_text[length++] = 0x0d;
	}

	while (s.ReadKey(false))
	{
		if ( ! strnicmp( s.GetKey(), ".line", 5 ))	// just a blank line.
		{
			length += strcpylen( &cmd.Scroll.m_text[length], " " );
		}
		else
		{
			length += strcpylen( &cmd.Scroll.m_text[length], s.GetKey());
		}
		cmd.Scroll.m_text[length++] = 0x0d;
	}

	cmd.Scroll.m_lentext = length;
	length += sizeof( cmd.Scroll ) - sizeof( cmd.Scroll.m_text );
	cmd.Scroll.m_len = length;

	xSendPkt( &cmd, length );
}

void CClient::addScrollFile( const TCHAR * pszSec, SCROLL_TYPE type, DWORD scrollID )
{
	//
	// type = 0 = TIPS
	// type = 2 = UPDATES

	CScriptLock s;
	if ( g_Serv.ScriptLock( s, SCPFILE_BOOK_2, pszSec ) == NULL )
		return;
	addScrollFile( s, type, scrollID );
}

void CClient::addVendorClose( const CChar * pVendor )
{
	// Clear the vendor display.
	CCommand cmd;
	cmd.VendorBuy.m_Cmd = XCMD_VendorBuy;
	cmd.VendorBuy.m_len = sizeof( cmd.VendorBuy );
	cmd.VendorBuy.m_UIDVendor = pVendor->GetUID();
	cmd.VendorBuy.m_flag = 0;	// 0x00 = no items following, 0x02 - items following
	xSendPkt( &cmd, sizeof( cmd.VendorBuy ));
}

int CClient::addShopItems( CChar * pVendor, LAYER_TYPE layer )
{
	// Player buying from vendor.
	// Show the Buy menu for the contents of this container
	// RETURN: the number of items in container.
	//   < 0 = error.
	CItemContainer * pContainer = pVendor->GetBank( layer );
	if ( pContainer == NULL ) 
		return( -1 );

	addItem( pContainer );
	addContents( pContainer, false, false, true );

	// add the names and prices for stuff.
	CCommand cmd;
	cmd.VendOpenBuy.m_Cmd = XCMD_VendOpenBuy;
	cmd.VendOpenBuy.m_VendorUID = pContainer->GetUID();
	int len = sizeof( cmd.VendOpenBuy ) - sizeof(cmd.VendOpenBuy.items);

	CCommand * pCur = &cmd;
	int count = 0;
	for ( CItem* pItem = pContainer->GetContentHead(); pItem!=NULL; pItem = pItem->GetNext())
	{
		if ( ! pItem->GetAmount()) 
			continue;
		// long lPrice = pItem->GetVendorPrice(true);
		CItemVendable * pVendItem = dynamic_cast <CItemVendable *> (pItem);
		if ( pVendItem == NULL )
			continue;
		long lPrice = pVendItem->GetBuyPrice();
		if ( pVendItem->GetQuality() && layer != LAYER_VENDOR_EXTRA )
		{
			lPrice = IMULDIV( lPrice, pVendItem->GetQuality(), 100 );
		}
		if ( ! lPrice )
		{
			pVendItem->IsValidNPCSaleItem();
			// This stuff will show up anyhow. unpriced.
			// continue;
			lPrice = 100000;
		}
		pCur->VendOpenBuy.items[0].m_price = lPrice;
		int lenname = strcpylen( pCur->VendOpenBuy.items[0].m_text, pVendItem->GetName());
		pCur->VendOpenBuy.items[0].m_len = lenname + 1;
		lenname += sizeof( cmd.VendOpenBuy.items[0] );
		len += lenname;
		pCur = (CCommand *)( ((BYTE*) pCur ) + lenname );
		if ( ++count >= MAX_ITEMS_CONT ) break;
	}

	// if ( ! count ) return( 0 );

	cmd.VendOpenBuy.m_len = len;
	cmd.VendOpenBuy.m_count = count;
	xSendPkt( &cmd, len );
	return( count );
}

bool CClient::addShopMenuBuy( CChar * pVendor )
{
	// Try to buy stuff that the vendor has.
	addPause();
	addChar( pVendor );			// Send the NPC again to make sure info is current. (OSI does this we might not have to)

	int iTotal = 0;
	int iRes = addShopItems( pVendor, LAYER_VENDOR_STOCK );
	if ( iRes < 0 )
		return( false );
	iTotal = iRes;
	iRes = addShopItems( pVendor, LAYER_VENDOR_EXTRA );
	if ( iRes < 0 )
		return( false );
	iTotal += iRes;
	if ( iTotal <= 0 ) 
		return( false );

	addOpenGump( pVendor, GUMP_VENDOR_RECT );
	addCharStatWindow( m_pChar->GetUID());	// Make sure the gold total has been updated.
	return( true );
}

int CClient::addShopMenuSellFind( CItemContainer * pSearch, CItemContainer * pVend1, CItemContainer * pVend2, CCommand * & pCur )
{
	// What things do you have in your inventory that the vendor would want ?
	// Search sub containers if necessary.
	// RETURN: How many items did we find.

	ASSERT(pSearch);

	int iCount = 0;

	for ( CItem* pItem = pSearch->GetContentHead() ; pItem!=NULL; pItem = pItem->GetNext())
	{
		// We won't buy containers with items in it.
		// But we will look in them.
		CItemContainer* pContItem = dynamic_cast <CItemContainer*>(pItem);
		if ( pContItem != NULL && pContItem->GetCount())
		{
			if ( pContItem->IsSearchable())
			{
				iCount += addShopMenuSellFind( pContItem, pVend1, pVend2, pCur );
			}
			continue;
		}
		CItemVendable * pVendItem = dynamic_cast <CItemVendable *> (pItem);
		if ( pVendItem == NULL )
			continue;
		CItemVendable * pItemSell = dynamic_cast <CItemVendable *> (pVend1->ContentFind( pItem->GetID()));
		if ( pItemSell == NULL )
		{
			pItemSell = dynamic_cast <CItemVendable *> (pVend2->ContentFind( pItem->GetID()));
			if ( pItemSell == NULL )
				continue;
		}
		if ( pVendItem->m_type != pItemSell->m_type ) 
			continue;
		if ( ! pVendItem->IsValidSaleItem( false )) 
			continue;

		pCur->VendOpenSell.items[0].m_UID = pVendItem->GetUID();
		pCur->VendOpenSell.items[0].m_id = pVendItem->GetDispID();

		COLOR_TYPE wColor = pVendItem->GetColor();
		if ( wColor > COLOR_QTY )  wColor &= COLOR_MASK;

		pCur->VendOpenSell.items[0].m_color = wColor;
		pCur->VendOpenSell.items[0].m_amount = pVendItem->GetAmount();
		if ( pVendItem->GetQuality() == 0 )
			pCur->VendOpenSell.items[0].m_price = pItemSell->GetSellPrice();
		else
			pCur->VendOpenSell.items[0].m_price = pItemSell->GetSellPrice() * pVendItem->GetQuality() / 100;

		int lenname = strcpylen( pCur->VendOpenSell.items[0].m_text, pVendItem->GetName());
		pCur->VendOpenSell.items[0].m_len = lenname + 1;
		pCur = (CCommand *)( ((BYTE*) pCur ) + lenname + sizeof( pCur->VendOpenSell.items[0] ));

		if ( ++iCount >= MAX_ITEMS_CONT ) break;	// 75
	}

	return( iCount );
}

bool CClient::addShopMenuSell( CChar * pVendor )
{
	// What things do you have in your inventory that the vendor would want ?
	// Should end with a returned Event_VendorSell()

	ASSERT(pVendor);
	addPause();
	CItemContainer * pContainer1 = pVendor->GetBank( LAYER_VENDOR_BUYS );
	addItem( pContainer1 );
	CItemContainer * pContainer2 = pVendor->GetBank( LAYER_VENDOR_STOCK );
	addItem( pContainer2 );
	CItemContainer * pContainer3 = pVendor->GetBank( LAYER_VENDOR_EXTRA );
	addItem( pContainer3 );	// do this or crash client.

	if ( pVendor->IsStat( STATF_Pet ))	// Player vendor.
	{
		pContainer2 = pContainer3;
	}

	CCommand cmd;
	cmd.VendOpenSell.m_Cmd = XCMD_VendOpenSell;
	cmd.VendOpenSell.m_UIDVendor = pVendor->GetUID();

	CCommand * pCur = (CCommand *)((BYTE*) &cmd );
	int iCount = addShopMenuSellFind( m_pChar->GetPackSafe(), pContainer1, pContainer2, pCur );
	if ( ! iCount )
	{
		pVendor->Speak( "Thou doth posses nothing of interest to me." );
		return( false );
	}

	cmd.VendOpenSell.m_len = (((BYTE*)pCur) - ((BYTE*) &cmd )) + sizeof( cmd.VendOpenSell ) - sizeof(cmd.VendOpenSell.items);
	cmd.VendOpenSell.m_count = iCount;

	xSendPkt( &cmd, cmd.VendOpenSell.m_len );
	return( true );
}

void CClient::addBankOpen( CChar * pChar, LAYER_TYPE layer )
{
	// open it up for this pChar.
	ASSERT( pChar );
	CItemContainer * pBankBox = pChar->GetBank(layer);
	if ( layer == LAYER_BANKBOX )
	{
		CGString sMsg;
		sMsg.Format( "You have %d stones in your bank box", pBankBox->GetWeight()/WEIGHT_UNITS );
		addSysMessage( sMsg );
	}
	addItem( pBankBox );	// may crash client if we dont do this.
	addContainerSetup( pBankBox );

	if ( pBankBox->GetID() == ITEMID_BANK_BOX ||
		pBankBox->GetID() == ITEMID_VENDOR_BOX )
	{
		// these are special. they can only be opened near the designated opener.
		// see CanTouch
		pBankBox->m_itEqBankBox.m_Open_Point = pChar->GetTopPoint();
	}
}

void CClient::addMap( CItemMap * pMap )
{
	// Make player drawn maps possible. (m_map_type=0) ???

	if ( pMap == NULL )
	{
blank_map:
		addSysMessage( "This map is blank." );
		return;
	}

	DEBUG_CHECK( pMap->m_type == ITEM_MAP );

	CRectMap rect;
	rect.SetRect( pMap->m_itMap.m_left,
		pMap->m_itMap.m_top,
		pMap->m_itMap.m_right,
		pMap->m_itMap.m_bottom );

	if ( ! rect.IsValid() || rect.IsEmpty())
	{
		goto blank_map;
	}

	addPause();

	CCommand cmd;
	cmd.MapDisplay.m_Cmd = XCMD_MapDisplay;
	cmd.MapDisplay.m_UID = pMap->GetUID();
	cmd.MapDisplay.m_Gump_Corner = GUMP_MAP_2_NORTH;
	cmd.MapDisplay.m_x_ul = rect.m_left;
	cmd.MapDisplay.m_y_ul = rect.m_top;
	cmd.MapDisplay.m_x_lr = rect.m_right;
	cmd.MapDisplay.m_y_lr = rect.m_bottom;
	cmd.MapDisplay.m_xsize = 0xc8;	// ??? we could make bigger maps ?
	cmd.MapDisplay.m_ysize = 0xc8;
	xSendPkt( &cmd, sizeof( cmd.MapDisplay ));

	addMapMode( pMap, MAP_UNSENT, false );

	// Now show all the pins
	cmd.MapEdit.m_Cmd = XCMD_MapEdit;
	cmd.MapEdit.m_UID = pMap->GetUID();
	cmd.MapEdit.m_Mode = 0x1;	// MAP_PIN?
	cmd.MapEdit.m_Req = 0x00;

	for ( int i=0; i < pMap->m_Pins.GetCount(); i++ )
	{
		cmd.MapEdit.m_pin_x = pMap->m_Pins[i].m_x;
		cmd.MapEdit.m_pin_y = pMap->m_Pins[i].m_y;
		xSendPkt( &cmd, sizeof( cmd.MapEdit ));
	}
}

void CClient::addMapMode( CItemMap * pMap, MAPCMD_TYPE iType, bool fEdit )
{
	// NOTE: MAPMODE_* depends on who is looking. Multi clients could interfere with each other ?

	ASSERT( pMap );
	DEBUG_CHECK( pMap->m_type == ITEM_MAP );
	pMap->m_fPlotMode = fEdit;

	CCommand cmd;
	cmd.MapEdit.m_Cmd = XCMD_MapEdit;
	cmd.MapEdit.m_UID = pMap->GetUID();
	cmd.MapEdit.m_Mode = iType;
	cmd.MapEdit.m_Req = fEdit;
	cmd.MapEdit.m_pin_x = 0;
	cmd.MapEdit.m_pin_y = 0;
	xSendPkt( &cmd, sizeof( cmd.MapEdit ));
}

void CClient::addBulletinBoard( const CItemContainer * pBoard )
{
	// Open up the bulletin board and all it's messages
	ASSERT( pBoard );
	DEBUG_CHECK( pBoard->m_type == ITEM_BBOARD );

	addPause();

	CCommand cmd;

	// Give the bboard name.
	cmd.BBoard.m_Cmd = XCMD_BBoard;
	int len = strcpylen( (TCHAR *) cmd.BBoard.m_data, pBoard->GetName(), MAX_NAME_SIZE );
	len += sizeof(cmd.BBoard);
	cmd.BBoard.m_len = len;
	cmd.BBoard.m_flag = 0; 	// 3= 0=board name, 4=associate message, 1=message header, 2=message body
	cmd.BBoard.m_UID = pBoard->GetUID(); // 4-7 = UID for the bboard.
	xSendPkt( &cmd, len );

	// Send Content messages for all the items on the bboard.
	// Not sure what x,y are here, date/time maybe ?
	addContents( pBoard, false, false, false );

	// The client will now ask for the headers it wants.
}

bool CClient::addBBoardMessage( const CItemContainer * pBoard, BYTE flag, CObjUID uidMsg )
{
	CItemMessage * pMsgItem = dynamic_cast <CItemMessage *> ( uidMsg.ItemFind());
	if ( ! pBoard->IsItemInContainer( pMsgItem ))
		return( false );

	addPause();

	// Send back the message header and/or body.
	CCommand cmd;
	cmd.BBoard.m_Cmd = XCMD_BBoard;
	cmd.BBoard.m_flag = ( flag == 3 ) ? 2 : 1;	// 3= 0=board name, 4=request associate header, 1=message header, 2=message body
	cmd.BBoard.m_UID = pBoard->GetUID();	// 4-7 = UID for the bboard.

	int len = 4;
	PACKDWORD(cmd.BBoard.m_data+0,pMsgItem->GetUID());

	if ( m_bin.BBoard.m_flag == 4 )
	{
		// just the header has this ? (replied to message?)
		PACKDWORD(cmd.BBoard.m_data+4,0);
		len += 4;
	}

	// author name. if it has one.
	if ( pMsgItem->m_sAuthor.IsEmpty())
	{
		cmd.BBoard.m_data[len++] = 0x01;
		cmd.BBoard.m_data[len++] = 0;
	}
	else
	{
		CChar * pChar = pMsgItem->m_uidLink.CharFind();
		if ( pChar == NULL )	// junk it if bad author. (deleted?)
		{
			pMsgItem->Delete();
			return false;
		}
		const TCHAR * pszName = pMsgItem->m_sAuthor;
		int lenstr = strlen(pszName) + 1;
		cmd.BBoard.m_data[len++] = lenstr;
		strcpy( (TCHAR*) &cmd.BBoard.m_data[len], pszName);
		len += lenstr;
	}

	// Pad this out with spaces to indent next field.
	int lenstr = strlen( pMsgItem->GetName()) + 1;
	cmd.BBoard.m_data[len++] = lenstr;
	strcpy( (TCHAR*) &cmd.BBoard.m_data[len], pMsgItem->GetName());
	len += lenstr;

	// Get the BBoard message time stamp. m_itBook.m_TimeID
	CGString sDate;
	sDate.Format( "Day %d", ( g_World.GetGameWorldTime( pMsgItem->m_itBook.m_TimeID &~ 0x80000000 ) / (24*60)) % 365 );
	lenstr = sDate.GetLength()+1;
	cmd.BBoard.m_data[len++] = lenstr;
	strcpy( (TCHAR*) &cmd.BBoard.m_data[len], sDate );
	len += lenstr;

	if ( m_bin.BBoard.m_flag == 3 )
	{
		// request for full message body
		//
		PACKDWORD(&(cmd.BBoard.m_data[len]),0);
		len += 4;

		// Pack the text into seperate lines.
		int lines = pMsgItem->GetPageCount();

		// number of lines.
		PACKWORD(&(cmd.BBoard.m_data[len]),lines);
		len += 2;

		// Now pack all the lines
		for ( int i=0; i<lines; i++ )
		{
			const TCHAR * pszText = pMsgItem->GetPageText(i);
			if ( pszText == NULL ) 
				continue;
			lenstr = strlen(pszText)+1;
			cmd.BBoard.m_data[len++] = lenstr;
			strcpy( (TCHAR*) &cmd.BBoard.m_data[len], pszText );
			len += lenstr;
		}
	}

	len = sizeof( cmd.BBoard ) - sizeof( cmd.BBoard.m_data ) + len;
	cmd.BBoard.m_len = len;
	xSendPkt( &cmd, len );
	return( true );
}

void CClient::addItemMenu( const CMenuItem * item, int count )
{
	// We must set GetTargMode() to show what mode we are in for menu select.
	// Will result in Event_MenuChoice()
	// cmd.ItemMenu.

	if ( ! count ) return;

	CCommand cmd;
	cmd.MenuItems.m_Cmd = XCMD_MenuItems;
	cmd.MenuItems.m_UID = m_pChar->GetUID();
	cmd.MenuItems.m_menuid = item[0].m_id;

	int len = sizeof( cmd.MenuItems ) - sizeof( cmd.MenuItems.items );
	int lenttitle = item[0].m_text.GetLength();
	cmd.MenuItems.m_lenname = lenttitle;
	strcpy( cmd.MenuItems.m_name, item[0].m_text );

	lenttitle --;
	len += lenttitle;
	CCommand * pCmd = (CCommand *)(((BYTE*)&cmd) + lenttitle );
	pCmd->MenuItems.m_count = count;

	// Strings in here and NOT null terminated.
	for ( int i=1; i<=count; i++ )
	{
		int lenitem = item[i].m_text.GetLength();
		if ( lenitem <= 0 || lenitem >= 256 )
		{
			DEBUG_ERR(("Bad option length %d in menu item %d\n", lenitem, i ));
			continue;
		}

		pCmd->MenuItems.items[0].m_id = item[i].m_id; // image next to menu.
		pCmd->MenuItems.items[0].m_zero2 = 0;	// check or not ?
		pCmd->MenuItems.items[0].m_lentext = lenitem;
		strcpy( pCmd->MenuItems.items[0].m_name, item[i].m_text );

		lenitem += sizeof( cmd.MenuItems.items[0] ) - 1;
		pCmd = (CCommand *)(((BYTE*)pCmd) + lenitem );
		len += lenitem;
	}

	cmd.MenuItems.m_len = len;
	xSendPkt( &cmd, len );

	SetTargMode( (TARGMODE_TYPE) item[0].m_id );
}

void CClient::addGumpTextDisp( const CObjBase * pObj, GUMP_TYPE gump, const TCHAR * pszName, const TCHAR * pszText )
{
	DEBUG_CHECK( gump < GUMP_QTY );

	// ??? how do we control where exactly the text goes ??

	// display a gump with some text on it.
	int lenname = pszName ? strlen( pszName ) : 0 ;
	lenname++;
	int lentext = pszText ? strlen( pszText ) : 0 ;
	lentext++;

	CCommand cmd;

	int len = ( sizeof(cmd.GumpTextDisp) - 2 ) + lenname + lentext;

	cmd.GumpTextDisp.m_Cmd = XCMD_GumpTextDisp;
	cmd.GumpTextDisp.m_len = len;
	cmd.GumpTextDisp.m_UID = pObj ? ((DWORD)( pObj->GetUID())) : UID_CLEAR;
	cmd.GumpTextDisp.m_gump = gump;
	cmd.GumpTextDisp.m_len_unktext = lenname;
	cmd.GumpTextDisp.m_unk11 = 0;	// ? not COLOR_TYPE, not x,
	strcpy( cmd.GumpTextDisp.m_unktext, ( pszName ) ? pszName : "" );

	CCommand * pCmd = (CCommand *)(((BYTE*)(&cmd))+lenname-1);
	strcpy( pCmd->GumpTextDisp.m_text, ( pszText ) ? pszText : "" );
	xSendPkt( &cmd, len );
}

void CClient::addGumpInputBox( TARGMODE_TYPE id, BYTE parent, BYTE button,
	bool fCancel, INPUTBOX_STYLE style,
	DWORD mask,
	const TCHAR *pszText1,
	const TCHAR *pszText2 )
{
	// Should result in Event_GumpTextIn
	CCommand cmd;

	cmd.GumpInputBox.m_Cmd = XCMD_GumpInputBox;
	cmd.GumpInputBox.m_len = sizeof(cmd.GumpInputBox);

	cmd.GumpInputBox.m_dialogID = id;
	cmd.GumpInputBox.m_parentID = parent;	// this will be truncated to a byte on return.
	cmd.GumpInputBox.m_buttonID = button;

	cmd.GumpInputBox.m_textlen1 = sizeof(cmd.GumpInputBox.m_text1);
	strcpy( cmd.GumpInputBox.m_text1, pszText1 );

	cmd.GumpInputBox.m_cancel = fCancel ? 1 : 0;
	cmd.GumpInputBox.m_style = (BYTE) style;
	cmd.GumpInputBox.m_mask = mask;
	cmd.GumpInputBox.m_textlen2 = sizeof(cmd.GumpInputBox.m_text2);
	strcpy( cmd.GumpInputBox.m_text2, "" );	// just in case.

	switch ( style )
	{
	case STYLE_NOEDIT: // None
		break;
	case STYLE_TEXTEDIT: // Text
		sprintf(cmd.GumpInputBox.m_text2,
			"%s (%i chars max)", pszText2, mask);
		break;
	case STYLE_NUMEDIT: // Numeric
		sprintf(cmd.GumpInputBox.m_text2,
			"%s (0 - %i)", pszText2, mask);
		break;
	}
	xSendPkt( &cmd, sizeof(cmd.GumpInputBox));
	SetTargMode( TARGMODE_GUMP_VAL );
}

void CClient::addEnableChatButton()
{
	CCommand cmd;
	cmd.ChatEnable.m_Cmd = XCMD_ChatEnable;
	cmd.ChatEnable.m_enable = 0x01;
	xSendPkt( &cmd, sizeof( cmd.ChatEnable ));
	xSendPkt( &cmd, sizeof( cmd.ChatEnable )); // for some reason OSI always sends this twice????

	cmd.ReDrawAll.m_Cmd = XCMD_ReDrawAll;
	xSendPkt( &cmd, sizeof(cmd.ReDrawAll)); // who knows what this does?
}

void CClient::addChatSystemMessage( CHATMSG_TYPE iType, const TCHAR * pszName1, const TCHAR * pszName2, const char * pszLang)
{
	CCommand cmd;
	cmd.ChatReq.m_Cmd = XCMD_ChatReq;
	cmd.ChatReq.m_subcmd = iType;

	if ( pszLang == NULL ) pszLang = "enu";
	if (iType >= CHATMSG_PlayerTalk && iType <= CHATMSG_PlayerPrivate) // These need the language stuff
		strcpy(cmd.ChatReq.m_lang, pszLang ); // unicode support: pszLang
	else
		strcpy(cmd.ChatReq.m_lang, "");
	cmd.ChatReq.m_lang[ sizeof( cmd.ChatReq.m_lang-1 ) ] = '\0';

	// ? If we're sending out player names, prefix name with moderator status

	if ( pszName1 == NULL ) pszName1 = "";
	int len1 = CvtSystemToNUNICODE( cmd.ChatReq.m_uname, pszName1, MAX_TALK_BUFFER );

	if ( pszName2 == NULL ) pszName2 = "";
	int len2 = CvtSystemToNUNICODE( cmd.ChatReq.m_uname+len1+1, pszName2, MAX_TALK_BUFFER );

	int len = sizeof(cmd.ChatReq) + (len1*2) + (len2*2);
	cmd.ChatReq.m_len = len;
	xSendPkt( &cmd, len );
}

void CClient::addGumpMenu( TARGMODE_TYPE dwGumpID, const CGString * psControls, int iControls, const CGString * psText, int iTexts, int x, int y, CObjBase * pObj )
{
	// Add a generic GUMP menu.
	// Should return a Event_GumpButton
	// NOTE: These packets can get rather LARGE.
	// x,y = where on the screen ?

	if ( pObj == NULL ) pObj = m_pChar;

	int lengthControls=1;
	int i=0;
	for ( ; i < iControls; i++)
	{
		lengthControls += psControls[i].GetLength() + 2;
	}

	int lengthText = lengthControls + 20 + 3;
	for ( i=0; i < iTexts; i++)
	{
		int lentext2 = psText[i].GetLength();
		DEBUG_CHECK( lentext2 < MAX_TALK_BUFFER );
		lengthText += (lentext2*2)+2;
	}

	// Send the fixed length stuff
	CCommand cmd;
	cmd.GumpDialog.m_Cmd = XCMD_GumpDialog;
	cmd.GumpDialog.m_len = lengthText;
	cmd.GumpDialog.m_UID = pObj->GetUID();
	cmd.GumpDialog.m_gump = dwGumpID;
	cmd.GumpDialog.m_x = x;
	cmd.GumpDialog.m_y = y;
	cmd.GumpDialog.m_lenCmds = lengthControls;
	xSend( &cmd, 21 );

	for ( i=0; i<iControls; i++)
	{
		CGString sTmp;
		int len = sTmp.Format( "{%s}", psControls[i] );
		xSend( sTmp, len );
	}

	// Pack up the variable length stuff
	BYTE Pkt_gump2[3];
	Pkt_gump2[0] = '\0';
	PACKWORD( &Pkt_gump2[1], iTexts );
	xSend( Pkt_gump2, 3);

	// Pack in UNICODE type format.
	for ( i=0; i < iTexts; i++)
	{
		int len1 = psText[i].GetLength();

		NWORD len2;
		len2 = len1;
		xSend( &len2, sizeof(NWORD));
		if ( len1 )
		{
			NCHAR szTmp[MAX_TALK_BUFFER];
			int len3 = CvtSystemToNUNICODE( szTmp, psText[i], COUNTOF(szTmp));
			xSend( szTmp, len2*sizeof(NCHAR));
		}
	}
	SetTargMode( dwGumpID );
}

bool CClient::Gump_FindSection( CScriptLock & s, TARGMODE_TYPE targ, const TCHAR * pszType )
{
	TCHAR szSection[ _MAX_PATH ];
	int len = sprintf( szSection, "GUMP %d", targ );

	if ( pszType )
	{
		szSection[len++] = ' ';
		strcpy( szSection+len, pszType );
	}

	return( g_Serv.ScriptLock( s, SCPFILE_GUMP_2, szSection ) != NULL );
}

void CClient::Gump_Button( TARGMODE_TYPE targ, DWORD dwButtonID, CObjBase * pObj )
{
	// one of the gump buttons was pressed.
	if ( pObj == NULL )		// object is gone ?
		return;

	CScriptLock s;
	if ( ! Gump_FindSection( s, targ, "BUTTON" ))
	{
		return;
	}

	while ( s.ReadKeyParse())
	{
		if ( ! s.IsKeyHead( "ON", 2 ))
			continue;
		if ( s.GetArgVal() != dwButtonID )
			continue;

		TRIGRET_TYPE iRet = pObj->OnTriggerRun( s, TRIGRUN_SECTION_TRUE, m_pChar );
		return;
	}
}

bool CClient::Gump_Setup( TARGMODE_TYPE targ, CObjBase * pObj )
{
	if ( pObj == NULL )
		return( false );

	CScriptLock s;
	if ( ! Gump_FindSection( s, targ, NULL ))
	{
		return false;
	}

	// read the size.
	if ( ! s.ReadKey())
	{
		return( false );
	}

	TCHAR * pszKey = s.GetKey();
	int x = Exp_GetVal(pszKey);
	int y = Exp_GetVal(pszKey);

	CGString sControls[1024];
	int iControls = 0;
	while ( s.ReadKey())
	{
		if ( iControls >= COUNTOF(sControls)-1 )
			break;
		pObj->ParseText( s.GetKey(), m_pChar );
		sControls[iControls++] = s.GetKey();
	}

	if ( ! Gump_FindSection( s, targ, "TEXT" ))
	{
		return false;
	}

	CGString sText[512];
	int iTexts=0;
	while ( s.ReadKey())
	{
		if ( iTexts >= COUNTOF(sText)-1 )
			break;
		pObj->ParseText( s.GetKey(), m_pChar );
		sText[iTexts++] = s.GetKey();
	}

	// Now pack it up to send,
	addGumpMenu( targ, sControls, iControls, sText, iTexts, x, y, pObj );
	return( true );
}

void CClient::addGumpPropMenu( TARGMODE_TYPE menuid )
{
	CObjBase * pObj = m_Prop_UID.ObjFind();
	if ( pObj == NULL )
		return;
	if ( menuid == TARGMODE_GUMP_PROP1 &&
		pObj->IsItem())
	{
		menuid = TARGMODE_GUMP_PROP4;
	}
	Gump_Setup( menuid, pObj );
}

void CClient::add_Admin_Dialog( int iAdPage )
{
	// Alter this routine at your own risk....if anymore
	// bytes get sent to the client, you WILL crash them
	// Take away, but don't add (heh) (actually there's like
	// 100 bytes available, but they get eaten up fast with
	// this gump stuff)

	static const TCHAR * szGumps[] = // These are on every page and don't change
	{
		"page 0",
		"resizepic 0 0 5100 610 300",
		"resizepic 210 245 5100 170 40",
		"text 200 10 0 0",
		"text 25 30 0 1"
	};

	CGString sControls[256];
	int iControls = 0;
	while ( iControls<COUNTOF(szGumps))
	{
		sControls[iControls] = szGumps[iControls];
		iControls++;
	}

	static const TCHAR * szText[] = // These are on every page and don't change
	{
		"Admin Control Panel",
		"Sock  Account         Name              IP Address     Location",
	};

	CGString sText[256];
	int iTexts=0;
	while ( iTexts<COUNTOF(szText))
	{
		sText[iTexts] = szText[iTexts];
		iTexts++;
	}

	// set standard gump arguments
	int iY = 50;
	int iIndex = COUNTOF(szText);

	CClient * pClient = g_Serv.GetClientHead();
	int i=0;
	for ( ; pClient && i<iAdPage * ADMIN_CLIENTS_PER_PAGE; pClient = pClient->GetNext())
	{
		if ( ! m_pChar->CanDisturb( pClient->GetChar())) continue;
		i++;
	}

	// Valid max page
	if ( pClient == NULL )
	{
		iAdPage = 0;
		pClient = g_Serv.GetClientHead();
	}

	m_adminPage = iAdPage;

	for ( i=0; pClient && i<ADMIN_CLIENTS_PER_PAGE; pClient = pClient->GetNext())
	{
		CChar * pChar = pClient->GetChar();
		if ( ! m_pChar->CanDisturb( pChar )) continue;

		// Sock buttons
		sControls[iControls++].Format(
			"button %i %i %i %i %i %i %i",
			8,		// X
			iY + 2, // Y
			1209,	// Down gump
			1210,	// Up gump
			1,		// pressable ?
			1,		// iPage
			901+i );	// id

		static const int iGumpsSpaces[] = // These are on every page and don't change
		{
			17,		// Text (sock)
			73,		// Text (account name)
			190,	// Text (name)
			340,	// Text (ip, p)
		};

		for ( int j=0; j<COUNTOF(iGumpsSpaces); j++ )
		{
			sControls[iControls++].Format(
				"text %i %i %i %i",
				8 + iGumpsSpaces[j], iY,
				0,	// iColor
				iIndex );
			iIndex ++; // advance to the next line of text
		}

		iY += 20; // go down a 'line' on the dialog

		struct sockaddr_in Name;
		pClient->GetPeerName( &Name );

		TCHAR accountName[MAX_NAME_SIZE];
		strncpy( accountName, pClient->GetName(), 12 );
		accountName[12] = 0; // max name size for this dialog....otherwise CRASH!!!, not a big enuf buffer :(

		if ( pClient->IsPriv(PRIV_GM) || pClient->GetPrivLevel() >= PLEVEL_Counsel )
		{
			memmove( accountName+1, accountName, 13 );
			accountName[0] = ( pChar && pChar->IsDND()) ? '*' : '+';
		}

		ASSERT( i<COUNTOF(m_tmMenu));
		if ( pChar != NULL )
		{
			m_tmMenu[i] = pChar->GetUID();

			TCHAR characterName[MAX_NAME_SIZE];
			strcpy(characterName, pChar->GetName());
			characterName[12] = 0; // same comment as the accountName...careful with this stuff!
			sText[iTexts++].Format("%x", pClient->GetSocket());
			sText[iTexts++] = accountName;
			sText[iTexts++] = characterName;

			CPointMap pt = pChar->GetTopPoint();
			sText[iTexts++].Format( "%s%12d,%d,%d",
				inet_ntoa( Name.sin_addr ),
				pt.m_x,
				pt.m_y,
				pt.m_z);
		}
		else
		{
			m_tmMenu[i] = 0;

			sText[iTexts++].Format( "%03x", pClient->GetSocket());
			sText[iTexts++] = accountName;
			sText[iTexts++] = "N/A";
			sText[iTexts++] = inet_ntoa( Name.sin_addr );
		}
		i++;
	}

	if ( m_adminPage )	// is there a previous ?
	{
		sControls[iControls++] = "button 230 250 1235 1236 1 0 801";	// previous = press and unpress gump = 1235, 1236, button id 801
	}
	if ( pClient )	// is there a next ?
	{
		sControls[iControls++] = "button 300 250 1232 1233 1 0 802";	// next = button 802
	}

	addGumpMenu( TARGMODE_GUMP_ADMIN, sControls, iControls, sText, iTexts, 0x10, 0x46 );
}

bool CClient::addWalkCode( EXTDATA_TYPE iType, int iCodes )
{
	// RETURN: true = new codes where sent.

	if ( m_Crypt.GetClientVersion() < 12600 )
		return false;
	if ( ! ( g_Serv.m_wDebugFlags & DEBUGF_WALKCODES ))
		return( false );

	if ( iType == 2 )
	{
		if ( m_Walk_InvalidEchos >= 0 )
			return false;					// they are stuck til they give a valid echo!
		// On a timer tick call this.
		if ( m_Walk_CodeQty >= COUNTOF(m_Walk_LIFO))	// They are appearently not moving fast.
			return false;
	}
	else
	{
		// Fill the buffer at start.
		ASSERT( m_Walk_CodeQty < 0 );
		m_Walk_CodeQty = 0;
	}

	ASSERT( iCodes <= COUNTOF(m_Walk_LIFO));

	// make a new code and send it out
	CCommand cmd;
	cmd.ExtData.m_Cmd = XCMD_ExtData;
	cmd.ExtData.m_type = iType;

	int len = (sizeof(cmd.ExtData) - sizeof(cmd.ExtData.m_data));
	DWORD * pdwCodes = (DWORD*)(cmd.ExtData.m_data);

	for ( int i=0; i < iCodes && m_Walk_CodeQty < COUNTOF(m_Walk_LIFO); m_Walk_CodeQty++, i++ )
	{
		DWORD dwCode = 0x88ca0000 + GetRandVal( 0xffff );
		m_Walk_LIFO[m_Walk_CodeQty] = dwCode;
		pdwCodes[i] = dwCode;
		len += sizeof(DWORD);
	}

	cmd.ExtData.m_len = len;
	xSendPkt( &cmd, len );
	return( true );
}

//---------------------------------------------------------------------
// Login type stuff.

void CClient::Setup_Start( CChar * pChar ) // Send character startup stuff to player
{
	// Play this char.
	ASSERT( m_pAccount );
	ASSERT( pChar );

	CharDisconnect();	// I'm already logged in as someone else ?

	g_Log.Event( LOGM_CLIENTS_LOG, "%x:Setup_Start acct='%s', char='%s'\n", GetSocket(), m_pAccount->GetName(), pChar->GetName());

#ifndef _DEBUG
	srand( getclock()); // Perform randomize
#endif

	bool fQuickLogIn = false;
	if ( ! pChar->IsDisconnected())
	{
		// The players char is already in game ! Client linger time re-login.
		fQuickLogIn = true;
	}

	addPlayerStart( pChar );
	ASSERT(m_pChar);

	addSysMessage( g_szServerDescription );
	if ( fQuickLogIn )
	{
		addSysMessage( "Welcome Back" );
	}
	else
	{
		CGString sMsg;
		sMsg.Format( (g_Serv.m_Clients.GetCount()==2) ?
			"There is 1 other player here." : "There are %d other players here.",
			g_Serv.m_Clients.GetCount()-1 );
		addSysMessage( sMsg );
		// Get the intro script.
		addScrollFile(( GetPrivLevel() <= PLEVEL_Guest ) ? "GUEST" : "MOTD", SCROLL_TYPE_UPDATES );
	}

	if ( ! g_Serv.m_fSecure )
	{
		addBark( "WARNING: The world is NOT running in SECURE MODE",
			NULL, COLOR_TEXT_DEF, TALKMODE_SYSTEM, FONT_BOLD );
	}
	if ( IsPriv( PRIV_GM_PAGE ))
	{
		CGString sMsg;
		sMsg.Format( "There are %d GM pages pending. Use /PAGE command.", g_World.m_GMPages.GetCount());
		addSysMessage( sMsg );
	}
	if ( g_Serv.m_fRequireEmail )
	{
		// Have you set your email notification ?
		CGString sMsg;
		if ( m_pAccount->m_sEMail.IsEmpty())
		{
			sMsg.Format( "Email address for account '%s' has not yet been set.",
				GetAccount()->GetName(), (const TCHAR*)( GetAccount()->m_sEMail ));
			addSysMessage( sMsg );
			addSysMessage( "Use the command /EMAIL name@yoursp.com to set it." );

			//set me to "GUEST" mode.
		}
		else if ( ! IsPriv( PRIV_EMAIL_VALID ))
		{
			if ( GetAccount()->m_iEmailFailures > 5 )
			{
				sMsg.Format( "Email '%s' for account '%s' has failed %d times.",
					GetAccount()->GetName(), (const TCHAR*)( GetAccount()->m_sEMail ), GetAccount()->m_iEmailFailures );
			}
			else
			{
				sMsg.Format( "Email '%s' for account '%s' has not yet verified.",
					GetAccount()->GetName(), (const TCHAR*)( GetAccount()->m_sEMail ));
			}
			addSysMessage( sMsg );
			addSysMessage( "Use the command /EMAIL name@yoursp.com to change it." );
		}
	}
	if ( IsPriv( PRIV_JAILED ))
	{
		m_pChar->Jail( &g_Serv, true );
	}
#if 0
	if ( ! fQuickLogIn && m_pChar->m_pArea != NULL &&
		m_pChar->m_pArea->IsGuarded() &&
		! m_pChar->m_pArea->IsFlag( REGION_FLAG_ANNOUNCE ))
	{
		CGString sMsg;
		sMsg.Format( "You are under the protection of %s guards", (const TCHAR *) m_pChar->m_pArea->m_sGuardAnnounce );
		addSysMessage( sMsg );
	}
#endif
	if ( g_Serv.m_Clock_Shutdown )
	{
		addBark( "WARNING: The system is scheduled to shutdown soon",
			NULL, COLOR_TEXT_DEF, TALKMODE_SYSTEM, FONT_BOLD );
	}

	// Announce you to the world.
	Announce( true );
	m_pChar->Update( this );

	if ( g_Serv.m_fRequireEmail && m_pAccount->m_sEMail.IsEmpty())
	{
		// Prompt to set this right now !
	}

	// don't login on the water ! (unless i can swim)
	if ( ! m_pChar->m_pDef->Can(CAN_C_SWIM) && 
		! IsPriv(PRIV_GM) &&
		m_pChar->IsSwimming())
	{
		// bring to the nearest shore.
		// drift to nearest shore ?
		int iDist = 1;
		for ( int i=0; i<20; i++)
		{
			// try diagonal in all directions
			int iDistNew = iDist + 20;
			for ( int iDir = DIR_NE; iDir <= DIR_NW; iDir += 2 )
			{
				if ( m_pChar->MoveToValidSpot( (DIR_TYPE) iDir, iDistNew, iDist ))
				{
					i = 100;	// breakout
					break;
				}
			}
			iDist = iDistNew;
		}
		if ( i < 100 )
		{
			addSysMessage( "You are stranded in the water !" );
		}
		else
		{
			addSysMessage( "You have drifted to the nearest shore." );
		}
	}

	DEBUG_MSG(( "%x:Setup_Start done\n", GetSocket()));
}

void CClient::Setup_CreateDialog() // All the character creation stuff
{
	ASSERT( m_pAccount );
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Setup_CreateDialog acct='%s'\n", GetSocket(), m_pAccount->GetName()));
	}
	if ( m_pChar != NULL )
	{
		// Loggin in as a new player while already on line !
		addSysMessage( "Already on line" );
		DEBUG_ERR(( "%x:Setup_CreateDialog acct='%s' already on line!\n", GetSocket(), m_pAccount->GetName()));
		return;
	}

	// ??? Make sure they don't already have too many chars !
	// g_Serv.m_iMaxCharsPerAccount

	CChar * pChar = CChar::CreateBasic( CREID_MAN );
	pChar->InitPlayer( &m_bin, this );
	Setup_Start( pChar );
}


bool CClient::Setup_Play( int iSlot ) // After hitting "Play Character" button
{
	// Mode == TARGMODE_SETUP_CHARLIST

	ASSERT( m_pAccount );
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Setup_Play slot %d\n", GetSocket(), iSlot ));
	}

	ASSERT( COUNTOF(m_bout.CharList.m_char)<=COUNTOF(m_tmSetupCharList));

	if ( iSlot >= COUNTOF(m_bout.CharList.m_char))
		return false;
	CChar * pChar = m_tmSetupCharList[ iSlot ].CharFind();
	if ( ! m_pAccount->IsMyAccountChar( pChar ))
		return false;

	Setup_Start( pChar );
	return( true );
}

bool CClient::Setup_Delete( int iSlot ) // Deletion of character
{
	ASSERT( m_pAccount );
	DEBUG_MSG(( "%x:Setup_Delete slot=%d\n", GetSocket(), iSlot ));
	ASSERT( COUNTOF(m_bout.CharList.m_char)<=COUNTOF(m_tmSetupCharList));

	if ( iSlot >= COUNTOF(m_bout.CharList.m_char))
		return false;

	CChar * pChar = m_tmSetupCharList[iSlot].CharFind();
	if ( ! m_pAccount->IsMyAccountChar( pChar ))
		return false;

	// Make sure the char is at least x days old.
	if ( g_Serv.m_iMinCharDeleteTime && g_World.GetTime() - pChar->m_time_create < g_Serv.m_iMinCharDeleteTime )
	{
		if ( GetPrivLevel() < PLEVEL_Seer )
		{
			addSysMessage( "The character is not old enough to delete." );
			return false;
		}
	}

	pChar->Delete();
	// refill the list.

	CCommand cmd;
	cmd.CharList2.m_Cmd = XCMD_CharList2;
	int len = sizeof( cmd.CharList2 );
	cmd.CharList2.m_len = len;
	cmd.CharList2.m_count = Setup_FillCharList( cmd.CharList2.m_char, m_pAccount->m_uidLastChar.CharFind());
	xSendPkt( &cmd, len );

	return( true );
}

void CClient::Setup_ListReq( const TCHAR * pszAccount, const TCHAR * pszPassword )
{
	// Gameserver login and character listing
	m_fGameServer = true;	// Client thinks it's talking to a game server now.
	if ( GetTargMode() == TARGMODE_SETUP_CONNECT ||
		GetTargMode() == TARGMODE_SETUP_RELAY )
	{
		ClearTargMode();
	}
	if ( ! addLoginErr( LogIn( pszAccount, pszPassword )))
		return;

	ASSERT( m_pAccount );

	CChar * pCharLast = m_pAccount->m_uidLastChar.CharFind();
	if ( pCharLast &&
		m_pAccount->IsMyAccountChar( pCharLast ) &&
		m_pAccount->GetPrivLevel() <= PLEVEL_GM && 
		! pCharLast->IsDisconnected())
	{
		// If the last char is lingering then log back into this char instantly.
		// m_iClientLingerTime
		Setup_Start(pCharLast); 
		return;
	}

	// DEBUG_MSG(( "%x:Setup_ListFill\n", GetSocket()));

	CCommand cmd;
	cmd.CharList.m_Cmd = XCMD_CharList;
	int len = sizeof( cmd.CharList ) - sizeof(cmd.CharList.m_start) + ( g_Serv.m_StartDefs.GetCount() * sizeof(cmd.CharList.m_start[0]));
	cmd.CharList.m_len = len;

	DEBUG_CHECK( COUNTOF(cmd.CharList.m_char) == MAX_CHARS_PER_ACCT );

	// list chars to your account that may still be logged in !
	// "LASTCHARUID" = list this one first.
	cmd.CharList.m_count = Setup_FillCharList( cmd.CharList.m_char, pCharLast );

	// now list all the starting locations. (just in case we create a new char.)

	int iCount = g_Serv.m_StartDefs.GetCount();
	cmd.CharList.m_startcount = iCount;
	for ( int i=0;i<iCount;i++)
	{
		cmd.CharList.m_start[i].m_id = i+1;
		strncpy( cmd.CharList.m_start[i].m_area, g_Serv.m_StartDefs[i]->m_sArea, sizeof(cmd.CharList.m_start[i].m_area));
		cmd.CharList.m_start[i].m_area[ sizeof(cmd.CharList.m_start[i].m_area)-1 ] = '\0';
		strncpy( cmd.CharList.m_start[i].m_name, g_Serv.m_StartDefs[i]->m_sName, sizeof(cmd.CharList.m_start[i].m_name));
		cmd.CharList.m_start[i].m_name[ sizeof(cmd.CharList.m_start[i].m_name)-1 ] = '\0';
	}

	xSendPkt( &cmd, len );
	SetTargMode( TARGMODE_SETUP_CHARLIST );
}

LOGIN_ERR_TYPE CClient::LogIn( const TCHAR * pszName, const TCHAR * pPassword )
{
	CGString sMsg;
	LOGIN_ERR_TYPE lErr = LogIn( pszName, pPassword, sMsg );
	if ( lErr != LOGIN_SUCCESS )
	{
		if ( ! sMsg.IsEmpty())
		{
			SysMessage( sMsg );
		}
	}
	return( lErr );
}

LOGIN_ERR_TYPE CClient::LogIn( const TCHAR * pszName, const TCHAR * pPassword, CGString & sMsg )
{
	// NOTE: addLoginErr() will get called after this.

	if ( m_pAccount ) // already logged in.
		return( LOGIN_SUCCESS );

	if ( pszName[0] == '\0' ||
		strlen( pszName ) > MAX_NAME_SIZE ||
		strlen( pPassword ) > MAX_NAME_SIZE )
	{
		g_Log.Event( LOGL_WARN|LOGM_CLIENTS_LOG, "%x: Bad login name format\n", GetSocket());

		TCHAR szVersion[ 128 ];
		m_Crypt.WriteClientVersion( szVersion );
		sMsg.Format( "Bad login format. Check Client Version '%s' ?", szVersion );
		return( LOGIN_ERR_NONE );
	}

	bool fGuestAccount = ! strnicmp( pszName, "GUEST", 5 );
	if ( fGuestAccount )
	{
		// trying to log in as some sort of guest.
		// Find or create a new guest account.

		for ( int i=0; 1; i++ )
		{
			if ( i>=g_Serv.m_nGuestsMax )
			{
				sMsg = "All guest accounts are currently used.";
				return( LOGIN_ERR_BLOCKED );
			}

			CGString sName;
			sName.Format( "GUEST%d", i );
			CAccount * pAccount = g_World.AccountFind( sName, true, true );
			ASSERT( pAccount );

			if ( pAccount->FindClient() == NULL )
			{
				pszName = (TCHAR*) pAccount->GetName();
				break;
			}
		}
	}
	else
	{
		if ( pPassword[0] == '\0' )
		{
			sMsg = "Must supply a password.";
			return( LOGIN_ERR_BAD_PASS );
		}
	}

	bool fAdmin = ( ! strcmpi( pszName, "Administrator" ) || ! strcmpi( pszName, "RemoteAdmin" ));
	bool fAutoCreate = ( g_Serv.m_eAccApp == ACCAPP_Free || g_Serv.m_eAccApp == ACCAPP_GuestAuto || g_Serv.m_eAccApp == ACCAPP_GuestTrial );
	bool fGuest = ( g_Serv.m_eAccApp == ACCAPP_GuestAuto || g_Serv.m_eAccApp == ACCAPP_GuestTrial );

	CAccount * pAccount = g_World.AccountFind( pszName, fAutoCreate && ! fAdmin, fGuest );
	if ( ! pAccount )
	{
		// No account by this name ? So select a guest account.
		g_Log.Event( LOGM_CLIENTS_LOG, "%x:ERR Login NO Account '%s', pass='%s'\n", GetSocket(), pszName, pPassword );
		sMsg = "Bad account name. Try using a 'guest' account.";
		return( LOGIN_ERR_NONE );
	}

	if ( pAccount->IsPriv( PRIV_BLOCKED ))
	{
		sMsg.Format( "Account blocked. Send email to admin %s.", g_Serv.m_sEMail );
		return( LOGIN_ERR_BLOCKED );
	}

	// Look for this account already in use.
	if ( pAccount->FindClient( this ) != NULL )
	{
		sMsg = "Account already in use.";
		return( LOGIN_ERR_USED );
	}

	if ( ! fGuestAccount )
	{
		if ( pAccount->GetPassword()[0] )
		{
			// Get the password.
			if ( strcmpi( pAccount->GetPassword(), pPassword ))
			{
				g_Log.Event( LOGM_CLIENTS_LOG, "%x: '%s' bad pass '%s' != '%s'\n", GetSocket(), pAccount->GetName(), pPassword, pAccount->GetPassword());
				sMsg = "Bad password for this account.";
				return( LOGIN_ERR_BAD_PASS );
			}
		}
		else
		{
			// First guy in sets the password.
			// check the account in use first.
			pAccount->r_SetVal( "PASSWORD", pPassword );
		}
	}

	if ( g_Serv.m_nClientsMax <= 1 )
	{
		// Allow no one but Administrator on.
		sMsg = "Server is in admin only mode.";
		if ( pAccount->GetPrivLevel() < PLEVEL_Admin )
			return( LOGIN_ERR_BLOCKED );
	}
	if ( pAccount->GetPrivLevel() < PLEVEL_GM &&
		g_Serv.m_Clients.GetCount() > g_Serv.m_nClientsMax  )
	{
		// Give them a polite goodbye.
		sMsg = "Sorry the server is full. Try again later.";
		return( LOGIN_ERR_BLOCKED );
	}
	if ( pAccount->GetPrivLevel() <= PLEVEL_Guest )
	{
		g_Serv.m_nGuestsCur ++;
	}

	m_Time_Login = g_World.GetTime();	// g_World clock of login time. "LASTCONNECTTIME"
	m_pAccount = pAccount;
	m_pAccount->CheckStart();

	g_Log.Event( LOGM_CLIENTS_LOG, "%x:Login '%s'\n", GetSocket(), m_pAccount->GetName());
#ifdef VISUAL_SPHERE
	g_pServerObject->Fire_ClientAttach(GetSocket());
#endif
	return( LOGIN_SUCCESS );
}

