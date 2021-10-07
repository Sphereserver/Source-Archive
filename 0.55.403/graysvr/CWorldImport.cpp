//
// CWorldImport.cpp
// Copyright Menace Software (www.menasoft.com).
//

#include "graysvr.h"	// predef header.

struct CImportSer : public CGObListRec
{
	// Temporary holding structure for new objects being impoted.
protected:
	DECLARE_MEM_DYNAMIC;
public:
	// Translate the import UID's into my UID's
	const DWORD m_dwSer;		// My Imported serial number
	CObjBase * m_pObj;	// new world object corresponding.
	DWORD m_dwContSer;	// My containers' serial number
	LAYER_TYPE m_layer;	// UOX does this diff than us. so store this here.
public:
	bool IsTopLevel() const
	{
		return( m_dwContSer == UID_UNUSED );
	}

	CImportSer( DWORD dwSer ) :
		m_dwSer( dwSer )
	{
		m_pObj = NULL;
		m_dwContSer = UID_UNUSED;
		m_layer = LAYER_NONE;
	}
	~CImportSer()
	{
		// if ( m_pObj != NULL ) delete m_pObj;
	}
};

struct CImportFile
{
	CObjBase * m_pCurObj;		// current entry.
	CImportSer * m_pCurSer;	// current entry.

	CGObList m_ListSer;	// list of imported objects i'm working on.

	const WORD m_wModeFlags;	// IMPFLAGS_TYPE
	const CPointMap m_ptCenter;
	const int m_iDist;	// distance fom center.

	TCHAR * m_pszArg1;	// account
	TCHAR * m_pszArg2;	// name

public:
	CImportFile( WORD wModeFlags, CPointMap ptCenter, int iDist ) :
		m_wModeFlags(wModeFlags),
		m_ptCenter(ptCenter),
		m_iDist(iDist)
	{
		m_pCurSer = NULL;
		m_pCurObj = NULL;
	}
	void CheckLast();
	void ImportFix();
	bool ImportSCP( CScript & s, WORD wModeFlags );
	bool ImportWSC( CScript & s, WORD wModeFlags );
};

void CImportFile::CheckLast()
{
	// Make sure the object we where last working on is linked to the world.

	if ( m_pCurSer != NULL )
	{
		if ( m_pCurObj != NULL && m_pCurSer->m_pObj == m_pCurObj )
		{
			// Do we even want it ?
			if ( m_iDist &&
				m_ptCenter.IsValidPoint() &&
				m_pCurSer->IsTopLevel() &&
				m_ptCenter.GetDist( m_pCurObj->GetTopPoint()) > m_iDist )
			{
				delete m_pCurSer;
			}
			else
			{
				m_pCurObj = NULL;	// accept it.
			}
		}
		else
		{
			delete m_pCurSer;
		}
		m_pCurSer = NULL;
	}
	if ( m_pCurObj != NULL )
	{
		delete m_pCurObj;
		m_pCurObj = NULL;
	}
}

void CImportFile::ImportFix()
{
	// adjust all the containered items and eliminate duplicates.

	CheckLast();

	int iRemoved = 0;

	CImportSer * pSerNext;
	m_pCurSer = STATIC_CAST <CImportSer*> ( m_ListSer.GetHead());
	for ( ; m_pCurSer != NULL; m_pCurSer = pSerNext )
	{
		pSerNext = STATIC_CAST <CImportSer*> ( m_pCurSer->GetNext());
		if ( m_pCurSer->m_pObj == NULL )		// NEver created correctly
		{
			delete m_pCurSer;
			continue;
		}

		// Make sure this item is not a dupe ?

		CItem * pItemTest;
		if ( m_pCurSer->IsTopLevel())	// top level only
		{
			if ( m_pCurSer->m_pObj->IsItem())
			{
				CItem * pItemCheck = dynamic_cast <CItem*>( m_pCurSer->m_pObj );
				ASSERT(pItemCheck);
#if 1
				pItemCheck->SetAttr(ATTR_MOVE_NEVER);
#endif
				CWorldSearch AreaItems( m_pCurSer->m_pObj->GetTopPoint());
				while (true)
				{
					CItem * pItem = AreaItems.GetItem();
					if ( pItem == NULL )
						break;
					if ( ! pItem->IsSameType( m_pCurSer->m_pObj ))
						continue;
					pItem->SetName( m_pCurSer->m_pObj->GetName());
					if ( ! ( m_pCurSer->m_pObj->GetTopZ() == pItem->GetTopZ()))
						continue;

					if ( g_Log.IsLogged( LOGL_TRACE ))
					{
						DEBUG_ERR(( "Import: delete dupe item\n" ));
					}
					goto item_delete;
				}
			}
			else
			{
				// dupe char ?
			}

			// Make sure the top level object is placed correctly.
			m_pCurSer->m_pObj->MoveTo( m_pCurSer->m_pObj->GetTopPoint());
			if ( ! m_pCurSer->m_pObj->IsContainer())
				delete m_pCurSer;
			continue;
		}

		pItemTest = dynamic_cast <CItem*> (m_pCurSer->m_pObj);
		if ( pItemTest == NULL )
		{
			if ( g_Log.IsLogged( LOGL_TRACE ))
			{
				DEBUG_ERR(( "Import:Invalid item in container\n" ));
			}
		item_delete:
			delete m_pCurSer->m_pObj;
			delete m_pCurSer;
			iRemoved ++;
			continue;
		}

		// Find it's container.
		CImportSer* pSerCont = STATIC_CAST <CImportSer*> ( m_ListSer.GetHead());
		CObjBase * pObjCont;
		for ( ; pSerCont != NULL; pSerCont = STATIC_CAST <CImportSer*> ( pSerCont->GetNext()))
		{
			if ( pSerCont->m_pObj == NULL )
				continue;
			if ( pSerCont->m_dwSer == m_pCurSer->m_dwContSer )
			{
				pObjCont = pSerCont->m_pObj;
				if ( ! pItemTest->LoadSetContainer( pObjCont->GetUID(), m_pCurSer->m_layer ))
				{
					goto item_delete;	// not in a cont ?
				}
				m_pCurSer->m_dwContSer = UID_UNUSED;	// found it.
				break;
			}
		}
		if ( ! m_pCurSer->IsTopLevel())
		{
			if ( g_Log.IsLogged( LOGL_TRACE ))
			{
				DEBUG_ERR(( "Import:Invalid item container\n" ));
			}
			goto item_delete;
		}

		// Is it a dupe in the container or equipped ?
		CItem * pItem = dynamic_cast <CContainer*>(pObjCont)->GetContentHead();
		for ( ; pItem != NULL; pItem = pItem->GetNext())
		{
			if ( pItemTest == pItem )
				continue;
			if ( pItemTest->IsItemEquipped())
			{
				if ( pItemTest->GetEquipLayer() != pItem->GetEquipLayer())
					continue;
			}
			else
			{
				if ( ! pItemTest->GetContainedPoint().IsSame2D( pItem->GetContainedPoint()))
					continue;
			}
			if ( ! pItemTest->IsSameType( pItem ))
				continue;
			if ( g_Log.IsLogged( LOGL_TRACE ))
			{
				DEBUG_ERR(( "Import: delete dupe item\n" ));
			}
			goto item_delete;
		}

		// done with it if not a container.
		if ( ! pItemTest->IsContainer())
			delete m_pCurSer;
	}

	if ( iRemoved )
	{
		DEBUG_ERR(( "Import: removed %d bad items\n", iRemoved ));
	}
	m_ListSer.DeleteAll();	// done with the list now.
}

bool CImportFile::ImportSCP( CScript & s, WORD wModeFlags )
{
	// Import a SPHERE format SCP file.

	while ( s.FindNextSection())
	{
		CheckLast();
		if ( s.IsSectionType( "ACCOUNT" ))
		{
			g_Cfg.LoadResourceSection( &s );
			continue;
		}
		else if ( s.IsSectionType( "WORLDCHAR" ))
		{
			ImportFix();
			if ( wModeFlags & IMPFLAGS_CHARS )
			{
				m_pCurObj = CChar::CreateBasic( (CREID_TYPE) g_Cfg.ResourceGetIndexType( RES_CHARDEF, s.GetArgStr()));
			}
		}
		else if ( s.IsSectionType( "WORLDITEM" ))
		{
			if ( wModeFlags & IMPFLAGS_ITEMS )
			{
				m_pCurObj = CItem::CreateTemplate( (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, s.GetArgStr()));
			}
		}
		else
		{
			continue;
		}

		if ( m_pCurObj == NULL )
			continue;

		while ( s.ReadKeyParse())
		{
			if ( s.IsKey( "SERIAL"))
			{
				if ( m_pCurSer != NULL )
					return( false );
				m_pCurSer = new CImportSer( s.GetArgVal());
				m_pCurSer->m_pObj = m_pCurObj;
				m_ListSer.InsertHead( m_pCurSer );
				continue;
			}

			if ( m_pCurSer == NULL )
				continue;

			if ( s.IsKey( "CONT" ))
			{
				m_pCurSer->m_dwContSer = s.GetArgVal();
			}
			else if ( s.IsKey( "LAYER" ))
			{
				m_pCurSer->m_layer = (LAYER_TYPE) s.GetArgVal();
			}
			else
			{
				m_pCurObj->r_LoadVal( s );
			}
		}
	}

	return( true );
}

bool CImportFile::ImportWSC( CScript & s, WORD wModeFlags )
{
	// This file is a WSC or UOX world script file.

	int mode = IMPFLAGS_NOTHING;
	CGString sName;
	CItem * pItem;
	CChar * pChar;

	while ( s.ReadTextLine(true))
	{
		if ( s.IsKeyHead( "SECTION WORLDITEM", 17 ))
		{
			CheckLast();
			mode = IMPFLAGS_ITEMS;
			continue;
		}
		else if ( s.IsKeyHead( "SECTION CHARACTER", 17 ))
		{
			CheckLast();
			mode = ( wModeFlags & IMPFLAGS_CHARS ) ? IMPFLAGS_CHARS : IMPFLAGS_NOTHING;
			continue;
		}
		else if ( s.GetKey()[0] == '{' )
		{
			CheckLast();
			continue;
		}
		else if ( s.GetKey()[0] == '}' )
		{
			CheckLast();
			mode = IMPFLAGS_NOTHING;
			continue;
		}
		else if ( mode == IMPFLAGS_NOTHING )
			continue;
		else if ( s.GetKey()[0] == '\\' )
			continue;

		// Parse the line.
		TCHAR * pArg = strchr( s.GetKey(), ' ' );
		if ( pArg != NULL )
		{
			*pArg++ = '\0';
			GETNONWHITESPACE(pArg);
		}
		else
		{
			pArg = "";
		}
		if ( s.IsKey("SERIAL" ))
		{
			if ( m_pCurSer != NULL )
				return( false );

			DWORD dwSerial = atoi( pArg );
			if ( dwSerial == UID_UNUSED )
			{
				DEBUG_ERR(( "Import:Bad serial number\n" ));
				break;
			}
			m_pCurSer = new CImportSer( dwSerial );
			m_ListSer.InsertHead( m_pCurSer );
			continue;
		}
		if ( s.IsKey("NAME" ))
		{
			sName = ( pArg[0] == '#' ) ? "" : pArg;
			if ( mode == IMPFLAGS_ITEMS )
				continue;
		}
		if ( m_pCurSer == NULL )
		{
			DEBUG_ERR(( "Import:No serial number\n" ));
			break;
		}
		if ( mode == IMPFLAGS_ITEMS )	// CItem.
		{
			if ( s.IsKey("ID" ))
			{
				if ( m_pCurObj != NULL )
					return( false );
				pItem = CItem::CreateTemplate( (ITEMID_TYPE) atoi( pArg ));
				pItem->SetName( sName );
				m_pCurObj = pItem;
				m_pCurSer->m_pObj = pItem;
				continue;
			}

			if ( m_pCurObj == NULL )
			{
				DEBUG_ERR(( "Import:Bad Item Key '%s'\n", s.GetKey()));
				break;
			}
			if ( s.IsKey("CONT" ))
			{
				m_pCurSer->m_dwContSer = atoi(pArg);
			}
			else if ( s.IsKey("X" ))
			{
				CPointMap pt = pItem->GetUnkPoint();
				pt.m_x = atoi(pArg);
				pItem->SetUnkPoint(pt);
				continue;
			}
			else if ( s.IsKey("Y" ))
			{
				CPointMap pt = pItem->GetUnkPoint();
				pt.m_y = atoi(pArg);
				pItem->SetUnkPoint(pt);
				continue;
			}
			else if ( s.IsKey("Z" ))
			{
				CPointMap pt = pItem->GetUnkPoint();
				pt.m_z = atoi(pArg);
				pItem->SetUnkPoint(pt);
				continue;
			}
			else if ( s.IsKey("COLOR" ))
			{
				pItem->SetHue( atoi(pArg));
				continue;
			}
			else if ( s.IsKey("LAYER" ))
			{
				m_pCurSer->m_layer = (LAYER_TYPE) atoi(pArg);
				continue;
			}
			else if ( s.IsKey("AMOUNT" ))
			{
				pItem->SetAmount( atoi(pArg));
				continue;
			}
			else if ( s.IsKey("MOREX" ))
			{
				pItem->m_itNormal.m_morep.m_x = atoi(pArg);
				continue;
			}
			else if ( s.IsKey("MOREY" ))
			{
				pItem->m_itNormal.m_morep.m_y = atoi(pArg);
				continue;
			}
			else if ( s.IsKey("MOREZ" ))
			{
				pItem->m_itNormal.m_morep.m_z = atoi(pArg);
				continue;
			}
			else if ( s.IsKey("MORE" ))
			{
				pItem->m_itNormal.m_more1 = atoi(pArg);
				continue;
			}
			else if ( s.IsKey("MORE2" ))
			{
				pItem->m_itNormal.m_more2 = atoi(pArg);
				continue;
			}
			else if ( s.IsKey("DYEABLE" ))
			{
				//if ( atoi(pArg))
				//	pItem->m_pDef->m_Can |= CAN_I_DYE;
				continue;
			}
			else if ( s.IsKey("ATT" ))
			{
				// pItem->m_pDef->m_attackBase = atoi(pArg);
			}
			else if ( s.IsKey("TYPE" ))
			{
				// ??? translate the type field.
				int i = atoi(pArg);

			}
#ifdef COMMENT
			fprintf(wscfile, "DEF %i\n", items[i].def);
			fprintf(wscfile, "VISIBLE %i\n", items[i].visible);
			fprintf(wscfile, "SPAWN %i\n", (items[i].spawn1*16777216)+(items[i].spawn2*65536)+(items[i].spawn3*256)+items[i].spawn4);
			fprintf(wscfile, "CORPSE %i\n", items[i].corpse);
			fprintf(wscfile, "OWNER %i\n", (items[i].owner1*16777216)+(items[i].owner2*65536)+(items[i].owner3*256)+items[i].owner4);
#endif
		}
		if ( mode == IMPFLAGS_CHARS )
		{
			if ( s.IsKey("NAME" ))
			{
				if ( m_pCurObj != NULL )
					return( false );
				pChar = CChar::CreateBasic( CREID_MAN );
				pChar->SetName( sName );
				m_pCurObj = pChar;
				m_pCurSer->m_pObj = pChar;
				continue;
			}
			if ( m_pCurObj == NULL )
			{
				DEBUG_ERR(( "Import:Bad Item Key '%s'\n", s.GetKey()));
				break;
			}
			if ( s.IsKey("X" ))
			{
				CPointMap pt = pChar->GetUnkPoint();
				pt.m_x = atoi(pArg);
				pChar->SetUnkPoint(pt);
				continue;
			}
			else if ( s.IsKey("Y" ))
			{
				CPointMap pt = pChar->GetUnkPoint();
				pt.m_y = atoi(pArg);
				pChar->SetUnkPoint(pt);
				continue;
			}
			else if ( s.IsKey("Z" ))
			{
				CPointMap pt = pChar->GetUnkPoint();
				pt.m_z = atoi(pArg);
				pChar->SetUnkPoint(pt);
				continue;
			}
			else if ( s.IsKey("BODY" ))
			{
				pChar->SetID( (CREID_TYPE) atoi(pArg));
				continue;
			}
			else if ( s.IsKey("SKIN" ))
			{
				pChar->SetHue( atoi(pArg));
				continue;
			}
			else if ( s.IsKey("DIR" ))
			{
				pChar->m_dirFace = (DIR_TYPE) atoi(pArg);
				continue;
			}
			else if ( s.IsKey("XBODY" ))
			{
				pChar->m_prev_id = (CREID_TYPE) atoi(pArg);
				continue;
			}
			else if ( s.IsKey("XSKIN" ))
			{
				pChar->m_prev_Hue = atoi(pArg);
				continue;
			}
			else if ( s.IsKey("FONT" ))
			{
				pChar->m_fonttype = (FONT_TYPE) atoi(pArg);
				continue;
			}
			else if ( s.IsKey("KARMA" ))
			{
				pChar->Stat_SetBase(STAT_KARMA,atoi(pArg));
				continue;
			}
			else if ( s.IsKey("FAME" ))
			{
				pChar->Stat_SetBase(STAT_FAME,atoi(pArg));
				continue;
			}
			else if ( s.IsKey("TITLE" ))
			{
				pChar->m_sTitle = pArg;
				continue;
			}
			else if ( s.IsKey("STRENGTH" ))
			{
				pChar->Stat_SetBase(STAT_STR,atoi(pArg));
			}
			else if ( s.IsKey("DEXTERITY" ))
			{
				pChar->Stat_SetBase(STAT_DEX,atoi(pArg));
			}
			else if ( s.IsKey("INTELLIGENCE" ))
			{
				pChar->Stat_SetBase(STAT_INT,atoi(pArg));
			}
			else if ( s.IsKey("HITPOINTS" ))
			{
				pChar->Stat_SetVal(STAT_STR,atoi(pArg));
			}
			else if ( s.IsKey("STAMINA" ))
			{
				pChar->Stat_SetVal(STAT_DEX,atoi(pArg));
			}
			else if ( s.IsKey( "MANA" ))
			{
				pChar->Stat_SetVal(STAT_INT,atoi(pArg));
			}
			else if ( s.IsKeyHead( "SKILL", 5 ))
			{
				SKILL_TYPE skill = (SKILL_TYPE) atoi( &(s.GetKey()[5]));
				if ( pChar->IsSkillBase(skill))
				{
					pChar->Skill_SetBase( skill, atoi(pArg));
				}
			}
			else if ( s.IsKey("ACCOUNT" ))
			{
				// What if the account does not exist ?
				pChar->SetPlayerAccount( pArg );
			}
			else if ( s.IsKey("KILLS" ) && pChar->m_pPlayer )
			{
				pChar->m_pPlayer->m_wMurders = atoi(pArg);
			}
			else if ( s.IsKey("NPCAITYPE" ))
			{
				// Convert to proper NPC type.
				int i = atoi( pArg );
				switch ( i )
				{
				case 0x01:	pChar->SetNPCBrain( NPCBRAIN_HEALER ); break;
				case 0x02:	pChar->SetNPCBrain( NPCBRAIN_MONSTER ); break;
				case 0x04:
				case 0x40:	pChar->SetNPCBrain( NPCBRAIN_GUARD ); break;
				case 0x08:	pChar->SetNPCBrain( NPCBRAIN_BANKER ); break;
				default:	pChar->SetNPCBrain( pChar->GetNPCBrain()); break;
				}
			}
#ifdef COMMENT
				"NPC"
			fprintf(wscfile, "SPEECH %i\n", chars[i].speech);

			fprintf(wscfile, "DEAD %i\n", chars[i].dead);
			fprintf(wscfile, "SPAWN %i\n", (chars[i].spawn1*16777216)+(chars[i].spawn2*65536)+(chars[i].spawn3*256)+chars[i].spawn4);
			fprintf(wscfile, "WAR %i\n", chars[i].war);
			fprintf(wscfile, "OWN %i\n", (chars[i].own1*16777216)+(chars[i].own2*65536)+(chars[i].own3*256)+chars[i].own4);

			fprintf(wscfile, "ALLMOVE %i\n", chars[i].priv2);
			fprintf(wscfile, "PACKITEM %i\n", chars[i].packitem);
			fprintf(wscfile, "RESERVED1 %i\n", chars[i].cell);
			fprintf(wscfile, "NPCWANDER %i\n", chars[i].npcWander);
			fprintf(wscfile, "FOLLOWTARGET %i\n", (chars[i].ftarg1*16777216)+(chars[i].ftarg2*65536)+(chars[i].ftarg3*256)+chars[i].ftarg4);
			fprintf(wscfile, "FX1 %i\n", chars[i].fx1);
			fprintf(wscfile, "FY1 %i\n", chars[i].fy1);
			fprintf(wscfile, "FZ1 %i\n", chars[i].fz1);
			fprintf(wscfile, "FX2 %i\n", chars[i].fx2);
			fprintf(wscfile, "FY2 %i\n", chars[i].fy2);
			fprintf(wscfile, "STRENGTH2 %i\n", chars[i].st2);
			fprintf(wscfile, "DEXTERITY2 %i\n", chars[i].dx2);
			fprintf(wscfile, "INTELLIGENCE2 %i\n", chars[i].in2);
			fprintf(wscfile, "ATT %i\n", chars[i].att);
			fprintf(wscfile, "DEF %i\n", chars[i].def);
			fprintf(wscfile, "DEATHS %i\n", chars[i].deaths);
			fprintf(wscfile, "ROBE %i\n", (chars[i].robe1*16777216)+(chars[i].robe2*65536)+(chars[i].robe3*256)+chars[i].robe4);
#endif
			continue;
		}
	}
	return( true );
}

bool CWorld::Import( LPCTSTR pszFilename, const CChar * pSrc, WORD wModeFlags, int iDist, TCHAR * pszArg1, TCHAR * pszArg2 )
{
	// wModeFlags = IMPFLAGS_TYPE
	//
	// iDistance = distance from the current point.

	// dx = change in x from file to world.
	// dy = change in y

	// NOTE: We need to set the IsLoading() for this ???

	// ??? What if i want to just import into the local area ?
	int iLen = strlen( pszFilename );
	if ( iLen <= 4 )
		return( false );
	CScript s;
	if ( ! s.Open( pszFilename ))
		return( false );

	CPointMap ptCenter;
	if ( pSrc )
	{
		ptCenter = pSrc->GetTopPoint();

		if ( wModeFlags & IMPFLAGS_RELATIVE )
		{
			// dx += ptCenter.m_x;
			// dy += ptCenter.m_y;
		}
	}

	CImportFile fImport( wModeFlags, ptCenter, iDist );

	fImport.m_pszArg1 = pszArg1;
	fImport.m_pszArg2 = pszArg2;

	if ( ! strcmpi( pszFilename + iLen - 4, ".WSC" ))
	{
		if ( ! fImport.ImportWSC(s, wModeFlags ))
			return( false );
	}
	else
	{
		// This is one of our files. ".SCP"
		if ( ! fImport.ImportSCP(s, wModeFlags ))
			return( false );
	}

	// now fix the contained items.
	fImport.ImportFix();

	GarbageCollection();
	return( true );
}



bool CWorld::DumpAreas( CTextConsole * pSrc, LPCTSTR pszFilename )
{
	if ( pSrc == NULL )
		return( false );

	if ( !*pszFilename )
		pszFilename	= "map_all.scp";
	else if ( strlen( pszFilename ) <= 4 )
		return( false );

	CScript s;
	if ( ! s.Open( pszFilename, OF_WRITE|OF_TEXT ))
		return( false );

	int	i		= 0;
	int	iMax	= g_Cfg.m_RegionDefs.GetCount();

	for ( i = 0; i < iMax; i++ )
	{
		CRegionBase * pRegion = dynamic_cast <CRegionBase*> (g_Cfg.m_RegionDefs.GetAt(i));
		if ( !pRegion  )
			continue;
		pRegion->r_Write( s );
	}

	s.WriteSection( "EOF" );
	return true;
}



bool CWorld::Export( LPCTSTR pszFilename, const CChar * pSrc, WORD wModeFlags, int iDist, int dx, int dy )
{
	// wModeFlags = IMPFLAGS_TYPE
	// Just get the items in the local area to export.
	// dx = change in x from world to file.
	// dy = change in y

	if ( pSrc == NULL )
		return( false );

	int iLen = strlen( pszFilename );
	if ( iLen <= 4 )
		return( false );

	CScript s;
	if ( ! s.Open( pszFilename, OF_WRITE|OF_TEXT ))
		return( false );

	if ( wModeFlags & IMPFLAGS_RELATIVE )
	{
		dx -= pSrc->GetTopPoint().m_x;
		dy -= pSrc->GetTopPoint().m_y;
	}

	int index = 0;
	if ( ! strcmpi( pszFilename + iLen - 4, ".WSC" ))
	{
		// Export as UOX format. for world forge stuff.
		CWorldSearch AreaItems( pSrc->GetTopPoint(), iDist );
		while(true)
		{
			CItem * pItem = AreaItems.GetItem();
			if ( pItem == NULL )
				break;
			pItem->WriteUOX( s, index++ );
		}
		return( true );
	}

	// (???NPC) Chars and the stuff they are carrying.
	if ( wModeFlags & IMPFLAGS_CHARS )
	{
		CWorldSearch AreaChars( pSrc->GetTopPoint(), iDist );
		AreaChars.SetAllShow( pSrc->IsPriv( PRIV_ALLSHOW ));	// show logged out chars?
		while(true)
		{
			CChar * pChar = AreaChars.GetChar();
			if ( pChar == NULL )
				break;
			pChar->r_WriteSafe( s );
		}
	}

	if ( wModeFlags & IMPFLAGS_ITEMS )
	{
		// Items on the ground.
		CWorldSearch AreaItems( pSrc->GetTopPoint(), iDist );
		AreaItems.SetAllShow( pSrc->IsPriv( PRIV_ALLSHOW ));	// show logged out chars?
		while(true)
		{
			CItem * pItem = AreaItems.GetItem();
			if ( pItem == NULL )
				break;
			pItem->r_WriteSafe( s );
		}
	}

	return( true );
}

