//
// CChar.cpp
// Copyright Menace Software (www.menasoft.com).
//  CChar is either an NPC or a Player.
//

#include "graysvr.h"	// predef header.


int CChar::GetHealthPercent() const
{
	return( IMULDIV( m_StatHealth, 100, Stat_Get(STAT_STR)));
}

bool CChar::IsSwimming() const
{
	// Am i in the water now ?

	CPointMap ptTop = GetTopPoint();
	CPointMap pt = g_World.FindItemTypeNearby( ptTop, ITEM_WATER );
	if ( ! pt.IsValid())
		return( false );	// no water here.

	int iDistZ = ptTop.m_z - pt.m_z;
	if ( iDistZ < -PLAYER_HEIGHT )
	{
		// standing far under the water some how.
		return( false );
	}
	if ( iDistZ <= 0 ) 
	{
		// we are in or below the water.
		return( true );
	}

	// Is there a solid surface under us ?
	WORD wBlockFlags = GetMoveBlockFlags();
	int iHeightZ = g_World.GetHeight( ptTop, wBlockFlags, m_pArea );
	if ( iHeightZ == pt.m_z )
	{
		return( true );
	}
	return( false );
}

bool CChar::IsMurderer() const
{
	return( m_pPlayer && m_pPlayer->m_Murders > g_Serv.m_iMurderMinCount );
}

NPCBRAIN_TYPE CChar::GetCreatureType() const
{
	// return 1 for animal, 2 for monster, 3 for NPC humans and PCs
	// For tracking and other purposes.

	// handle the exceptions.
	CREID_TYPE id = GetDispID();
	if ( id >= CREID_MAN )
	{
		if ( id == CREID_BLADES || id == CREID_VORTEX )
			return NPCBRAIN_BESERK;
		return( NPCBRAIN_HUMAN );
	}
	if ( id >= CREID_HORSE1 )
		return( NPCBRAIN_ANIMAL );
	switch ( id )
	{
	case CREID_EAGLE:
	case CREID_BIRD:
	case CREID_GORILLA:
	case CREID_Snake:
	case CREID_Dolphin:
	case CREID_Giant_Toad:	// T2A 0x50 = Giant Toad
	case CREID_Bull_Frog:	// T2A 0x51 = Bull Frog
		return NPCBRAIN_ANIMAL;
	}
	return( NPCBRAIN_MONSTER );
}

const TCHAR * CChar::GetPronoun() const
{
	switch ( GetDispID())
	{
	case CREID_CHILD_MB:
	case CREID_CHILD_MD:
	case CREID_MAN:
	case CREID_GHOSTMAN:
		return( "he" );
	case CREID_CHILD_FB:
	case CREID_CHILD_FD:
	case CREID_WOMAN:
	case CREID_GHOSTWOMAN:
		return( "she" );
	default:
		return( "it" );
	}
}

const TCHAR * CChar::GetPossessPronoun() const
{
	switch ( GetDispID())
	{
	case CREID_CHILD_MB:
	case CREID_CHILD_MD:
	case CREID_MAN:
	case CREID_GHOSTMAN:
		return( "his" );
	case CREID_CHILD_FB:
	case CREID_CHILD_FD:
	case CREID_WOMAN:
	case CREID_GHOSTWOMAN:
		return( "her" );
	default:
		return( "it's" );
	}
}

BYTE CChar::GetLightLevel() const
{
	// Get personal light level.
	if ( IsPriv( PRIV_GM ))
		return( LIGHT_BRIGHT );
	if ( IsStat( STATF_DEAD ))	// dead don't need light.
		return( LIGHT_BRIGHT + 1 );
	if ( IsStat( STATF_Sleeping ))	// eyes closed.
		return( LIGHT_DARK/2 );
	if ( IsStat( STATF_NightSight ))
		return( LIGHT_BRIGHT );
	return( GetTopSector()->GetLight());
}

CItem * CChar::GetSpellbook() const
{
	// Look for the equipped spellbook first.
	CItem * pBook = ContentFindType( ITEM_SPELLBOOK, 0, false );
	if ( pBook != NULL )
		return( pBook );
	// Look in the top level of the pack only.
	const CItemContainer * pPack = GetPack();
	if ( pPack )
	{
		pBook = pPack->ContentFindType( ITEM_SPELLBOOK, 0, false );
		if ( pBook != NULL )
			return( pBook );
	}
	// Sorry no spellbook found.
	return( NULL );
}

int CChar::GetFoodLevelPercent() const
{
	if ( m_pDef->m_MaxFood == 0 )
		return 100;
	else
		return IMULDIV( m_food, 100, m_pDef->m_MaxFood );
}

const TCHAR * CChar::GetFoodLevelMessage( bool fPet, bool fHappy ) const
{
	if ( m_pDef->m_MaxFood == 0)
		return "unaffected by food";
	int index = IMULDIV( m_food, 8, m_pDef->m_MaxFood );

	if ( fPet )
	{
		static const TCHAR * szPetHunger[] =
		{
			"confused",
			"ravenously hungry",
			"very hungry",
			"hungry",
			"a little hungry",
			"satisfied",
			"very satisfied",
			"full",
		};
		static const TCHAR * szPetHappy[] =
		{
			"confused",
			"very unhappy",
			"unhappy",
			"fairly content",
			"content",
			"happy",
			"very happy",
			"extremely happy",
		};

		if ( index >= COUNTOF(szPetHunger)-1 )
			index = COUNTOF(szPetHunger)-1;

		return( fHappy ? szPetHappy[index] : szPetHunger[index] );
	}

	static const TCHAR * szFoodLevel[] =
	{
		"starving",			// "weak with hunger",
		"very hungry",
		"hungry",
		"fairly content",	// "a might pekish",
		"content",
		"fed",
		"well fed",
		"stuffed",
	};

	if ( index >= COUNTOF(szFoodLevel)-1 )
		index = COUNTOF(szFoodLevel)-1;

	return( szFoodLevel[ index ] );
}

int CChar::CanEat( const CChar * pChar ) const
{
	// Would i want to eat this creature ? hehe

	return( 50 );
}

int CChar::CanEat( const CItem * pItem ) const
{
	// would i want to eat some of this item ?
	// 0 = not at all.
	// 10 = only if starving.
	// 20 = needs to be prepared.
	// 50 = not bad.
	// 75 = yummy.
	// 100 = my favorite (i will drop other things to get this).
	//

	if ( m_pPlayer && pItem->m_type != ITEM_FOOD )
	{
		// Am i a creature that can eat raw food ?
		if ( ! m_pDef->Can( CAN_C_EATRAW ))
		{
			return( 0 );
		}
	}

	// ???
	return( 50 );
}

const TCHAR * CChar::GetTradeTitle() const // Paperdoll title for character p (2)
{
	if ( ! m_sTitle.IsEmpty())
		return( m_sTitle );

	TCHAR * pTemp = GetTempStr();

	// Incognito ?
	// If polymorphed then use the poly name.
	if ( IsStat( STATF_Incognito ) ||
		! IsHuman() ||
		( m_pNPC && m_pDef->GetTypeName() != m_pDef->GetTradeName()))
	{
		if ( ! IsIndividualName())
			return( "" );	// same as type anyhow.
		sprintf( pTemp, "the %s", m_pDef->GetTradeName());
		return( pTemp );
	}

	SKILL_TYPE skill = Skill_GetBest();

	static const CValStr titles[] =
	{
		"", INT_MIN,
		"Neophyte", 300,
		"Novice", 400,
		"Apprentice", 500,
		"Journeyman", 600,
		"Expert", 700,
		"Adept", 800,
		"Master", 900,
		"Grandmaster", 980,
		NULL, INT_MAX,
	};

	int len = sprintf( pTemp, "%s ", titles->FindName( Skill_GetBase(skill)));
	sprintf( pTemp+len, g_Serv.m_SkillDefs[skill]->m_sTitle, CCharBase::IsFemaleID(GetDispID()) ? "woman" : "man" );
	return( pTemp );
}

bool CChar::CanDisturb( CChar * pChar ) const
{
	// I can see/disturb only players with priv same/less than me.
	if ( pChar == NULL )
		return( false );
	if ( GetPrivLevel() < pChar->GetPrivLevel())
	{
		return( ! pChar->IsDND());
	}
	return( true );
}

bool CChar::CanSee( const CObjBaseTemplate * pObj ) const
{
	// Can I see this object ( char or item ) ?
	// Am I blind ?

	if ( pObj == NULL ) 
		return( false );
	if ( pObj->IsItem())
	{
		const CItem * pItem = STATIC_CAST <const CItem*>(pObj);
		ASSERT(pItem);
		if ( ! IsPriv( PRIV_GM ))
		{
			if ( pItem->IsAttr( ATTR_INVIS ))
				return( false );
		}
		CObjBase * pObjCont = pItem->GetContainer();
		if ( pObjCont != NULL )
		{
			return( CanSee( pObjCont ));
		}
	}
	else
	{
		const CChar * pChar = STATIC_CAST <const CChar*>(pObj);
		if ( this == pChar )
			return( true );
		ASSERT(pChar);
		if ( pChar->IsStat(STATF_DEAD) && m_pNPC )
		{
			if ( m_pNPC->m_Brain != NPCBRAIN_HEALER )
				return( false );
		}
		else if ( pChar->IsStat( STATF_Insubstantial | STATF_Invisible | STATF_Hidden ))
		{
			// Characters can be invisible, but not to GM's (true sight ?)
			if ( GetPrivLevel() <= pChar->GetPrivLevel())
				return( false );
		}
		if ( pChar->IsDisconnected() && pChar->IsStat(STATF_Ridden))
		{
			CChar * pCharRider = Horse_GetMountChar();
			if ( pCharRider )
			{
				return( CanSee( pCharRider ));
			}
		}
		if ( IsPriv( PRIV_ALLSHOW ))
		{
			// don't check for logged out.
			return( GetTopDist( pChar ) <= pChar->GetVisualRange());
		}
	}

	return( GetDist( pObj ) <= pObj->GetVisualRange());
}

bool CChar::CanSeeLOS( const CPointMap & ptDst, CPointMap * pptBlock, int iMaxDist ) const
{
	// Max distance of iMaxDist
	// Line of sight check
	// NOTE: if not blocked. pptBlock is undefined.
	if ( IsPriv( PRIV_GM ))
		return( true );

	CPointMap ptSrc = GetTopPoint();
	ptSrc.m_z += PLAYER_HEIGHT/2;

	int iDist = ptSrc.GetDist( ptDst );

	// Walk towards the object. If any spot is too far above our heads
	// then we can not see what's behind it.

	int iDistTry = 0;
	while ( --iDist > 0 )
	{
		DIR_TYPE dir = ptSrc.GetDir( ptDst );
		ptSrc.Move( dir );	// NOTE: The dir is very coarse and can change slightly.

		WORD wBlockFlags = CAN_C_SWIM | CAN_C_WALK | CAN_C_FLY;
		signed char z = g_World.GetHeight( ptSrc, wBlockFlags, ptSrc.GetRegion( REGION_TYPE_MULTI ));
		if ( wBlockFlags & ( CAN_I_BLOCK | CAN_I_DOOR ))
		{
blocked:
			if ( pptBlock != NULL )
				* pptBlock = ptSrc;
			return false; // blocked
		}
		if ( iDistTry > iMaxDist && ! IsPriv( PRIV_GM ))
		{
			// just went too far.
			goto blocked;
		}
		iDistTry ++;
	}

	return true; // made it all the way to the object with no obstructions.
}

bool CChar::CanSeeLOS( const CObjBaseTemplate * pObj ) const
{
	if ( ! CanSee( pObj ))
		return( false );
	pObj = pObj->GetTopLevelObj();
	return( CanSeeLOS( pObj->GetTopPoint(), NULL, pObj->GetVisualRange()));
}

bool CChar::CanTouch( const CPointMap & pt ) const
{
	// Can I reach this from where i am.
	// swords, spears, arms length = x units.
	// Use or Grab.
	// Check for blocking objects.
	// It this in a container we can't get to ?

	return( CanSeeLOS( pt, NULL, 6 ));
}

bool CChar::CanTouch( const CObjBase * pObj ) const
{
	// Can I reach this from where i am.
	// swords, spears, arms length = x units.
	// Use or Grab. May be in snooped container.
	// Check for blocking objects.
	// Is this in a container we can't get to ?

	if ( pObj == NULL )
		return( false );

	bool fDeathImmune = IsPriv( PRIV_GM );
	if ( pObj->IsItem())	// some objects can be used anytime. (even by the dead.)
	{
		const CItem * pItem = dynamic_cast <const CItem*> (pObj);
		ASSERT( pItem );
		switch ( pItem->m_type  )
		{
		case ITEM_SIGN_GUMP:	// can be seen from a distance.
		case ITEM_TELESCOPE:
		case ITEM_SHRINE:	// We can use shrines when dead !!
			fDeathImmune = true;
			break;
		case ITEM_SHIP_SIDE:
		case ITEM_SHIP_SIDE_LOCKED:
		case ITEM_SHIP_PLANK:
			if ( IsStat( STATF_Sleeping | STATF_Freeze | STATF_Stone ))
				break;
			return( GetTopDist3D( pItem->GetTopLevelObj()) <= UO_MAP_VIEW_SIZE );
		}
	}

	// Search up to the top level object.
	while (true)
	{
		const CItem * pItem = dynamic_cast <const CItem*>(pObj);
		if ( pItem == NULL )
			break;

		// What is this inside of ?
		CObjBase * pObjCont = pItem->GetContainer();
		if ( pObjCont == NULL ) 
			break;	// reached top level.
		pObj = pObjCont;
		const CItemContainer* pContItem = dynamic_cast <const CItemContainer*>(pObj);
		if ( pContItem == NULL )
			break;

		if ( ! pContItem->IsSearchable())	// bank box etc.
		{
			// Make some special cases for searchable.
			const CChar * pChar = dynamic_cast <const CChar*> ( pContItem->GetTopLevelObj()) ;
			if ( pChar == NULL )
				return( false );
			if ( ! pChar->NPC_IsOwnedBy( this ))	// player vendor boxes.
				return( false );

			if ( pContItem->GetID() == ITEMID_BANK_BOX ||
				pContItem->GetID() == ITEMID_VENDOR_BOX )
			{
				// must be in the same position i opened it legitamitly.
				// see addBankOpen
				if ( ! IsPriv(PRIV_GM) && 
					pContItem->m_itEqBankBox.m_Open_Point != pChar->GetTopPoint())
				{
					if ( IsClient())
					{
						g_Log.Event( LOGL_WARN|LOGM_CHEAT, "%x:Cheater '%s' is using 3rd party tools to access bank box\n", GetClient()->GetSocket(), GetClient()->GetAccount()->GetName());
					}
					return( false );
				}
			}

			pObj = pChar;
			break;
		}
	}

	// We now have the top level object.
	DEBUG_CHECK( pObj->IsTopLevel() || pObj->IsDisconnected());
	if ( ! fDeathImmune )
	{
		if ( IsStat( STATF_DEAD | STATF_Sleeping | STATF_Freeze | STATF_Stone ))
			return( false );
	}

	if ( pObj->IsChar())
	{
		const CChar * pChar = dynamic_cast <const CChar*> (pObj);
		ASSERT( pChar );
		if ( pChar == this )	// something i am carrying. short cut.
		{
			return( true );
		}
		if ( IsPriv( PRIV_GM ))
		{
			return( GetPrivLevel() >= pChar->GetPrivLevel());
		}
		if ( pChar->IsStat( STATF_DEAD|STATF_Stone ))
			return( false );
	}
	else
	{
		if ( IsPriv( PRIV_GM ))
			return( true );
	}

	if ( GetTopDist3D( pObj ) > 6 )
		return( false );

	return( CanSeeLOS( pObj ));
}

bool CChar::CanMove( CItem * pItem, bool fMsg ) const
{
	// Is it possible that i could move this ?
	// NOT: test if need to steal. IsTakeCrime()
	// NOT: test if close enough. CanTouch()

	if ( IsPriv( PRIV_ALLMOVE | PRIV_DEBUG | PRIV_GM ))
		return( true );
	if ( IsStat( STATF_Stone | STATF_Freeze | STATF_Insubstantial | STATF_DEAD | STATF_Sleeping ))
	{
		if ( fMsg )
		{
			SysMessagef( "you can't reach anything in your state." );
		}
		return( false );
	}
	if ( pItem == NULL ) 
		return( false );

#if 0
	// ??? Trigger to test 'can move'
	if ( ! pItem->OnTrigger( ITRIG_CAN_MOVE ))
	{
		return( false );
	}
#endif

	if ( pItem->IsAttr(ATTR_MOVE_NEVER|ATTR_INVIS))
	{
		return( false );
	}

	if ( pItem->IsTopLevel() )
	{
		if ( pItem->m_uidLink.IsValidUID())	// linked to something.
		{
			CRegionBase * pArea = pItem->GetTopPoint().GetRegion( REGION_TYPE_MULTI );

			if ( pArea && pArea->IsMatchType( pItem->m_uidLink ))
			{
				if ( fMsg )
				{
					SysMessage( "It appears to be locked to the structure." );
				}
				return false;
			}
		}
	}
	else	// equipped or in a container.
	{
		if ( pItem->IsAttr( ATTR_CURSED|ATTR_CURSED2 ) && pItem->IsEquipped())
		{
			// "It seems to be stuck where it is"
			//
			if ( fMsg )
			{
				pItem->m_Attr |= ATTR_IDENTIFIED;
				SysMessagef( "%s appears to be cursed.", pItem->GetName());
			}
			return false;
		}

		// Can't steal/move newbie items on another cchar. (even if pet)
		if ( pItem->IsAttr( ATTR_NEWBIE|ATTR_BLESSED2|ATTR_CURSED|ATTR_CURSED2 ))
		{
			const CObjBaseTemplate * pObjTop = pItem->GetTopLevelObj();
			if ( pObjTop->IsItem())	// is this a corpse or sleeping person ?
			{
				const CItemCorpse * pCorpse = dynamic_cast <const CItemCorpse *>(pObjTop);
				if ( pCorpse )
				{
					CChar * pChar = pCorpse->IsCorpseSleeping();
					if ( pChar && pChar != this )
					{
						return( false );
					}
				}
			}
			else if ( pObjTop->IsChar() && pObjTop != this )
			{
				if ( ! pItem->IsEquipped() ||
					pItem->GetEquipLayer() != LAYER_DRAGGING )
				{
					return( false );
				}
			}
		}
	}

	return( pItem->IsMovable());
}

bool CChar::IsTakeCrime( const CItem * pItem, CChar ** ppCharMark ) const
{
	// We are snooping or stealing.
	// Is taking this item a crime ?
	// RETURN: ppCharMark = The character we are offending.
	//  false = no crime.

	if ( IsPriv(PRIV_GM|PRIV_ALLMOVE)) 
		return( false );

	CObjBaseTemplate * pObjTop = pItem->GetTopLevelObj();
	CChar * pCharMark = dynamic_cast <CChar *> (pObjTop);
	if ( ppCharMark != NULL )
	{
		*ppCharMark = pCharMark;
	}

	if ( static_cast <const CObjBase *>(pObjTop) == this )
	{
		// this is yours
		return( false );
	}

	if ( pCharMark == NULL )	// In some (or is) container.
	{
		if ( pItem->IsAttr(ATTR_OWNED))
			return( true );

		CItemContainer * pCont = dynamic_cast <CItemContainer *> (pObjTop);
		if (pCont)
		{
			if ( pCont->IsAttr(ATTR_OWNED))
				return( true );
			// ??? what if the container is locked ?
		}

		return( false );	// i guess it's not a crime.
	}

	if ( pCharMark->NPC_IsOwnedBy( this ))	// He let's you
		return( false );

	// Pack animal has no owner ?
	if ( pCharMark->GetCreatureType() == NPCBRAIN_ANIMAL &&	// free to take.
		! pCharMark->IsStat( STATF_Pet ))
		return( false );

	return( true );
}

bool CChar::CanUse( CItem * pItem ) const
{
	// Can the Char use the item where it is ?
	return( CanTouch(pItem) &&
		CanMove(pItem) &&	// ? using does not imply moving in all cases ! such as reading ?
		! IsTakeCrime( pItem ));
}

CRegionBase * CChar::CheckValidMove( CPointBase & ptDest, WORD * pwBlockFlags ) const
{
	// Is it ok to move here ?
	// ignore other characters for now.
	// RETURN:
	//  The new region we may be in.
	//  Fill in the proper ptDest.m_z value for this location. (if walking)
	//  pwBlockFlags = what is blocking me. (can be null = don't care)

	CRegionBase * pArea = ptDest.GetRegion( REGION_TYPE_MULTI | REGION_TYPE_AREA );
	if ( pArea == NULL ) 
		return( NULL );

	// In theory the client only lets us go valid places.
	// But in reality the client and server are not really in sync.
	// if ( IsClient()) return( pArea );

	WORD wCan = GetMoveBlockFlags();
	WORD wBlockFlags = wCan;
	signed char z = g_World.GetHeight( ptDest, wBlockFlags, pArea );

	if ( wCan != 0xFFFF )
	{
		if ( wBlockFlags &~ wCan )
			return( NULL );
		if ( ! m_pDef->Can( CAN_C_FLY ))
		{
			if ( z > ptDest.m_z + PLAYER_HEIGHT )	// Too high to climb.
				return( NULL );
		}
		if ( z >= UO_SIZE_Z )
			return( NULL );
	}

	if ( pwBlockFlags )
	{
		*pwBlockFlags = wBlockFlags;
	}

	ptDest.m_z = z;
	return( pArea );
}

