//
// CCharNPCFood.CPP
// Copyright Menace Software (www.menasoft.com).
//
// Actions specific to an NPC.
//

#include "graysvr.h"	// predef header.

#if 0

bool CChar::NPC_Food_MeatCheck(int iAmount, int iBite, int iSearchDist)
{
	// TRUE = I found something to get

	// Do I have any in my pack?
	CItem * pMeat = ContentFind( IT_MEAT_RAW );
	if ( pMeat == NULL )
		pMeat = ContentFind( IT_FISH );
	if ( pMeat != NULL)
	{
		// Eat it
		Use_EatQty( pMeat, min( iAmount - Stat_GetVal(STAT_FOOD), iBite ));
		return true;
	}

	// Any corpses around?
	CWorldSearch AreaItems( GetTopPoint(), iSearchDist );
	while (true)
	{
		CItem * pItem = AreaItems.GetItem();
		if (pItem == NULL)
			break;
		if ( ! CanSee( pItem ))
			continue;

		if ( NPC_WantThisItem( pItem ) || pItem->IsType(IT_CORPSE) )
		{
			// Do I remember this from something else?
			CItemMemory * pMemory = Memory_FindObj( pItem );
			if ( pMemory != NULL )
			{
				continue;
			}
			m_Act_Targ = pItem->GetUID();
???			Skill_Start( NPCACT_LOOTING );
			return true;
		}
	}

	// Anything living that I can eat around here?
	CWorldSearch AreaChars( GetTopPoint(), iSearchDist * 4 );
	CChar * pTarg = NULL;
	while (true)
	{
		CChar * pChar = AreaChars.GetChar();
		if ( pChar == NULL )
			break;
		if ( pChar == this )
			continue;
		if ( NPC_GetHostilityLevelToward( pChar ) < 0 )
			continue;
		if ( pTarg == NULL && NPC_MonsterCheck( pChar ))
			pTarg = pChar;
		else
		{
			if ( NPC_GetAttackMotivation( pChar ) > NPC_GetAttackMotivation( pTarg ))
				pTarg = pChar;
		}
	}
	if ( pTarg != NULL )
	{
		this->NPC_MonsterCheck( pTarg );
		return true;
	}
	return false;
}

bool CChar::NPC_Food_FruitCheck(int iAmount, int iBite, int iSearchDist )
{
	// Do I have any in my pack?
	CItem * pItem = GetContentHead();
	for ( ; pItem != NULL; pItem = pItem->GetNext())
	{
		if ( pItem->IsType(IT_FRUIT))
		{
			// I actually have some in my pack
			// How much do I want?
			Use_EatQty( pItem, min( iAmount - Stat_GetVal(STAT_FOOD), iBite ));
			return true;
		}
	}

	// Look for some fruit nearby
	int iDist = INT_MAX;
	CItem * pClosestFruit = NULL;
	CWorldSearch AreaItems( GetTopPoint(), iSearchDist);
	while (true)
	{
		CItem * pItem = AreaItems.GetItem();
		if ( pItem == NULL)
			break;
  		if ( ! pItem->IsType(IT_FRUIT))
			continue;
		if ( GetDist( pItem ) >= iDist )
			continue;
		if ( ! CanMove( pItem ))
			continue;
		pClosestFruit = pItem;
		iDist = GetDist( pClosestFruit );
  	}

  	if ( pClosestFruit != NULL )
  	{
  		// Is it right here?
  		if ( iDist <= 1 )
  		{
			// Can I pick it up?
			if ( m_pDef->Can(CAN_C_USEHANDS))
			{
				if ( CanCarry( pClosestFruit ))
				{
					ItemBounce( pClosestFruit );
					return true;
					// I'll eat it on the next tick.
				}
			}
  			// Couldn't pick it up, so I'll just eat it
  			// How much is in this stack?
  			int iEaten = pClosestFruit->ConsumeAmount( iBite );
			if ( pClosestFruit->GetAmount() == 0 )
			{
  				pClosestFruit->Plant_CropReset();	// Set it back to stage 1 of growth
  			}
			EatAnim( pClosestFruit->GetName(), iEaten );
  			return true;
  		}
  		else
  		{
  			// Go there damn it!
			CPointMap pt = pClosestFruit->GetTopPoint();
  			if ( CanMoveWalkTo( pt ))
  			{
  				m_Act_p = pt;
  				m_atGoto.m_SkillPrv = NPCACT_EATING;
  				Skill_Start( NPCACT_GOTO );
  				return true;
  			}
  		}
  	}
	return false;
}

bool CChar::NPC_Food_CropCheck(int iAmount, int iBite, int iSearchDist )
{
 	// Look for some Crop nearby
  	CItem * pClosestCrop = NULL;
  	CWorldSearch AreaItems( GetTopPoint(), iSearchDist );
  	while (true)
  	{
  		CItem * pItem = AreaItems.GetItem();
  		if ( pItem == NULL )
  			break;
  		if ( pItem->IsType(IT_CROPS))
  		{
  			// Where is it?
  			if (pClosestCrop != NULL)
  			{
  				if ( GetDist(pItem) < GetDist( pClosestCrop ) &&
					CanSee( pItem ))
  					pClosestCrop = pItem;
  			}
  			else
  				pClosestCrop = pItem;
  		}
  	}
  	if ( pClosestCrop != NULL )
  	{
  		// Is it right here?
  		if ( GetDist( pClosestCrop ) <= 1 )
  		{
  			// How much is in this stack?
  			int iEaten = pClosestCrop->ConsumeAmount( iBite );
			if ( pClosestCrop->GetAmount() == 0 )
			{
  				// Eat it all
  				pClosestCrop->Plant_CropReset();	// Set it back to stage 1 of growth
  			}
  			// Eat it
			EatAnim( pClosestCrop->GetName(), iEaten );
  			return true;
  		}
  		else
  		{
  			// Go there damn it!
			CPointMap pt = pClosestCrop->GetTopPoint();
  			if ( CanMoveWalkTo( pt ))
  			{
  				m_Act_p = pt;
  				m_atGoto.m_SkillPrv = NPCACT_EATING;
  				Skill_Start( NPCACT_GOTO );
  				return true;
  			}
  		}
  	}
	return false;
}

bool CChar::NPC_Food_GrainCheck(int iAmount, int iBite, int iSearchDist )
{
  	CItem * pClosestGrain = NULL;
  	CWorldSearch AreaItems( GetTopPoint(), iSearchDist );
  	while (true)
  	{
  		CItem * pItem = AreaItems.GetItem();
  		if ( pItem == NULL )
  			break;
  		if ( pItem->IsType(IT_GRAIN))
  		{
  			// Where is it?
  			if (pClosestGrain != NULL)
  			{
  				if ( GetDist(pItem) < GetDist( pClosestGrain ))
  					pClosestGrain = pItem;
  			}
  			else
  				pClosestGrain = pItem;
  		}
  	}
  	if ( pClosestGrain != NULL )
  	{
  		// Is it right here?
  		if ( GetDist( pClosestGrain ) <= 1 )
  		{
  			// Eat it
  			// How much is in this stack?
  			Use_EatQty( pClosestGrain, iBite );
  			return true;
  		}
  		else
  		{
  			// Go there if we can
  			for (int i = 0; i < DIR_QTY; i++)
  			{
  				// Since we can't actually step on hay, go to a spot next to it.
  				CPointMap pt = pClosestGrain->GetTopPoint();
  				pt.Move( (DIR_TYPE) Calc_GetRandVal( DIR_QTY ) );
  				if ( CanMoveWalkTo( pt ) )
  				{
  					m_Act_p = pt;
  					m_atGoto.m_SkillPrv = NPCACT_EATING;
  					Skill_Start( NPCACT_GOTO );
  					return true;
  				}
  			}
  			// Apparently, we can't get there...don't even bother trying.
  		}
  	}
	return false;
}

bool CChar::NPC_Food_FishCheck(int iAmount, int iBite, int iSearchDist )
{
	CItem * pResBit = g_World.CheckNaturalResource( GetTopPoint(), IT_WATER, true, this );
  	if ( pResBit && pResBit->GetAmount() > 0 )
  	{
  		// Fish aren't easy to catch...check my fishing skill
  		if ( Calc_GetRandVal( 100) < 25 )
  		{
  			// I caught something!
  			int iEaten = pResBit->ConsumeAmount( min( Stat_GetVal(STAT_FOOD), iBite ));
			EatAnim( "fish", iEaten );
  		}
  		return true;
  	}

  	// None here...look for some nearby
  	CPointMap pt = g_World.FindItemTypeNearby( GetTopPoint(), IT_WATER, iSearchDist );
  	if ( pt.m_x != 0 && pt.m_y != 0 )
  	{
  		if ( CanMoveWalkTo( pt ))
  		{
  			m_Act_p = pt;
  			m_atGoto.m_SkillPrv = NPCACT_EATING;
  			Skill_Start( NPCACT_GOTO );
  			return true;
  		}
  	}
	return false;
}

bool CChar::NPC_Food_GrassCheck(int iAmount, int iBite, int iSearchDist )
{
    // Look for some grass nearby
    CItem * pResBit = g_World.CheckNaturalResource( GetTopPoint(), IT_GRASS, true, this );
    if ( pResBit && pResBit->GetAmount())
    {
    	// Yummy!  There is some here!  Eat it.
    	// First, find out how much I want.
		int iEaten = pResBit->ConsumeAmount( min( Stat_GetVal(STAT_FOOD)*10, iBite ));
		EatAnim( "grass", iEaten / 10 );
    	return true;
    }

    // None here...see if we can find some nearby
  	CPointMap pt = g_World.FindItemTypeNearby( GetTopPoint(), IT_GRASS, iSearchDist);
  	if ( pt.m_x != 0 && pt.m_y != 0)
  	{
  		if ( CanMoveWalkTo( pt ))
  		{
  			m_Act_p = pt;
  			m_atGoto.m_SkillPrv = NPCACT_EATING;
  			Skill_Start( NPCACT_GOTO );
  			return true;
  		}
  	}
	return false;
}

bool CChar::NPC_Food_GarbageCheck(int iAmount, int iBite, int iSearchDist )
{
	return false;
}

bool CChar::NPC_Food_LeatherCheck(int iAmount, int iBite, int iSearchDist )
{
	return false;
}

bool CChar::NPC_Food_HayCheck(int iAmount, int iBite, int iSearchDist )
{
 	// Look for some Hay nearby
  	CItem * pClosestHay = NULL;
  	CWorldSearch AreaItems( GetTopPoint(), iSearchDist );
  	while (true)
  	{
  		CItem * pItem = AreaItems.GetItem();
  		if ( pItem == NULL )
  			break;
  		if ( pItem->GetDispID() == ITEMID_HAY )
  		{
  			// Where is it?
  			if (pClosestHay != NULL)
  			{
  				if ( GetDist(pItem) < GetDist( pClosestHay) &&
					!pItem->IsAttr(ATTR_MOVE_NEVER|ATTR_MOVE_ALWAYS))
  					pClosestHay = pItem;
  			}
  			else
  				pClosestHay = pItem;
  		}
  	}
  	if ( pClosestHay != NULL )
  	{
  		// Is it right here?
  		if ( GetDist( pClosestHay ) <= 1 )	// Can't actually step on hay, so we need to be right next to it
  		{
  			// Eat it
  			Use_EatQty( pClosestHay, iBite );
  			return true;
  		}
  		else
  		{
  			// Go there if we can
  			for (int i = 0; i < DIR_QTY; i++)
  			{
  				// Since we can't actually step on hay, go to a spot next to it.
  				CPointMap pt = pClosestHay->GetTopPoint();
  				pt.Move( (DIR_TYPE) Calc_GetRandVal( DIR_QTY ) );
  				if ( CanMoveWalkTo( pt ) )
  				{
  					m_Act_p = pt;
  					m_atGoto.m_SkillPrv = NPCACT_EATING;
  					Skill_Start( NPCACT_GOTO );
  					return true;
  				}
  			}
  			// Apparently, we can't get there...don't even bother trying.
  		}
  	}
	return false;
}

bool CChar::NPC_Food_EdibleCheck(int iAmount, int iBite, int iSearchDist )
{
	// NOTE: ??? The NPC will go looking for food even if frozen

	// Check my pack for food
	CItem * pFood = ContentFindType( IT_FOOD );
	if ( pFood != NULL )
	{
		// Hey!  I have some!
		// How much do I want?
		Use_EatQty( pFood, min( iAmount - Stat_GetVal(STAT_FOOD), iBite ));
		return true;
	}

	// Do I have any raw food in my pack?
	pFood = ContentFindType( IT_FOOD_RAW );
	if ( pFood != NULL )
	{
		// Am I standing near something I can cook on?

		// IT_CAMPFIRE
		CPointMap ptFire = g_World.FindItemTypeNearby( GetTopPoint(), IT_FIRE, iSearchDist );
		if ( ptFire.IsValid())
		{
			// is it right here?
			if ( GetTopPoint().GetDist( ptFire ) <= 1 )
			{
				// Cook it
				m_Act_Targ = pFood->GetUID();
				Skill_Cooking( SKTRIG_SUCCESS );
			}
			else
			{
				// Go there
				m_atGoto.m_SkillPrv = NPCACT_EATING;
				m_Act_p = ptFire;
				Skill_Start( NPCACT_GOTO );
			}
			return true;
		}
	}

	// No food in my pack...Look for some nearby
	CItem * pClosestFood = NULL;
	CWorldSearch AreaItems( GetTopPoint(), iSearchDist );
	while (true)
	{
  		CItem * pItem = AreaItems.GetItem();
  		if ( pItem == NULL )
  			break;
  		if ( Food_CanEat(pItem))
  		{
  			// Where is it?
  			if (pClosestFood != NULL)
  			{
  				if ( GetDist(pItem) < GetDist( pClosestFood) &&
					!pItem->IsAttr(ATTR_MOVE_NEVER|ATTR_MOVE_ALWAYS))
  					pClosestFood = pItem;
  			}
  			else
  				pClosestFood = pItem;
  		}
	}

	if ( pClosestFood != NULL )
	{
		// Where is it?
  		// Is it right here?
  		if ( GetDist( pClosestFood ) <= 1 )
  		{
			// Can I pick it up?
			if ( m_pDef->Can(CAN_C_USEHANDS))
			{
				if ( CanCarry( pClosestFood ))
				{
					ItemBounce( pClosestFood );
					return true;
					// I'll eat it on the next tick.
				}
			}

			Use_EatQty( pClosestFood, iBite );
  			return true;
  		}
  		else
  		{
  			// Go there damn it!
			CPointMap pt = pClosestFood->GetTopPoint();
  			if ( CanMoveWalkTo( pt ))
  			{
  				m_Act_p = pt;
  				m_atGoto.m_SkillPrv = NPCACT_EATING;
  				Skill_Start( NPCACT_GOTO );
  				return true;
  			}
  		}
	}
	return false;
}

bool CChar::NPC_Food_VendorCheck(int iAmount, int iBite)
{
	// Look through my stock for food
	// First see what I've bought recently

	CItemContainer * pVendBox = GetBank( LAYER_VENDOR_EXTRA );
	CItem * pItem = pVendBox->ContentFindType(IT_FOOD);
	if ( pItem != NULL )
	{
		// Move it to my pack
		ItemBounce( pItem );
		return true;
	}

	// I haven't bought any recently...let's see if I have any in stock
	pVendBox = GetBank( LAYER_VENDOR_STOCK );
	pItem = pVendBox->ContentFindType(IT_FOOD);
	if ( pItem != NULL )
	{
		// Move some of it to my pack
		pItem->UnStackSplit( min( iAmount - Stat_GetVal(STAT_FOOD), pItem->GetAmount()), this );
		ItemBounce( pItem );
		return true;
	}

	return false;
}

CItem * CChar::NPC_Food_Trade( bool fTest )
{
	// Try to trade for food.
	if ( ! NPC_IsVendor())
		return( NULL );

	LPCTSTR pszSpeech = "I'm sorry, but I'm out of food at the moment.";
	CItem * pFood = NULL;

	for ( int i=LAYER_VENDOR_STOCK; i<=LAYER_VENDOR_EXTRA; i++ )
	{
		CItemContainer * pBank = GetBank( (LAYER_TYPE) i );
		ASSERT( pBank );
		pFood = pBank->ContentFindType(IT_FOOD);
		if ( pFood != NULL && pFood->GetAmount())
		{
			if ( ! fTest )
			{
				pFood->UnStackSplit( min( 15, pFood->GetAmount()), this );
				pszSpeech = "Here you are.";
			}
			break;
		}
	}

	if ( ! fTest )
	{
		Speak( pszSpeech );
	}
	return pFood;
}

bool CChar::NPC_Food_VendorFind(int iSearchDist)
{
	// RETURN: false = can't find vendor.

	ASSERT( m_pNPC );

	CChar * pFoodVendor = NULL;
	CWorldSearch AreaChars( GetTopPoint(), iSearchDist * 10 );
	while (true)
	{
		CChar * pTestChar = AreaChars.GetChar();
		if ( pTestChar == NULL )
			break;
		if ( ! pTestChar->NPC_Food_Trade( true ))
			continue;
		if ( pFoodVendor == NULL ||
			GetDist( pFoodVendor ) > GetDist( pTestChar ))
		{
			pFoodVendor = pTestChar;
		}
	}

	if ( pFoodVendor == NULL )
		return( false );

	// Is the vendor right here?
	if ( GetDist( pFoodVendor ) <= 3 )		// Close enough
	{
		CGString sSpeech;
		sSpeech.Format( "Hail %s, I am hungry", (LPCTSTR) pFoodVendor->GetName());
		Speak( sSpeech );

		CItem * pFood = pFoodVendor->NPC_Food_Trade( false );
		if ( pFood == NULL )
			return false;

		ItemBounce( pFood );

		// Did I teleport here?
		if ( m_pNPC->m_Act_p_Prev.IsValid())
		{
			m_Act_p = m_pNPC->m_Act_p_Prev;
			m_pNPC->m_Act_p_Prev.Init();
			Skill_Start( NPCACT_GOTO );
		}
	}
	else
	{
		// Go there
		if ( ! m_pNPC->m_Act_p_Prev.IsValid())
			m_pNPC->m_Act_p_Prev = GetTopPoint();

		m_Act_p = pFoodVendor->GetTopPoint();
  		m_atGoto.m_SkillPrv = NPCACT_EATING;
  		Skill_Start( NPCACT_GOTO );
  	}

  	return true;
}

void CChar::NPC_Food_Search()
{
    // Ok ... I'm hungry...time to find some food.
  	m_Act_TargPrv = m_Act_Targ;
  	Skill_Start( NPCACT_EATING );

	// Figure out how far away I will look for food
	int iFoodLevel = Food_GetLevelPercent();
	if ( iFoodLevel >= 100 )
		return;

	int iSearchDist = 0;
	switch (iFoodLevel)
	{
	case 0:
		iSearchDist = 20;
		break;
	case 1:
		iSearchDist = 12;
		break;
	case 2:
		iSearchDist = 8;
		break;
	case 3:
		iSearchDist = 4;
		break;
	default:
		iSearchDist = 2;
		break;
	}

    TCHAR *pszFoodEntries = Str_GetTemp();
    strcpylen( pszFoodEntries, m_pDef->m_sFoodType, SCRIPT_MAX_LINE_LEN);
    TCHAR * pFoodString = strtok(pszFoodEntries, ",");

    // Try to find food I like nearby
    while ( pFoodString != NULL && *pFoodString )
    {
    	// What food do I like?
    	TCHAR szFoodType [256];
    	TCHAR szTmp[256];
    	TCHAR * pTmp = &szTmp[0];

    	Str_Parse(pFoodString, &pTmp, " ");
    	strcpy( szFoodType, pFoodString );

    	// How much of it.
    	int iAmount;
    	pFoodString = pTmp;
    	if (pFoodString != NULL)
    	{
    		Str_Parse(pFoodString, &pTmp, " ");
    		iAmount = atoi(pFoodString);
    	}

    	// How many bites ?
    	int iBite = 1;
    	if (pTmp != "")
    	{
    		if (*pTmp == '(')
    		{
    			pTmp++;
    			iBite = atoi(pTmp);
    		}
    	}

	    static LPCTSTR const sm_szFoodTypes [] =
		{
    		"ANY"
    		"CROPS",
    		"FISH",
    		"FRUIT",
    		"GARBAGE",
    		"GRAIN",
    		"GRASS",
    		"HAY",
    		"LEATHER",
    		"MEAT",
    		"NONE",
		};

    	switch ( FindTableSorted( szFoodType, sm_szFoodTypes, COUNTOF(sm_szFoodTypes), sizeof(TCHAR *)))
    	{
    	case 0: // Any
			if ( NPC_Food_EdibleCheck( iAmount, iBite, iSearchDist ))
				return;
			if ( NPC_Food_FruitCheck( iAmount, iBite, iSearchDist ))
				return;
			if ( NPC_Food_FishCheck( iAmount, iBite, iSearchDist ))
				return;
			if ( m_pNPC->m_Brain == NPCBRAIN_MONSTER || m_pNPC->m_Brain == NPCBRAIN_ANIMAL || this->m_pNPC->m_Brain == NPCBRAIN_DRAGON )
			{
				// Humans don't check for this stuff
				if ( NPC_Food_CropCheck( iAmount, iBite, iSearchDist ))
					return;
				if ( NPC_Food_GrassCheck( iAmount, iBite, iSearchDist ))
					return;
				if ( NPC_Food_HayCheck( iAmount, iBite, iSearchDist ))
					return;
				if ( NPC_Food_GrainCheck( iAmount, iBite, iSearchDist ))
					return;
				if ( NPC_Food_GarbageCheck( iAmount, iBite, iSearchDist ))
					return;
				if ( NPC_Food_LeatherCheck( iAmount, iBite, iSearchDist ))
					return;
			}
			// Am I a food vendor?  If so, take some from my inventory
			if ( this->m_pNPC->m_Brain == NPCBRAIN_VENDOR )
			{
				if ( NPC_Food_VendorCheck( iAmount, iBite ))
					return;
			}
			// Still no food nearby...if I'm a non-evil human, look for nearby vendors
			if ( IsHuman() && ! Noto_IsEvil())
			{
				if ( NPC_Food_VendorFind( iSearchDist ))
					return;
			}
			// If no vendors, look for animals to kill
			if ( NPC_Food_MeatCheck( iAmount, iBite, iSearchDist ))
				return;
			break;
  		case 1: // Crops
			if ( NPC_Food_CropCheck( iAmount, iBite, iSearchDist ))
				return;
    		break;
  		case 2: // Fish
			if ( NPC_Food_FishCheck( iAmount, iBite, iSearchDist ))
				return;
   			break;
    	case 3: // FRUIT
			if ( NPC_Food_FruitCheck( iAmount, iBite, iSearchDist ))
				return;
    		break;
    	case 4: // Garbage
			if ( NPC_Food_GarbageCheck( iAmount, iBite, iSearchDist ))
				return;
    		break;
    	case 5: // Grain
			if ( NPC_Food_GrainCheck( iAmount, iBite, iSearchDist ))
				return;
			break;
    	case 6:	// Grass
			if ( NPC_Food_GrassCheck( iAmount, iBite, iSearchDist ))
				return;
    		break;
  		case 7: // HAY
			if ( NPC_Food_HayCheck( iAmount, iBite, iSearchDist ))
				return;
  			break;
    	case 8: // Leather
			if ( NPC_Food_LeatherCheck( iAmount, iBite, iSearchDist ))
				return;
    		break;
    	case 9:	// MEAT
			if ( NPC_Food_MeatCheck( iAmount, iBite, iSearchDist ))
				return;
    		break;
    	case 10: // NONE
    		// I never get hungry
    		return;
    		break;
		default: // Some food I don't know about
			break;

    	}
    	pFoodString = strtok( NULL, ",");
    }
	// Hmm...there doesn't seem to be any food nearby...wander around a bit
	Skill_Start( NPCACT_WANDER );
}

#endif

void CChar::NPC_Food_Search()
{
	//	TODO:
	//	make search for food actualy working
	Skill_Start(SKILL_NONE);
}

void CChar::NPC_Act_Eating()
{
	// We are on our way (walking to) or eating food
	// m_Act_Targ = the food source.

	// Are we still hungry?
	int nFoodLevel = Food_GetLevelPercent();
	if ( nFoodLevel > 65 ) Skill_Start(SKILL_NONE);	// Go back to doing what we were originally doing.
	else NPC_Food_Search();		// Keep Looking for food.
}
