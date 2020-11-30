//
// CWorld.CPP
// Copyright Menace Software (www.menasoft.com).
//

#ifdef VISUAL_SPHERE
	#include "..\Visual Sphere\stdafx.h"
	#include "..\Visual Sphere\Visual Sphere.h"
	#include "..\Visual Sphere\ServerObject.h"
#else
	#include "graysvr.h"	// predef header.
#endif


bool World_fDeleteCycle = false;

static const SOUND_TYPE Sounds_Ghost[] =
{
	SOUND_GHOST_1,
	SOUND_GHOST_2,
	SOUND_GHOST_3,
	SOUND_GHOST_4,
	SOUND_GHOST_5,
};

//////////////////////////////////////////////////////////////////
// -CGMPage

CGMPage::CGMPage( const TCHAR * pszAccount ) :
	m_sAccount( pszAccount )
{
	m_pGMClient = NULL;
	m_lTime = g_World.GetTime();
	// Put at the end of the list.
	g_World.m_GMPages.InsertTail( this );
}

CGMPage::~CGMPage()
{
	if ( m_pGMClient )	// break the link to the client.
	{
		ASSERT( m_pGMClient->m_pGMPage == this );
		m_pGMClient->m_pGMPage = NULL;
		ClearGMHandler();
	}
}

void CGMPage::r_Write( CScript & s ) const
{
	s.WriteSection( "GMPAGE %s", GetName());
	s.WriteKey( "REASON", GetReason());
	s.WriteKeyHex( "TIME", m_lTime );
	s.WriteKey( "P", m_p.Write());
}

bool CGMPage::r_LoadVal( CScript & s )
{
	if ( s.IsKey( "REASON" ))
	{
		SetReason( s.GetArgStr());
	}
	else if ( s.IsKey( "TIME" ))
	{
		m_lTime = s.GetArgHex();
	}
	else if ( s.IsKey( "P" ))
	{
		m_p.Read( s.GetArgStr());
	}
	else
	{
		return( false );
	}
	return( true );
}

CAccount * CGMPage::FindAccount() const
{
	return( g_World.AccountFind( m_sAccount ));
}


//////////////////////////////////////////////////////////////////
// -CWorldSearch

CWorldSearch::CWorldSearch( const CPointMap p, int iDist ) :
	m_p( p ),
	m_iDist( iDist )
{
	// define a search of the world.
	m_fInertCharUse = false;
	m_pObj = m_pObjNext = NULL;
	m_fInertToggle = false;

	m_pSectorBase = m_pSector = p.GetSector();

	int i = p.m_x - iDist;
	i = max( i, 0 );
	m_rectSector.m_left = i &~ (SECTOR_SIZE_X-1);
	i = ( p.m_x + iDist ) ;
	m_rectSector.m_right = min( i, UO_SIZE_X ) | (SECTOR_SIZE_X-1);
	i = p.m_y - iDist;
	i = max( i, 0 );
	m_rectSector.m_top = i &~ (SECTOR_SIZE_Y-1);
	i = ( p.m_y + iDist ) ;
	m_rectSector.m_bottom = min( i, UO_SIZE_Y ) | (SECTOR_SIZE_Y-1);
	m_rectSector.NormalizeRect();

	// Get upper left of search rect.
	m_pntSector.m_x = m_rectSector.m_left;
	m_pntSector.m_y = m_rectSector.m_top;
	m_pntSector.m_z = 0;

	//if ( m_rectSector.m_left == 0x13e3 && m_rectSector.m_right == 0x147f &&
		//m_rectSector.m_top == 0x13 && m_rectSector.m_bottom == 0x7f )
	//{
	//	DEBUG_ERR(( "Test this\n" ));
	//}
}

bool CWorldSearch::GetNextSector()
{
	// Move search into near by CSector(s) if necessary

	if ( ! m_iDist )
		return( false );

	for ( ; m_pntSector.m_y < m_rectSector.m_bottom; m_pntSector.m_y += SECTOR_SIZE_Y )
	{
		while ( m_pntSector.m_x < m_rectSector.m_right )
		{
			m_pSector = m_pntSector.GetSector();
			m_pntSector.m_x += SECTOR_SIZE_X;
			if ( m_pSectorBase == m_pSector )
				continue;	// same as base.

			m_pObj = NULL;	// start at head of next Sector.
			return( true );
		}
		m_pntSector.m_x = m_rectSector.m_left;
	}
	return( false );	// done searching.
}

CItem * CWorldSearch::GetItem()
{
	while (true)
	{
		if ( m_pObj == NULL )
		{
			m_fInertToggle = false;
			m_pObj = STATIC_CAST <CObjBase*> ( m_pSector->m_Items_Inert.GetHead());
		}
		else
		{
			m_pObj = m_pObjNext;
		}
		if ( m_pObj == NULL )
		{
			if ( ! m_fInertToggle )
			{
				m_fInertToggle = true;
				m_pObj = STATIC_CAST <CObjBase*> ( m_pSector->m_Items_Timer.GetHead());
				if ( m_pObj != NULL )
					goto jumpover;
			}
			if ( GetNextSector())
				continue;
			return( NULL );
		}

jumpover:
		m_pObjNext = m_pObj->GetNext();
		if ( m_p.GetDist( m_pObj->GetTopPoint()) <= m_iDist )
			return( STATIC_CAST <CItem *> ( m_pObj ));
	}
}

CChar * CWorldSearch::GetChar()
{
	while (true)
	{
		if ( m_pObj == NULL )
		{
			m_fInertToggle = false;
			m_pObj = STATIC_CAST <CObjBase*> ( m_pSector->m_Chars.GetHead());
		}
		else
		{
			m_pObj = m_pObjNext;
		}
		if ( m_pObj == NULL )
		{
			if ( ! m_fInertToggle && m_fInertCharUse )
			{
				m_fInertToggle = true;
				m_pObj = STATIC_CAST <CObjBase*> ( m_pSector->m_Chars_Disconnect.GetHead());
				if ( m_pObj != NULL )
					goto jumpover;
			}
			if ( GetNextSector())
				continue;
			return( NULL );
		}

jumpover:
		m_pObjNext = m_pObj->GetNext();
		if ( m_p.GetDist( m_pObj->GetTopPoint()) <= m_iDist )
			return( STATIC_CAST <CChar *> ( m_pObj ));
	}
}

//////////////////////////////////////////////////////////////////
// -CWorld

CWorld::CWorld()
{
	m_iSaveCount = 0;
	m_iSaveStage = 0;
	m_fSaveForce = false;
	m_Clock_Sector = 0;
	m_Clock_Respawn = 0;

	m_Clock_PrevSys = 0;
	m_Clock_Time = 0;
	m_Clock_Startup = 0;
}

DWORD CWorld::AllocUID( DWORD dwIndex, CObjBase * pObj )
{
	if ( dwIndex == 0 )
	{
		// Find an unused UID slot. Start at some random point.
		DWORD dwCount = GetUIDCount() - 1;
		DWORD dwCountStart = dwCount;
		dwIndex = GetRandVal( dwCount ) + 1;
		while ( m_UIDs[dwIndex] != NULL )
		{
			if ( ! -- dwIndex )
			{
				dwIndex = dwCountStart;
			}
			if ( ! -- dwCount )
			{
				dwIndex = GetUIDCount();
				goto setcount;
			}
		}
	}
	else if ( dwIndex >= GetUIDCount())
	{
setcount:
		// We have run out of free UID's !!! Grow the array
		m_UIDs.SetCount(( dwIndex + 0x1000 ) &~ 0xFFF );
	}

	CObjBase * pObjPrv = m_UIDs[ dwIndex ];
	if ( pObjPrv != NULL )
	{
		// NOTE: We cannot use Delete() in here because the UID will still be assigned til the async cleanup time.
		DEBUG_ERR(( "UID conflict delete 0%lx, '%s'\n", dwIndex, pObjPrv->GetName()));
		delete pObjPrv;	// Delete()	will not work here !
		// ASSERT( 0 );
	}

	m_UIDs[ dwIndex ] = pObj;
	return( dwIndex );
}

///////////////////////////////////////////////
// Loading and Saving.

void CWorld::GetBackupName( CGString & sArchive, TCHAR chType ) const
{
	int iCount = m_iSaveCount;
	int iGroup = 0;
	for ( ; iGroup<g_Serv.m_iSaveBackupLevels; iGroup++ )
	{
		if ( iCount & 0x7 )
			break;
		iCount >>= 3;
	}
	sArchive.Format( "%s" GRAY_FILE "b%d%d%c%s",
		( chType == 'a' ) ? ((const TCHAR*) g_Serv.m_sAcctBaseDir ) : ((const TCHAR*) g_Serv.m_sWorldBaseDir ),
		iGroup, iCount&0x07,
		chType,
		( g_Serv.m_fSaveBinary && chType == 'w' ) ? GRAY_BINARY : GRAY_SCRIPT );
}

bool CWorld::SaveStage() // Save world state in stages.
{
	// Do the next stage of the save.
	// RETURN: true = continue;
	//  false = done.

	ASSERT( IsSaving());

	switch ( m_iSaveStage )
	{
	case -1:
		if ( ! g_Serv.m_fSaveGarbageCollect )
		{
			GarbageCollection_New();
			GarbageCollection_GMPages();
		}
		break;
	default:
		ASSERT( m_iSaveStage < SECTOR_QTY );
		// NPC Chars in the world secors and the stuff they are carrying.
		// Sector lighting info.
		m_Sectors[m_iSaveStage].r_Write( m_File );
		break;

	case SECTOR_QTY:
		{
			// GM_Pages.
			CGMPage * pPage = dynamic_cast <CGMPage*>( m_GMPages.GetHead());
			for ( ; pPage!= NULL; pPage = pPage->GetNext())
			{
				pPage->r_Write( m_File );
			}
		}
		break;

	case SECTOR_QTY+1:
		// Save all my servers some place.
		if ( g_Serv.m_fMainLogServer )
		{
			for ( int i=0; i<g_Serv.m_Servers.GetCount(); i++ )
			{
				CServRef * pServ = g_Serv.m_Servers[i];
				if ( pServ == NULL )
					continue;
				pServ->WriteCreated( m_File );
			}
		}
		break;

	case SECTOR_QTY+2:
		// Now make a backup of the account file.
		SaveAccounts();
		break;

	case SECTOR_QTY+3:
		{
			// EOF marker to show we reached the end.
			m_File.WriteSection( "EOF" );

			m_iSaveCount++;	// Save only counts if we get to the end winout trapping.
			m_Clock_Save = GetTime() + g_Serv.m_iSavePeriod;	// next save time.

			g_Log.Event( LOGM_SAVE, "World data saved (%s).\n", m_File.GetFilePath());

			// Now clean up all the held over UIDs
			for ( int i=1; i<GetUIDCount(); i++ )
			{
				if ( m_UIDs[i] == UID_PLACE_HOLDER )
					m_UIDs[i] = NULL;
			}

			m_File.Close();
		}
		return( false );	// done.
	}

	if ( g_Serv.m_iSaveBackgroundTime )
	{
		int iNextTime = g_Serv.m_iSaveBackgroundTime / SECTOR_QTY;
		if ( iNextTime > TICK_PER_SEC/2 ) iNextTime = TICK_PER_SEC/2;	// max out at 30 minutes or so.
		m_Clock_Save = GetTime() + iNextTime;
	}
	m_iSaveStage ++;
	return( true );
}

void CWorld::SaveForce() // Save world state
{
	m_fSaveForce = true;
	Broadcast( "World save has been initiated." );
#ifdef VISUAL_SPHERE
	g_pServerObject->Fire_SaveBegin();
#endif
	while ( SaveStage())
	{
		if (! ( m_iSaveStage & 0xFF ))
		{
#ifndef VISUAL_SPHERE
			g_Serv.PrintPercent( m_iSaveStage, SECTOR_QTY+3 );
#endif
#ifdef _WIN32
			// Linux doesn't need to know about this
#ifdef VISUAL_SPHERE
			g_pServerObject->Fire_SavePercent(MulDiv( m_iSaveStage, 100, SECTOR_QTY+3 ));
#endif
			if ( g_Service.IsServiceStopPending())
			{
				g_Service.ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 5000);
			}
#endif
		}
	}
#ifdef VISUAL_SPHERE
	g_pServerObject->Fire_SaveEnd();
#endif
	DEBUG_MSG(( "Save Done\n" ));
}

void CWorld::SaveTry( bool fForceImmediate ) // Save world state
{
	if ( m_File.IsFileOpen())
	{
		// Save is already active !
		ASSERT( IsSaving());
		if ( fForceImmediate )	// finish it now !
		{
			SaveForce();
		}
		else if ( g_Serv.m_iSaveBackgroundTime )
		{
			SaveStage();
		}
		return;
	}

	// Do the write async from here in the future.
	if ( g_Serv.m_fSaveGarbageCollect )
	{
		GarbageCollection();
	}

	// Determine the save name based on the time.
	// exponentially degrade the saves over time.
	CGString sArchive;
	GetBackupName( sArchive, 'w' );

	// remove previous archive of same name
	remove( sArchive );

	// rename previous save to archive name.
	CGString sSaveName;
	sSaveName.Format( "%s" GRAY_FILE "world%s",
		(const TCHAR*) g_Serv.m_sWorldBaseDir,
		g_Serv.m_fSaveBinary ? GRAY_BINARY : GRAY_SCRIPT );

	if ( rename( sSaveName, sArchive ))
	{
		// May not exist if this is the first time.
		g_Log.Event( LOGM_SAVE|LOGL_ERROR, "World Save Rename to '%s' FAILED\n", (const TCHAR*) sArchive );
	}

	if ( ! m_File.Open( sSaveName, OF_WRITE|((g_Serv.m_fSaveBinary)?OF_BINARY:OF_TEXT)))
	{
		g_Log.Event( LOGM_SAVE|LOGL_CRIT, "Save '%s' failed\n", sSaveName );
		return;
	}

	m_fSaveParity = ! m_fSaveParity; // Flip the parity of the save.
	m_iSaveStage = -1;
	m_fSaveForce = false;
	m_Clock_Save = 0;

	m_File.WriteKey( "TITLE", GRAY_TITLE " World Script" );
	m_File.WriteKey( "VERSION", GRAY_VERSION );
	m_File.WriteKeyVal( "TIME", GetTime());
	m_File.WriteKeyVal( "SAVECOUNT", m_iSaveCount );
	m_File.Flush();	// Force this out to the file now.

	if ( fForceImmediate || ! g_Serv.m_iSaveBackgroundTime )	// Save now !
	{
		SaveForce();
	}
}

void CWorld::Save( bool fForceImmediate ) // Save world state
{
	if ( g_Serv.m_fSecure )	// enable the try code.
	{
		try
		{
			SaveTry(fForceImmediate);
		}
		catch (...)	// catch all
		{
			g_Log.Event( LOGL_CRIT|LOGM_SAVE, "Save FAILED. " GRAY_TITLE " is UNSTABLE!\n" );
			Broadcast( "Save FAILED. " GRAY_TITLE " is UNSTABLE!" );
			m_File.Close();	// close if not already closed.
		}
	}
	else
	{
		SaveTry(fForceImmediate); // Save world state
	}
}

/////////////////////////////////////////////////////////////////////

bool CWorld::LoadSection()
{
	// Load another section of the *WORLD.SCP file.

	if ( ! m_File.FindNextSection())
	{
		if ( m_File.IsSectionType( "EOF" ))
		{
			// The only valid way to end.
			m_File.Close();
			return( true );
		}

		g_Log.Event( LOGM_INIT|LOGL_CRIT, "No [EOF] marker. '%s' is corrupt!\n", m_File.GetFilePath());

		// Auto change to the most recent previous backup !
		// Try to load a backup file instead ?
		Close();
		LoadRegions();	// reload these as they are killed by the Close()

		CGString sArchive;
		GetBackupName( sArchive, 'w' );

		if ( ! sArchive.CompareNoCase( m_File.GetFilePath()))	// ! same file ?
		{
			return( false );
		}
		if ( ! m_File.Open( sArchive ))
		{
			g_Log.Event( LOGL_FATAL|LOGM_INIT, "No previous backup '%s'!\n", m_File.GetFilePath());
			return( false );
		}

		g_Log.Event( LOGM_INIT, "Loading previous backup '%s'\n", m_File.GetFilePath());

		// Find the size of the file.
		m_lLoadSize = m_File.GetLength();
		m_iSaveStage = 0;	// start over at the top of the count.

		// Read the header stuff first.
		return( CScriptObj::r_Load( m_File ));
	}

	if (! ( ++m_iSaveStage & 0xFF ))	// don't update too often
	{
#ifdef VISUAL_SPHERE
	   g_pServerObject->Fire_LoadPercent(MulDiv( m_File.GetPosition(), 100, m_lLoadSize ));
#else
		int iPercent = g_Serv.PrintPercent( m_File.GetPosition(), m_lLoadSize );
#endif
	}

	if ( m_File.IsSectionType( "SECTOR" ))
	{
		CPointMap p;
		p.Read( m_File.GetArgStr());
		return( p.GetSector()->r_Load(m_File));
	}
	else if ( m_File.IsSectionType( "WORLDCHAR" ))
	{
		return( CChar::CreateBasic((CREID_TYPE) m_File.GetArgHex())->r_Load(m_File));
	}
	else if ( m_File.IsSectionType( "WORLDITEM" ))
	{
		return( CItem::CreateScript((ITEMID_TYPE) m_File.GetArgHex())->r_Load(m_File));
	}
	else if ( m_File.IsSectionType( "GMPAGE" ))
	{
		return(( new CGMPage( m_File.GetArgStr()))->r_Load( m_File ));
	}
	else if ( m_File.IsSectionType( "SERVER" ))
	{
		CServRef * pServ = new CServRef( m_File.GetArgStr(), SOCKET_LOCAL_ADDRESS );
		pServ->r_Load( m_File );
		if ( pServ->GetName()[0] == '\0' )
		{
			delete pServ;
		}
		else
		{
			g_Serv.m_Servers.AddSortKey( pServ, pServ->GetName() );
		}
	}

	return( true );
}

bool CWorld::Load() // Load world from script
{
#ifdef VISUAL_SPHERE
	g_pServerObject->Fire_LoadBegin();
#endif
	DEBUG_CHECK( g_Serv.IsLoading());

	// The world has just started.
	m_UIDs.SetCount( 8*1024 );	// start count. (will grow as needed)
	m_Clock_Time = 0;

	// Load region info.
	if ( ! LoadRegions())
		return( false );
	// Load all the accounts.
	if ( ! LoadAccounts( false ))
		return( false );

	CGString sSaveName;
	sSaveName.Format( "%s" GRAY_FILE "world", (const TCHAR*) g_Serv.m_sWorldBaseDir );

	if ( ! m_File.Open( sSaveName ))
	{
		g_Log.Event( LOGM_INIT|LOGL_ERROR, "No world data file\n" );
	}
	else
	{
		g_Serv.SysMessage( "World Is Loading...\n" );

		// Find the size of the file.
		m_lLoadSize = m_File.GetLength();
		m_iSaveStage = 0;	// Load stage as well.
		g_Log.SetScriptContext( &m_File );

		// Read the header stuff first.
		CScriptObj::r_Load( m_File );

		while ( m_File.IsFileOpen())
		{
			if ( g_Serv.m_fSecure )	// enable the try code.
			{
				try
				{
					if ( ! LoadSection())
					{
						g_Log.SetScriptContext( NULL );
						return( false );
					}
				}
				catch (...)	// catch all
				{
					g_Log.Event( LOGM_INIT|LOGL_CRIT, "Load Exception line %d " GRAY_TITLE " is UNSTABLE!\n", m_File.GetLineNumber());
				}
			}
			else
			{
				if ( ! LoadSection())
				{
					g_Log.SetScriptContext( NULL );
					return( false );
				}
			}
		}
		m_File.Close();
	}

	g_Log.SetScriptContext( NULL );

	m_Clock_Startup = GetTime();
	m_Clock_Save = GetTime() + g_Serv.m_iSavePeriod;	// next save time.
	m_Clock_PrevSys = getclock();

	CItemBase::FindItemBase( ITEMID_ArrowX );	// Make sure these get loaded.
	CItemBase::FindItemBase( ITEMID_XBoltX );
	CItemBase::FindItemBase( ITEMID_Arrow );
	CItemBase::FindItemBase( ITEMID_XBolt );

	// Set all the sector light levels now that we know the time.
	for ( int j=0; j<SECTOR_QTY; j++ )
	{
		if ( ! m_Sectors[j].IsLightOverriden())
		{
			m_Sectors[j].SetLight(-1);
		}

		// Is this area too complex ?
		int iCount = m_Sectors[j].m_Items_Timer.GetCount() + m_Sectors[j].m_Items_Inert.GetCount();
		if ( iCount > 1024 )
		{
			DEBUG_ERR(( "Warning: %d items at %s, Sector too complex!\n", iCount, m_Sectors[j].GetBase().Write()));
		}
	}

	GarbageCollection();

	// Set the current version now.
	r_SetVal( "VERSION", GRAY_VERSION );	// Set m_iSaveVersion
#ifdef VISUAL_SPHERE
	g_pServerObject->Fire_LoadEnd();
#endif
	return( true );
}

void CWorld::ReLoadBases()
{
	// After a pause/resync all the items need to resync their m_pDef pointers. (maybe)

	for ( int i=1; i<GetUIDCount(); i++ )
	{
		CObjBase * pObj = m_UIDs[i];
		if ( pObj == NULL || pObj == UID_PLACE_HOLDER ) 
			continue;	// not used.

		if ( pObj->IsItem())
		{
			CItem * pItem = dynamic_cast <CItem *>(pObj);
			ASSERT(pItem);
			pItem->SetBaseID( pItem->GetID()); // re-eval the m_pDef stuff.
		}
		else
		{
			CChar * pChar = dynamic_cast <CChar *>(pObj);
			ASSERT(pChar);
			pChar->SetID( pChar->GetID());	// re-eval the m_pDef stuff.
		}
	}
}

/////////////////////////////////////////////////////////////////

const TCHAR * CWorld::sm_Table[] =	// static
{
	"SAVECOUNT",
	"TIME",
	"TITLE",
	"VERSION",
};

bool CWorld::r_WriteVal( const TCHAR *pKey, CGString &sVal, CTextConsole * pSrc )
{
	switch ( FindTableSorted( pKey, sm_Table, COUNTOF(sm_Table)))
	{
	case 0: // "SAVECOUNT"
		sVal.FormatVal( m_iSaveCount );
		break;
	case 1:	// "TIME"
		sVal.FormatVal( GetTime());
		break;
	case 2: // 	"TITLE",
		sVal = GRAY_TITLE " World Script";
		break;
	case 3: // "VERSION"
		sVal = GRAY_VERSION;
		break;
	default:
		return( false );
	}
	return( true );
}

bool CWorld::r_LoadVal( CScript &s )
{
	switch ( FindTableSorted( s.GetKey(), sm_Table, COUNTOF(sm_Table)))
	{
	case 0: // "SAVECOUNT"
		m_iSaveCount = s.GetArgVal();
		break;
	case 1:	// "TIME"
		if ( m_Clock_Time )
		{
			DEBUG_MSG(( "Setting TIME while running is BAD!\n" ));
		}
		m_Clock_Time = s.GetArgVal();
		break;
	case 2: // 	"TITLE",
		return( false );
	case 3: // "VERSION"
		if ( s.m_pArg[0] == '0' ) 
			s.m_pArg++;
		m_iSaveVersion = s.GetArgVal();
		break;
	default:
		return( false );
	}

	return( true );
}

bool CWorld::r_Verb( CScript & s, CTextConsole * pSrc )
{
#ifdef COMMENT
	"SAVE",
	"BROADCAST"
#endif
	return( CScriptObj::r_Verb( s, pSrc ));
}

void CWorld::RespawnDeadNPCs()
{
	// Respawn dead NPC's
	for ( int i = 0; i<SECTOR_QTY; i++ )
	{
		m_Sectors[i].RespawnDeadNPCs();
	}
}

void CWorld::Restock()
{
	for ( int i = 0; i<SECTOR_QTY; i++ )
	{
		m_Sectors[i].Restock();
	}
}

void CWorld::Close()
{
	if ( IsSaving())	// Must complete save now !
	{
		Save( true );
	}
	m_File.Close();
	for ( int i = 0; i<SECTOR_QTY; i++ )
	{
		m_Sectors[i].Close();
	}

	m_ObjDelete.DeleteAll();	// clean up our delete list.
	m_ItemsNew.DeleteAll();
	m_CharsNew.DeleteAll();
	m_UIDs.RemoveAll();
	m_Regions.DeleteAll();
	m_GMPages.DeleteAll();
}

int CWorld::FixObjTry( CObjBase * pObj, int iUID )
{
	// RETURN: 0 = success.
	if ( iUID )
	{
		if (( pObj->GetUID() & UID_MASK ) != iUID )
		{
			// Miss linked in the UID table !!! BAD
			// Hopefully it was just not linked at all. else How the hell should i clean this up ???
			DEBUG_ERR(( "UID 0%x, '%s', Mislinked\n", iUID, pObj->GetName()));
			return( 0x7101 );
		}
	}
	return pObj->FixWeirdness();
}

int CWorld::FixObj( CObjBase * pObj, int iUID )
{
	// Attempt to fix problems with this item.
	// Ignore any children it may have for now.
	// RETURN: 0 = success.
	//  

	int iResultCode;
	if ( g_Serv.m_fSecure )	// enable the try code.
	{
		try
		{
			iResultCode = FixObjTry(pObj,iUID);
		}
		catch (...)	// catch all
		{
			iResultCode = 0xFFFF;	// bad mem ?
		}
	}
	else
	{
		iResultCode = FixObjTry(pObj,iUID);
	}
	if ( ! iResultCode ) 
		return( 0 );

#ifdef _DEBUG
	CItem * pItem = dynamic_cast <CItem*>(pObj);
	CChar * pChar = dynamic_cast <CChar*>(pObj);
#endif

	if ( g_Serv.m_fSecure )	// enable the try code.
	{
		try
		{
			iUID = pObj->GetUID();

			// is it a real error ?
			if ( pObj->IsItem())
			{
				CItem * pItem = dynamic_cast <CItem*>(pObj);
				if ( pItem && pItem->m_type == ITEM_EQ_MEMORY_OBJ )
				{
					pObj->Delete();
					return iResultCode;
				}
			}
			DEBUG_ERR(( "UID=0%x, id=0%x '%s', Invalid code=%0x\n", iUID, pObj->GetBaseID(), pObj->GetName(), iResultCode ));
			pObj->Delete();
		}
		catch (...)	// catch all
		{
			DEBUG_ERR(( "UID=0%x, Asserted cleanup\n", iUID ));
		}
	}
	else
	{
		// is it a real error ?
		iUID = pObj->GetUID();
		if ( pObj->IsItem())
		{
			CItem * pItem = dynamic_cast <CItem*>(pObj);
			if ( pItem && pItem->m_type == ITEM_EQ_MEMORY_OBJ )
			{
				pObj->Delete();
				return iResultCode;
			}
		}
		DEBUG_ERR(( "UID=0%x, id=0%x '%s', Invalid code=%0x\n", iUID, pObj->GetBaseID(), pObj->GetName(), iResultCode ));
		pObj->Delete();
	}
	return( iResultCode );
}

void CWorld::GarbageCollection_New()
{
	if ( m_ItemsNew.GetCount())
	{
		g_Log.Event( LOGL_ERROR, "%d Lost items deleted\n", m_ItemsNew.GetCount());
		m_ItemsNew.DeleteAll();
	}
	if ( m_CharsNew.GetCount())
	{
		g_Log.Event( LOGL_ERROR, "%d Lost chars deleted\n", m_CharsNew.GetCount());
		m_CharsNew.DeleteAll();
	}
}

void CWorld::GarbageCollection_GMPages()
{
	// Make sure all GM pages have accounts.
	CGMPage * pPage = dynamic_cast <CGMPage*>( m_GMPages.GetHead());
	while ( pPage!= NULL )
	{
		CGMPage * pPageNext = pPage->GetNext();
		if ( ! pPage->FindAccount()) // Open script file
		{
			DEBUG_ERR(( "GM Page has invalid account '%s'\n", pPage->GetName()));
			delete pPage;
		}
		pPage = pPageNext;
	}
}

void CWorld::GarbageCollection()
{
	g_Log.Flush();
	GarbageCollection_New();
	GarbageCollection_GMPages();

	// Go through the m_ppUIDs looking for Objects without links to reality.
	// This can take a while.

	int iCount = 0;
	for ( int i=1; i<GetUIDCount(); i++ )
	{
		CObjBase * pObj = m_UIDs[i];
		if ( pObj == NULL || pObj == UID_PLACE_HOLDER ) 
			continue;	// not used.

		// Look for anomolies and fix them (that might mean delete it.)
		int iResultCode = FixObj( pObj, i );
		if ( iResultCode )
		{
			// Do an immediate delete here instead of Delete()
			try
			{
				delete pObj;
			}
			catch(...)
			{
			}
			FreeUID(i);	// Get rid of junk uid if all fails..
			continue;
		}
#ifdef VISUAL_SPHERE
		g_pServerObject->Fire_LoadPercent(MulDiv( iCount, 100, GetUIDCount() ));
#else
		g_Serv.PrintPercent( iCount, GetUIDCount());
#endif
		if (! (iCount & 0xFF ))
		{
			g_Serv.PrintPercent( iCount, GetUIDCount());
		}
		iCount ++;
	}

	World_fDeleteCycle = true;
	m_ObjDelete.DeleteAll();	// clean up our delete list.
	World_fDeleteCycle = false;

	if ( iCount != CObjBase::sm_iCount )	// All objects must be accounted for.
	{
		g_Log.Event( LOGL_ERROR, "Object memory leak %d!=%d\n", iCount, CObjBase::sm_iCount );
	}
	else
	{
		g_Log.Event( LOGL_EVENT, "%d Objects accounted for\n", iCount );
	}

	g_Log.Flush();
}

void CWorld::Speak( const CObjBaseTemplate * pSrc, const TCHAR * pText, COLOR_TYPE color, TALKMODE_TYPE mode, FONT_TYPE font )
{
	bool fSpeakAsGhost = false;	// I am a ghost ?
	int iHearRange = 0xFFFF;	// Broadcast i guess ?
	const CChar * pCharSrc = NULL;
	if ( pSrc != NULL )
	{
		pSrc = pSrc->GetTopLevelObj();
		switch ( mode )
		{
		case TALKMODE_YELL:
			iHearRange = pSrc->GetVisualRange() * 2;
			break;
		case TALKMODE_BROADCAST:
			iHearRange = 0xFFFF;
			break;
		case TALKMODE_WHISPER:
			iHearRange = 3;
			break;
		default:
			iHearRange = UO_MAP_VIEW_SIZE;
			break;
		}
		if ( pSrc->IsChar())
		{
			// Are they dead ? Garble the text. unless we have SS
			pCharSrc = dynamic_cast <const CChar*> (pSrc);
			fSpeakAsGhost = pCharSrc->IsSpeakAsGhost();
		}
	}

	CGString sTextName;	// name labelled text.
	CGString sTextGhost; // ghost speak.

	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient->IsConsole())
		{
			if ( iHearRange != 0xFFFF ) 
				continue;	// not broadcast.
			pClient->SysMessage(pText);
			continue;
		}

		const TCHAR * pszSpeak = pText;
		if ( pSrc != NULL )
		{
			CChar * pChar = pClient->GetChar();
			if ( pChar == NULL ) 
				continue;	// not logged in.
			int iDist = pChar->GetTopDist3D( pSrc ) ;
			if ( iDist > iHearRange )
			{
				if ( ! pClient->IsPriv( PRIV_HEARALL ))	// in range based on mode ?
				{
					continue;
				}
				if ( pCharSrc && pCharSrc->GetPrivLevel() > pClient->GetPrivLevel())
				{
					continue;
				}
			}
			if ( iDist > UO_MAP_VIEW_SIZE )	// Must label the text.
			{
				if ( sTextName.IsEmpty())
				{
					if ( pClient->IsPriv( PRIV_HEARALL ))
					{
						sTextName.Format( "<%s [%lx]>%s", pSrc->GetName(), pSrc->GetUID(), pText );
					}
					else
					{
						sTextName.Format( "<%s>%s", pSrc->GetName(), pText );
					}
				}
				pszSpeak = sTextName;
			}
			else if ( fSpeakAsGhost && ! pChar->CanUnderstandGhost())
			{
				if ( sTextGhost.IsEmpty())	// Garble ghost.
				{
					sTextGhost = pText;
					for ( int i=0; i<sTextGhost.GetLength(); i++ )
					{
						if ( sTextGhost[i] != ' ' &&  sTextGhost[i] != '\t' )
						sTextGhost[i] = GetRandVal(2) ? 'O' : 'o';
					}
				}
				pszSpeak = sTextGhost;
			}
		}

		if ( fSpeakAsGhost && 
			g_Serv.m_fPlayerGhostSounds &&
			pSrc != NULL &&
			! pClient->GetChar()->CanUnderstandGhost())
		{
			pClient->addSound( Sounds_Ghost[ GetRandVal( COUNTOF( Sounds_Ghost )) ], pSrc );
		}

		pClient->addBark( pszSpeak, pSrc, color, mode, font );
	}
}

void CWorld::SpeakUNICODE( const CObjBaseTemplate * pSrc, const NCHAR * pText, COLOR_TYPE color, TALKMODE_TYPE mode, FONT_TYPE font, const char * pszLanguage )
{
	bool fSpeakAsGhost = false;	// I am a ghost ?
	int iHearRange = 0xFFFF;	// Broadcast i guess ?
	const CChar * pCharSrc = NULL;
	if ( pSrc != NULL )
	{
		pSrc = pSrc->GetTopLevelObj();
		switch ( mode )
		{
		case TALKMODE_YELL:
			iHearRange = pSrc->GetVisualRange() * 2;
			break;
		case TALKMODE_BROADCAST:
			iHearRange = 0xFFFF;
			break;
		case TALKMODE_WHISPER:
			iHearRange = 3;
			break;
		default:
			iHearRange = UO_MAP_VIEW_SIZE;
			break;
		}
		if ( pSrc->IsChar())
		{
			// Are they dead ? Garble the text. unless we have SS
			pCharSrc = dynamic_cast <const CChar*> (pSrc);
			fSpeakAsGhost = pCharSrc->IsSpeakAsGhost();
		}
	}

	//NCHAR wTextName[256];	// name labelled text.
	//NCHAR wTextGhost[256]; // ghost speak.

	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient->IsConsole())
		{
			if ( iHearRange != 0xFFFF )
				continue;
			// pClient->SysMessage(pText);
			continue;
		}
		const NCHAR * pszSpeak = pText;
		if ( pSrc != NULL )
		{
			CChar * pChar = pClient->GetChar();
			if ( pChar == NULL )
				continue;	// not logged in.
			int iDist = pChar->GetTopDist3D( pSrc ) ;
			if ( iDist > iHearRange )
			{
				if ( ! pClient->IsPriv( PRIV_HEARALL ))	// in range based on mode ?
				{
					continue;
				}
				if ( pCharSrc && pCharSrc->GetPrivLevel() > pClient->GetPrivLevel())
				{
					continue;
				}
			}

#if 0
			if ( iDist > UO_MAP_VIEW_SIZE )	// Must label the text.
			{
				if ( sTextName.IsEmpty())
				{
					if ( pClient->IsPriv( PRIV_HEARALL ))
					{
						sTextName.Format( "<%s [%lx]>%s", pSrc->GetName(), pSrc->GetUID(), pText );
					}
					else
					{
						sTextName.Format( "<%s>%s", pSrc->GetName(), pText );
					}
				}
				pszSpeak = sTextName;
			}
			else if ( fSpeakAsGhost && ! pChar->CanUnderstandGhost())
			{
				if ( sTextGhost.IsEmpty())	// Garble ghost.
				{
					sTextGhost = pText;
					for ( int i=0; i<sTextGhost.GetLength(); i++ )
					{
						if ( sTextGhost[i] != ' ' && sTextGhost[i] != '\t' )
							sTextGhost[i] = GetRandVal(2) ? 'O' : 'o';
					}
				}
				pszSpeak = sTextGhost;
			}
#endif

		}

		if ( fSpeakAsGhost && 
			g_Serv.m_fPlayerGhostSounds &&
			pSrc != NULL &&
			! pClient->GetChar()->CanUnderstandGhost())
		{
			pClient->addSound( Sounds_Ghost[ GetRandVal( COUNTOF( Sounds_Ghost )) ], pSrc );
		}

		pClient->addBarkUNICODE( pszSpeak, pSrc, color, mode, font, pszLanguage );
	}
}

void CWorld::Broadcast( const TCHAR *pMsg ) // System broadcast in bold text
{
	Speak( NULL, pMsg, COLOR_TEXT_DEF, TALKMODE_BROADCAST, FONT_BOLD );
	g_Serv.SocketsFlush();
}

CItem * CWorld::Explode( CChar * pSrc, CPointMap p, int iDist, int iDamage, WORD wFlags )
{
	// Purple potions and dragons fire.
	// degrade damage the farther away we are. ???

	CItem * pItem = CItem::CreateBase( ITEMID_FX_EXPLODE_3 );
	pItem->m_Attr |= ATTR_MOVE_NEVER | ATTR_CAN_DECAY;
	pItem->m_type = ITEM_EXPLOSION;
	pItem->m_uidLink = pSrc ? pSrc->GetUID() : ((CObjUID) UID_CLEAR );
	pItem->m_itExplode.m_iDamage = iDamage;
	pItem->m_itExplode.m_wFlags = wFlags | DAMAGE_GENERAL | DAMAGE_HIT;
	pItem->m_itExplode.m_iDist = iDist;
	pItem->MoveTo( p );
	pItem->SetTimeout( 1 );	// Immediate.
	pItem->Sound( 0x207 );

	return( pItem );
}

//////////////////////////////////////////////////////////////////
// Game time.

#ifndef _WIN32
unsigned long int getclock()
{
	// clock_t times(struct tms *buf);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long TempTime;
    TempTime = ((((tv.tv_sec - 912818000) * CLOCKS_PER_SEC) +
		tv.tv_usec / CLOCKS_PER_SEC));
	return (TempTime);
}
#endif

DWORD CWorld::GetGameWorldTime( DWORD basetime ) const
{
	// basetime = TICK_PER_SEC time.
	// Get the time of the day in GameWorld minutes
	// 8 real world seconds = 1 game minute.
	// 1 real minute = 7.5 game minutes
	// 3.2 hours = 1 game day.
	return( basetime / g_Serv.m_iGameMinuteLength );
}

DWORD CWorld::GetNextNewMoon( bool bMoonIndex ) const
{
	// "Predict" the next new moon for this moon
	// Get the period
	DWORD iSynodic = bMoonIndex ? FELUCCA_SYNODIC_PERIOD : TRAMMEL_SYNODIC_PERIOD;

	// Add a "month" to the current game time
	DWORD iNextMonth = g_World.GetTime() + iSynodic;

	// Get the game time when this cycle will start
	DWORD iNewStart = (DWORD) (iNextMonth -
		(double) (iNextMonth % iSynodic));

	// Convert to ticks
	return( iNewStart * g_Serv.m_iGameMinuteLength * TICK_PER_SEC );
}

int CWorld::GetMoonPhase (bool bMoonIndex) const
{
	// bMoonIndex is FALSE if we are looking for the phase of Trammel,
	// TRUE if we are looking for the phase of Felucca.

	// There are 8 distinct moon phases:  New, Waxing Crescent, First Quarter, Waxing Gibbous,
	// Full, Waning Gibbous, Third Quarter, and Waning Crescent

	// To calculate the phase, we use the following formula:
	//				CurrentTime % SynodicPeriod
	//	Phase = 	-----------------------------------------     * 8
	//			              SynodicPeriod
	//

	DWORD dwCurrentTime = GetGameWorldTime();

	if (!bMoonIndex)
	{
		// Trammel
		return( IMULDIV( dwCurrentTime % TRAMMEL_SYNODIC_PERIOD, 8, TRAMMEL_SYNODIC_PERIOD ));
	}
	else
	{
		// Luna2
		return( IMULDIV( dwCurrentTime % FELUCCA_SYNODIC_PERIOD, 8, FELUCCA_SYNODIC_PERIOD ));
	}
}

const TCHAR * CWorld::GetGameTime() const
{
	return( GetTimeMinDesc( GetGameWorldTime()));
}

void CWorld::OnTick()
{
	// Set the game time from the real world clock. getclock()
	// Do this once per tick.
	// 256 real secs = 1 GRAYhour. 19 light levels. check every 10 minutes or so.

	if ( g_Serv.m_fResyncPause )
		return;

	DWORD Clock_Sys = getclock();	// get the system time.
	int iTimeSysDiff = Clock_Sys - m_Clock_PrevSys;
	iTimeSysDiff = IMULDIV( TICK_PER_SEC, iTimeSysDiff, CLOCKS_PER_SEC );

	if ( iTimeSysDiff <= 0 )
	{
		if ( iTimeSysDiff == 0 ) // time is less than TICK_PER_SEC
		{
			return;
		}

		DEBUG_ERR(( "WARNING:system clock 0%xh overflow - recycle\n", Clock_Sys ));
		m_Clock_PrevSys = Clock_Sys;
		// just wait til next cycle and we should be ok
		return;
	}

	m_Clock_PrevSys = Clock_Sys;

	// NOTE: WARNING: time_t is signed !
	time_t Clock_New = m_Clock_Time + iTimeSysDiff;
	if ( Clock_New <= m_Clock_Time )	// should not happen (overflow)
	{
		if ( m_Clock_Time == Clock_New )
		{
			// Weird. This should never happen ?!
			g_Log.Event( LOGL_CRIT, "Clock corruption?" );
			return;	
		}

		// Someone has probably messed with the "TIME" value.
		// Very critical !
		g_Log.Event( LOGL_CRIT, "Clock overflow, reset from 0%x to 0%x\n", m_Clock_Time, Clock_New );
		m_Clock_Time = Clock_New;	// this may cause may strange things.
		return;
	}

	m_Clock_Time = Clock_New;

	if ( m_Clock_Sector <= GetTime())
	{
		// Only need a SECTOR_TICK_PERIOD tick to do world stuff.
		m_Clock_Sector = GetTime() + SECTOR_TICK_PERIOD;	// Next hit time.
		m_Sector_Pulse ++;
		for ( int i=0; i<SECTOR_QTY; i++ )
		{
			m_Sectors[i].OnTick( m_Sector_Pulse );
		}

		World_fDeleteCycle = true;
		m_ObjDelete.DeleteAll();	// clean up our delete list.
		World_fDeleteCycle = false;
	}
	if ( m_Clock_Save <= GetTime())
	{
		// Auto save world
		m_Clock_Save = GetTime() + g_Serv.m_iSavePeriod;
		g_Log.Flush();
		Save( false );
	}
	if ( m_Clock_Respawn <= GetTime())
	{
		// Time to regen all the dead NPC's in the world.
		m_Clock_Respawn = GetTime() + (20*60*TICK_PER_SEC);
		RespawnDeadNPCs();
	}
}

