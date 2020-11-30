//
// CSector.cpp
// Copyright Menace Software (www.menasoft.com).
//

#include "graysvr.h"	// predef header.

////////////////////////////////////////////////////////////////////////
// -CCharsActiveList

void CCharsActiveList::OnRemoveOb( CGObListRec * pObRec )
{
	// Override this = called when removed from group.
	CChar * pChar = STATIC_CAST <CChar*>(pObRec);
	ASSERT( pChar );
	DEBUG_CHECK( pChar->IsTopLevel());
	if ( pChar->IsClient())
	{
		ClientDetach();
		m_LastClientTime = g_World.GetTime();	// mark time in case it's the last client
	}
	CGObList::OnRemoveOb(pObRec);
	DEBUG_CHECK( pChar->GetParent() == NULL );
	pChar->SetContainerFlags(UID_SPEC);
}

//////////////////////////////////////////////////////////////
// -CItemList

void CItemsList::OnRemoveOb( CGObListRec * pObRec )
{
	// Override this = called when removed from group.
	// Item is picked up off the ground.
	CItem * pItem = STATIC_CAST <CItem*>(pObRec);
	ASSERT( pItem );
	// ASSERT( pItem->m_p.IsValid());
	DEBUG_CHECK( pItem->IsTopLevel());
	CGObList::OnRemoveOb(pObRec);
	DEBUG_CHECK( pItem->GetParent() == NULL );
	pItem->SetContainerFlags(UID_SPEC);	// It is no place for the moment.
}

//////////////////////////////////////////////////////////////////
// -CSector

CSector::CSector()
{
	m_weather = WEATHER_DRY;
	m_locallight = LIGHT_BRIGHT;	// set based on time later.
	SetDefaultWeatherChance();
}

const TCHAR * CSector::sm_KeyTable[] =
{
	"COLDCHANCE",
	"LOCALLIGHT",
	"RAINCHANCE",
};

bool CSector::r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc )
{
	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0: // "COLDCHANCE",
		sVal.FormatVal( GetColdChance());
		return( true );
	case 1:	// "LOCALLIGHT"
		sVal.FormatVal( GetLight());
		return( true );
	case 2: // "RAINCHANCE",
		sVal.FormatVal( GetRainChance());
		return( true );
	}

	if ( ! strnicmp( pKey, "COMPLEXITY.", 11 ))
	{
		pKey += 11;
		static const CValStr titles[] =
		{
			"HIGH", INT_MIN,	// speech can be very complex if low char count
			"MEDIUM", 5,
			"LOW", 10,
			NULL, INT_MAX,
		};
		sVal = ( ! strcmpi( pKey, titles->FindName( GetComplexity()))) ? "1" : "0";
		return( true );
	}

	static const TCHAR * table[] =	// Write only variables.
	{
		"COMPLEXITY",
		"LOCALTIME",
		"LOCALTOD",
		"NIGHTTIME",
	};

	switch ( FindTableSorted( pKey, table, COUNTOF( table )))
	{
	case 0: // "COMPLEXITY"
		sVal.FormatVal( GetComplexity());
		return( true );
	case 1: // "LOCALTIME"
		sVal = GetGameTime();
		return( true );
	case 2: // "LOCALTOD"
		sVal.FormatVal( GetLocalTime());
		return( true );
	case 3:	// "NIGHTTIME"
		{
			int iMinutes = GetLocalTime();
			sVal = ( iMinutes < 7*60 || iMinutes > (9+12)*60 ) ? "1" : "0";
		}
		return( true );
	}

	return( false );
}

bool CSector::r_LoadVal( CScript &s )
{
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0: // "COLDCHANCE",
		SetWeatherChance( false, s.HasArgs() ? s.GetArgVal() : -1 );
		return( true );
	case 1:	// "LOCALLIGHT"
		m_locallight = s.GetArgVal() | LIGHT_OVERRIDE;
		return( true );
	case 2: // "RAINCHANCE",
		SetWeatherChance( true, s.HasArgs() ? s.GetArgVal() : -1 );
		return( true );
	}
	return( false );
}

bool CSector::r_Verb( CScript & s, CTextConsole * pSrc )
{
	static const TCHAR * table[] =
	{
		"DRY",
		"LIGHT",
		"RAIN",
		"RESPAWN",
		"RESTOCK",
		"SNOW",
	};

	switch ( FindTableSorted( s.GetKey(), table, COUNTOF(table)))
	{
	case 0:	// "DRY"
		SetWeather( WEATHER_DRY );
		break;
	case 1:	// "LIGHT"
		SetLight( (s.HasArgs()) ? s.GetArgVal() : -1 );
		break;
	case 2:	// "RAIN"
		SetWeather( WEATHER_RAIN );
		break;
	case 3: // "RESPAWN"
		if ( toupper( s.GetArgStr()[0] ) == 'A' )
		{
			g_World.RespawnDeadNPCs();
		}
		else
		{
			RespawnDeadNPCs();
		}
		break;
	case 4:	// "RESTOCK" x
		// set restock time of all vendors in World.
		// set the respawn time of all spawns in World.
		if ( toupper( s.GetArgStr()[0] ) == 'A' )
		{
			g_World.Restock();
		}
		else
		{
			Restock( s.GetArgVal());
		}
		break;
	case 5:	// "SNOW"
		SetWeather( WEATHER_SNOW );
		break;
	default:
		return( CScriptObj::r_Verb( s, pSrc ));
	}
	return( true );
}

void CSector::r_Write( CScript & s )
{
	if ( m_fSaveParity == g_World.m_fSaveParity ) return;	// already saved.
	m_fSaveParity = g_World.m_fSaveParity;

	if ( IsLightOverriden() || IsRainOverriden() || IsColdOverriden())
	{
		CPointMap pt = GetBase();
		s.WriteSection( "SECTOR %d,%d", pt.m_x, pt.m_y );
	}
	if ( IsLightOverriden())
	{
		s.WriteKeyVal( "LOCALLIGHT", GetLight());
	}
	if ( IsRainOverriden())
	{
		s.WriteKeyVal( "RAINCHANCE", GetRainChance());
	}
	if ( IsColdOverriden())
	{
		s.WriteKeyVal( "COLDCHANCE", GetColdChance());
	}

	// Chars in the sector.
	CChar * pCharNext;
	CChar * pChar = STATIC_CAST <CChar*>( m_Chars.GetHead());
	for ( ; pChar != NULL; pChar = pCharNext )
	{
		pCharNext = pChar->GetNext();
		pChar->WriteTry( s );
	}
	// Inactive Client Chars, ridden horses and dead npcs
	// NOTE: ??? Push inactive player chars out to the account files here ?
	pChar = STATIC_CAST <CChar*> (m_Chars_Disconnect.GetHead());
	for ( ; pChar != NULL; pChar = pCharNext )
	{
		pCharNext = pChar->GetNext();
		pChar->WriteTry( s );
	}

	// Items on the ground.
	CItem * pItemNext;
	CItem * pItem = STATIC_CAST <CItem*> (m_Items_Inert.GetHead());
	for ( ; pItem != NULL; pItem = pItemNext )
	{
		pItemNext = pItem->GetNext();
		pItem->WriteTry( s );
	}
	pItem = STATIC_CAST <CItem*> (m_Items_Timer.GetHead());
	for ( ; pItem != NULL; pItem = pItemNext )
	{
		pItemNext = pItem->GetNext();
		pItem->WriteTry( s );
	}
}

const CGrayMapBlock * CSector::GetMapBlock( const CPointMap & pt )
{
	// Get a map block from the cache. load it if not.

	ASSERT( pt.IsValid());
	CPointMap pntBlock( UO_BLOCK_ALIGN(pt.m_x), UO_BLOCK_ALIGN(pt.m_y));
	ASSERT( m_CacheMap.GetCount() <= (UO_BLOCK_SIZE * UO_BLOCK_SIZE));

	// Find it in cache.
	for ( int i=0; i<m_CacheMap.GetCount(); i++ )
	{
		if ( pntBlock.IsSame2D( m_CacheMap[i]->m_p ))
		{
			m_CacheMap[i]->HitCacheTime();
			return( m_CacheMap[i] );
		}
	}

	// else load it.
	CGrayMapBlock * pMapBlock;
	try
	{
		pMapBlock = new CGrayMapBlock( pntBlock );
	}
	catch(...)
	{
		return( NULL );
	}

	// Add it to the cache.
	m_CacheMap.Add( pMapBlock );
	return( pMapBlock );
}

int CSector::GetLocalTime() const
{
	// The local time of day (in minutes) is based on Global Time and latitude
	//
	CPointMap pt = GetBase();

	// Time difference between adjacent sectors in minutes
	int iSectorTimeDiff = (24*60) / SECTOR_COLS;

	// Calculate the # of columns between here and Castle Britannia ( x = 1400 )
	int iSectorOffset = ( pt.m_x / SECTOR_SIZE_X ) - ( (24*60) / SECTOR_SIZE_X );

	// Calculate the time offset from global time
	int iTimeOffset = iSectorOffset * iSectorTimeDiff;

	// Calculate the local time
	int iLocalTime = ( g_World.GetGameWorldTime() + iTimeOffset ) % (24*60);

	return ( iLocalTime );
}

const TCHAR * CSector::GetGameTime() const
{
	return( GetTimeMinDesc( GetLocalTime()));
}

bool CSector::IsMoonVisible(int iPhase, int iLocalTime) const
{
	// When is moon rise and moon set ?
	iLocalTime %= (24*60);		// just to make sure (day time in minutes) 1440
	switch (iPhase)
	{
	case 0:	// new moon
		return( (iLocalTime > 360) && (iLocalTime < 1080));
	case 1:	// waxing crescent
		return( (iLocalTime > 540) && (iLocalTime < 1270));
	case 2:	// first quarter
		return( iLocalTime > 720 );
	case 3:	// waxing gibbous
		return( (iLocalTime < 180) || (iLocalTime > 900));
	case 4:	// full moon
		return( (iLocalTime < 360) || (iLocalTime > 1080));
	case 5:	// waning gibbous
		return( (iLocalTime < 540) || (iLocalTime > 1270));
	case 6:	// third quarter
		return( iLocalTime < 720 );
	case 7:	// waning crescent
		return( (iLocalTime > 180) && (iLocalTime < 900));
	default: // How'd we get here?
		ASSERT(0);
		return (false);
	}
}

BYTE CSector::GetLightCalc( bool fQuickSet ) const
{
	// What is the light level default here in this sector.

	if ( IsLightOverriden())
		return( m_locallight );

	if ( GetBase().IsInDungeon())
		return( g_Serv.m_iLightDungeon );

	int localtime = GetLocalTime();
	int hour = ( localtime / ( 60)) % 24;
	bool fNight = ( hour < 6 || hour > 12+8 );	// Is it night or day ?
	int iTargLight = (fNight) ? g_Serv.m_iLightNight : g_Serv.m_iLightDay;	// Target light level.

	// Check for clouds...if it is cloudy, then we don't even need to check for the effects of the moons...
	if ( GetWeather())
	{
		// Clouds of some sort...
		if (fNight)
			iTargLight += ( GetRandVal( 2 ) + 1 );	// 1-2 light levels darker if cloudy at night
		else
			iTargLight += ( GetRandVal( 4 ) + 1 );	// 1-4 light levels darker if cloudy during the day.
	}

	if ( fNight )
	{
		// Factor in the effects of the moons
		// Trammel
		int iTrammelPhase = g_World.GetMoonPhase( false );
		// Check to see if Trammel is up here...

		if ( IsMoonVisible( iTrammelPhase, localtime ))
		{
static const BYTE TrammelPhaseBrightness[] =
{
	0, // New Moon
	TRAMMEL_FULL_BRIGHTNESS / 4,	// Crescent Moon
	TRAMMEL_FULL_BRIGHTNESS / 2, 	// Quarter Moon
	( TRAMMEL_FULL_BRIGHTNESS * 3) / 4, // Gibbous Moon
	TRAMMEL_FULL_BRIGHTNESS,		// Full Moon
	( TRAMMEL_FULL_BRIGHTNESS * 3) / 4, // Gibbous Moon
	TRAMMEL_FULL_BRIGHTNESS / 2, 	// Quarter Moon
	TRAMMEL_FULL_BRIGHTNESS / 4,	// Crescent Moon
};
			ASSERT( iTrammelPhase < COUNTOF(TrammelPhaseBrightness));
			iTargLight -= TrammelPhaseBrightness[iTrammelPhase];
		}

		// Felucca
		int iFeluccaPhase = g_World.GetMoonPhase( true );
		if ( IsMoonVisible( iFeluccaPhase, localtime ))
		{
static const BYTE FeluccaPhaseBrightness[] =
{
	0, // New Moon
	FELUCCA_FULL_BRIGHTNESS / 4,	// Crescent Moon
	FELUCCA_FULL_BRIGHTNESS / 2, 	// Quarter Moon
	( FELUCCA_FULL_BRIGHTNESS * 3) / 4, // Gibbous Moon
	FELUCCA_FULL_BRIGHTNESS,		// Full Moon
	( FELUCCA_FULL_BRIGHTNESS * 3) / 4, // Gibbous Moon
	FELUCCA_FULL_BRIGHTNESS / 2, 	// Quarter Moon
	FELUCCA_FULL_BRIGHTNESS / 4,	// Crescent Moon
};
			ASSERT( iFeluccaPhase < COUNTOF(FeluccaPhaseBrightness));
			iTargLight -= FeluccaPhaseBrightness[iFeluccaPhase];
		}
	}

	if ( iTargLight < LIGHT_BRIGHT ) iTargLight = LIGHT_BRIGHT;
	if ( iTargLight > LIGHT_DARK ) iTargLight = LIGHT_DARK;

	if ( fQuickSet || m_locallight == iTargLight )
	{
		// Initializing the sector
		return( iTargLight );
	}

	// Gradual transition to global light level.
	if ( m_locallight > iTargLight )
		return( m_locallight - 1 );
	else
		return( m_locallight + 1 );
}

void CSector::SetLightNow( bool fFlash )
{
	// Set the light level for all the CClients here.

	if ( ! HasClients())
		return;

	CChar * pChar = STATIC_CAST <CChar*>( m_Chars.GetHead());
	for ( ; pChar != NULL; pChar = pChar->GetNext())
	{
		if ( ! pChar->IsClient())
			continue;
		if ( pChar->IsStat( STATF_DEAD | STATF_NightSight ))
			continue;

		CClient * pClient = pChar->GetClient();
		ASSERT(pClient);

		if ( fFlash )	// This does not seem to work predicably !
		{
			BYTE bPrvLight = m_locallight;
			m_locallight = LIGHT_BRIGHT;	// full bright.
			pClient->addLight();
			pClient->addPause( false );	// show the new light level.
			pClient->xFlush();
			m_locallight = bPrvLight;	// back to previous.
		}
		pClient->addLight();
	}
}

void CSector::SetLight( int light )
{
	// GM set light level command
	// light =LIGHT_BRIGHT, LIGHT_DARK=dark

	// DEBUG_MSG(( "CSector.SetLight(%d)\n", light ));

	if ( light < LIGHT_BRIGHT || light > LIGHT_DARK )
	{
		m_locallight &= ~LIGHT_OVERRIDE;
		m_locallight = (BYTE) GetLightCalc( true );
	}
	else
	{
		m_locallight = (BYTE) ( light | LIGHT_OVERRIDE );
	}
	SetLightNow(false);
}

void CSector::SetDefaultWeatherChance()
{
	CPointMap pt = GetBase();
	int iPercent = IMULDIV( pt.m_y, 100, UO_SIZE_Y );	// 100 = south
	if ( iPercent < 50 )
	{
		// Anywhere north of the Britain Moongate is a good candidate for snow
		m_ColdChance = 1 + ( 49 - iPercent ) * 2;
		m_RainChance = 15;
	}
	else
	{
		// warmer down south
		m_ColdChance = 1;
		// rain more likely down south.
		m_RainChance = 15 + ( iPercent - 50 ) / 10;
	}
}

WEATHER_TYPE CSector::GetWeatherCalc() const
{
	// (1 in x) chance of some kind of weather change at any given time
	if ( GetBase().IsInDungeon() || g_Serv.m_fNoWeather )
		return( WEATHER_DRY );

	int iRainRoll = GetRandVal( 100 );
	if ( ( GetRainChance() * 2) < iRainRoll )
		return( WEATHER_DRY );

	// Is it just cloudy?
	if ( iRainRoll > GetRainChance())
		return WEATHER_CLOUDY;

	// It is snowing
	if ( GetRandVal(100) <= GetColdChance()) // Can it actually snow here?
		return WEATHER_SNOW;

	// It is raining
	return WEATHER_RAIN;
}

void CSector::SetWeather( WEATHER_TYPE w )
{
	// Set the immediate weather type.
	// 0=dry, 1=rain or 2=snow.
	if ( m_weather != WEATHER_DRY && w != WEATHER_DRY )
	{
		SetWeather( WEATHER_DRY );	// fix weird client transition problem.
	}
	m_weather = w;

	if ( ! HasClients())
		return;

	CChar * pChar = STATIC_CAST <CChar*>( m_Chars.GetHead());
	for ( ; pChar != NULL; pChar = pChar->GetNext())
	{
		if ( ! pChar->IsClient())
			continue;
		pChar->GetClient()->addWeather( w );
	}
}

void CSector::SetWeatherChance( bool fRain, int iChance )
{
	// Set via the client.
	// Transfer from snow to rain does not work ! must be DRY first.

	if ( iChance > 100 ) iChance = 100;
	if ( iChance < 0 )
	{
		// just set back to defaults.
		SetDefaultWeatherChance();
	}
	else if ( fRain )
	{
		m_RainChance = iChance | LIGHT_OVERRIDE;
	}
	else
	{
		m_ColdChance = iChance | LIGHT_OVERRIDE;
	}

	// Recalc the weather immediatly.
	SetWeather( GetWeatherCalc());
}

void CSector::MoveToSector( CChar * pChar )
{
	// Move a CChar into this CSector.
	// m_p is still the old location.
	// ASSUME: called from CChar.MoveTo() assume ACtive char.

	if ( IsCharActiveIn( pChar ))
		return;	// already here

	// Check my save parity vs. this sector's
	if ( pChar->IsStat( STATF_SaveParity ) != m_fSaveParity )
	{
		// DEBUG_MSG(( "Movement based save uid 0%x\n", pChar->GetUID()));
		if ( g_World.IsSaving())
		{
			if ( m_fSaveParity == g_World.m_fSaveParity )
			{
				// Save out the CChar now.
				pChar->r_Write( g_World.m_File );
			}
			else
			{
				// We need to write out this CSector now. (before adding client)
				ASSERT( pChar->IsStat( STATF_SaveParity ) == g_World.m_fSaveParity );
				r_Write( g_World.m_File );
			}
		}
		else
		{
			ASSERT( g_World.IsSaving());
		}
	}

	if ( pChar->IsClient())
	{
		// Send new weather and light for this sector
		// Only send if different than last ???

		CClient * pClient = pChar->GetClient();
		ASSERT( pClient );

		if ( pClient->m_pntSoundRecur.IsValid())
		{
			if ( pClient->m_pntSoundRecur.GetDist( pChar->GetTopPoint()) > UO_MAP_VIEW_SIZE )
			{
				pClient->addSound( SOUND_CLEAR );
			}
		}

		if ( ! pChar->IsStat( STATF_DEAD | STATF_NightSight | STATF_Sleeping ))
		{
			pClient->addLight();
		}
		// Provide the weather as an arg as we are not in the new location yet.
		pClient->addWeather( GetWeather());
	}

	m_Chars.AddToSector( pChar );	// remove from previous spot.
}

inline bool CSector::IsSectorSleeping() const
{
	long iAge = g_World.GetTime() - GetLastClientTime();
	return( iAge > 10*60*TICK_PER_SEC );
}

void CSector::SetSectorWakeStatus()
{
	// Ships may enter a sector before it's riders ! ships need working timers to move !
	m_Chars.m_LastClientTime = g_World.GetTime();
}

void CSector::Close( void )
{
	// Clear up all dynamic data for this sector.
	m_Items_Timer.DeleteAll();
	m_Items_Inert.DeleteAll();
	m_Chars.DeleteAll();
	m_Chars_Disconnect.DeleteAll();
	m_Regions.DeleteAll();
	m_Teleports.RemoveAll();
}

const CTeleport * CSector::GetTeleport( const CPointMap & pt ) const
{
	// Any teleports here at this point ?

	for ( int i=0; i<m_Teleports.GetCount(); i++ )
	{
		const CTeleport * pTel = m_Teleports[i];
		ASSERT(pTel);
		if ( ! pt.IsSame2D( pTel->m_Src ))
			continue;
		int zdiff = pt.m_z - pTel->m_Src.m_z;
		if ( abs(zdiff) > 5 )
			continue;
		return( pTel );
	}
	return( NULL );
}

void CSector::UnLoadRegions()
{
	CWorld::UnLoadRegions( m_Regions );
	m_Teleports.RemoveAll();
}

CItem * CSector::CheckNaturalResource( const CPointMap & pt, ITEM_TYPE Type, bool fTest )
{
	// Check/Decrement natural resources at this location.
	// We create an invis object to time out and produce more.
	// RETURN: Quantity they wanted. 0 = none here.

	if ( fTest )	// Is the resource avail at all here ?
	{
		if ( ! g_World.IsItemTypeNear( pt, Type ))
			return( NULL );
	}

	// Find the resource object.
	CItem * pItem;
	CWorldSearch Area(pt);
	while(true)
	{
		pItem = Area.GetItem();
		if ( ! pItem )
			break;
		// NOTE: Not all resource objects are world gems. should they be ?
		// I wanted to make tree stumps etc be the resource block some day.

		if ( pItem->m_type == Type && pItem->GetID() == ITEMID_WorldGem )
			break;
	}

	// If none then create one.
	if ( ! pItem )
	{
		pItem = CItem::CreateScript( ITEMID_WorldGem );
		pItem->m_type = Type;
		pItem->m_Attr |= ATTR_INVIS | ATTR_NEWBIE;	// use the newbie flag just to show that it has just been created.

		int iAmount;
		int iDecayMin;
		switch( Type )
		{
		case ITEM_TREE:
			// Timber, a little more resource than mining, but
			// the same decaytime to force people to move around
			// while lumberjacking.
			iAmount = GetRandVal( 20 ) + 4;
			iDecayMin = 60;
			break;
		case ITEM_ROCK:
			// Ore, a very small amount, and 1 hour decaytime.
			iAmount = GetRandVal( 20 ) + 4;
			iDecayMin = 60;
			break;
		case ITEM_WATER:
			// Water (fish), a very small amount
			iAmount = GetRandVal(2) + 1;
			iDecayMin = 60;
			break;
		case ITEM_GRASS:
			// Grass...a good deal in each spot
			iAmount = GetRandVal(50) + 100;
			iDecayMin = 240;
			break;
		default:
			DEBUG_CHECK(0);	// Not sure why this is here.
			iAmount = GetRandVal( 20 ) + 10;
			iDecayMin = 20;
			break;
		}

		pItem->SetAmount( iAmount );
		pItem->SetDecayTime( iDecayMin*60*TICK_PER_SEC );	// Delete myself in this amount of time.
		pItem->MoveTo( pt );
	}

	else if ( ! pItem->IsAttr(ATTR_INVIS))
	{
		pItem->m_Attr |= ATTR_MOVE_NEVER;	// try not to kill existing stuff.
	}

	return( pItem );
}

void CSector::RespawnDeadNPCs()
{
	// Respawn dead NPC's
	CChar * pCharNext;
	CChar * pChar = STATIC_CAST <CChar *>( m_Chars_Disconnect.GetHead());
	for ( ; pChar != NULL; pChar = pCharNext )
	{
		pCharNext = pChar->GetNext();
		if ( ! pChar->m_pNPC )
			continue;
		if ( ! pChar->m_Home.IsValid())
			continue;
		if ( ! pChar->IsStat( STATF_DEAD ))
			continue;

		// Res them back to their "home".
		int iDist = pChar->m_pNPC->m_Home_Dist_Wander;
		pChar->MoveNear( pChar->m_Home, ( iDist != 0xFFFF ) ? iDist : 4 );
		pChar->Spell_Resurrection(-1);

		// Restock them with npc stuff.
		pChar->NPC_LoadScript(true);
	}
}

void CSector::Restock( long iTime )
{
	// ARGS: iTime = time in seconds
	// set restock time of all vendors in Sector.
	// set the respawn time of all spawns in Sector.

	CChar * pCharNext;
	CChar * pChar = STATIC_CAST <CChar*>( m_Chars.GetHead());
	for ( ; pChar != NULL; pChar = pCharNext )
	{
		pCharNext = pChar->GetNext();
		pChar->NPC_Vendor_Restock( iTime );
	}
}

void CSector::OnTick( int iPulseCount )
{
	// CWorld gives OnTick() to all CSectors.

	// Check for light change before putting the sector to sleep
	// If we don't, sectors that are asleep will end up with wacked-out
	// light levels due to the gradual light level changes not getting
	// applied fast enough.

	bool fLightChange = false;
	if ( ! ( iPulseCount & 0x7f ))	// 30 seconds or so.
	{
		// check for local light level change ?
		BYTE blightprv = m_locallight;
		m_locallight = GetLightCalc( false );
		if ( m_locallight != blightprv ) fLightChange = true;
	}

	bool fSleeping = false;
	long lPulseCount = g_World.GetTime()/SECTOR_TICK_PERIOD;
	if ( ! HasClients())
	{
		// Put the sector to sleep if no clients been here in a while.
		fSleeping = IsSectorSleeping();
		if ( fSleeping )
		{
			// Stagger sectors a bit. so it is not jerky.
			// 15 seconds or so.
			// Tuned to the CWorld OnTick of SECTOR_TICK_PERIOD !!!
//#ifndef _DEBUG
			if (( iPulseCount & g_Serv.m_iSectorSleepMask ) != ( GetIndex() & g_Serv.m_iSectorSleepMask ))
				return;
//#endif
		}
	}

	// random weather noises and effects.
	SOUND_TYPE sound = 0;
	bool fWeatherChange = false;

	if ( ! ( iPulseCount & 0x7f ))	// 30 seconds or so.
	{
		// Only do this every x minutes or so (TICK_PER_SEC)
		// check for local weather change ?
		WEATHER_TYPE weatherprv = m_weather;
		if ( ! GetRandVal( 10 ))	// change less often
		{
			m_weather = GetWeatherCalc();
			if ( weatherprv != m_weather ) fWeatherChange = true;
		}

		// Random area noises. Only do if clients about.
		if ( HasClients())
		{
			static const WORD SfxRain[] = { 0x10, 0x11 };
			static const WORD SfxWind[] = { 0x14, 0x15, 0x16 };
			static const WORD SfxThunder[] = { 0x28, 0x29 , 0x206 };

			// Lightning ?	// wind, rain,
			switch ( GetWeather())
			{
			case WEATHER_CLOUDY:
				break;

			case WEATHER_SNOW:
				if ( ! GetRandVal(5))
				{
					sound = SfxWind[ GetRandVal( COUNTOF( SfxWind )) ];
				}
				break;

			case WEATHER_RAIN:
				{
					int iVal = GetRandVal(30);
					if ( iVal < 5 )
					{
						// Mess up the light levels for a sec..
						LightFlash();
						sound = SfxThunder[ GetRandVal( COUNTOF( SfxThunder )) ];
					}
					else if ( iVal < 10 )
					{
						sound = SfxRain[ GetRandVal( COUNTOF( SfxRain )) ];
					}
					else if ( iVal < 15 )
					{
						sound = SfxWind[ GetRandVal( COUNTOF( SfxWind )) ];
					}
				}
				break;
			}
		}
	}

	// regen all creatures and do AI

	g_Serv.m_Profile.Start( PROFILE_CHARS );

	CChar * pCharNext;
	for ( CChar * pChar = STATIC_CAST <CChar*>( m_Chars.GetHead()); pChar != NULL; pChar = pCharNext )
	{
		DEBUG_CHECK( ! pChar->IsWeird());
		ASSERT( pChar->IsTopLevel());	// not deleted or off line.
		pCharNext = pChar->GetNext();
		if ( pChar->IsClient())
		{
			CClient * pClient = pChar->GetClient();
			ASSERT( pClient );
			if ( sound )
			{
				pClient->addSound( sound, pChar );
			}
			if ( fLightChange && ! pChar->IsStat( STATF_DEAD | STATF_NightSight ))
			{
				pClient->addLight();
			}
			if ( fWeatherChange )
			{
				pClient->addWeather( GetWeather());
			}
		}
		// Can only die on your own tick.
		if ( ! pChar->OnTick())
		{
			pChar->Delete();
		}
	}

	// decay items on ground = time out spells / gates etc.. etc..
	// No need to check these so often !

	g_Serv.m_Profile.Start( PROFILE_ITEMS );

	CItem * pItemNext;
	for ( CItem * pItem = STATIC_CAST <CItem*>( m_Items_Timer.GetHead()); pItem != NULL; pItem = pItemNext )
	{
		DEBUG_CHECK( ! pItem->IsWeird());
		DEBUG_CHECK( pItem->IsTimerSet());
		pItemNext = pItem->GetNext();
		if ( ! pItem->IsTimerExpired()) 
			continue;	// not ready yet.
		if ( ! pItem->OnTick())
		{
			pItem->Delete();
		}
	}

	g_Serv.m_Profile.Start( PROFILE_OVERHEAD );

	if ( fSleeping || ! ( iPulseCount & 0x7f ))	// 30 seconds or so.
	{
		// delete the static CGrayMapBlock items that have not been used recently.
		for ( int i=0; i<m_CacheMap.GetCount(); i++ )
		{
			if ( fSleeping || m_CacheMap[i]->GetCacheAge() >= g_Serv.m_iMapCacheTime )
			{
				m_CacheMap.DeleteAt(i);
				i--;
			}
		}
	}
}

