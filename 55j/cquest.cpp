//
// cQuest.cpp
// Copyright Menace Software (www.menasoft.com).
//

#include "graysvr.h"	// predef header.
#include "cclient.h"

//*****************************************************************
// -CCharRefArray

int CCharRefArray::FindChar( const CChar * pChar ) const
{
	if ( pChar == NULL )
	{
		return( -1 );
	}
	CGrayUID uid( pChar->GetUID());
	int iQty = m_uidCharArray.GetCount();
	for ( int i=0; i<iQty; i++ )
	{
		if ( uid == m_uidCharArray[i] )
			return( i );
	}
	return( -1 );
}

int CCharRefArray::AttachChar( const CChar * pChar )
{
	int i = FindChar( pChar );
	if ( i >= 0 )
		return( i );
	return m_uidCharArray.Add( pChar->GetUID());
}

void CCharRefArray::DetachChar( int i )
{
	m_uidCharArray.RemoveAt(i);
}

int CCharRefArray::DetachChar( const CChar * pChar )
{
	int i = FindChar( pChar );
	if ( i < 0 )
		return( -1 );
	DetachChar( i );
	return( i );
}

void CCharRefArray::DeleteChars()
{
	int iQty = m_uidCharArray.GetCount();
	for ( int k=0; k<iQty; k++ )
	{
		CChar * pChar = m_uidCharArray[k].CharFind();
		if ( pChar )
		{
			delete pChar;	// pChar->Delete(); // must delete now to avoid player link problems later.
		}
	}
}

void CCharRefArray::WritePartyChars( CScript & s )
{
	int iQty = m_uidCharArray.GetCount();
	for ( int j=0; j<iQty; j++ )	// write out links to all my chars
	{
		s.WriteKeyHex( "CHARUID", m_uidCharArray[j] );
	}
}

//*****************************************************************
// -CPartyDef

CPartyDef::CPartyDef( CChar * pChar1, CChar *pChar2 )
{
	// pChar1 = the master.
	ASSERT(pChar1);
	ASSERT(pChar2);
	pChar1->m_pParty = this;
	pChar2->m_pParty = this;
	AttachChar(pChar1);
	AttachChar(pChar2);
	SendAddList( UID_CLEAR, NULL );	// send full list to all.
}

int CPartyDef::AttachChar( CChar * pChar )
{
	// RETURN: 
	//  index of the char in the group. -1 = not in group.
	int i = m_Chars.AttachChar( pChar );
	if (i>=0)
	{
		m_fLootFlags.InsertAt(i,false);
	}
	return( i );
}

int CPartyDef::DetachChar( CChar * pChar )
{
	// RETURN: 
	//  index of the char in the group. -1 = not in group.
	int i = m_Chars.DetachChar( pChar );
	if (i>=0)
	{
		m_fLootFlags.RemoveAt(i);
	}
	return( i );
}

void CPartyDef::SetLootFlag( CChar * pChar, bool fSet )
{
	ASSERT( pChar );
	int i = m_Chars.FindChar( pChar );
	if ( i >= 0 )
	{
		m_fLootFlags[i] = fSet;
	}

	pChar->SysMessage( fSet ?
		"You have chosen to allow your party to loot your corpse" :
		"You have chosen to prevent your party from looting your corpse" );
}

bool CPartyDef::GetLootFlag( const CChar * pChar )
{
	int i = m_Chars.FindChar( pChar );
	if ( i >= 0 )
	{
		return m_fLootFlags[i];
	}
	return( false );
}

void CPartyDef::SysMessageAll( LPCTSTR pText )
{
	// SysMessage to all members of the party.
	int iQty = m_Chars.GetCharCount();
	for ( int i=0; i<iQty; i++ )
	{
		CChar * pChar = m_Chars.GetChar(i).CharFind();
		ASSERT(pChar);
		pChar->SysMessage( pText );
	}
}

bool CPartyDef::SendMemberMsg( CChar * pCharDest, const CExtData * pExtData, int iLen )
{
	if ( pCharDest == NULL )
	{
		SendAll( pExtData, iLen );
		return( true );
	}

	// Weirdness check.
	if ( pCharDest->m_pParty != this )	
	{
		if ( DetachChar( pCharDest ) >= 0 )	// this is bad!
			return( false );
		return( true );
	}
	else if ( ! m_Chars.IsCharIn( pCharDest ))
	{
		pCharDest->m_pParty = NULL;
		return( true );
	}

	if ( pCharDest->IsClient())
	{
		CClient * pClient = pCharDest->GetClient();
		ASSERT(pClient);
		pClient->addExtData( EXTDATA_Party_Msg, pExtData, iLen );
	}

	return( true );
}

void CPartyDef::SendAll( const CExtData * pExtData, int iLen )
{
	// Send this to all members of the party.
	int iQty = m_Chars.GetCharCount();
	for ( int i=0; i<iQty; i++ )
	{
		CChar * pChar = m_Chars.GetChar(i).CharFind();
		ASSERT(pChar);
		if ( ! SendMemberMsg( pChar, pExtData, iLen ))
		{
			iQty--;
			i--;
		}
	}
}

void CPartyDef::AcceptMember( CChar * pChar )
{
	// This person has accepted to be part of the party.
	ASSERT(pChar);

	SendAddList( pChar->GetUID(), NULL );	// tell all that there is a new member.

	pChar->m_pParty = this;
	AttachChar(pChar);

	// "You have been added to the party"
	// NOTE: We don't have to send the full party to everyone. just the new guy.
	SendAddList( UID_CLEAR, pChar );
}

void CPartyDef::SendAddList( CGrayUID uid, CChar * pCharDest )
{
	// Send out the full party manifest to all the clients.

	CExtData ExtData;
	ExtData.Party_Msg_Data.m_code = PARTYMSG_Add;

	int iQty;
#if 0
	if ( uid.IsValidUID())
	{
		// Send just this one.
		ExtData.Party_Msg_Data.m_uids[0] = uid;
		iQty = 1;
	}
	else
#endif
	{
		// Send All.
		iQty = m_Chars.GetCharCount();
		ASSERT(iQty);
		for ( int i=0; i<iQty; i++ )
		{
			ExtData.Party_Msg_Data.m_uids[i] = m_Chars.GetChar(i);
		}
	}

	// Now send out the list to all that are clients.
	ExtData.Party_Msg_Data.m_Qty = iQty;
	SendMemberMsg( pCharDest, &ExtData, (iQty*sizeof(DWORD))+2 );
}

void CPartyDef::SendRemoveList( CChar * pCharRemove, CGrayUID uidAct )
{
	// Send out the party manifest to the client.
	// uid = remove just this char. 0=all.
	// uidAct = Who did the removing.

	CExtData ExtData;
	ExtData.Party_Msg_Data.m_code = PARTYMSG_Remove; 

	int iQty;
	if ( pCharRemove )
	{
		// just removing this one person.
		ExtData.Party_Msg_Data.m_uids[0] = pCharRemove->GetUID();
		iQty = 1;
	}
	else
	{
		// We are disbanding.
		iQty = m_Chars.GetCharCount();
		ASSERT(iQty);
		for ( int i=0; i<iQty; i++ )
		{
			ExtData.Party_Msg_Data.m_uids[i] = m_Chars.GetChar(i);
		}
	}

	ExtData.Party_Msg_Data.m_Qty = iQty;	// last uid is really the source.
	ExtData.Party_Msg_Data.m_uids[iQty++] = uidAct;

	SendAll( &ExtData, (iQty*sizeof(DWORD))+2 );
}

void CPartyDef::MessageClient( CClient * pClient, CGrayUID uidSrc, const NCHAR * pText, int ilenmsg ) // static
{
	// Message to a single client

	ASSERT(pClient);
	if ( pText == NULL )
		return;

	CExtData ExtData;
	ExtData.Party_Msg_Rsp.m_code = PARTYMSG_Msg;
	ExtData.Party_Msg_Rsp.m_UID = uidSrc;
	if ( ilenmsg > MAX_TALK_BUFFER )
		ilenmsg = MAX_TALK_BUFFER;
	memcpy( ExtData.Party_Msg_Rsp.m_msg, pText, ilenmsg );

	pClient->addExtData( EXTDATA_Party_Msg, &ExtData, ilenmsg+5 );
}

bool CPartyDef::MessageMember( CGrayUID uidTo, CGrayUID uidSrc, const NCHAR * pText, int ilenmsg )
{
	// Message to a single members of the party.
	if ( pText == NULL )
		return false;
	if ( g_Log.IsLoggedMask( LOGM_PLAYER_SPEAK ))
	{
		// g_Log.Event( LOGM_PLAYER_SPEAK, "%x:'%s' Says '%s' mode=%d\n", m_Socket.GetSocket(), m_pChar->GetName(), szText, mode );
	}

	CChar * pChar = uidTo.CharFind();
	if ( pChar == NULL )
		return( false );

	CExtData ExtData;
	ExtData.Party_Msg_Rsp.m_code = PARTYMSG_Msg;
	ExtData.Party_Msg_Rsp.m_UID = uidSrc;
	if ( ilenmsg > MAX_TALK_BUFFER )
		ilenmsg = MAX_TALK_BUFFER;
	memcpy( ExtData.Party_Msg_Rsp.m_msg, pText, ilenmsg );

	return SendMemberMsg( pChar, &ExtData, ilenmsg+5 );
}

void CPartyDef::MessageAll( CGrayUID uidSrc, const NCHAR * pText, int ilenmsg )
{
	// Message to all members of the party.

	if ( pText == NULL )
		return;

	if ( g_Log.IsLoggedMask( LOGM_PLAYER_SPEAK ))
	{
		// g_Log.Event( LOGM_PLAYER_SPEAK, "%x:'%s' Says '%s' mode=%d\n", m_Socket.GetSocket(), m_pChar->GetName(), szText, mode );
	}

	CExtData ExtData;
	ExtData.Party_Msg_Rsp.m_code = PARTYMSG_Msg;
	ExtData.Party_Msg_Rsp.m_UID = uidSrc;
	if ( ilenmsg > MAX_TALK_BUFFER )
		ilenmsg = MAX_TALK_BUFFER;
	memcpy( ExtData.Party_Msg_Rsp.m_msg, pText, ilenmsg );

	SendAll( &ExtData, ilenmsg+5 );
}

bool CPartyDef::Disband( CGrayUID uidMaster )
{
	// Make sure i am the master.
	if ( ! m_Chars.GetCharCount())
	{
		return( false );
	}

	CGrayUID uidMasterCheck = m_Chars.GetChar(0);
	if ( uidMasterCheck != uidMaster )
	{
		return false;
	}

	SysMessageAll("Your party has disbanded.");
	SendRemoveList( NULL, uidMaster );

	int iQty = m_Chars.GetCharCount();
	ASSERT(iQty);
	for ( int i=0; i<iQty; i++ )
	{
		CChar * pChar = m_Chars.GetChar(i).CharFind();
		if ( pChar == NULL )
			continue;
		pChar->m_pParty = NULL;
	}

	delete this;	// should remove itself from the world list.
	return( true );
}

bool CPartyDef::DeclineEvent( CChar * pCharDecline, CGrayUID uidInviter )	// static
{
	// This should happen after a timeout as well.
	// " You notify %s that you do not wish to join the party"

	return( true );
}

bool CPartyDef::AcceptEvent( CChar * pCharAccept, CGrayUID uidInviter )	// static
{
	// We are accepting the invite to join a party
	// Check to see if the invite is genuine. ??? No security Here !!!

	// Party master is only one that can add ! GetChar(0)

	ASSERT( pCharAccept );

	CChar * pCharInviter = uidInviter.CharFind();
	if ( pCharInviter == NULL )
	{
		return false;
	}

	CPartyDef * pParty = pCharInviter->m_pParty;

	if ( pCharAccept->m_pParty != NULL )	// Aready in a party !
	{
		if ( pParty == pCharAccept->m_pParty )	// already in this party
			return( true );

		// So leave previous party.
		pCharAccept->m_pParty->RemoveChar( pCharAccept->GetUID(), pCharAccept->GetUID() );
		DEBUG_CHECK(pCharAccept->m_pParty == NULL );
		pCharAccept->m_pParty = NULL;
	}

	CGString sMsg;
	sMsg.Format( "%s has joined the party", (LPCTSTR) pCharAccept->GetName() );

	if ( pParty == NULL )
	{
		// create the party now.
		pParty = new CPartyDef( pCharInviter, pCharAccept );
		ASSERT(pParty);
		g_World.m_Parties.InsertHead( pParty );
		pCharInviter->SysMessage( sMsg );
	}
	else
	{
		// Just add to existing party. Only the party master can do this !
		pParty->SysMessageAll( sMsg );	// tell everyone already in the party about this.
		pParty->AcceptMember( pCharAccept );
	}

	pCharAccept->SysMessage( "You have been added to the party" );
	return( true );
}

bool CPartyDef::RemoveChar( CGrayUID uid, CGrayUID uidAct )
{
	// ARGS:
	//  uid = Who is being removed.
	//  uidAct = who removed this person. (Only the master or self can remove)
	//
	// NOTE: remove of the master will cause the party to disband.

	if ( ! m_Chars.GetCharCount())
	{
		return( false );
	}

	CGrayUID uidMaster = m_Chars.GetChar(0);
	if ( uid != uidAct && uidAct != uidMaster )
	{
		// cheating ?
		// must be removed by self or master !
		return( false );
	}

	CChar * pChar = uid.CharFind();	// who is to be removed.
	if ( pChar == NULL || ! m_Chars.IsCharIn(pChar))
	{
		return( false );		
	}

	if ( uid == uidMaster )
	{
		return( Disband( uidMaster ));
	}

	LPCTSTR pszForceMsg = ( (DWORD) uidAct != (DWORD) uid ) ? "been removed from" : "left";

	// Tell the kicked person they are out
	pChar->SysMessagef( "You have %s the party", pszForceMsg );

	// Send the remove message to all.
	SendRemoveList( pChar, uidAct );

	DetachChar( pChar );
	pChar->m_pParty = NULL;

	if ( m_Chars.GetCharCount() <= 1 )
	{
		// Disband the party
		// "The last person has left the party"
		Disband( uidMaster );
	}
	else
	{
		// Tell the others he is gone
		CGString sMsg;
		sMsg.Format( "%s has %s your party", (LPCTSTR) pChar->GetName(), (LPCTSTR) pszForceMsg );
		SysMessageAll(sMsg);
	}
	return true;
}

#if 0

void CPartyDef::ShareKarmaAndFame( int iFame, int iKarma )
{
	// We have all made a kill.
	//
}

#endif
