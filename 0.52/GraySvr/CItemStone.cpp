//
// CItemStone.cpp
// Copyright Menace Software (www.menasoft.com).
//

#include "graysvr.h"	// predef header.

//////////
// -CStoneMember

CStoneMember::CStoneMember( CItemStone * pStone, CObjUID uid, STONEPRIV_TYPE iType, const TCHAR * pTitle, CObjUID loyaluid, bool fVal1, bool fVal2 )
{
	m_LinkUID = uid;
	m_sTitle = pTitle;
	m_iPriv = iType;
	m_LoyalToUID = loyaluid;

	// union.
	if ( iType == STONEPRIV_ENEMY )
	{
		m_Enemy.m_fTheyDeclared = fVal1;	
		m_Enemy.m_fWeDeclared = fVal2;	
	}
	else
	{
		m_Member.m_fAbbrev = fVal1;
		m_Member.m_iVoteTally = fVal2;		// Temporary space to calculate votes.
	}

	if ( ! g_Serv.IsLoading() && pStone->GetMemoryType())
	{
		CChar * pChar = uid.CharFind();
		if ( pChar != NULL )
		{
			pChar->Memory_AddObjTypes( pStone, pStone->GetMemoryType());
			if ( pStone->IsTopLevel())
			{
				pChar->m_Home = pStone->GetTopPoint();	// Our new home.
			}
		}
	}

	pStone->InsertTail( this );
}

CStoneMember::~CStoneMember()
{
	CItemStone * pStone = dynamic_cast <CItemStone*> ( GetParent());
	DEBUG_CHECK(pStone);
	if ( ! pStone ) 
		return;

	RemoveSelf();
	pStone->ElectMaster();	// May have changed the vote count.

	if ( m_iPriv == STONEPRIV_ENEMY )
	{
		// same as declaring peace.
		CItemStone * pStoneEnemy = dynamic_cast <CItemStone *>( GetLinkUID().ItemFind());
		if ( pStoneEnemy != NULL )
		{
			pStoneEnemy->TheyDeclarePeace( pStone, true );
		}
	}
	else if ( pStone->GetMemoryType())
	{
		CChar * pChar = GetLinkUID().CharFind();
		if ( pChar )
		{
			pChar->Memory_ClearTypes( pStone->GetMemoryType()); 	// Make them forget they were ever in this guild
		}
	}
}

bool CStoneMember::SetLoyalTo( const CChar * pCharLoyal )
{
	CChar * pCharMe = GetLinkUID().CharFind();
	DEBUG_CHECK(pCharMe);
	if ( pCharMe == NULL ) return false;

	m_LoyalToUID = GetLinkUID();	// set to self for default.
	if ( pCharLoyal == NULL )
		return true;

	if ( GetPriv() == STONEPRIV_CANDIDATE)
	{
		// non members can't vote
		pCharMe->SysMessage("Candidates aren't elligible to vote.");
		return false;
	}

	CItemStone * pStone = dynamic_cast <CItemStone*>( GetParent());
	DEBUG_CHECK(pStone);
	if ( pStone == NULL ) return( false );

	CStoneMember * pNewLoyalTo = pStone->GetMember(pCharLoyal);
	if ( pNewLoyalTo == NULL || pNewLoyalTo->GetPriv() == STONEPRIV_CANDIDATE)
	{
		// you can't vote for candidates
		pCharMe->SysMessage( "Can only vote for full members.");
		return false;
	}

	m_LoyalToUID = pCharLoyal->GetUID();

	// new vote tally
	pStone->ElectMaster();
	return( true );
}

//////////
// -CItemStone

CItemStone::CItemStone( ITEMID_TYPE id, CItemBase * pDef ) : CItem( id, pDef )
{
	m_itStone.m_iAlign = STONEALIGN_STANDARD;
	m_sWebPageURL = GRAY_URL;
	g_World.m_Stones.Add( this );
}

CItemStone::~CItemStone()
{
	m_type = ITEM_NORMAL;	// Tell everyone we are deleting.
	DeletePrepare();	// Must remove early because virtuals will fail in child destructor.

	// Remove this stone from the links of guilds in the world
	g_World.m_Stones.RemovePtr( this );

	// all members are deleted automatically.
	Empty();	// do this manually to preserve the parents type cast
}

const TCHAR * CItemStone::GetTypeName() const
{
	switch ( m_type )
	{
	case ITEM_STONE_GUILD:
		return( _TEXT("Guild"));
	case ITEM_STONE_TOWN:
		return( _TEXT("Town"));
	case ITEM_STONE_ROOM:
		return( _TEXT("Structure"));
	}
	return( _TEXT("Unk"));
}

void CItemStone::r_Write( CScript & s )
{
	CItem::r_Write( s );
	s.WriteKeyVal( "ALIGN", GetAlignType());
	if ( ! m_sAbbrev.IsEmpty())
	{
		s.WriteKey( "ABBREV", m_sAbbrev );
	}

	for ( int i = 0; i<COUNTOF(m_sCharter); i++ )
	{
		if ( ! m_sCharter[i].IsEmpty())
		{
			CGString sStr;
			sStr.Format("CHARTER%i", i);
			s.WriteKey( sStr, m_sCharter[i] );
		}
	}

	s.WriteKey( "WEBPAGE", GetWebPageURL() );
	// s.WriteKey( "//", "uid,title,priv,loyaluid,abbr&theydecl,wedecl");

	CStoneMember * pMember = STATIC_CAST <CStoneMember *>(GetHead());
	for ( ; pMember != NULL; pMember = pMember->GetNext())
	{
		if (pMember->GetLinkUID().IsValidUID()) // To protect against characters that were deleted!
		{
			s.WriteKeyFormat( "MEMBER",
				"0%x,%s,%i,0%x,%i,%i",
				pMember->GetLinkUID() | (pMember->GetLinkUID().IsItem() ? UID_ITEM : 0),
				pMember->GetTitle(),
				pMember->GetPriv(),
				pMember->GetLoyalTo(),
				pMember->m_UnDef.m_Val1, 
				pMember->m_UnDef.m_Val2 );
		}
	}
}

const TCHAR * CItemStone::sm_KeyTable[] =
{
	"ABBREV",
	"ALIGN",
	"MEMBER",
	"WEBPAGE",
};

bool CItemStone::r_LoadVal( CScript & s ) // Load an item Script
{
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0: // "ABBREV"
		m_sAbbrev = s.GetArgStr();
		return true;
	case 1: // "ALIGN"
		SetAlignType( (STONEALIGN_TYPE) s.GetArgVal());
		return true;
	case 2: // "MEMBER"
		{
		TCHAR *Arg_ppCmd[8];		// Maximum parameters in one line
		int Arg_Qty = ParseCmds( s.GetArgStr(), Arg_ppCmd, COUNTOF( Arg_ppCmd ), "," );
		CStoneMember * pNew = new CStoneMember(
			this,
			ahextoi(Arg_ppCmd[0]),					// Member's UID
			(STONEPRIV_TYPE)atoi(Arg_ppCmd[2]),	// Members priv level (use as a type)
			Arg_ppCmd[1],						// Title
			ahextoi(Arg_ppCmd[3]),					// Member is loyal to this
			atoi( Arg_ppCmd[4] ),			// Paperdoll stone abbreviation (also if they declared war)
			atoi( Arg_ppCmd[5] )			// If we declared war
			);
		}
		return true;
	case 3: // "WEBPAGE"
		m_sWebPageURL = s.GetArgStr();
		return true;
	}

	if ( s.IsKeyHead( "CHARTER", 7 ))
	{
		int i = atoi(s.GetKey()+7);
		if ( i >= COUNTOF(m_sCharter))
			return( false );
		m_sCharter[i] = s.GetArgStr();
		return( true );
	}
	return( CItem::r_LoadVal( s ));
}

const TCHAR * CItemStone::GetAlignName() const
{
	static const TCHAR * sm_AlignName[] = // STONEALIGN_TYPE
	{
		"standard",	// STONEALIGN_STANDARD
		"Order",	// STONEALIGN_ORDER
		"Chaos",	// STONEALIGN_CHAOS
	};
	int iAlign = GetAlignType();
	if ( iAlign >= COUNTOF( sm_AlignName )) iAlign = 0;
	return( sm_AlignName[ iAlign ] );
}

bool CItemStone::r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc )
{
	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0: // "ABBREV"
		sVal = m_sAbbrev;
		return true;
	case 1: // "ALIGN"
		sVal.FormatVal( GetAlignType());
		return true;
	case 3: // "WEBPAGE"
		sVal = GetWebPageURL();
		return true;
	}

	static const TCHAR * sm_ActionTable[] =
	{
		"AbbreviationToggle",
		"AlignType",
		"LoyalTo",
		"Master",
		"MasterGenderTitle",
		"MasterTitle",
	};

	CChar * pCharSrc = pSrc->GetChar();

	switch ( FindTableSorted( pKey, sm_ActionTable, COUNTOF( sm_ActionTable )))
	{
	case 0: // "AbbreviationToggle"
		{
			CStoneMember * pMember = GetMember(pCharSrc);
			if ( pMember == NULL )
			{
				sVal = "nonmember";
				return( true );
			}
			sVal = pMember->IsAbbrevOn() ? "On" : "Off";
		}
		return true;

	case 1:	// "AlignType"
		sVal = GetAlignName();
		return true;

	case 2: // "LoyalTo"
		{
			CStoneMember * pMember = GetMember(pCharSrc);
			if ( pMember == NULL )
			{
				sVal = "nonmember";
				return( true );
			}
			CObjUID uid( pMember->GetLoyalTo());
			CChar * pLoyalTo = uid.CharFind();
			if (pLoyalTo == NULL || pLoyalTo == pCharSrc )
				sVal = "yourself";
			else
				sVal = pLoyalTo->GetName();
		}
		return( true );

	case 3: // "MASTER"
		{
			CChar * pMaster = GetMaster();
			sVal = (pMaster) ? pMaster->GetName() : "vote pending";
		}
		return( true );

	case 4: // "MasterGenderTitle"
		{
			CChar * pMaster = GetMaster();
			if ( pMaster == NULL )
				sVal = ""; // If no master (vote pending)
			else if (CCharBase::IsFemaleID( pMaster->GetDispID()))
				sVal = "Mistress";
			else
				sVal = "Master";
		}
		return( true );

	case 5: // "MasterTitle
		// "MasterTitle",
		{
			CStoneMember * pMember = GetMasterMember();
			sVal = (pMember) ? pMember->GetTitle() : "";
		}
		return( true );
	}

	return( CItem::r_WriteVal( pKey, sVal, pSrc ));
}

CStoneMember * CItemStone::GetMasterMember() const
{
	CStoneMember * pMember = STATIC_CAST <CStoneMember *>(GetHead());
	for ( ; pMember != NULL; pMember = pMember->GetNext())
	{
		if ( pMember->GetPriv() == STONEPRIV_MASTER )
			return pMember;
	}
	return NULL;
}

CChar * CItemStone::GetMaster() const
{
	CStoneMember * pMember = GetMasterMember();
	if ( pMember == NULL )
		return( NULL );
	return pMember->GetLinkUID().CharFind();
}

CStoneMember * CItemStone::GetMember( const CObjBase * pObj ) const
{
	// Get member info for this char/item (if it has member info)
	if (!pObj)
		return NULL;
	CObjUID otherUID = pObj->GetUID();
	CStoneMember * pMember = STATIC_CAST <CStoneMember *>(GetHead());
	for ( ; pMember != NULL; pMember = pMember->GetNext())
	{
		if ( pMember->GetLinkUID() == otherUID )
			return pMember;
	}
	return NULL;
}

bool CItemStone::NoMembers() const
{
	CStoneMember * pMember = STATIC_CAST <CStoneMember *>(GetHead());
	for ( ; pMember != NULL; pMember = pMember->GetNext())
	{
		if (pMember->GetPriv() == STONEPRIV_MASTER ||
			pMember->GetPriv() == STONEPRIV_MEMBER)
			return false;
	}
	return true;
}

CStoneMember * CItemStone::AddRecruit( const CChar * pChar )
{
	if ( !pChar || ! pChar->IsClient() || ! pChar->m_pPlayer )
	{
		Speak( "Only players can be recruits!");
		return NULL;
	}

	CStoneMember * pMember = GetMember(pChar);
	if (pMember)
	{
		CGString sStr;
		if ( pMember->GetPriv() == STONEPRIV_BANISHED )
		{
			sStr.Format("%s has been banished.", pChar->GetName());
		}
		else
		{
			sStr.Format("%s is already a member.", pChar->GetName());
		}
		Speak(sStr);
		return NULL;
	}

	CItemStone * pStone = pChar->Guild_Find( GetMemoryType());
	if (pStone)
	{
		CGString sStr;
		sStr.Format( "%s appears to belong to %s. Must resign previous %s", pChar->GetName(), pStone->GetName(), GetTypeName());
		Speak(sStr);
		return NULL;
	}

	CStoneMember * pNewMember = new CStoneMember( this, pChar->GetUID(), STONEPRIV_CANDIDATE );

	if ( m_type == ITEM_STONE_TOWN && IsAttr(ATTR_OWNED))
	{
		PromoteRecruit( pChar );
	}

	CGString sStr;
	sStr.Format("%s has been recruited into %s", pChar->GetName(), GetName());
	Speak(sStr);
	return( pNewMember );
}

CStoneMember * CItemStone::PromoteRecruit( const CChar * pChar )
{
	CStoneMember * pRecruit = GetMember(pChar);
	if ( pRecruit == NULL )	// Not even a member yet.
	{
		pRecruit = AddRecruit( pChar );
		if ( pRecruit == NULL )
			return( NULL );
	}
	else
	{
		if ( pRecruit->GetPriv() == STONEPRIV_BANISHED )
			return( NULL );
	}
	pRecruit->SetPriv(STONEPRIV_MEMBER);
	pRecruit->SetLoyalTo(GetMaster());
	ElectMaster();	// just in case this is the first.
	return( pRecruit );
}

void CItemStone::ElectMaster()
{
	// Check who is loyal to who and find the new master.
	if ( m_type == ITEM_NORMAL ) 
		return;	// no reason to elect new if the stone is dead.

	int iResultCode = FixWeirdness();	// try to eliminate bad members.
	if ( iResultCode )
	{
		// The stone is bad ?
		// iResultCode
	}

	int iCountMembers = 0;
	CStoneMember * pMaster = NULL;

	// Validate the items and Clear the votes field
	CStoneMember * pMember = STATIC_CAST <CStoneMember *>(GetHead());
	for ( ; pMember != NULL; pMember = pMember->GetNext())
	{
		if ( pMember->GetPriv() == STONEPRIV_MASTER )
		{
			pMaster = pMember;	// find current master.
		}
		else if ( pMember->GetPriv() != STONEPRIV_MEMBER )
		{
			continue;
		}
		pMember->m_Member.m_iVoteTally = 0;
		iCountMembers++;
	}

	// Now tally the votes.
	pMember = STATIC_CAST <CStoneMember *>(GetHead());
	for ( ; pMember != NULL; pMember = pMember->GetNext())
	{
		if ( pMember->GetPriv() != STONEPRIV_MASTER &&
			pMember->GetPriv() != STONEPRIV_MEMBER )
			continue;

		CChar * pCharVote = pMember->GetLoyalTo().CharFind();
		if ( pCharVote != NULL )
		{
			CStoneMember * pMemberVote = GetMember( pCharVote );
			if ( pMemberVote != NULL )
			{
				pMemberVote->m_Member.m_iVoteTally ++;
				continue;
			}
		}

		// not valid to vote for. change to self.
		pMember->SetLoyalTo(NULL);
		// Assume I voted for myself.
		pMember->m_Member.m_iVoteTally ++;
	}

	// Find who won.
	bool fTie = false;
	CStoneMember * pMemberHighest = NULL;
	pMember = STATIC_CAST <CStoneMember *>(GetHead());
	for ( ; pMember != NULL; pMember = pMember->GetNext())
	{
		if ( pMember->GetPriv() != STONEPRIV_MASTER &&
			pMember->GetPriv() != STONEPRIV_MEMBER )
			continue;
		if ( pMemberHighest == NULL )
		{
			pMemberHighest = pMember;
			continue;
		}
		if ( pMember->m_Member.m_iVoteTally == pMemberHighest->m_Member.m_iVoteTally )
		{
			fTie = true;
		}
		if ( pMember->m_Member.m_iVoteTally > pMemberHighest->m_Member.m_iVoteTally )
		{
			fTie = false;
			pMemberHighest = pMember;
		}
	}

	// In the event of a tie, leave the current master as is
	if ( ! fTie && pMemberHighest )
	{
		if (pMaster)
			pMaster->SetPriv(STONEPRIV_MEMBER);
		pMemberHighest->SetPriv(STONEPRIV_MASTER);
	}

	if ( ! iCountMembers )
	{
		// No more members, declare peace (by force)
		pMember = STATIC_CAST <CStoneMember *>(GetHead());
		for (; pMember != NULL; pMember = pMember->GetNext())
		{
			WeDeclarePeace(pMember->GetLinkUID(), true);
		}
	}
}

bool CItemStone::IsUniqueName( const TCHAR * pName ) // static
{
	for ( int i=0; i<g_World.m_Stones.GetCount(); i++ )
	{
		if ( ! strcmpi( pName, g_World.m_Stones[i]->GetName()))
			return false;
	}
	return true;
}

bool CItemStone::CheckValidMember( CStoneMember * pMember )
{
	ASSERT(pMember);
	ASSERT( pMember->GetParent() == this );

	switch ( pMember->GetPriv())
	{
	case STONEPRIV_MASTER:
	case STONEPRIV_MEMBER:
	case STONEPRIV_CANDIDATE:
		if ( GetMemoryType())
		{
			// Make sure the member has a memory that links them back here.
			CChar * pChar = pMember->GetLinkUID().CharFind();
			if ( pChar == NULL )
				break;
			if ( pChar->Guild_Find( GetMemoryType()) != this )
				break;
		}
		return( true );
	case STONEPRIV_BANISHED:
		{
			CChar * pChar = pMember->GetLinkUID().CharFind();
			if ( pChar == NULL )
				break;
		}
		return( true );
	case STONEPRIV_ENEMY:
		{
			CItemStone * pEnemyStone = dynamic_cast <CItemStone *>( pMember->GetLinkUID().ItemFind());
			if ( pEnemyStone == NULL )
				break;
			CStoneMember * pEnemyMember = pEnemyStone->GetMember(this);
			if ( pEnemyMember == NULL )
				break;
			if ( pMember->GetWeDeclared() && ! pEnemyMember->GetTheyDeclared())
				break;
			if ( pMember->GetTheyDeclared() && ! pEnemyMember->GetWeDeclared())
				break;
		}
		return( true );
	}

	// just delete this member. (it is mislinked)
	DEBUG_ERR(( "Stone UID=0%x has mislinked member uid=0%x\n", GetUID(), pMember->GetLinkUID()));
	return( false );
}

int CItemStone::FixWeirdness()
{
	// Check all my members. Make sure all wars are reciprocated and members are flaged.

	int iResultCode = CItem::FixWeirdness();
	if ( iResultCode )
	{
		return( iResultCode );
	}

	bool fChanges = false;
	CStoneMember * pMember = STATIC_CAST <CStoneMember *>(GetHead());
	while ( pMember != NULL )
	{
		CStoneMember * pMemberNext = pMember->GetNext();
		if ( ! CheckValidMember(pMember))
		{
			ITEM_TYPE oldtype = m_type;
			m_type = ITEM_NORMAL;	// turn off validation for now. we don't want to delete other members.
			delete pMember;
			m_type = oldtype;
			fChanges = true;
		}
		pMember = pMemberNext;
	}

	if ( fChanges )
	{
		ElectMaster();	// May have changed the vote count.
	}
	return( 0 );
}

bool CItemStone::IsAtWarWith( const CItemStone * pEnemyStone ) const
{
	// Boths guild shave declared war on each other.

	if ( pEnemyStone == NULL )
		return( false );

	// Just based on align type.
	if ( m_type == ITEM_STONE_GUILD &&
		GetAlignType() != STONEALIGN_STANDARD &&
		pEnemyStone->GetAlignType() != STONEALIGN_STANDARD )
	{
		return( GetAlignType() != pEnemyStone->GetAlignType());
	}

	// we have declared or they declared.
	CStoneMember * pEnemyMember = GetMember(pEnemyStone);
	if (pEnemyMember) // Ok, we might be at war
	{
		DEBUG_CHECK( pEnemyMember->GetPriv() == STONEPRIV_ENEMY );
		if ( pEnemyMember->GetTheyDeclared() && pEnemyMember->GetWeDeclared())
			return true;
	}

	return false;
}

void CItemStone::AnnounceWar( const CItemStone * pEnemyStone, bool fWeDeclare, bool fWar )
{
	// Announce we are at war or peace.

	ASSERT(pEnemyStone);

	bool fAtWar = IsAtWarWith(pEnemyStone);

	TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
	int len = sprintf( szTemp, (fWar) ? "%s %s declared war on %s." : "%s %s requested peace with %s.",
		(fWeDeclare) ? "You" : pEnemyStone->GetName(),
		(fWeDeclare) ? "have" : "has",
		(fWeDeclare) ? pEnemyStone->GetName() : "You" );

	if ( fAtWar )
	{
		sprintf( szTemp+len, " War is ON!" );
	}
	else
	{
		sprintf( szTemp+len, " War is OFF." );
	}

	CStoneMember * pMember = STATIC_CAST <CStoneMember *>(GetHead());
	for ( ; pMember != NULL; pMember = pMember->GetNext())
	{
		CChar * pChar = pMember->GetLinkUID().CharFind();
		if ( pChar == NULL ) 
			continue;
		if ( ! pChar->IsClient()) 
			continue;
		pChar->SysMessage( szTemp );
	}
}

bool CItemStone::WeDeclareWar(CItemStone * pEnemyStone)
{
	if (!pEnemyStone)
		return false;
	// Make sure they have actual members first
	if ( pEnemyStone->NoMembers())
	{
		//Speak( "Enemy guild has no members!" );
		return false;
	}
	// Order cannot declare on Order.
	if ( GetAlignType() == STONEALIGN_ORDER &&
		pEnemyStone->GetAlignType() == STONEALIGN_ORDER )
	{
		//Speak( "Order cannot declare on Order!" );
		return( false );
	}

	// See if they've already declared war on us
	CStoneMember * pMember = GetMember(pEnemyStone);
	if ( pMember )
	{
		DEBUG_CHECK( pMember->GetPriv() == STONEPRIV_ENEMY );
		if ( pMember->GetWeDeclared())
			return true;
	}
	else // They haven't, make a record of this
	{
		pMember = new CStoneMember( this, pEnemyStone->GetUID(), STONEPRIV_ENEMY );
	}
	pMember->SetWeDeclared(true);

	// Now inform the other stone
	// See if they have already declared war on us
	CStoneMember * pEnemyMember = pEnemyStone->GetMember(this);
	if (!pEnemyMember) // Not yet it seems
	{
		pEnemyMember = new CStoneMember( pEnemyStone, GetUID(), STONEPRIV_ENEMY );
	}
	else
	{
		DEBUG_CHECK( pEnemyMember->GetPriv() == STONEPRIV_ENEMY );
		DEBUG_CHECK( pEnemyMember->GetWeDeclared());
	}
	pEnemyMember->SetTheyDeclared(true);

	// announce to both sides.
	AnnounceWar( pEnemyStone, true, true );
	pEnemyStone->AnnounceWar( this, false, true );
	return( true );
}

void CItemStone::TheyDeclarePeace( CItemStone* pEnemyStone, bool fForcePeace )
{
	// Now inform the other stone
	// Make sure we declared war on them
	CStoneMember * pEnemyMember = GetMember(pEnemyStone);
	if ( ! pEnemyMember ) 
		return;

	bool fPrevTheyDeclared = pEnemyMember->GetTheyDeclared();

	DEBUG_CHECK( pEnemyMember->GetPriv() == STONEPRIV_ENEMY );
	if (!pEnemyMember->GetWeDeclared() || fForcePeace) // If we're not at war with them, delete this record
		delete pEnemyMember;
	else
		pEnemyMember->SetTheyDeclared(false);

	if ( ! fPrevTheyDeclared ) 
		return;

	// announce to both sides.
	pEnemyStone->AnnounceWar( this, true, false );
	AnnounceWar( pEnemyStone, false, false );
}

void CItemStone::WeDeclarePeace(CObjUID uid, bool fForcePeace)
{
	CItemStone * pEnemyStone = dynamic_cast <CItemStone*>( uid.ItemFind());
	if (!pEnemyStone)
		return;

	CStoneMember * pMember = GetMember(pEnemyStone);
	if ( ! pMember ) // No such member
		return;
	DEBUG_CHECK( pMember->GetPriv() == STONEPRIV_ENEMY );

	// Set my flags on the subject.
	if (!pMember->GetTheyDeclared() || fForcePeace) // If they're not at war with us, delete this record
		delete pMember;
	else
		pMember->SetWeDeclared(false);

	pEnemyStone->TheyDeclarePeace( this, fForcePeace );
}

void CItemStone::SetupEntryGump( CClient * pClient )
{
	if ( pClient == NULL )
		return;

	int i=0;
	if ( ! pClient->IsPriv(PRIV_GM))
	{
		CStoneMember * pMember = GetMember(pClient->GetChar());
		if ( ! pMember )
		{
			i+=3;	// non-member view.
		}
		else if ( !pMember->IsMaster())
		{
			i++;
		}
	}
	if ( m_type == ITEM_STONE_TOWN )
	{
		i += TARGMODE_MENU_TOWNSTONE1;
	}
	else
	{
		i += TARGMODE_MENU_GUILDSTONE1;
	}
	pClient->Cmd_Script_Menu( (TARGMODE_TYPE) i );
}

void CItemStone::SetupMasterGump( CClient * pClient )
{
	if ( pClient == NULL ) return;
	CStoneMember * pMember = GetMember(pClient->GetChar());

	if ( ! pClient->IsPriv(PRIV_GM))
	{
		if ( pMember == NULL || ! pMember->IsMaster())
		{
			SetupEntryGump( pClient );
			return;
		}
	}
	pClient->Cmd_Script_Menu( (m_type == ITEM_STONE_TOWN) ? TARGMODE_MENU_TOWNSTONE3 : TARGMODE_MENU_GUILDSTONE3 );
}

bool CItemStone::r_Verb( CScript & s, CTextConsole * pSrc ) // Execute command from script
{
	// NOTE:: ONLY CALL this from CChar::r_Verb !!!
	// Little to no security checking here !!!

	static const TCHAR * table[] =
	{
		"ACCEPTCANDIDATE",
		"APPLYTOJOIN",
		"CHANGEALIGN",
		"DECLAREFEALTY",
		"DECLAREPEACE",
		"DECLAREWAR",
		"DISMISSMEMBER",
		"GRANTTITLE",
		"JOINASMEMBER",
		"MASTERMENU",
		"RECRUIT",
		"REFUSECANDIDATE",
		"RESIGN",
		"RETURNMAINMENU",
		"SETABBREVIATION",
		"SETCHARTER",
		"SETGMTITLE",
		"SETNAME",
		// "TELEPORT",
		"TOGGLEABBREVIATION",
		"VIEWCANDIDATES",
		"VIEWCHARTER",
		"VIEWENEMYS",
		"VIEWROSTER",
		"VIEWTHREATS",
	};

	CChar * pCharSrc = pSrc->GetChar();
	if ( pCharSrc == NULL || ! pCharSrc->IsClient())
	{
		return( CItem::r_Verb( s, pSrc ));
	}

	CStoneMember * pMember = GetMember(pCharSrc);
	CClient * pClient = pCharSrc->GetClient();

	switch ( FindTableSorted( s.GetKey(), table, COUNTOF(table))) // ??? FindTableSorted
	{
	case 0: // "ACCEPTCANDIDATE"
		addStoneGump(pClient,STONEGUMP_ACCEPTCANDIDATE);
		break;
	case 1:	// "APPLYTOJOIN",
		AddRecruit( pClient->GetChar());
		break;
	case 2: // "CHANGEALIGN"
		if ( s.HasArgs())
		{
			SetAlignType( (STONEALIGN_TYPE) s.GetArgVal());
			CGString sMsg;
			sMsg.Format( "%s is now a %s %s\n", GetName(), GetAlignName(), GetTypeName());
			Speak( sMsg );
		}
		else
		{
			pClient->Cmd_Script_Menu( TARGMODE_MENU_GUILDSTONE5 );
		}
		break;
	case 3: // "DECLAREFEALTY"
		addStoneGump(pClient,STONEGUMP_FEALTY);
		break;
	case 4: // "DECLAREPEACE"
		addStoneGump(pClient,STONEGUMP_DECLAREPEACE);
		break;
	case 5: // "DECLAREWAR"
		addStoneGump(pClient,STONEGUMP_DECLAREWAR);
		break;
	case 6: // "DISMISSMEMBER"
		addStoneGump(pClient,STONEGUMP_DISMISSMEMBER);
		break;
	case 7: // "GRANTTITLE"
		addStoneGump(pClient,STONEGUMP_GRANTTITLE);
		break;
	case 8: // "JOINASMEMBER",
		PromoteRecruit( pClient->GetChar());
		break;
	case 9: // "MASTERMENU"  // Non GM's shouldn't get here, but in case they do (exploit), give them the non GM menu
		SetupMasterGump( pClient );
		break;
	case 10: // "RECRUIT"
		if ( pMember == NULL )
			break;
		if ( pMember->GetPriv() == STONEPRIV_MASTER ||
			pMember->GetPriv() == STONEPRIV_MEMBER )
			pClient->addTarget( TARGMODE_STONE_RECRUIT, "Who do you want to recruit into the guild?" );
		else
			pCharSrc->m_Act_Targ.ItemFind()->Speak("Only members can recruit.");
		break;
	case 11: // "REFUSECANDIDATE"
		addStoneGump(pClient,STONEGUMP_REFUSECANDIDATE);
		break;
	case 12: // "RESIGN"
		if ( pMember == NULL ) 
			break;
		delete pMember;
		break;
	case 13: // "RETURNMAINMENU"
		SetupEntryGump( pClient );
		break;
	case 14: // "SETABBREVIATION"
		pClient->addPromptConsole( TARGMODE_STONE_SET_ABBREV, "What shall the abbreviation be?" );
		break;
	case 15: // "SETCHARTER"
		addStoneGump(pClient,STONEGUMP_SETCHARTER);
		break;
	case 16: // "SETGMTITLE"
		pClient->addPromptConsole( TARGMODE_STONE_SET_TITLE, "What shall thy title be?" );
		break;
	case 17: // "SETNAME"
		{
			CGString sMsg;
			sMsg.Format( "What would you like to rename the %s to?", GetTypeName());
			pClient->addPromptConsole( TARGMODE_STONE_NAME, sMsg );
		}
		break;
	//case 23: // "TELEPORT"
	//	break;
	case 18: // "TOGGLEABBREVIATION"
		if ( pMember == NULL ) 
			break;
		pMember->ToggleAbbrev();
		SetupEntryGump( pClient );
		break;
	case 19: // "VIEWCANDIDATES"
		addStoneGump(pClient,STONEGUMP_CANDIDATES);
		break;
	case 20: // "VIEWCHARTER"
		addStoneGump(pClient,STONEGUMP_VIEWCHARTER);
		break;
	case 21: // "VIEWENEMYS"
		addStoneGump(pClient,STONEGUMP_VIEWENEMYS);
		break;
	case 22: // "VIEWROSTER"
		addStoneGump(pClient,STONEGUMP_ROSTER);
		break;
	case 23: // "VIEWTHREATS"
		addStoneGump(pClient,STONEGUMP_VIEWTHREATS);
		break;
	default:
		return( CItem::r_Verb( s, pSrc ));
	}
	return( true );
}

void CItemStone::addStoneSetViewCharter( CClient * pClient, STONEGUMP_TYPE iStoneMenu )
{
	static const TCHAR * szDefControls[] =
	{
		"page 0",							// Default page
		"resizepic 0 0 2520 350 400",	// Background pic
		"tilepic 30 50 %d",			// Picture of a stone
		"tilepic 275 50 %d",			// Picture of a stone
		"gumppic 76 126 95",				// Decorative separator
		"gumppic 85 135 96",				// Decorative separator
		"gumppic 264 126 97",			// Decorative separator
		"text 65 35 2301 0",				// Stone name at top
		"text 110 70 0 1",				// Page description
		"text 120 85 0 2",				// Page description
		"text 140 115 0 3",				// Section description
		"text 125 290 0 10",				// Section description
		"gumppic 76 301 95",				// Decorative separator
		"gumppic 85 310 96",				// Decorative separator
		"gumppic 264 301 97",			// Decorative separator
		"text 40 370 0 12",				// Directions
		"button 195 370 2130 2129 1 0 %i",	// Save button
		"button 255 370 2121 2120 1 0 0"		// Cancel button
	};

	CGString sControls[COUNTOF(szDefControls)+10+COUNTOF(m_sCharter)];
	int iControls = 0;
	while ( iControls < COUNTOF(szDefControls))
	{
		// Fix the button ID so we can trap it later
		sControls[iControls].Format( szDefControls[iControls], ( iControls > 4 ) ? (int) iStoneMenu : (int) GetDispID());
		iControls ++;
	}

	bool fView = (iStoneMenu == STONEGUMP_VIEWCHARTER);

	CGString sText[10+COUNTOF(m_sCharter)];
	int iTexts=0;
	sText[iTexts++] = GetName();
	sText[iTexts++].Format( "%s %s Charter", (fView) ? "View": "Set", GetTypeName());
	sText[iTexts++] = "and Web Link";
	sText[iTexts++] = "Charter";

	for ( int iLoop = 0; iLoop < COUNTOF(m_sCharter); iLoop++)
	{
		if ( fView )
		{
			sControls[iControls++].Format( "text 50 %i 0 %i", 155 + (iLoop*22), iLoop + 4);
		}
		else
		{
			sControls[iControls++].Format( "gumppic 40 %i 1143", 152 + (iLoop*22));
			sControls[iControls++].Format( "textentry 50 %i 250 32 0 %i %i", 155 + (iLoop*22), iLoop + 1000, iLoop + 4 );
		}
		if ( fView && iLoop == 0 && m_sCharter[0].IsEmpty())
		{
			sText[iTexts++] = "No charter has been specified.";
		}
		else
		{
			sText[iTexts++] = GetCharter(iLoop);
		}
	}

	if ( fView )
	{
		sControls[iControls++] = "text 50 331 0 11";
	}
	else
	{
		sControls[iControls++] = "gumppic 40 328 1143";
		sControls[iControls++] = "textentry 50 331 250 32 0 1006 11";
	}

	sText[iTexts++] = "Web Link";
	sText[iTexts++] = GetWebPageURL();
	sText[iTexts++] = (fView) ? "Go to the web page": "Save this information";

	ASSERT( iTexts <= COUNTOF(sText));
	ASSERT( iControls <= COUNTOF(sControls));

	pClient->addGumpMenu( TARGMODE_GUMP_GUILD, sControls, iControls, sText, iTexts, 0x6e, 0x46 );
}

bool CItemStone::IsStoneMenuMember( STONEGUMP_TYPE iStoneMenu, const CStoneMember * pMember ) const
{
	ASSERT( pMember );
	switch ( iStoneMenu )
	{
	case STONEGUMP_ROSTER:
	case STONEGUMP_FEALTY:
	case STONEGUMP_DISMISSMEMBER:
	case STONEGUMP_GRANTTITLE:
		if ( pMember->GetPriv() != STONEPRIV_MEMBER &&
			pMember->GetPriv() != STONEPRIV_MASTER)
			return( false );
		break;
	case STONEGUMP_ACCEPTCANDIDATE:
	case STONEGUMP_REFUSECANDIDATE:
	case STONEGUMP_CANDIDATES:
		if ( pMember->GetPriv() != STONEPRIV_CANDIDATE )
			return( false );
		break;
	case STONEGUMP_DECLAREPEACE:
	case STONEGUMP_VIEWENEMYS:
		if ( pMember->GetPriv() != STONEPRIV_ENEMY)
			return( false );
		if ( !pMember->GetWeDeclared())
			return( false );
		break;
	case STONEGUMP_VIEWTHREATS:
		if ( pMember->GetPriv() != STONEPRIV_ENEMY)
			return( false );
		if ( !pMember->GetTheyDeclared())
			return( false );
		break;
	default:
		return( false );
	}
	return( true );
}

bool CItemStone::IsStoneMenuMember( STONEGUMP_TYPE iStoneMenu, const CItemStone * pOtherStone ) const
{
	if ( iStoneMenu != STONEGUMP_DECLAREWAR )
		return( false );

	if ( pOtherStone == this )
		return( false );

	CStoneMember * pMember = GetMember( pOtherStone );
	if (pMember)
	{
		if ( pMember->GetWeDeclared())	// already declared.
			return( false );
	}
	else
	{
		if ( pOtherStone->GetCount() <= 0 )	// Only stones with members can have war declared against them
			return( false );
	}

	return( true );
}

int CItemStone::addStoneGumpListSetup( STONEGUMP_TYPE iStoneMenu, CGString * psText, int iTexts )
{
	// ARGS: psText = NULL if i just want to count.

	if ( iStoneMenu == STONEGUMP_DECLAREWAR )
	{
		// This list is special.
		for ( int i=0; i<g_World.m_Stones.GetCount(); i++ )
		{
			CItemStone * pOtherStone = g_World.m_Stones[i];
			if ( ! IsStoneMenuMember( STONEGUMP_DECLAREWAR, pOtherStone ))
				continue;
			if ( psText )
			{
				psText[iTexts] = pOtherStone->GetName();
			}
			iTexts ++;
		}
		return( iTexts );
	}

	CStoneMember * pMember = STATIC_CAST <CStoneMember *>(GetHead());
	for ( ; pMember != NULL; pMember = pMember->GetNext())
	{
		if ( ! IsStoneMenuMember( iStoneMenu, pMember ))
			continue;

		if ( psText )
		{
			CChar * pChar = pMember->GetLinkUID().CharFind();
			if ( pChar )
			{
				TCHAR szTmp[256];
				strcpy( szTmp, pChar->GetName());
				if (strlen( pMember->GetTitle()) > 0)
				{
					strcat( szTmp, ", ");
					strcat( szTmp, pMember->GetTitle());
				}
				psText[iTexts] = szTmp;
			}
			else
			{
				CItem * pItem = pMember->GetLinkUID().ItemFind();
				if (pItem)
				{
					CItemStone * pStoneItem = dynamic_cast <CItemStone*> ( pItem );
					if (pStoneItem)
						psText[iTexts] = pStoneItem->GetName();
					else
						psText[iTexts] = "Bad stone";
				}
				else
				{
					psText[iTexts] = "Bad member";
				}
			}
		}
		iTexts ++;
	}
	return( iTexts );
}

void CItemStone::addStoneGumpList( CClient * pClient, STONEGUMP_TYPE iStoneMenu )
{
	// Add a list of members of a type.
	// Estimate the size first.
	static const TCHAR * szDefControls[] =
	{
		// "nomove",
		"page 0",
		"resizepic 0 0 5100 400 350",
		"text 15 10 0 0",
		"button 13 290 5050 5051 1 0 %i",
		"text 45 292 0 3",
		"button 307 290 5200 5201 1 0 0",
	};

	CGString sControls[512];
	int iControls = 0;

	int iControlLimit = COUNTOF(szDefControls);
	switch ( iStoneMenu )
	{
	case STONEGUMP_FEALTY:
	case STONEGUMP_ACCEPTCANDIDATE:
	case STONEGUMP_REFUSECANDIDATE:
	case STONEGUMP_DISMISSMEMBER:
	case STONEGUMP_DECLAREWAR:
	case STONEGUMP_DECLAREPEACE:
	case STONEGUMP_GRANTTITLE:
		break;
	case STONEGUMP_ROSTER:
	case STONEGUMP_CANDIDATES:
	case STONEGUMP_VIEWENEMYS:
	case STONEGUMP_VIEWTHREATS:
		iControlLimit --;
		break;
	}

	while ( iControls < iControlLimit )
	{
		// Fix the button's number so we know what screen this is later
		sControls[iControls].Format( szDefControls[iControls], iStoneMenu );
		iControls++;
	}

	static const TCHAR * szDefText[] =
	{
		"%s %s",
		"Previous page",
		"Next page",
		"%s",
	};

	CGString sText[512];
	int iTexts=1;
	for ( ; iTexts<COUNTOF(szDefText) - 1; iTexts++ )
	{
		ASSERT( iTexts < COUNTOF(sText));
		sText[iTexts] = szDefText[iTexts];
	}

	switch ( iStoneMenu )
	{
	case STONEGUMP_ROSTER:
		sText[0].Format(szDefText[0], GetName(), "Roster");
		break;
	case STONEGUMP_CANDIDATES:
		sText[0].Format(szDefText[0], GetName(), "Candidates");
		break;
	case STONEGUMP_FEALTY:
		sText[0].Format(szDefText[0], "Declare your fealty", "" );
		sText[COUNTOF(szDefText)-1].Format(szDefText[COUNTOF(szDefText)-1],
			"I have selected my new lord");
		break;
	case STONEGUMP_ACCEPTCANDIDATE:
		sText[0].Format(szDefText[0], "Accept candidate for", this->GetName());
		sText[COUNTOF(szDefText)-1].Format(szDefText[COUNTOF(szDefText)-1],
			"Accept this candidate for membership");
		break;
	case STONEGUMP_REFUSECANDIDATE:
		sText[0].Format(szDefText[0], "Refuse candidate for", this->GetName());
		sText[COUNTOF(szDefText)-1].Format(szDefText[COUNTOF(szDefText)-1],
			"Refuse this candidate membership");
		break;
	case STONEGUMP_DISMISSMEMBER:
		sText[0].Format(szDefText[0], "Dismiss member from", this->GetName());
		sText[COUNTOF(szDefText)-1].Format(szDefText[COUNTOF(szDefText)-1],
			"Dismiss this member");
		break;
	case STONEGUMP_DECLAREWAR:
		sText[0].Format(szDefText[0], "Declaration of war by", GetName());
		sText[COUNTOF(szDefText)-1].Format(	"Declare war on this %s", GetTypeName());
		break;
	case STONEGUMP_DECLAREPEACE:
		sText[0].Format(szDefText[0], "Declaration of peace by", GetName());
		sText[COUNTOF(szDefText)-1].Format( "Declare peace with this %s", GetTypeName());
		break;
	case STONEGUMP_GRANTTITLE:
		sText[0] = "To whom do you wish to grant a title?";
		sText[COUNTOF(szDefText)-1].Format(szDefText[COUNTOF(szDefText)-1],
			"Grant this member a title");
		break;
	case STONEGUMP_VIEWENEMYS:
		sText[0].Format( "%ss we have declared war on", GetTypeName());
		break;
	case STONEGUMP_VIEWTHREATS:
		sText[0].Format( "%ss which have declared war on us", GetTypeName());
		break;
	}

	switch ( iStoneMenu )
	{
	case STONEGUMP_ROSTER:
	case STONEGUMP_CANDIDATES:
	case STONEGUMP_VIEWTHREATS:
	case STONEGUMP_VIEWENEMYS:
		sText[COUNTOF(szDefText)-1].Format(szDefText[COUNTOF(szDefText)-1], "Done");
		break;
	}
	iTexts++;

	// First count the appropriate members
	int iMemberCount = addStoneGumpListSetup( iStoneMenu, NULL, 0 );

	if ( iMemberCount+iTexts > COUNTOF(sText))
	{
		DEBUG_ERR(( "Too Many Guilds !!!\n" ));
		iMemberCount = COUNTOF(sText) - iTexts - 1;
	}

	int iPages = 0;
	for ( int iLoop = 0; iLoop < iMemberCount; iLoop++)
	{
		if (iLoop % 10 == 0)
		{
			iPages += 1;
			sControls[iControls++].Format("page %i", iPages);
			if (iPages > 1)
			{
				sControls[iControls++].Format("button 15 320 5223 5223 0 %i", iPages - 1);
				sControls[iControls++] = "text 40 317 0 1";
			}
			if ( iMemberCount > iPages * 10)
			{
				sControls[iControls++].Format("button 366 320 5224 5224 0 %i", iPages + 1);
				sControls[iControls++] = "text 288 317 0 2";
			}
		}
		switch ( iStoneMenu )
		{
		case STONEGUMP_FEALTY:
		case STONEGUMP_DISMISSMEMBER:
		case STONEGUMP_ACCEPTCANDIDATE:
		case STONEGUMP_REFUSECANDIDATE:
		case STONEGUMP_DECLAREWAR:
		case STONEGUMP_DECLAREPEACE:
		case STONEGUMP_GRANTTITLE:
			{
				sControls[iControls++].Format("radio 20 %i 5002 5003 0 %i", ((iLoop % 10) * 25) + 35, iLoop + 1000);
			}
		case STONEGUMP_ROSTER:
		case STONEGUMP_CANDIDATES:
		case STONEGUMP_VIEWENEMYS:
		case STONEGUMP_VIEWTHREATS:
			{
				sControls[iControls++].Format("text 55 %i 0 %i", ((iLoop % 10) * 25) + 32, iLoop + 4);
			}
			break;
		}
		ASSERT( iControls < COUNTOF(sControls));
	}

	iTexts = addStoneGumpListSetup( iStoneMenu, sText, iTexts );

	pClient->addGumpMenu( TARGMODE_GUMP_GUILD, sControls, iControls, sText, iTexts, 0x6e, 0x46 );
}

void CItemStone::addStoneGump( CClient * pClient, STONEGUMP_TYPE menuid )
{
	ASSERT( pClient );

	// Use this for a stone dispatch routine....
	switch (menuid)
	{
	case STONEGUMP_ROSTER:
	case STONEGUMP_CANDIDATES:
	case STONEGUMP_FEALTY:
	case STONEGUMP_ACCEPTCANDIDATE:
	case STONEGUMP_REFUSECANDIDATE:
	case STONEGUMP_DISMISSMEMBER:
	case STONEGUMP_DECLAREWAR:
	case STONEGUMP_DECLAREPEACE:
	case STONEGUMP_VIEWENEMYS:
	case STONEGUMP_VIEWTHREATS:
	case STONEGUMP_GRANTTITLE:
		addStoneGumpList(pClient,menuid);
		break;
	case STONEGUMP_VIEWCHARTER:
	case STONEGUMP_SETCHARTER:
		addStoneSetViewCharter(pClient,menuid);
		break;
	}
}

bool CItemStone::OnGumpButton( CClient * pClient, STONEGUMP_TYPE type, DWORD * pdwCheckID, int iCheckQty, CGString * psText, WORD * piTextID, int iTextQty )
{
	// Button presses come here
	ASSERT( pClient );
	switch ( type )
	{
	case STONEGUMP_NONE: // They right clicked out, or hit the cancel button, no more stone functions
		return true;

	case STONEGUMP_VIEWCHARTER:
		// The only button is the web button, so just go there
		pClient->addWebLaunch( GetWebPageURL());
		return true;

	case STONEGUMP_SETCHARTER:
		{
			for (int i = 0; i < iTextQty; i++)
			{
				int id = piTextID[i] - 1000;
				switch ( id )
				{
				case 0:	// Charter[0]
				case 1:	// Charter[1]
				case 2:	// Charter[2]
				case 3:	// Charter[3]
				case 4:	// Charter[4]
				case 5:	// Charter[5]
					SetCharter(id, psText[i]);
					break;
				case 6:	// Weblink
					SetWebPage( psText[i] );
					break;
				}
			}
		}
		return true;

	case STONEGUMP_DISMISSMEMBER:
	case STONEGUMP_ACCEPTCANDIDATE:
	case STONEGUMP_REFUSECANDIDATE:
	case STONEGUMP_FEALTY:
	case STONEGUMP_DECLAREWAR:
	case STONEGUMP_DECLAREPEACE:
	case STONEGUMP_GRANTTITLE:
		break;

	case STONEGUMP_ROSTER:
	case STONEGUMP_VIEWTHREATS:
	case STONEGUMP_VIEWENEMYS:
	case STONEGUMP_CANDIDATES:
		SetupEntryGump( pClient );
		return( true );

	default:
		return( false );
	}

	if ( iCheckQty == 0 )	 // If they hit ok but didn't pick one, treat it like a cancel
		return true;

	int iMember = pdwCheckID[0] - 1000;

	CStoneMember * pMember = NULL;
	bool fFound = false;
	int iLoop = 0;
	int iStoneIndex = 0;

	if ( type == STONEGUMP_DECLAREWAR )
	{
		for ( ; iStoneIndex<g_World.m_Stones.GetCount(); iStoneIndex ++ )
		{
			CItemStone * pOtherStone = g_World.m_Stones[iStoneIndex];
			if ( ! IsStoneMenuMember( STONEGUMP_DECLAREWAR, pOtherStone ))
				continue;
			if (iLoop == iMember)
			{
				fFound = true;
				break;
			}
			iLoop ++;
		}
	}
	else
	{
		pMember = STATIC_CAST <CStoneMember *>(GetHead());
		for (; pMember != NULL; pMember = pMember->GetNext())
		{
			if ( ! IsStoneMenuMember( type, pMember ))
				continue;
			if (iLoop == iMember)
			{
				fFound = true;
				break;
			}
			iLoop ++;
		}
	}

	if (fFound)
	{
		switch ( type ) // Button presses come here
		{
		case STONEGUMP_DECLAREWAR:
			if ( ! WeDeclareWar(g_World.m_Stones[iStoneIndex]))
			{
				pClient->SysMessage( "Cannot declare war" );
			}
			break;
		case STONEGUMP_ACCEPTCANDIDATE:
			ASSERT( pMember );
			PromoteRecruit( pMember->GetLinkUID().CharFind());
			break;
		case STONEGUMP_REFUSECANDIDATE:
			ASSERT( pMember );
			delete pMember;
			break;
		case STONEGUMP_DISMISSMEMBER:
			ASSERT( pMember );
			delete pMember;
			break;
		case STONEGUMP_FEALTY:
			ASSERT( pMember );
			{
				CStoneMember * pMe = GetMember(pClient->GetChar());
				if ( pMe == NULL ) return( false );
				pMe->SetLoyalTo( pMember->GetLinkUID().CharFind());
			}
			break;
		case STONEGUMP_DECLAREPEACE:
			ASSERT( pMember );
			WeDeclarePeace(pMember->GetLinkUID());
			break;
		case STONEGUMP_GRANTTITLE:
			ASSERT( pMember );
			pClient->m_Targ_PrvUID = pMember->GetLinkUID();
			pClient->addPromptConsole( TARGMODE_STONE_GRANT_TITLE, "What title dost thou grant?" );
			return( true );
		}
	}
	else
	{
		pClient->SysMessage("Who is that?");
	}

	// Now send them back to the screen they came from

	switch ( type )
	{
	case STONEGUMP_ACCEPTCANDIDATE:
	case STONEGUMP_REFUSECANDIDATE:
	case STONEGUMP_DISMISSMEMBER:
	case STONEGUMP_DECLAREPEACE:
	case STONEGUMP_DECLAREWAR:
		SetupMasterGump( pClient );
		break;
	default:
		SetupEntryGump( pClient );
		break;
	}

	return true;
}

void CItemStone::SetTownName()
{
	// For town stones.
	if ( ! IsTopLevel()) 
		return;
	CRegionBase * pArea = GetTopPoint().GetRegion(( m_type == ITEM_STONE_TOWN ) ? REGION_TYPE_AREA : REGION_TYPE_ROOM );
	if ( pArea )
	{
		pArea->SetName( GetIndividualName());
	}
}

bool CItemStone::MoveTo( CPointMap pt )
{
	// Put item on the ground here.
	if ( m_type == ITEM_STONE_TOWN || m_type == ITEM_STONE_ROOM )
	{
		SetTownName();
	}
	return CItem::MoveTo(pt);
}

bool CItemStone::SetName( const TCHAR * pszName )
{
	// If this is a town then set the whole regions name.

	if ( ! CItem::SetName( pszName ))
		return( false );
	if ( IsTopLevel() && ( m_type == ITEM_STONE_TOWN || m_type == ITEM_STONE_ROOM ))
	{
		SetTownName();
	}
	return( true );
}

bool CItemStone::OnPromptResp( CClient * pClient, TARGMODE_TYPE TargMode, const TCHAR * pszText, CGString & sMsg )
{
	ASSERT( pClient );
	switch ( TargMode )
	{
	case TARGMODE_STONE_NAME:
		// Set the stone or town name !
		if ( ! CItemStone::IsUniqueName( pszText ))
		{
			if (!strcmpi( pszText, GetName()))
			{
				pClient->SysMessage( "Name is unchanged." );
				return false;
			}
			pClient->SysMessage( "That name is already taken." );
			CGString sMsg;
			sMsg.Format( "What would you like to rename the %s to?", GetTypeName());
			pClient->addPromptConsole( TARGMODE_STONE_NAME, sMsg );
			return false;
		}

		SetName( pszText );
		if ( NoMembers()) // No members? It must be a brand new stone then, fix it up
		{
			PromoteRecruit( pClient->GetChar());
		}
		sMsg.Format( "%s renamed: %s", GetTypeName(), pszText );
		break;

	case TARGMODE_STONE_SET_ABBREV:
		SetAbbrev(pszText);
		sMsg.Format( "Abbreviation set: %s", pszText );
		break;

	case TARGMODE_STONE_GRANT_TITLE:
		{
			CStoneMember * pMember = GetMember( pClient->m_Targ_PrvUID.CharFind());
			if (pMember)
			{
				pMember->SetTitle(pszText);
				sMsg.Format( "Title set: %s", pszText);
			}
		}
		break;
	case TARGMODE_STONE_SET_TITLE:
		{
			CStoneMember * pMaster = GetMasterMember();
			pMaster->SetTitle(pszText);
			sMsg.Format( "Title set: %s", pszText);
		}
		break;
	}
	return( true );
}

void CItemStone::Use_Item( CClient * pClient )
{
	if ( NoMembers() && m_type == ITEM_STONE_GUILD ) // Everyone resigned...new master
	{
		PromoteRecruit( pClient->GetChar());
	}
	SetupEntryGump( pClient );
}
