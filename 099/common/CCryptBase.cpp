    // CCryptBase.cpp
    // Copyright Menace Software (www.menasoft.com).
    // Common for client and server.
    //
    
    #ifdef GRAY_AGENT
    #include "../grayagent/stdafx.h"
    #include "../grayagent/grayagent.h"
    
    #else
    #include "graycom.h"
    #include "graymul.h"
    #include "grayproto.h"
    #endif
    
    ////////////////////////////////////////////////////////////////////
    
    bool CCryptBase::SetClientVerEnum( int iVer )
    {
    	switch ( iVer )
    	{
    	case 0x0:	// This turns off crypt for sphereclient.
    		break;
    
    	// VER=3.0.2b
    	case 1:
    	case 0x300020:
    	case 0x300022:	// 06/12/2001 @ 02cef0
    		iVer = 0x300011;
    		m_MasterHi = 0x2de3addd;
    		m_MasterLo = 0xa3e5227f;
    		break;
    
    
    	// VER=3.0.1a
    	case 2:
    	case 0x300010:
    	case 0x300011:	// 05/15/2001 @ 02d140,
    		iVer = 0x300011;
    		m_MasterHi = 0x2daba7ed;
    		m_MasterLo = 0xa3c17e7f;
    		break;
    
    	// VER=3.0.0a	// 03/20/2001 @ 02cd30, sep = 81f2
    	// VER=3.0.0c	// 04/04/2001 @ 
    	case 3:
    		iVer = 0x300003;
    	case 0x300000:
    	case 0x300001:
    	case 0x300003:
    		m_MasterHi = 0x2d93a5fd;
    		m_MasterLo = 0xa3dd527f;
    		break;
    
    	// VER=2.0.8k	// 03/10/2001 @ 02ce30, sep = 81f2
    	case 4:
    		iVer = 0x20008b;
    	case 0x200080:
    	case 0x20008b:
    		m_MasterHi = 0x2cd3257d;
    		m_MasterLo = 0xa37f527f;
    		break;
    
    	// VER=2.0.7	// 02/13/2001 @ 02cb39, sep = 81f2
    	case 5:
    		iVer = 0x200070;
    	case 0x200070:
    		m_MasterHi = 0x2c9bc78d;
    		m_MasterLo = 0xa35bfe7f;
    		break;
    
    	// VER=2.0.6c	// 01/28/2001 @ 02c720, sep = 81f2
    	case 6:
    		iVer = 0x200060;
    	case 0x200060:
    		m_MasterHi = 0x2c43ed9d;
    		m_MasterLo = 0xa334227f;
    		break;
    
    	// VER=2.0.4	// 12/28/2000 @ 025bd0, sep = 81f2
    	case 7:
    		iVer = 0x200040;
    	case 0x200040:
    		m_MasterHi = 0x2df385bd;
    		m_MasterLo = 0xa3ed127f;
    		break;
    
    	// VER=2.0.3	// 10/17/2000 @ 0c0ed8, sep = 81f2
    	case 8:
    		iVer = 0x200030;
    	case 0x200030:
    		m_MasterHi = 0x2dbbb7cd;
    		m_MasterLo = 0xa3c95e7f;
    		break;
    
    	// VER=2.0.2	// 9/29/2000 @ 0c0837,
    	case 9:
    		iVer = 0x200020;
    	case 0x200020:
    		m_MasterHi = 0x2d63addd;
    		m_MasterLo = 0xa3a5227f;
    		break;
    
    	// VER=2.0.1	// 9/27/2000 @ 0c0aa7,0c0aad
    	case 10:
    		iVer = 0x200010;
    	case 0x200010:
    		m_MasterHi = 0x2d2ba7ed;
    		m_MasterLo = 0xa3817e7f;
    		break;
    
    	// VER=2.0.0	// 4/19/2000 by UOX team
    	case 11:
    		iVer = 0x200005;
    	case 0x200005: // VER=2.0.0e = 6/27/2000 @ 0bd657 and 0bd65d
    	case 0x200004:
    	case 0x200003:	// VER=2.0.0c
    		m_MasterHi = 0x2d13a5fd;
    		m_MasterLo = 0xa39d527f;
    		break;
    
    	case 12:
    		iVer = 0x200002;
    	case 0x200002:	// VER=2.0.0b
    	case 0x200001:
    	case 0x200000:	// VER=2.0.0 = 4/17/2000
    		m_MasterHi = 0x2d13a5fd;
    		m_MasterLo = 0xa39d527f;
    		break;
    
    	// VER=1.26.4	// 1/27/2000 by Westy
    	case 13:
    		iVer = 0x126040;
    	case 0x126040:	// VER=1.26.4 = 1/27/00 by Westy
    		m_MasterHi = 0x32750719;
    		m_MasterLo = 0x0a2d100b;
    		break;
    
    	// VER=1.26.3	// 1/18/2000 by Westy and Beosil (beosil@mcb.at)
    	case 14:
    		iVer = 0x126030;
    	case 0x126030:	// VER=1.26.3 = 1/18/00 by Westy and Beosil (beosil@mcb.at)
    		m_MasterHi = 0x323d3569;
    		m_MasterLo = 0x0a095c0b;
    		break;
    
    	// VER=1.26.2	// 11/23/99 by Westy and Beosil (beosil@mcb.at)
    	case 15:
    		iVer = 0x126020;
    	case 0x126020:	// 11/23/99 by Westy and Beosil (beosil@mcb.at)
    		m_MasterHi = 0x32e52f79;
    		m_MasterLo = 0x0a65200b;
    		break;
    
    	// VER=1.26.1	// 09/09/99 by Westy and Beosil (beosil@mcb.at)
    	case 16:
    		iVer = 0x126010;
    	case 0x126010:	// 09/09/99 by Westy and Beosil (beosil@mcb.at)
    		m_MasterHi = 0x32ad2549;
    		m_MasterLo = 0x0a417c0b;
    		break;
    	// VER=1.26.0	// 08/35/99 by Westy and Beosil (beosil@mcb.at)
    	case 17:
    		iVer = 0x126000;
    	case 0x126000:	// 08/35/99 by Westy and Beosil (beosil@mcb.at)
    		m_MasterHi = 0x32952759;
    		m_MasterLo = 0x0a5d500b;
    		break;
    	// VER=1.25.37	// 03/18/99 by Westy and Beosil (beosil@mcb.at)
    	case 18:
    		iVer = 0x125370;
    	case 0x125370:	// 03/18/99 by Westy and Beosil (beosil@mcb.at)
    		m_MasterHi = 0x378757DC;
    		m_MasterLo = 0x0595DCC6;
    		break;
    	// VER=1.25.36	// 12/1/98 by Beosil (beosil@mcb.at)
    #define CLIKEY_12536_HI1 0x387fc5cc
    #define CLIKEY_12536_HI2 0x35ce9581
    #define CLIKEY_12536_HI3 0x07afcc37
    #define CLIKEY_12536_LO1 0x021510c6
    #define CLIKEY_12536_LO2 0x4c3a1353
    #define CLIKEY_12536_LO3 0x16ef783f
    	case 19:
    		iVer = 0x125360;
    	case 0x125360:	// 12/1/98 by Beosil (beosil@mcb.at)
    		// Special multi key.
    		m_MasterHi = CLIKEY_12536_HI1;
    		m_MasterLo = CLIKEY_12536_LO1;
    		break;
    	// VER=1.25.35	// Released on the T2A CD.
    	case 20:
    		iVer = 0x125350;
    	case 0x125350:	// Released on the T2A CD.
    		m_MasterHi = 0x383477BC;
    		m_MasterLo = 0x02345CC6;
    		break;
    	// VER=1.25.34
    	case 0x125340:
    		m_MasterHi = 0x38ECEDAC;
    		m_MasterLo = 0x025720C6;
    		break;
    	// VER=1.25.33
    	case 0x125330:
    		m_MasterHi = 0x38A5679C;
    		m_MasterLo = 0x02767CC6;
    		break;
    	// VER=1.25.32
    	case 0x125320:
    		m_MasterHi = 0x389DE58C;
    		m_MasterLo = 0x026950C6;
    		break;
    	// VER=1.25.31
    	case 0x125310:
    		m_MasterHi = 0x395A647C;
    		m_MasterLo = 0x0297BCC6;
    		break;
    	default:
    		if ( ! m_fIgnition )
    		{
    			m_iClientVersion = -1;
    			return( false );
    		}
    	}
    	m_iClientVersion = iVer;
    
    	if ( m_fInit )	// must re-init if we are changing the crypt code.
    	{
    		Init();
    	}
    
    	return( true );
    }
    
    void CCryptBase::Init( DWORD dwIP )
    {
    	// Generate the decrypt key based on the ip we get from the client.
    	// The client will use the key of 127.0.0.1 randomly with it's real ip for no good reason.
    
    	m_fInit = true;
    	m_seed = dwIP;	// 0x7f00001 = 127.0.0.1
    
    	if ( ! m_seed )	// Don't bother with encryption. (new client)
    	{
    		m_iClientVersion = 0;
    	}
    	else
    	{
    		m_CryptMaskLo = (((~m_seed) ^ 0x00001357) << 16) | ((( m_seed) ^ 0xffffaaaa) & 0x0000ffff);
    		m_CryptMaskHi = ((( m_seed) ^ 0x43210000) >> 16) | (((~m_seed) ^ 0xabcdffff) & 0xffff0000);
    	}
    }
    
    TCHAR* CCryptBase::WriteClientVer( TCHAR * pszVersion ) const
    {
    	int iClientVersion = GetClientVer();
    	int iLen = sprintf( pszVersion, _TEXT( "%s%x.%x.%x" ),
    		GetClientIgnition() ? "I" : "",
    		iClientVersion/0x100000,
    		(iClientVersion/0x1000)%0x100,
    		(iClientVersion/0x10)%0x100 );
    	if ( iClientVersion & 0x0f )
    	{
    		pszVersion = ( iClientVersion & 0x0f ) + 'a' - 1;
    		pszVersion = '\0';
    	}
    	return( pszVersion );
    }
    
    bool CCryptBase::SetClientVer( LPCTSTR pszVersion )
    {
    
    	// They have this annoying habit of putting letters at the end of the version string.
    	// 2.0.0d for example.
    
    	if ( pszVersion == NULL || *pszVersion == '\0' )
    		return false;
    
    	SetClientIgnition( false );
    
    	int iVer = 0;
    	int iPoint = 0;
    	int iDigPoint = 0;
    	for ( int i=0; true; i++ )
    	{
    		TCHAR ch = pszVersion;
    		if ( isdigit(ch))
    		{
    			iVer *= 0x10;
    			iVer += ( ch - '0' );
    			iDigPoint ++;
    			continue;
    		}
    
    		if ( iPoint )
    		{
    			if ( iDigPoint == 0 )
    			{
    				iVer *= 0x100;
    			}
    			else if ( iDigPoint == 1 )
    			{
    				int iTmp = iVer & 0x0f;
    				iVer &= ~0x0f;
    				iVer *= 0x10;
    				iVer += iTmp;
    			}
    		}
    		else
    		{
    			if ( i == 0 && tolower(ch) == 'i' )
    			{
    				SetClientIgnition( true );
    				continue;
    			}
    		}
    
    		if ( ch == '.' )
    		{
    			if ( ++iPoint > 2 )
    			{
    				iVer *= 0x10;
    				break;
    			}
    			iDigPoint = 0;
    			continue;
    		}
    
    		if ( ! iPoint && ! isalpha(ch))
    		{
    			iVer *= 0x10;
    			break;
    		}
    
    		while ( iPoint< 2 )
    		{
    			iVer *= 0x100;
    			iPoint++;
    		}
    
    		// last char digit slot.
    		iVer *= 0x10;
    		if ( isalpha(ch))
    		{
    			iVer += ( tolower(ch) - 'a' ) + 1;
    		}
    		break;
    	}
    
    	m_fInit = false;
    
    	if ( ! SetClientVerEnum( iVer ))
    	{
    		DEBUG_ERR(( "Unsupported ClientVersion '%s'. Use Ignition?\n", pszVersion ));
    		return( false );
    	}
    
    	return( true );
    }
    
    void CCryptBase::Decrypt( BYTE * pOutput, const BYTE * pInput, int iLen )
    {
    	// In previous versions (<12600) decrypt and encrypt are the same.
    	ASSERT( m_fInit );
    
    	if ( ! m_fIgnition )
    	{
    		// Old fashioned rotary crypt stuff.
    		if ( GetClientVer() >= 0x125370 )
    		{
    			for ( int i=0; i<iLen; i++ )
    			{
    				pOutput = pInput ^ (BYTE) m_CryptMaskLo;
    				DWORD MaskLo = m_CryptMaskLo;
    				DWORD MaskHi = m_CryptMaskHi;
    				m_CryptMaskLo = ((MaskLo >> 1) | (MaskHi << 31)) ^ m_MasterLo;
    				MaskHi =		((MaskHi >> 1) | (MaskLo << 31)) ^ m_MasterHi;
    				m_CryptMaskHi = ((MaskHi >> 1) | (MaskLo << 31)) ^ m_MasterHi;
    			}
    			return;
    		}
    
    		if ( GetClientVer() == 0x125360 )
    		{
    			for ( int i=0; i<iLen; i++ )
    			{
    				pOutput = pInput ^ (BYTE) m_CryptMaskLo;
    				DWORD MaskLo = m_CryptMaskLo;
    				DWORD MaskHi = m_CryptMaskHi;
    				m_CryptMaskHi =
    					(m_MasterHi >> ((5 * MaskHi * MaskHi) & 0xff))
    					+ (MaskHi * m_MasterHi)
    					+ (MaskLo * MaskLo * CLIKEY_12536_HI2)
    					+ CLIKEY_12536_HI3;
    				m_CryptMaskLo =
    					(m_MasterLo >> ((3 * MaskLo * MaskLo) & 0xff))
    					+ (MaskLo * m_MasterLo)
    					- (m_CryptMaskHi * m_CryptMaskHi * CLIKEY_12536_LO2)
    					+ CLIKEY_12536_LO3;
    			}
    			return;
    		}
    
    		if ( GetClientVer() ) // GRAY_CLIENT_VER <= 0x125350
    		{
    			for ( int i=0; i<iLen; i++ )
    			{
    				pOutput = pInput ^ (BYTE) m_CryptMaskLo;
    				DWORD MaskLo = m_CryptMaskLo;
    				DWORD MaskHi = m_CryptMaskHi;
    				m_CryptMaskLo = ((MaskLo >> 1) | (MaskHi << 31)) ^ m_MasterLo;
    				m_CryptMaskHi = ((MaskHi >> 1) | (MaskLo << 31)) ^ m_MasterHi;
    			}
    			return;
    		}
    	}
    
    	if ( pOutput != pInput )
    	{
    		memcpy( pOutput, pInput, iLen );
    	}
    }
    
    void CCryptBase::Encrypt( BYTE * pOutput, const BYTE * pInput, int iLen )
    {
    	Decrypt( pOutput, pInput, iLen );
    }
    
    CCryptBase::CCryptBase()
    {
    	m_fInit = false;
    	m_fIgnition = false;
    	SetClientVerEnum(1);	// default version.
    }
    
    //////////////////////////////////////////
    
    #if 0
    
    const WORD g_Packet_Lengths =	
    {
    	// @010e700 in 2.0.0 
    	// if it's == 0xFFFF then it's variable len, or >= 08000
    	// else it's fixed to what ever the value is.
    
    	0x0068,    //XCMD_Create
    	0x0005,    //
    	0x0007,    //XCMD_Walk
    	0xFFFF,    //XCMD_Talk
    	0x0002,    //
    	0x0005,    //XCMD_Attack
    	0x0005,    //XCMD_DClick
    	0x0007,    //XCMD_ItemPickupReq
    	0x000E,    //XCMD_ItemDropReq
    	0x0005,    //XCMD_Click
    	0x000B,    //
    	0x010A,    //
    	0xFFFF,    //
    	0x0003,    //
    	0xFFFF,    //
    	0x003D,    //
    	0x00D7,    // 0x10
    	0xFFFF,    //XCMD_Status
    	0xFFFF,    //XCMD_ExtCmd
    	0x000A,    //XCMD_ItemEquipReq
    	0x0006,    //XCMD_GMToolMsg
    	0x0009,    //XCMD_Follow
    	0x0001,    //
    	0xFFFF,    //
    	0xFFFF,    //
    	0xFFFF,    //
    	0xFFFF,    //XCMD_Put
    	0x0025,    //XCMD_Start
    	0xFFFF,    //XCMD_Speak
    	0x0005,    //XCMD_Remove
    	0x0004,    //
    	0x0008,    //
    	0x0013,    //XCMD_View
    	0x0008,    //XCMD_WalkCancel
    	0x0003,    //XCMD_WalkAck
    	0x001A,    //XCMD_DragAnim
    	0x0007,    //XCMD_ContOpen
    	0x0014,    //XCMD_ContAdd
    	0x0005,    //XCMD_Kick
    	0x0002,    //XCMD_DragCancel
    	0x0005,    //XCMD_ClearSquare
    	0x0001,    //XCMD_Unk29
    	0x0005,    //
    	0x0002,    //
    	0x0002,    //XCMD_DeathMenu
    	0x0011,    //
    	0x000F,    //XCMD_ItemEquip
    	0x000A,    //XCMD_Fight
    	0x0005,    // 0x30
    	0x0001,    //
    	0x0002,    //
    	0x0002,    //XCMD_Pause
    	0x000A,    //XCMD_CharStatReq
    	0x028D,    //
    	0xFFFF,    //
    	0x0008,    //
    	0x0007,    //XCMD_PathFind
    	0x0009,    //XCMD_ChangeGroup
    	0xFFFF,    //XCMD_Skill
    	0xFFFF,    //XCMD_VendorBuy
    	0xFFFF,    //XCMD_Content
    	0x0002,    //XCMD_Unk3d
    	0x0025,    //
    	0xFFFF,    //
    	0x00C9,    // 0x40
    	0xFFFF,    //
    	0xFFFF,    //
    	0x0229,    //
    	0x02C9,    //
    	0x0005,    //
    	0xFFFF,    //
    	0x000B,    //
    	0x0049,    //
    	0x005D,    //
    	0x0005,    //
    	0x0009,    //
    	0xFFFF,    //
    	0xFFFF,    //
    	0x0006,    //XCMD_LightPoint
    	0x0002,    //XCMD_Light
    	0xFFFF,    // 0x50
    	0xFFFF,    //
    	0xFFFF,    //
    	0x0002,    //XCMD_IdleWarn
    	0x000C,    //XCMD_Sound
    	0x0001,    //XCMD_ReDrawAll
    	0x000B,    //XCMD_MapEdit
    	0x006E,    //
    	0x006A,    //
    	0xFFFF,    //
    	0xFFFF,    //
    	0x0004,    //XCMD_Time
    	0x0002,    //
    	0x0049,    //XCMD_CharPlay
    	0xFFFF,    //
    	0x0031,    //
    	0x0005,    // 0x60
    	0x0009,    //
    	0x000F,    //
    	0x000D,    //
    	0x0001,    //
    	0x0004,    //XCMD_Weather
    	0xFFFF,    //XCMD_BookPage
    	0x0015,    //
    	0xFFFF,    //
    	0xFFFF,    //XCMD_Options
    	0x0003,    //
    	0x0009,    //
    	0x0013,    //XCMD_Target
    	0x0003,    //XCMD_PlayMusic
    	0x000E,    //XCMD_CharAction
    	0xFFFF,    //XCMD_SecureTrade
    	0x001C,    //XCMD_Effect
    	0xFFFF,    //XCMD_BBoard
    	0x0005,    //XCMD_War
    	0x0002,    //XCMD_Ping
    	0xFFFF,    //XCMD_VendOpenBuy
    	0x0023,    //XCMD_CharName
    	0x0010,    //XCMD_ZoneChange
    	0x0011,    //XCMD_CharMove
    	0xFFFF,    //XCMD_Char
    	0x0009,    //
    	0xFFFF,    //
    	0x0002,    //
    	0xFFFF,    //XCMD_MenuItems
    	0x000D,    //XCMD_MenuChoice
    	0x0002,    //
    	0xFFFF,    //
    	0x003E,    //XCMD_ServersReq
    	0xFFFF,    //XCMD_CharList3
    	0x0002,    //XCMD_LogBad
    	0x0027,    //XCMD_CharDelete
    	0x0045,    //
    	0x0002,    //XCMD_DeleteBad
    	0xFFFF,    //XCMD_CharList2
    	0xFFFF,    //
    	0x0042,    //XCMD_PaperDoll
    	0xFFFF,    //XCMD_CorpEquip
    	0xFFFF,    //
    	0xFFFF,    //XCMD_GumpTextDisp
    	0x000B,    //XCMD_Relay
    	0xFFFF,    //
    	0xFFFF,    //
    	0xFFFF,    //
    	0x0013,    //XCMD_MapDisplay
    	0x0041,    //XCMD_CharListReq
    	0xFFFF,    //
    	0x0063,    //XCMD_BookOpen
    	0xFFFF,    //
    	0x0009,    //XCMD_DyeVat
    	0xFFFF,    //
    	0x0002,    //XCMD_WalkForce
    	0xFFFF,    //
    	0x001A,    //XCMD_TargetMulti
    	0xFFFF,    //XCMD_Prompt
    	0x0102,    //XCMD_HelpPage
    	0x0135,    //
    	0x0033,    //
    	0xFFFF,    //XCMD_VendOpenSell
    	0xFFFF,    //XCMD_VendorSell
    	0x0003,    //XCMD_ServerSelect
    	0x0009,    //XCMD_StatChngStr
    	0x0009,    //XCMD_StatChngInt
    	0x0009,    //XCMD_StatChngDex
    	0x0095,    //XCMD_Spy
    	0xFFFF,    //XCMD_Web
    	0xFFFF,    //XCMD_Scroll
    	0x0004,    //XCMD_TipReq
    	0xFFFF,    //XCMD_ServerList
    	0xFFFF,    //XCMD_CharList
    	0x0005,    //XCMD_AttackOK
    	0xFFFF,    //XCMD_GumpInputBox
    	0xFFFF,    //XCMD_GumpText
    	0xFFFF,    //XCMD_TalkUNICODE
    	0xFFFF,    //XCMD_SpeakUNICODE
    	0x000D,    //XCMD_CharDeath
    	0xFFFF,    //XCMD_GumpDialog
    	0xFFFF,    //XCMD_GumpButton
    	0xFFFF,    //XCMD_ChatReq
    	0xFFFF,    //XCMD_ChatText
    	0xFFFF,    //XCMD_TargetItems
    	0x0040,    //XCMD_Chat
    	0x0009,    //XCMD_ToolTipReq
    	0xFFFF,    //XCMD_ToolTip
    	0xFFFF,    //XCMD_CharProfile
    	0x0003,    //XCMD_ChatEnable
    	0x0006,    //XCMD_Arrow
    	0x0009,    //XCMD_MailMsg
    	0x0003,    //XCMD_Unk_bc
    	0xFFFF,    //XCMD_ClientVersion
    	0xFFFF,    //be
    	0xFFFF,    //XCMD_ExtData
    	0x0024,
    	0xFFFF,    //XCMD_SpeakTable
    	0xFFFF,
    	0xFFFF,
    	0x0006,		//c4
    };	
    
    #else
    
    const WORD g_Packet_Lengths =	
    {
    	// @ in 3.0.0a 
    	// if it's == 0xFFFF then it's variable len, or >= 08000
    	// else it's fixed to what ever the value is.
    
    	0x0068,
    	0x0005,
    	0x0007,
    	0xFFFF,
    	0x0002,
    	0x0005,
    	0x0005,
    	0x0007,
    	0x000e,
    	0x0005,
    	0x000b,
    	0x010a,
    	0xFFFF,
    	0x0003,
    	0xFFFF,
    	0x003d,
    	0x00d7,
    	0xFFFF,
    	0xFFFF,
    	0x000a,
    	0x0006,
    	0x0009,
    	0x0001,
    	0xFFFF,
    	0xFFFF,
    	0xFFFF,
    	0xFFFF,
    	0x0025,
    	0xFFFF,
    	0x0005,
    	0x0004,
    	0x0008,
    	0x0013,
    	0x0008,
    	0x0003,
    	0x001a,
    	0x0007,
    	0x0014,
    	0x0005,
    	0x0002,
    	0x0005,
    	0x0001,
    	0x0005,
    	0x0002,
    	0x0002,
    	0x0011,
    	0x000f,
    	0x000a,
    	0x0005,
    	0x0001,
    	0x0002,
    	0x0002,
    	0x000a,
    	0x028d,
    	0xFFFF,
    	0x0008,
    	0x0007,
    	0x0009,
    	0xFFFF,
    	0xFFFF,
    	0xFFFF,
    	0x0002,
    	0x0025,
    	0xFFFF,
    	0x00c9,
    	0xFFFF,
    	0xFFFF,
    	0x0229,
    	0x02c9,
    	0x0005,
    	0xFFFF,
    	0x000b,
    	0x0049,
    	0x005d,
    	0x0005,
    	0x0009,
    	0xFFFF,
    	0xFFFF,
    	0x0006,
    	0x0002,
    	0xFFFF,
    	0xFFFF,
    	0xFFFF,
    	0x0002,
    	0x000c,
    	0x0001,
    	0x000b,
    	0x006e,
    	0x006a,
    	0xFFFF,
    	0xFFFF,
    	0x0004,
    	0x0002,
    	0x0049,
    	0xFFFF,
    	0x0031,
    	0x0005,
    	0x0009,
    	0x000f,
    	0x000d,
    	0x0001,
    	0x0004,
    	0xFFFF,
    	0x0015,
    	0xFFFF,
    	0xFFFF,
    	0x0003,
    	0x0009,
    	0x0013,
    	0x0003,
    	0x000e,
    	0xFFFF,
    	0x001c,
    	0xFFFF,
    	0x0005,
    	0x0002,
    	0xFFFF,
    	0x0023,
    	0x0010,
    	0x0011,
    	0xFFFF,
    	0x0009,
    	0xFFFF,
    	0x0002,
    	0xFFFF,
    	0x000d,
    	0x0002,
    	0xFFFF,
    	0x003e,
    	0xFFFF,
    	0x0002,
    	0x0027,
    	0x0045,
    	0x0002,
    	0xFFFF,
    	0xFFFF,
    	0x0042,
    	0xFFFF,
    	0xFFFF,
    	0xFFFF,
    	0x000b,
    	0xFFFF,
    	0xFFFF,
    	0xFFFF,
    	0x0013,
    	0x0041,
    	0xFFFF,
    	0x0063,
    	0xFFFF,
    	0x0009,
    	0xFFFF,
    	0x0002,
    	0xFFFF,
    	0x001a,
    	0xFFFF,
    	0x0102,
    	0x0135,
    	0x0033,
    	0xFFFF,
    	0xFFFF,
    	0x0003,
    	0x0009,
    	0x0009,
    	0x0009,
    	0x0095,
    	0xFFFF,
    	0xFFFF,
    	0x0004,
    	0xFFFF,
    	0xFFFF,
    	0x0005,
    	0xFFFF,
    	0xFFFF,
    	0xFFFF,
    	0xFFFF,
    	0x000d,
    	0xFFFF,
    	0xFFFF,
    	0xFFFF,
    	0xFFFF,
    	0xFFFF,
    	0x0040,
    	0x0009,
    	0xFFFF,
    	0xFFFF,
    	0x0003,
    	0x0006,
    	0x0009,
    	0x0003,
    	0xFFFF,
    	0xFFFF,
    	0xFFFF,
    	0x0024,
    	0xFFFF,
    	0xFFFF,
    	0xFFFF,
    	0x0006,
    	
    	//0x00cb,
    	//0x0001,
    	//0x0031,
    	//0x0002,
    	//0x0006,
    	//0x0006,
    	//0x0007,
    	//0xFFFF,
    };
    	
    #endif
    	
    /////////////////////////
    // -CCompressTree
    	
    void CCompressTree::CompressXOR( BYTE * pData, int iLen, DWORD & dwIndex )	// static 
    {
    	// NOTE: It seems this needs to change based on the m_seed.
    	// Only used in Ver 2.0.4 and above.
    
    	// This table generated basec on DWORD id passed at start. 127.0.0.1
    	static const BYTE sm_bData = { 0x05, 0x92, 0x66, 0x23, 0x67, 0x14, 0xE3, 0x62, 0xDC, 0x60, 0x8C, 0xD6, 0xFE, 0x7C, 0x25, 0x69 };
    
    	// @ 04264A5 in 2.0.4
    	DWORD dwTmpIndex = dwIndex;
    	for ( int i=0; i<iLen; i++ )
    	{
    		pData ^= sm_bData;
    		dwTmpIndex++;
    	}
    	dwIndex = dwTmpIndex;
    }
    
    const WORD CCompressTree::sm_xCompress_Base =	// static
    {
    	// The "golden" key for (0.0.0.0)
    	// lowest 4 bits is the length. other source uses 2 int's per WORD here.
    	// @ 014b389 in 2.0.3
    	// @ 010a3d8 in 2.0.4
    	0x0002, 0x01f5, 0x0226, 0x0347, 0x0757, 0x0286, 0x03b6, 0x0327,
    	0x0e08, 0x0628, 0x0567, 0x0798, 0x19d9, 0x0978, 0x02a6, 0x0577,
    	0x0718, 0x05b8, 0x1cc9, 0x0a78, 0x0257, 0x04f7, 0x0668, 0x07d8,
    	0x1919, 0x1ce9, 0x03f7, 0x0909, 0x0598, 0x07b8, 0x0918, 0x0c68,
    	0x02d6, 0x1869, 0x06f8, 0x0939, 0x1cca, 0x05a8, 0x1aea, 0x1c0a,
    	0x1489, 0x14a9, 0x0829, 0x19fa, 0x1719, 0x1209, 0x0e79, 0x1f3a,
    	0x14b9, 0x1009, 0x1909, 0x0136, 0x1619, 0x1259, 0x1339, 0x1959,
    	0x1739, 0x1ca9, 0x0869, 0x1e99, 0x0db9, 0x1ec9, 0x08b9, 0x0859,
    	0x00a5, 0x0968, 0x09c8, 0x1c39, 0x19c9, 0x08f9, 0x18f9, 0x0919,
    	0x0879, 0x0c69, 0x1779, 0x0899, 0x0d69, 0x08c9, 0x1ee9, 0x1eb9,
    	0x0849, 0x1649, 0x1759, 0x1cd9, 0x05e8, 0x0889, 0x12b9, 0x1729,
    	0x10a9, 0x08d9, 0x13a9, 0x11c9, 0x1e1a, 0x1e0a, 0x1879, 0x1dca,
    	0x1dfa, 0x0747, 0x19f9, 0x08d8, 0x0e48, 0x0797, 0x0ea9, 0x0e19,
    	0x0408, 0x0417, 0x10b9, 0x0b09, 0x06a8, 0x0c18, 0x0717, 0x0787,
    	0x0b18, 0x14c9, 0x0437, 0x0768, 0x0667, 0x04d7, 0x08a9, 0x02f6,
    	0x0c98, 0x0ce9, 0x1499, 0x1609, 0x1baa, 0x19ea, 0x39fa, 0x0e59,
    	0x1949, 0x1849, 0x1269, 0x0307, 0x06c8, 0x1219, 0x1e89, 0x1c1a,
    	0x11da, 0x163a, 0x385a, 0x3dba, 0x17da, 0x106a, 0x397a, 0x24ea,
    	0x02e7, 0x0988, 0x33ca, 0x32ea, 0x1e9a, 0x0bf9, 0x3dfa, 0x1dda,
    	0x32da, 0x2eda, 0x30ba, 0x107a, 0x2e8a, 0x3dea, 0x125a, 0x1e8a,
    	0x0e99, 0x1cda, 0x1b5a, 0x1659, 0x232a, 0x2e1a, 0x3aeb, 0x3c6b,
    	0x3e2b, 0x205a, 0x29aa, 0x248a, 0x2cda, 0x23ba, 0x3c5b, 0x251a,
    	0x2e9a, 0x252a, 0x1ea9, 0x3a0b, 0x391b, 0x23ca, 0x392b, 0x3d5b,
    	0x233a, 0x2cca, 0x390b, 0x1bba, 0x3a1b, 0x3c4b, 0x211a, 0x203a,
    	0x12a9, 0x231a, 0x3e0b, 0x29ba, 0x3d7b, 0x202a, 0x3adb, 0x213a,
    	0x253a, 0x32ca, 0x23da, 0x23fa, 0x32fa, 0x11ca, 0x384a, 0x31ca,
    	0x17ca, 0x30aa, 0x2e0a, 0x276a, 0x250a, 0x3e3b, 0x396a, 0x18fa,
    	0x204a, 0x206a, 0x230a, 0x265a, 0x212a, 0x23ea, 0x3acb, 0x393b,
    	0x3e1b, 0x1dea, 0x3d6b, 0x31da, 0x3e5b, 0x3e4b, 0x207a, 0x3c7b,
    	0x277a, 0x3d4b, 0x0c08, 0x162a, 0x3daa, 0x124a, 0x1b4a, 0x264a,
    	0x33da, 0x1d1a, 0x1afa, 0x39ea, 0x24fa, 0x373b, 0x249a, 0x372b,
    	0x1679, 0x210a, 0x23aa, 0x1b8a, 0x3afb, 0x18ea, 0x2eca, 0x0627,
    	0x00d4 // terminator
    } ;
    
    bool CCompressTree::AddBranch( int Value, WORD wCode, int iBits )
    {
    	// adds a hex value related to it's binary compression code to the decompression tree.
    
        if ( ! iBits )
    		return false;
    
        CCompressBranch *pCur=&m_Root;
    	WORD wMask = 1 << (iBits-1);
    
        // loop through each bit in the bitfield
    	for ( ;wMask; wMask >>= 1 )
        {
    		// if a compression value read in conflicts with an existing
    		// compression value, the table is invalid.  Exit program.
    		if ( pCur->m_Value >= 0 )
    		{
    			// if you get here, this means that a new compression value is mutually exclusive
    			// with another value.  I.E. both 1101 and 110100 cannot exist in a tree because
    			// any decompression routine will stop as soon as it reads 1101
    			ASSERT(0);
    			return false;
    		}
            if ( wCode & wMask )
    		{	// if the bit is a 1
    			// make a new branch if we need to
    			if ( pCur->m_pOne == NULL )
    			{
    				pCur->m_pOne = new CCompressBranch;
    				ASSERT(pCur->m_pOne);
    			}
    			pCur = pCur->m_pOne;	// traverse to the (possibly new) branch
    		}
    		else
    		{	// if the bit is a 0
    			// make a new branch if we need to
    			if ( pCur->m_pZero == NULL )
    			{
    				pCur->m_pZero = new CCompressBranch;
    				ASSERT(pCur->m_pZero);
    			}
    			pCur = pCur->m_pZero;	// traverse to the (possibly new) branch
    		}
        }
    	// if the end of this bitvalue is within an existing bitvalue's path
    	// the table is in error.  I.E. 111 is within 11101's path
        if ( pCur->m_pOne != NULL || pCur->m_pZero != NULL )
        {
    		ASSERT(0);
    		return( false );
        }
    	// an entry already exists with this binary value
        else if (pCur->m_Value>=0)
        {
    		if (((BYTE)pCur->m_Value)!=Value)
    		{
    			// if they are not the same, the table is in error
    			ASSERT(0);
    			return( false );
    		}
        }
    	else
    	{
    		// set the read in hex value at this compression location
    		pCur->m_Value=Value;
    	}
    	return( true );
    }
    
    int CCompressTree::Decode( BYTE * pOutput, const BYTE * pInput, int inpsize )
    {
    	// RETURN:
    	//  valid output length. m_iDecodeIndex may be larger.
    	//  0 = not big enough for a logical packet.
    
    	ASSERT( m_Root.IsLoaded() );
    
    	int outIndex = m_iDecodeIndex;
    	int outValidIndex = 0;
    
    	for ( int inpIndex=0; inpIndex<inpsize; inpIndex++ )
    	{
    		BYTE ch = pInput;
    		BYTE Mask = 0x80;
    		for ( int i=0;i<8;i++)
    		{
    			if ( ch & Mask )
    				m_pDecodeCur=m_pDecodeCur->m_pOne;
    			else
    				m_pDecodeCur=m_pDecodeCur->m_pZero;
    			if ( m_pDecodeCur == NULL )
    			{
    				// This data stream is JUNK!
    				Reset();
    				return( -1 );	// this should not happen !
    			}
    
    			if (m_pDecodeCur->m_Value>=0)
    			{
    				if ( m_pDecodeCur->m_Value > 0xFF )
    				{
    					// End of a logical packet. (process it)
    					Reset();
    					outValidIndex = outIndex;
    					break;		// rest of byte should be padded out. so skip it.
    				}
    
    				pOutput = m_pDecodeCur->m_Value;
    				m_pDecodeCur = &m_Root;
    			}
    			Mask >>= 1;
    		}
    	}
    
    	m_iDecodeIndex = outIndex - outValidIndex;	// continue from here next time.
    	return( outValidIndex );	// not done yet.
    }
    
    bool CCompressTree::Load()
    {
    	if ( ! IsLoaded())
    	{
    		Reset();
    		for ( int i=0; i<COMPRESS_TREE_SIZE; i++ )
    		{
    			WORD wCode = sm_xCompress_Base;
    			int iBits = wCode & 0xF;
    			wCode >>= 4;
    			if ( ! AddBranch( i, wCode, iBits ))
    				return( false );
    		}
    	}
    	return( true );
    }
    
    int CCompressTree::Encode( BYTE * pOutput, const BYTE * pInput, int inplen ) // static
    {
    	int iLen=0;
    	int bitidx=0;	// Offset in output byte (xOutVal)
    	BYTE xOutVal=0;	// Don't bother to init this. It will just roll off all junk anyhow.
    
    	for ( int i=0; i <= inplen; i++ )
    	{
    		WORD value = sm_xCompress_Base ];
    		int nBits = value & 0xF;
    		value >>= 4;
    		while ( nBits-- )
    		{
    			xOutVal <<= 1;
    			xOutVal |= (value >> nBits) & 0x1;
    			if ( ++bitidx == 8)
    			{
    				bitidx = 0;
    				pOutput = xOutVal;
    			}
    		}
    	}
    	if (bitidx)	// flush odd bits.
    	{
    		pOutput = xOutVal << (8-bitidx);
    	}
    
    	return( iLen );
    }