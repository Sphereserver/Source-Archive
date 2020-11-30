//
// GrayProto.h
// Copyright Menace Software (www.menasoft.com).
//
// Protocol formats.
// Define all the data passed from Client to Server.

#ifndef _INC_GRAYPROTO_H
#define _INC_GRAYPROTO_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//---------------------------PROTOCOL DEFS---------------------------

#ifndef GRAY_CLIENT_VER
#define GRAY_CLIENT_VER 20000 // 1.25.37 = 12537 = default version.
#endif

// GRAY_CLIENT_VER == 20000	// 4/19/2000 by UOX team
#define CLIKEY_20000_HI 0x2d13a5fd
#define CLIKEY_20000_LO 0xa39d527f
// GRAY_CLIENT_VER == 12604	// 1/27/2000 by Westy
#define CLIKEY_12604_HI 0x32750719
#define CLIKEY_12604_LO 0x0a2d100b
// GRAY_CLIENT_VER == 12603	// 1/18/2000 by Westy and Beosil (beosil@mcb.at)
#define CLIKEY_12603_HI 0x323d3569
#define CLIKEY_12603_LO 0x0a095c0b
// GRAY_CLIENT_VER == 12602	// 11/23/99 by Westy and Beosil (beosil@mcb.at)
#define CLIKEY_12602_HI 0x32e52f79
#define CLIKEY_12602_LO 0x0a65200b
// GRAY_CLIENT_VER == 12601	// 09/09/99 by Westy and Beosil (beosil@mcb.at)
#define CLIKEY_12601_HI 0x32ad2549
#define CLIKEY_12601_LO 0x0a417c0b
// GRAY_CLIENT_VER == 12600	// 08/35/99 by Westy and Beosil (beosil@mcb.at)
#define CLIKEY_12600_HI 0x32952759
#define CLIKEY_12600_LO 0x0a5d500b
// GRAY_CLIENT_VER == 12537	// 03/18/99 by Westy and Beosil (beosil@mcb.at)
#define CLIKEY_12537_HI 0x378757DC
#define CLIKEY_12537_LO 0x0595DCC6
// GRAY_CLIENT_VER == 12536	// 12/1/98 by Beosil (beosil@mcb.at)
#define CLIKEY_12536_HI1 0x387fc5cc
#define CLIKEY_12536_HI2 0x35ce9581
#define CLIKEY_12536_HI3 0x07afcc37
#define CLIKEY_12536_LO1 0x021510c6
#define CLIKEY_12536_LO2 0x4c3a1353
#define CLIKEY_12536_LO3 0x16ef783f
// GRAY_CLIENT_VER == 12535	// Released on the T2A CD.
#define CLIKEY_12535_HI 0x383477BC
#define CLIKEY_12535_LO 0x02345CC6
// GRAY_CLIENT_VER == 12534
#define CLIKEY_12534_HI 0x38ECEDAC	
#define CLIKEY_12534_LO 0x025720C6
// GRAY_CLIENT_VER == 12533
#define CLIKEY_12533_HI 0x38A5679C
#define CLIKEY_12533_LO 0x02767CC6
// GRAY_CLIENT_VER == 12532
#define CLIKEY_12532_HI 0x389DE58C
#define CLIKEY_12532_LO 0x026950C6
// GRAY_CLIENT_VER == 12531
#define CLIKEY_12531_HI 0x395A647C
#define CLIKEY_12531_LO 0x0297BCC6

// All these structures must be BYTE packed.
#if defined _WIN32 && (!__MINGW32__)
// Microsoft dependant pragma
#pragma pack(1)	
#define PACK_NEEDED
#else
// GCC based compiler you can add:
#define PACK_NEEDED __attribute__ ((packed))
#endif

// Pack/unpack in network order.

struct NWORD
{
	// Deal with the stupid Little Endian / Big Endian crap just once.
	// On little endian machines this code would do nothing.
	WORD m_val;
	operator WORD () const
	{
		return( ntohs( m_val ));
	}
	NWORD & operator = ( WORD val )
	{
		m_val = htons( val );
		return( * this );
	}

#define PACKWORD(p,w)	(p)[0]=HIBYTE(w);(p)[1]=LOBYTE(w)
#define UNPACKWORD(p)	MAKEWORD((p)[1],(p)[0])	// low,high

} PACK_NEEDED;
struct NDWORD
{
	DWORD m_val;
	operator DWORD () const
	{
		return( ntohl( m_val ));
	}
	NDWORD & operator = ( DWORD val )
	{
		m_val = htonl( val );
		return( * this );
	}

#define PACKDWORD(p,d)	(p)[0]=((d)>>24)&0xFF;(p)[1]=((d)>>16)&0xFF;(p)[2]=HIBYTE(d);(p)[3]=LOBYTE(d)
#define UNPACKDWORD(p)	MAKEDWORD( MAKEWORD((p)[3],(p)[2]), MAKEWORD((p)[1],(p)[0]))

} PACK_NEEDED;

#define	NCHAR	NWORD			// a UNICODE text char on the network.

extern int CvtSystemToNUNICODE( NCHAR * pOut, const TCHAR * pInp, int iSizeInWideChars );
extern int CvtNUNICODEToSystem( TCHAR * pOut, const NCHAR * pInp, int iSize );

enum XCMD_TYPE	// XCMD_* messages are unique in both directions.
{
	XCMD_Create			= 0x00,
	//
	XCMD_Walk			= 0x02,
	XCMD_Talk			= 0x03,
	//
	XCMD_Attack			= 0x05,
	XCMD_DClick			= 0x06,
	XCMD_ItemPickupReq	= 0x07,
	XCMD_ItemDropReq	= 0x08,
	XCMD_Click			= 0x09,
	//
	//
	//
	//
	//
	//
	// 0x10
	XCMD_Status			= 0x11,
	XCMD_ExtCmd			= 0x12,	// text command
	XCMD_ItemEquipReq	= 0x13,
	// XCMD_GMToolMsg		= 0x14,	// not really used here.
	XCMD_Follow			= 0x15,
	//
	//
	//
	//
	XCMD_Put			= 0x1a,
	XCMD_Start			= 0x1b,
	XCMD_Speak			= 0x1c,
	XCMD_Remove			= 0x1d,
	//
	//
	XCMD_View			= 0x20,
	XCMD_WalkCancel		= 0x21,
	XCMD_WalkAck		= 0x22,
	XCMD_DragAnim		= 0x23,
	XCMD_ContOpen		= 0x24,
	XCMD_ContAdd		= 0x25,
	XCMD_Kick			= 0x26,
	XCMD_DragCancel		= 0x27,
	XCMD_ClearSquare	= 0x28,
	XCMD_Unk29			= 0x29,
	//
	//
	XCMD_DeathMenu		= 0x2c,
	//
	XCMD_ItemEquip		= 0x2e,
	XCMD_Fight			= 0x2f,
	// 0x30
	//
	//
	XCMD_Pause			= 0x33,
	XCMD_CharStatReq	= 0x34,
	//
	//
	//
	XCMD_PathFind		= 0x38,
	XCMD_ChangeGroup	= 0x39,
	XCMD_Skill			= 0x3a,
	XCMD_VendorBuy		= 0x3b,
	XCMD_Content		= 0x3c,
	XCMD_Unk3d			= 0x3d,
	//
	//
	// 0x40
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	XCMD_LightPoint		= 0x4e,
	XCMD_Light			= 0x4f,
	// 0x50
	//
	//
	XCMD_IdleWarn		= 0x53,
	XCMD_Sound			= 0x54,
	XCMD_ReDrawAll		= 0x55,
	XCMD_MapEdit		= 0x56,
	//
	//
	//
	//
	XCMD_Time			= 0x5b,
	//
	XCMD_CharPlay		= 0x5d,
	//
	//
	// 0x60
	//
	//
	//
	//
	XCMD_Weather		= 0x65,
	XCMD_BookPage		= 0x66,
	//
	//
	XCMD_Options		= 0x69,
	//
	//
	XCMD_Target			= 0x6c,
	XCMD_PlayMusic		= 0x6d,
	XCMD_CharAction		= 0x6e,
	XCMD_SecureTrade	= 0x6f,
	XCMD_Effect			= 0x70,
	XCMD_BBoard			= 0x71,
	XCMD_War			= 0x72,
	XCMD_Ping			= 0x73,
	XCMD_VendOpenBuy	= 0x74,
	XCMD_CharName		= 0x75,
	XCMD_ZoneChange		= 0x76,
	XCMD_CharMove		= 0x77,
	XCMD_Char			= 0x78,
	//
	//
	//
	XCMD_MenuItems		= 0x7c,
	XCMD_MenuChoice		= 0x7d,
	//
	//
	XCMD_ServersReq		= 0x80,
	XCMD_CharList3		= 0x81,
	XCMD_LogBad			= 0x82,
	XCMD_CharDelete		= 0x83,
	//
	XCMD_DeleteBad		= 0x85,
	XCMD_CharList2		= 0x86,
	//
	XCMD_PaperDoll		= 0x88,
	XCMD_CorpEquip		= 0x89,
	//
	XCMD_GumpTextDisp	= 0x8b,
	XCMD_Relay			= 0x8c,
	//
	//
	//
	XCMD_MapDisplay		= 0x90,
	XCMD_CharListReq	= 0x91,
	XCMD_EditMultiMul	= 0x92,
	XCMD_BookOpen		= 0x93,
	XCMD_EditSkillsMul	= 0x94,
	XCMD_DyeVat			= 0x95,
	//
	XCMD_WalkForce		= 0x97,
	//
	XCMD_TargetMulti	= 0x99,
	XCMD_Prompt			= 0x9a,
	XCMD_HelpPage		= 0x9b,
	//
	//
	XCMD_VendOpenSell	= 0x9e,
	XCMD_VendorSell		= 0x9f,
	XCMD_ServerSelect	= 0xa0,
	XCMD_StatChngStr	= 0xa1,
	XCMD_StatChngInt	= 0xa2,
	XCMD_StatChngDex	= 0xa3,
	XCMD_Spy			= 0xa4,
	XCMD_Web			= 0xa5,
	XCMD_Scroll			= 0xa6,
	XCMD_TipReq			= 0xa7,
	XCMD_ServerList		= 0xa8,
	XCMD_CharList		= 0xa9,
	XCMD_AttackOK		= 0xaa,
	XCMD_GumpInputBox	= 0xab,
	XCMD_GumpText		= 0xac,
	XCMD_TalkUNICODE	= 0xad,
	XCMD_SpeakUNICODE	= 0xae,
	XCMD_CharDeath		= 0xaf,
	XCMD_GumpDialog		= 0xb0,
	XCMD_GumpButton		= 0xb1,
	XCMD_ChatReq		= 0xb2,
	XCMD_ChatText		= 0xb3,
	XCMD_TargetItems	= 0xb4,
	XCMD_Chat			= 0xb5,
	XCMD_ToolTipReq		= 0xb6,
	XCMD_ToolTip		= 0xb7,
	XCMD_CharProfile	= 0xb8,
	XCMD_ChatEnable		= 0xb9,
	XCMD_Arrow			= 0xba,
	XCMD_MailMsg		= 0xbb,
	XCMD_Unk_bc			= 0xbc,
	XCMD_ClientVersion	= 0xbd,
	//
	XCMD_ExtData		= 0xbf,
	//
	XCMD_SpeakTable		= 0xc1,
	//
	//
	//
	XCMD_QTY			= 0xc5,
};

extern const WORD Packet_Lengths_20000[XCMD_QTY]; // from client 2.0.0 @offset 010e700

enum EXTDATA_TYPE
{
	EXTDATA_WalkCode_Prime = 1,		// send to client
	EXTDATA_WalkCode_Add = 2,		// send to client
	EXTDATA_Party_UnkFromCli5 = 5,	// len=8
	EXTDATA_Party_Add = 6,	// len=5 data for total of 10. Client wants to add to the party.
	EXTDATA_Arrow_Click = 7,	// Client clicked on the quest arrow.
	EXTDATA_Party_UnkFromCli11 = 11,	// len=3
};

struct CEventCharDef
{
#define MAX_NAME_SIZE	30		// imposed by client for protocol
	char m_name[MAX_NAME_SIZE];
	char m_pass[MAX_NAME_SIZE];
};

struct CEvent	// event buffer from client to server..
{
#define MAX_BUFFER			15360	// Buffer Size (For socket operations)
#define MAX_ITEMS_CONT		256		// Max items in a container. (arbitrary)
#define MAX_MENU_ITEMS		64		// number of items in a menu. (arbitrary)
#define MAX_CHARS_PER_ACCT	5
#define MAX_TALK_BUFFER		1024

	// Some messages are bydirectional.

	union
	{
		BYTE m_Raw[ MAX_BUFFER ];

		DWORD m_CryptHeader;	// This may just be a crypt header from the client.

		struct 
		{
			BYTE m_Cmd;		// XCMD_TYPE, 0 = ?
			BYTE m_Arg[1];	// unknown size.
		} Default;	// default unknown type.

		struct	// XCMD_Create, size = 100	// create a new char
		{
			BYTE m_Cmd;		// 0=0
			BYTE m_unk1[9];
			char m_name[MAX_NAME_SIZE];		// 10
			char m_pass[MAX_NAME_SIZE];		// 40

			BYTE m_sex;		// 70, 0 = male
			BYTE m_str;		// 71
			BYTE m_dex;		// 72
			BYTE m_int;		// 73

			BYTE m_skill1;
			BYTE m_val1;
			BYTE m_skill2;
			BYTE m_val2;
			BYTE m_skill3;
			BYTE m_val3;

			NWORD m_color;	// 0x50 // COLOR_TYPE
			NWORD m_hairid;
			NWORD m_haircolor;
			NWORD m_beardid;
			NWORD m_beardcolor;	// 0x58
			NWORD m_startloc;	// 0x5B
			NWORD m_unk5c;
			NWORD m_slot;
			BYTE m_clientip[4];
		} Create_v25;

		struct	// XCMD_Create, size = 100	// create a new char
		{
			BYTE m_Cmd;		// 0=0
			BYTE m_unk1[9];
			char m_name[MAX_NAME_SIZE];		// 10
			char m_pass[MAX_NAME_SIZE];		// 40

			BYTE m_sex;		// 70, 0 = male
			BYTE m_str;		// 71
			BYTE m_dex;		// 72
			BYTE m_int;		// 73

			BYTE m_skill1;
			BYTE m_val1;
			BYTE m_skill2;
			BYTE m_val2;
			BYTE m_skill3;
			BYTE m_val3;

			NWORD m_color;	// 0x50 // COLOR_TYPE
			NWORD m_hairid;
			NWORD m_haircolor;
			NWORD m_beardid;
			NWORD m_beardcolor;	// 0x58
			BYTE m_unk2;
			BYTE m_startloc;
			BYTE m_unk3;
			BYTE m_unk4;
			BYTE m_unk5;
			BYTE m_slot;
			BYTE m_clientip[4];
			NWORD m_shirtcolor;
			NWORD m_pantscolor;
		} Create;

		struct	// size = 3
		{
			BYTE m_Cmd;		// 0 = 0x02
			BYTE m_dir;		// 1 = DIR_TYPE (| 0x80 = running)
			BYTE m_count; 	// 2 = just a count that goes up as we walk. (handshake)
		} Walk_v25;

		struct	// size = 7
		{
			BYTE m_Cmd;		// 0 = 0x02
			BYTE m_dir;		// 1 = DIR_TYPE (| 0x80 = running)
			BYTE m_count; 	// 2 = just a count that goes up as we walk. (handshake)
			NDWORD m_cryptcode;
		} Walk_v26;

		struct	// size = >3	// user typed in text.
		{
			BYTE m_Cmd;		// 0 = 0x03
			NWORD m_len;	// 1,2=length of packet
			BYTE m_mode;	// 3=mode(9=yell) TALKMODE_TYPE
			NWORD m_color;
			NWORD m_font;	// 6,7 = FONT_TYPE
			char m_text[1];	// 8=var size
		} Talk;

		struct	// size = 5
		{
			BYTE m_Cmd;	// 0 = 0x05, 0x06 or 0x09
			NDWORD m_UID;
		} Click;	// Attack, DClick, Click 

		struct // size = 7 = pick up an item
		{
			BYTE m_Cmd;			// 0 = 0x07
			NDWORD m_UID;		// 1-4
			NWORD m_amount;
		} ItemPickupReq;

		struct	// size = 14 = drop item on ground or in container.
		{
			BYTE m_Cmd;			// 0 = 0x08
			NDWORD m_UID;		// 1-4 = object being dropped.
			NWORD m_x;		// 5,6 = 255 = invalid
			NWORD m_y;		// 7,8
			BYTE m_z;			// 9
			NDWORD m_UIDCont;	// 10 = dropped on this object. 0xFFFFFFFF = no object
		} ItemDropReq;

		struct	// size > 3
		{
			BYTE m_Cmd;		// 0 = 0x12
			NWORD m_len;	// 1-2 = len
			BYTE m_type;	// 3 = 0xc7=anim, 0x24=skill...
			char m_name[1];	// 4=var size string
		} ExtCmd;

		struct	// size = 10 = item dropped on paper doll.
		{
			BYTE m_Cmd;		// 0 = 0x13
			NDWORD m_UID;	// 1-4	item UID.
			BYTE m_layer;	// LAYER_TYPE
			NDWORD m_UIDChar;
		} ItemEquipReq;

		struct // size = ? reserved GM tool message.
		{
			BYTE m_Cmd;		// 0 = 0x11

		} GMToolMsg;

		struct // size = 3	// WalkAck gone bad = request a resync
		{
			BYTE m_Cmd;		// 0 = 0x22
			BYTE m_unk1[2];
		} WalkAck;

		struct // size = 7(m_mode==0) or 2(m_mode!=0)  // Manifest ghost (War Mode) or auto res ?
		{
			BYTE m_Cmd;		// 0 = 0x2c
			BYTE m_mode;	// 1 = 0=manifest or unmanifest, 1=res w/penalties, 2=play as a ghost
			BYTE m_unk2;	// 2 = 72 or 73
			BYTE m_manifest;// 3 = manifest or not. = war mode.
			BYTE m_unk4[3]; // 4 = unk = 00 32 00
		} DeathMenu;

		struct	// size = 10	// Client requests stats.
		{
			BYTE m_Cmd;		// 0 =0x34
			NDWORD m_edededed;	// 1-4 = 0xedededed
			BYTE m_type;	// 5 = 4 = Basic Stats (Packet 0x11 Response)  5 = Request Skills (Packet 0x3A Response) 
			NDWORD m_UID;	// 6 = character UID for status
		} CharStatReq;

		struct // size = ??? = Get a skill lock (or multiple)
		{
			BYTE m_Cmd;		// 0= 0x3A
			NWORD m_len;	// 1= varies
			struct
			{
				NWORD m_index;	// skill index
				BYTE m_lock;	// SKILLLOCK_TYPE current lock mode (0 = none (up), 1 = down, 2 = locked)
			} skills[1];
		} Skill;

		struct // size = variable // Buy item from vendor.
		{
			BYTE m_Cmd;			// 0 =0x3b
			NWORD m_len;		// 1-2=
			NDWORD m_UIDVendor;	// 3-6=
			BYTE m_flag;	// 0x00 = no items following, 0x02 - items following 
			struct 
			{
				BYTE m_layer; // (0x1A) 
				NDWORD m_UID; // itemID (from 3C packet) 
				NWORD m_amount; //  # bought 
			} items[1];
		} VendorBuy;

		struct // size = 7	// ??? No idea
		{
			BYTE m_Cmd;		// 0 =0x3d
			BYTE m_unk1[6];
		} Unk3d;

		struct	// size = 11	// plot course on map.
		{
			BYTE m_Cmd;		// 0 = 0x56
			NDWORD m_UID;  // uid of the map
			BYTE m_action;	// 1, 1: add pin, 5: delete pin, 6: toggle edit/noedit or cancel while in edit
			BYTE m_pin;
			NWORD m_pin_x;
			NWORD m_pin_y;
		} MapEdit;

		struct	// size = 0x49 = 73	// play this slot char
		{
			BYTE m_Cmd;			// 0 = 0x5D
			NDWORD m_edededed;	// 1-4 = ed ed ed ed
			char m_name[MAX_NAME_SIZE];
			char m_pass[MAX_NAME_SIZE];
			NDWORD m_slot;		// 0x44 = slot
			BYTE m_clientip[4];	// = 18 80 ea 15
		} CharPlay;

		struct	// size > 3 // user is changing a book page.
		{
			BYTE m_Cmd;		// 0 = 0x66
			NWORD m_len;	// 1-2
			NDWORD m_UID;	// 3-6 = the book
			NWORD m_pages;	// 7-8 = 0x0001 = # of pages here
			// repeat these fields for multiple pages.
			struct
			{
				NWORD m_pagenum;	// 9-10=page number  (1 based)
				NWORD m_lines;	// 11
				char m_text[1];
			} m_page[1];
		} BookPage;

		struct	// size = var // Client text color changed but it does not tell us to what !!
		{	
			BYTE m_Cmd;		// 0=0x69
			NWORD m_len;
			NWORD m_index;
		} Options;

		struct	// size = 19
		{
			BYTE m_Cmd;		// 0 = 0x6C
			BYTE m_fAllowGround;	// 1 = 0=select object, 1=x,y,z
			NDWORD m_code;	// 2-5 = we sent this at target setup.
			BYTE m_unk6;		// 6= 0
			
			NDWORD m_UID;	// 7-10	= serial number of the target.
			NWORD m_x;		// 11,12
			NWORD m_y;		// 13,14
			BYTE m_unk2;	// 15 = fd ?
			BYTE m_z;		// 16
			NWORD m_id;		// 17-18 = static id of tile
		} Target;

		struct // size = var // Secure trading
		{
			BYTE m_Cmd;		// 0=0x6F
			NWORD m_len;	// 1-2 = len
			BYTE m_action;	// 3 = trade action
			NDWORD m_UID;	// 4-7 = uid 
			NDWORD m_UID1;	// 8-11
		} SecureTrade;

		struct // size = var (0x0c)	// Bulletin request
		{
			BYTE m_Cmd;		// 0=0x71
			NWORD m_len;	// 1-2 = len = 0x0c
			BYTE m_flag;	// 3= 0=board name, 5=new message, 4=associate message, 1=message header, 2=message body
			NDWORD m_UID;	// 4-7 = UID for the bboard.
			NDWORD m_UIDMsg; // 8- = name or links data.
			BYTE m_data[1];	// submit new message data here.
		} BBoard;

		struct // size = 5	// Client wants to go into war mode.
		{
			BYTE m_Cmd;		// 0 = 0x72
			BYTE m_warmode;	// 1 = 0 or 1 
			BYTE m_unk2[3];	// 2 = 00 32 00
		} War;

		struct // size = 2	// ping goes both ways.
		{
			BYTE m_Cmd;		// 0 = 0x73
			BYTE m_unk1;	// 1 = ?
		} Ping;

		struct // size = 35	// set the name for this NPC (pet) char
		{
			BYTE m_Cmd;		// 0=0x75
			NDWORD m_UID;
			char m_name[ MAX_NAME_SIZE ];
		} CharName;

		struct	// size = 13	// choice off a menu
		{
			BYTE m_Cmd;		// 0 = 0x7d
			NDWORD m_UID;	// 1 = char id from 0x7c message
			NWORD m_menuid;	// 5,6
			NWORD m_select;	// 7,8	= what item selected from menu. 1 based ?
			BYTE m_unk9[4];
		} MenuChoice;

#define MAX_ACCOUNT_NAME_SIZE MAX_NAME_SIZE

		struct // size = 62		// first login to req listing the servers.
		{
			BYTE m_Cmd;	// 0 = 0x80 = XCMD_ServersReq
			char m_name[ MAX_ACCOUNT_NAME_SIZE ];
			char m_password[ MAX_NAME_SIZE ];
			BYTE m_unk;	// 61 = ff
		} ServersReq;

		struct	// size = 39  // delete the char in this slot. 
		{
			BYTE m_Cmd;		// 0 = 0x83
			BYTE m_password[MAX_NAME_SIZE];
			NDWORD m_slot;	// 0x22
			BYTE m_clientip[4];
		} CharDelete;

		struct	// size = 65	// request to list the chars I can play.
		{
			BYTE m_Cmd;		  // 0 = 0x91
			NDWORD m_Account; // 1-4 = account id from Relay message to log server.
			char m_account[MAX_ACCOUNT_NAME_SIZE];	// This is corrupted or encrypted seperatly ?
			char m_password[MAX_NAME_SIZE];
		} CharListReq;

		struct // size = 98 // Get a change to the book title.
		{
			BYTE m_Cmd;			// 0 = 0x93
			NDWORD m_UID;		// 1-4 = book
			BYTE m_writable;	// 5 = 0 = non writable, 1 = writable. (NOT USED HERE)
			NWORD m_pages;		// 6-7 = number of pages. (NOT USED HERE)
			char m_title[ 2 * MAX_NAME_SIZE ];
			char m_author[ MAX_NAME_SIZE ];
		} BookOpen_v25;

		struct // size = 98 // Get a change to the book title.
		{
			BYTE m_Cmd;			// 0 = 0x93
			NDWORD m_UID;		// 1-4 = book
			BYTE m_writable;	// 5 = 0 = non writable, 1 = writable. (NOT USED HERE)
			BYTE m_NEWunk1;
			NWORD m_pages;		// 6-7 = number of pages. (NOT USED HERE)
			char m_title[ 2 * MAX_NAME_SIZE ];
			char m_author[ MAX_NAME_SIZE ];
		} BookOpen_v26;

		struct	// size = 9		// select color from dye chart.
		{
			BYTE m_Cmd;		// 0x95
			NDWORD m_UID;	// 1-4
			NWORD m_unk5;
			NWORD m_color;	// 7,8
		} DyeVat;

		struct // size = 16+var = console Prompt response.
		{
			BYTE m_Cmd;		// 0x9a
			NWORD m_len;
			BYTE m_unk3[12];
			char m_text[1];	// null terminated text.
		} Prompt;

		struct // size = 0x102	// GM page = request for help.
		{
			BYTE m_Cmd;		// 0 = 0x9b
			BYTE m_unk1[0x101];
		} HelpPage;

		struct // size = var // Sell stuff to vendor
		{
			BYTE m_Cmd;		// 0=0x9F
			NWORD m_len;	// 1-2
			NDWORD m_UIDVendor;	// 3-6
			NWORD m_count;		// 7-8
			struct
			{
				NDWORD m_UID;
				NWORD m_amount;
			} items[1];
		} VendorSell;

		struct	// size = 3;	// relay me to the server i choose.
		{
			BYTE m_Cmd;		// 0 = 0xa0
			NWORD m_select;	// 2 = selection this server number
		} ServerSelect;

		struct	// sizeof = 149 // not sure what this is for.
		{
			BYTE m_Cmd;		// 0=0xA4
			BYTE m_unk1[ 148 ];	// Just junk about your computers equip.
		} Spy;

		struct // size = 5	// scroll close
		{
			BYTE m_Cmd;			// 0=0xA6
			NDWORD m_scrollID;	// 1= got this from Scroll setup.
		} Scroll;

		struct	// size = 4 // request for tip
		{
			BYTE m_Cmd;		// 0=0xA7
			NWORD m_index;
			BYTE m_type;	// 0=tips, 1=notice
		} TipReq;

		struct // size = var // Get text from gump input
		{
			BYTE m_Cmd;		// 0=0xAC
			NWORD m_len;
			NDWORD m_dialogID;
			BYTE m_parentID; // original calling dialog, just an arbitrary tracking byte
			BYTE m_buttonID;
			BYTE m_retcode; // 0=canceled, 1=okayed
			NWORD m_textlen; // length of text entered
			char m_text[MAX_NAME_SIZE];
		} GumpText;

		struct // size = var // Unicode Speech Request
		{
			BYTE m_Cmd;		// 0=0xAD
			NWORD m_len;
			BYTE m_mode;	// (0=say, 2=emote, 8=whisper, 9=yell) TALKMODE_TYPE
			NWORD m_color;
			NWORD m_font;	// FONT_TYPE
			char m_lang[4];	// lang (null terminated, "ENU" for US english.) 
			NCHAR m_utext[1];	// NCHAR[?] text (null terminated, ?=(msgsize-12)/2)
		} TalkUNICODE;

		struct // size = var // Get gump button change
		{
			BYTE m_Cmd;				//  0 =  0xB1
			NWORD m_len;			//  1 -  2
			NDWORD m_UID;			//  1 -  4
			NDWORD m_dialogID;	//  7 - 10
			NDWORD m_buttonID;		// 11 - 14
			NDWORD m_checkQty;	// 15 - 18
			NDWORD m_checkIds[1];	// 19 - ?? id of a checkbox or radio button that is checked
			NDWORD m_textQty;	// textentry boxes have returned data
			struct
			{
				NWORD m_id;		// id specified in the gump definition file
				NWORD m_len;		// length of the string (in unicode so multiply by 2!)
				NCHAR m_utext[1];	// character data (in unicode!):
			} m_texts[1];
		} GumpButton;

		struct // size = 13?	// ?Request user name
		{
			BYTE m_Cmd;		// 0 = 0xb2
			NDWORD m_uid;	// 1 = targ uid ?
			NDWORD m_unk2;	
			NDWORD m_unk3;	
		} ChatReq;

		struct // size = various
		{
			BYTE m_Cmd;		//	0   = 0xb3
			NWORD m_len;	//  1-2 = packet length
			char m_lang[4];	//	3-6 = unicode language code
			NCHAR m_utext[1];	//	7-? = text from client
		} ChatText;

		struct // size = 64	// response to pressing the chat button
		{
			BYTE m_Cmd;		// 0 = 0xb5
			BYTE m_unk1;	// 1 = ???
			NCHAR m_uname[31];	// 2-63 - unicode name
		} Chat;

		struct // size = 9	// request for tooltip
		{
			BYTE m_Cmd;			// 0 = 0xb6
			NDWORD m_UID;		// 1-4
			BYTE m_langID;	// 5 = 1 = english
			BYTE m_lang[3];	// 6-8 = ENU = english
		} ToolTipReq;

		struct	// size = 8 // Get Character Profile.
		{
			BYTE m_Cmd;		// 0=0xB8
			NWORD m_len;	// 1 - 2
			BYTE m_mode;	// 3; 0 = Get profile, 1 = Set profile
			NDWORD m_UID;	// 4=uid
			BYTE m_unk1;	// 8
			BYTE m_retcode; // ????? 0=canceled, 1=okayed or something similar???
			NWORD m_textlen; // length of text entered
			NCHAR m_utext[1024]; // Text, unicode!
		} CharProfile;

		struct	// size = 9 // mail msg. get primed with itself at start.
		{
			BYTE m_Cmd;		// 0=0xBB
			NDWORD m_uid1;	// 1-4 = uid
			NDWORD m_uid2;	// 5-8 = uid
		} MailMsg;

		struct	// size = ??
		{
			BYTE m_Cmd;		// 0=0xBD
			NWORD m_len;	// 1-2 = len of packet
			char m_text[1024];
		} ClientVersion;

		struct	// we get walk codes from the > 1.26.0 server to be sent back in the Walk messages.
		{
			// walk codes from the > 1.26.0 server to be sent back in the Walk messages
			BYTE m_Cmd;	// 0=0xbf
			NWORD m_len;	// 1 - 2 (len = 5 + (# of codes * 4)
			NWORD m_type;	// 3 - 4 EXTDATA_TYPE 1=prime LIFO stack, 2=add to beginning of LIFO stack, 6=add to party	
			BYTE m_data[1];	// 5 
		} ExtData;

	};
} PACK_NEEDED;

struct CCommand	// command buffer from server to client.
{
	union
	{
		BYTE m_Raw[ MAX_BUFFER ];

#if 0
Message: 0x2d:
Useage: who knows
Structure: byte, dword, word, word, word, word, word, word
Length: 17

Message: 0x30
Usage: who knows
Structure: byte, dword
length: 5

Message: 0x31
use: ???
structure: byte
len: 1

#endif

		struct 
		{
			BYTE m_Cmd;		// 0 = ?
			BYTE m_Arg[1];	// unknown size.
		} Default;	// default unknown type.

		struct // size = 36 + 30 = 66	// Get full status on char.
		{
			BYTE m_Cmd;			// 0 = 0x11
			NWORD  m_len;		// 1-2 = size of packet (2 bytes)
            NDWORD m_UID;		// 3-6 = (first byte is suspected to be an id byte - 0x00 = player)
            char m_name[ MAX_NAME_SIZE ];	// 7-36= character name (30 bytes) ( 00 padded at end)
			NWORD m_health;		// 37-38 = current health 
            NWORD m_maxhealth;	// 39-40 = max health 
			BYTE m_perm;		// 41 = permission to change name ? 0=stats invalid, 1=stats valid., 0xff = i can change the name
			BYTE m_ValidStats;	// 42 = 1 = valid stats ? not sure.
			// only send the rest here if player.
            BYTE m_sex;			// 43 = sex (0 = male)
            NWORD m_str;		// 44-45 = strength 
            NWORD m_dex;		// 46-47 = dexterity
            NWORD m_int;		// 48-49 = intelligence 
            NWORD m_stam;		// 50-51 = current stamina 
            NWORD m_maxstam;	// 52-53 = max stamina 
            NWORD m_mana;		// 54-55 = current mana 
            NWORD m_maxmana;	// max mana (2 bytes)
            NDWORD m_gold;		// 58-61 = gold 
            NWORD m_armor;		// 62-63 = armor class
            NWORD m_weight;		// 64-65 = weight in stones
		} Status;

		struct // size = 9 = follow me
		{
			BYTE m_Cmd;				// 0x15
            NDWORD m_UID_Slave;
            NDWORD m_UID_Master;	
		} Follow;

// All it does is it sends a "Now following" message 
// if you specifify 2 valid uids, 
// if the slave uid is all 0's, 
// it sends a "Not following" message. 
// This message appears over the head of the slave 
// if you tell it to follow you, or over your head if you cancel it (with all 0's).

		struct // size = 19	or var len // draw the item at a location
		{
			BYTE m_Cmd;		// 0 = 0x1a
			NWORD m_len;	// 1-2 = var len = 0x0013 or 0x000e or 0x000f
			NDWORD m_UID;	// 3-6 = UID | UID_SPEC
			NWORD m_id;		// 7-8
			NWORD m_amount;	// 9-10 - only present if m_UID | UID_SPEC = pile (optional)
			NWORD m_x;		// 11-12 - | 0x8000 = m_dir arg.
			NWORD m_y;		// 13-14 = y | 0xC000 = m_color and m_flags fields.	
			BYTE m_dir;		// (optional)
			BYTE m_z;		// 15 = signed char
			NWORD m_color;	// 16-17 = COLOR_TYPE (optional)
			BYTE m_flags;	// 18 = 0x20 = is it movable ? (direction?) (optional)
		} Put;

		// Item flags
#define ITEMF_MOVABLE	0x20
#define ITEMF_INVIS		0x80

		struct // size = 37	// start up player
		{
			BYTE	m_Cmd;			// 0 = 0x1B
			NDWORD	m_UID;			// 1-4
			NDWORD	m_unk_5_8;		// 5-8 = 0
			NWORD	m_id;			// 9-10
			NWORD	m_x;			// 11-12
			NWORD	m_y;			// 13-14
			NWORD	m_z;			// 15-16
			BYTE	m_dir;			// 17
			BYTE	m_unk_18;		// 18
			NDWORD	m_unk_19_22;	// 19-22
			NWORD	m_unk_23_24;	// 23-14
			NWORD	m_unk_25_26;	// 23-14
			NWORD	m_mode;			// 27-28
			NWORD	m_unk_29_30;	// 29-30
			BYTE	m_unk31[6];
		} Start;

		// Char mode flags
#define CHARMODE_POISON		0x04	// green status bar.
#define CHARMODE_YELLOW		0x08	// ? yellow status bar.
#define CHARMODE_WAR		0x40
#define CHARMODE_INVIS		0x80

		struct // size = 14 + 30 + var
		{
			BYTE m_Cmd;			// 0 = 0x1C
			NWORD m_len;		// 1-2 = var len size.
			NDWORD m_UID;		// 3-6 = UID num of speaker. 01010101 = system
			NWORD m_id;			// 7-8 = CREID_TYPE of speaker.
			BYTE m_mode;		// 9 = TALKMODE_TYPE
			NWORD m_color;		// 10-11 = COLOR_TYPE.
			NWORD m_font;		// 12-13 = FONT_TYPE	
			char m_name[MAX_NAME_SIZE];	// 14
			char m_text[1];		// var size.
		} Speak;

		struct // sizeo = 5	// remove an object (item or char)
		{
			BYTE m_Cmd;			// 0 = 0x1D
			NDWORD m_UID;		// 1-4 = object UID.
		} Remove;

		struct // size = 19 // set client player view x,y,z.
		{
			BYTE m_Cmd;			// 0 = 0x20
			NDWORD m_UID;		// 1-4 = my UID.
			NWORD m_id;			// 5-6
			BYTE m_zero7;		// 7 = 0
			NWORD m_color;		// 8-9
			BYTE m_mode;		// 10 = 0=normal, 4=poison, 0x40=attack, 0x80=hidden	
			NWORD m_x;			// 11-12
			NWORD m_y;			// 13-14
			NWORD m_zero15;		// 15-16 = noto ?
			BYTE m_dir;			// 17 = high bit set = run
			BYTE m_z;			// 18
		} View;	

		struct // size = 8
		{
			BYTE m_Cmd;			// 0 = 0x21
			BYTE m_count;		// sequence # 
			NWORD m_x;
			NWORD m_y;
			BYTE m_dir;
			BYTE m_z;
		} WalkCancel;

		struct	// size=3
		{
			BYTE m_Cmd;		// 0 = 0x22
			BYTE m_count; 	// 1 = goes up as we walk. (handshake)
			BYTE m_flag;	// 2 = 0 or 0x41 (Not sure?) noto ?
		} WalkAck;

		struct // size = 26 
		{
			BYTE m_Cmd;		// 0=0x23
			NWORD m_id;
			NWORD m_unk3;	// 3-4 = 0
			NWORD m_unk5;	// 5-9 = ?
			BYTE  m_unk7;	// 7 = 0, 72, 99, c1, 73
			NDWORD m_srcUID; // NULL if from ground.
			NWORD m_src_x;
			NWORD m_src_y;
			BYTE m_src_z;
			NDWORD m_dstUID;
			NWORD m_dst_x;
			NWORD m_dst_y;
			BYTE m_dst_z;
		} DragAnim;

		struct // size = 7	// Open a container gump
		{
			BYTE m_Cmd;		// 0 = 0x24
			NDWORD m_UID;	// 1-4
			NWORD m_gump;	// 5 = gump graphic id. GUMP_TYPE
		} ContOpen;

		struct // size = 20	// Add Single Item To Container.
		{
			BYTE m_Cmd;		// 0 = 0x25
			NDWORD m_UID;	// 1-4
			NWORD m_id;
			BYTE m_zero7;
			NWORD m_amount;
			NWORD m_x;
			NWORD m_y;
			NDWORD m_UIDCont;	// the container.
			NWORD m_color;
		} ContAdd;

		struct // size = 5	// Kick the player off.
		{
			BYTE m_Cmd;		// 0 = 0x26
			NDWORD m_unk1;	// 1-4 = 0 = GM who issued kick ?
		} Kick;

		struct // size = 2	// kill a drag
		{
			BYTE m_Cmd;		// 0 = 0x27
			BYTE m_type;	// 1 = bounce type ? = 5 = drag
		} DragCancel;

		struct // size = 5	// clear square. Never used this.
		{
			BYTE m_Cmd;		// 0 = 0x28
			NWORD m_x;
			NWORD m_y;
		} ClearSquare;

		struct // size = 1 unknown. follows status ? Reported by Pete.
		{
			BYTE m_Cmd;		// 0 = 0x29
		} Unk29;	

		struct // size = 2 Death Menu
		{
			BYTE m_Cmd;		// 0 = 0x2c
			BYTE m_shift;	// 1 = 0
		} DeathMenu;
		
		struct // size = 15 = equip this item
		{
			BYTE m_Cmd;			// 0 = 0x2e
			NDWORD m_UID;		// 1-4 = object UID.
			NWORD m_id;			// 5-6
			BYTE m_zero7;
			BYTE m_layer;		// 8=layer
			NDWORD m_UIDchar;	// 9-12=UID num of owner.
			NWORD m_color;
		} ItemEquip;

		struct // size = 10	// There is a fight some place.
		{
			BYTE m_Cmd;		// 0 = 0x2f
			BYTE m_dir;		// 1 = 0 = DIR_TYPE
			NDWORD m_AttackerUID;
			NDWORD m_AttackedUID;
		} Fight;

		struct // size = 2
		{
			BYTE m_Cmd;		// 0 = 0x33
			BYTE m_Arg;		// 1 = (1=pause,0=restart)
		} Pause;

		struct // size = 9 // Makes the client run to a coord (must be sent 20 times!!!!)
		{
			BYTE m_Cmd; // 0 = 0x38
			NWORD m_x; // 1-2 = x coord to run to (must be within 2 screens)
			NWORD m_y; // 3-4 = y coord to run to
			NWORD m_z; // 5-6 = z coord to run to
		} PathFind;

		struct // size = 9	// just puts messages above uid heads.
		{
			BYTE m_Cmd;		// 0 = 0x39
			NDWORD m_uid1;	// 1-4 = uid of group owner (it's the group number), or 0 to remove
			NDWORD m_uid2;	// 5-8 = uid of char to add to the group, or 0 to remove (1 = illegal group)
		} ChangeGroup;

		struct // size = 0xbe = Fill in the skills list.
		{
			BYTE m_Cmd;		// 0= 0x3A
			NWORD m_len;	// 1= 0x00be
			BYTE m_single;	// 3=0 = all w/ 0 term, ff=single and no terminator
			struct
			{
				NWORD m_index;	// 1 based, 0 = terminator. (no val)
				NWORD m_val;	// Skill * 10 	
			} skills[1];
		} Skill_v261;

		struct // size = ?? = Fill in the skills list.
		{
			BYTE m_Cmd;		// 0= 0x3A
			NWORD m_len;	// 1= 
			BYTE m_single;	// 3=0 = all w/ 0 term, ff=single and no terminator
			struct
			{
				NWORD m_index;	// 1 based, 0 = terminator. (no val)
				NWORD m_val;	// Skill * 10 (stat modified!)
				NWORD m_valraw; // Skill * 10 (stat unmodified!)
				BYTE m_lock;	// current lock mode (0 = none (up), 1 = down, 2 = locked)
			} skills[1];
		} Skill;

		struct // size = variable	// close the vendor window.
		{
			BYTE m_Cmd;		// 0 =0x3b
			NWORD m_len;
			NDWORD m_UIDVendor;
			BYTE m_flag;	// 0x00 = no items following, 0x02 - items following 
		} VendorBuy;

		struct // size = 5 + ( x * 19 ) // set up the content of some container.
		{
			BYTE m_Cmd;		// 0 = 0x3C
			NWORD m_len;
			NWORD m_count;

			struct // size = 19
			{
				NDWORD m_UID;	// 0-3
				NWORD m_id;	// 4-5
				BYTE m_zero6;
				NWORD m_amount;	// 7-8
				NWORD m_x;	// 9-10	
				NWORD m_y;	// 11-12
				NDWORD m_UIDCont;	// 13-16 = What container is it in.
				NWORD m_color;	// 17-18
			} items[ MAX_ITEMS_CONT ];
		} Content;

		struct // size = 6	// personal light level.
		{
			BYTE m_Cmd;		// 0 = 0x4e
			NDWORD m_UID;	// 1 = creature uid
			BYTE m_level;	// 5 = 0 = shape of the light. (LIGHT_PATTERN?)
		} LightPoint;

		struct // size = 2
		{
#define LIGHT_BRIGHT	0
#define LIGHT_DARK		30
#define LIGHT_DARK_OLD	19
			BYTE m_Cmd;			// 0 = 0x4f
			BYTE m_level;		// 1=0-19, 19=dark, On t2a 30=dark
		} Light;

		struct // size = 2	// idle warning
		{
			BYTE m_Cmd;			// 0 = 0x53
			BYTE m_unk;			// 1=7
		} IdleWarn;

		struct // size = 12
		{
			BYTE m_Cmd;		// 0 = 0x54
			BYTE m_flags;	// 1 = quiet, infinite repeat or single shot (0/1)
			NWORD m_id;		// 2-3=sound id (SOUND_TYPE)
			NWORD m_speed;	// 4-5=0 = (speed/volume modifier?)
			NWORD m_x;		// 6-7
			NWORD m_y;		// 8-9	
			NWORD m_z;		// 10-11
		} Sound;

		struct	// size = 1	// Sent at start up. Cause Client to do full reload and redraw of config.
		{
			BYTE m_Cmd;		// 0 =0x55
		} ReDrawAll;

		struct	// size = 11
		{ 
			BYTE m_Cmd; // 0 = 0x56
			NDWORD m_UID;
			BYTE m_Mode; // 1: Add a pin location, 5: Display map, pins to follow 7: Editable, no pins follow 8: read only
			BYTE m_Req;	// 0: req for read mode, 1: request for edit mode
			NWORD m_pin_x;
			NWORD m_pin_y;
		} MapEdit;

		struct	// size = 4	// Set Game Time. not sure why the client cares about this.
		{
			BYTE m_Cmd;		// 0 =0x5b
			BYTE m_hours;
			BYTE m_min;
			BYTE m_sec;
		} Time;

		struct // size = 4
		{
			BYTE m_Cmd;			// 0 = 0x65
			BYTE m_type;		// 1 = type. 0=dry, 1=rain, 2=snow
			NWORD m_ext;		// 2 = other weather info (severity?)
		} Weather;

		struct	// size = 13 + var // send a book page.
		{
			BYTE m_Cmd;		// 0 = 0x66
			NWORD m_len;	// 1-2
			NDWORD m_UID;	// 3-6 = the book
			NWORD m_pages;	// 7-8 = 0x0001 = # of pages here
			// repeat these fields for multiple pages.
			struct
			{
				NWORD m_pagenum;	// 9-10=page number  (1 based)
				NWORD m_lines;	// 11
				char m_text[1];
			} m_page[1];
		} BookPage;

		struct	// size = var	// seems to do nothing here.
		{
			BYTE m_Cmd;		// 0=0x69
			NWORD m_len;
			NWORD m_index;
		} Options;

		struct	// size = 19
		{
			BYTE m_Cmd;		// 0 = 0x6C
			BYTE m_fAllowGround;	// 1 = 0=select object, 1=x,y,z
			NDWORD m_code;	// 2-5 = we sent this at target setup.
			BYTE m_unk6;		// 6= 0
			
			// Unused junk.
			NDWORD m_UID;	// 7-10	= serial number of the target.
			NWORD m_x;		// 11,12
			NWORD m_y;		// 13,14
			BYTE m_unk2;	// 15 = fd ?
			BYTE m_z;		// 16
			NWORD m_id;		// 17-18 = static id of tile
		} Target;

		struct // size = 3 // play music
		{
			BYTE m_Cmd;		// 0 = 0x6d
			NWORD m_musicid;// 1= music id number	
		} PlayMusic;

		struct // size = 14 // Combat type animation.
		{
			BYTE m_Cmd;			// 0 = 0x6e
			NDWORD m_UID;		// 1-4=uid
			NWORD m_action;		// 5-6 = ANIM_TYPE
			BYTE m_zero7;		// 7 = 0 or 5 ?
			BYTE m_dir;
			NWORD m_repeat;		// 9-10 = repeat count. 0=forever.
			BYTE m_backward;	// 11 = backwards (0/1)
			BYTE m_repflag;		// 12 = 0=dont repeat. 1=repeat
			BYTE m_framedelay;	// 13 = 0=fastest.
		} CharAction;
		
		struct // size = var < 47 // Secure trading
		{
			BYTE m_Cmd;		// 0=0x6F
			NWORD m_len;	// 1-2 = len
			BYTE m_action;	// 3 = trade action
			NDWORD m_UID;	// 4-7 = uid 
			NDWORD m_UID1;	// 8-11
			NDWORD m_UID2;	// 12-15
			BYTE m_fname;	// 16 = 0=none, 1=name
			char m_name[ MAX_NAME_SIZE ];
		} SecureTrade;

		struct // size = 28 // Graphical effect.
		{
			BYTE m_Cmd;			// 0 = 0x70
			BYTE m_motion;		// 1= the motion type. (0=point to point,3=static) EFFECT_TYPE
			NDWORD m_UID;		// 2-5 = The target item. or source item if point to point.
			NDWORD m_targUID;	// 6-9 = 0
			NWORD m_id;			// 10-11 = base display item 0x36d4 = fireball
			NWORD m_srcx;		// 12
			NWORD m_srcy;		// 14	
			BYTE  m_srcz;		// 16
			NWORD m_dstx;		// 17
			NWORD m_dsty;		// 19
			BYTE  m_dstz;		// 21
			BYTE  m_speed;		// 22= 0=very fast, 7=slow.
			BYTE  m_loop;		// 23= 0 is really long.  1 is the shortest.
			WORD  m_color;		// 24=0 unknown ? color ?
			BYTE  m_OneDir;		// 26=1=point in single dir else turn.
			BYTE  m_explode;	// 27=effects that explode on impact.
		} Effect;

		struct // size = var (38)	// Bulletin Board stuff
		{
			BYTE m_Cmd;		// 0=0x71
			NWORD m_len;	// 1-2 = len
			BYTE m_flag;	// 3= 0=board name, 4=associate message, 1=message header, 2=message body
			NDWORD m_UID;	// 4-7 = UID for the bboard.
			BYTE m_data[1];	// 8- = name or links data.
		} BBoard;

		struct // size = 5	// put client to war mode.
		{
			BYTE m_Cmd;		// 0 = 0x72
			BYTE m_warmode;	// 1 = 0 or 1 
			BYTE m_unk2[3];	// 2 = 00 32 00
		} War;

		struct // size = var // Open buy window
		{
			BYTE m_Cmd;		// 0 = 0x74
			NWORD m_len;
			NDWORD m_VendorUID;
			BYTE m_count;
			struct
			{
				NDWORD m_price;
				BYTE m_len;
				char m_text[1];
			} items[1];
		} VendOpenBuy;

		struct // size = 16	// Move to a new server. Not sure why the client cares about this.
		{
			BYTE m_Cmd;		// 0 = 0x76
			NWORD m_x;
			NWORD m_y;
			NWORD m_z;
			BYTE m_unk7;	// is a toggle of some sort
			WORD m_unk8;	// distances ?
			WORD m_unk10;
			WORD m_unk12;
			WORD m_unk14;	// describes the new server some how.
		} ZoneChange;

		// 00 14 00 00 00 04 00 09 00

		struct // size = 17	// simple move of a char already on screen.
		{
			BYTE m_Cmd;		// 0 = 0x77
			NDWORD m_UID;	// 1-4
			NWORD m_id;		// 5-6 = id
			NWORD m_x;		// 7-8 = x
			NWORD m_y;		// 9-10
			BYTE m_z;		// 11
			BYTE m_dir;		// 12 = DIR_TYPE (| 0x80 = running ?)
			NWORD m_color;	// 13-14 = COLOR_TYPE
			BYTE m_mode;	// 15 = CHARMODE_WAR
			BYTE m_noto;	// 16 = NOTO_TYPE
		} CharMove;

		struct // size = 23 or var len // draw char
		{
			BYTE m_Cmd;		// 0 = 0x78
			NWORD m_len;	// 1-2 = 0x0017 or var len?
			NDWORD m_UID;	// 3-6=
			NWORD m_id;	// 7-8
			NWORD m_x;	// 9-10
			NWORD m_y;	// 11-12
			BYTE m_z;		// 13
			BYTE m_dir;		// 14 = DIR_TYPE
			NWORD m_color;	// 15-16 = COLOR_TYPE
			BYTE m_mode;	// 17 = CHARMODE_WAR 
			BYTE m_noto;	// 18 = NOTO_TYPE

			struct	// This packet is extendable to show equip.
			{
				NDWORD m_UID;	// 0-3 = 0 = end of the list.
				NWORD m_id;		// 4-5 = | 0x8000 = include m_color.
				BYTE m_layer;	// LAYER_TYPE
				NWORD m_color;	// only if m_id | 0x8000
			} equip[1];

		} Char;

		struct // size = var // put up a menu of items.
		{
			BYTE m_Cmd;			// 0=0x7C
			NWORD m_len;
			NDWORD m_UID;		// if player then gray menu choice bar.
			NWORD m_menuid;		// The menu id (sent back as part of choice)(0x7d)
			BYTE m_lenname;
			char m_name[1];		// var len. (not null term)
			BYTE m_count;
			struct		// size = var
			{
				NWORD m_id;		// image next to menu.
				NWORD m_zero2;	// check or not ?
				BYTE m_lentext;
				char m_name[1];	// var len. (not null term)
			} items[1];
		} MenuItems;

		struct // size = var // Switch to this player? (in game)
		{
			// have no clue what it does to the crypt keys (probably nothing) 
			// all I know is it puts up the list and sends back a 0x5d (Play) message...
			// if it goes further than that, it'll be awesome...what else can we do with this?

			BYTE m_Cmd; // 0 = 0x81
			NWORD m_len;
			BYTE m_count;
			BYTE m_unk;	// ? not sure.
			CEventCharDef m_char[MAX_CHARS_PER_ACCT];
		} CharList3;

		struct // size = 2
		{
			BYTE m_Cmd;		// 0 = 0x82
			BYTE m_code;	// 1 = LOGIN_ERR_*
		} LogBad;

		struct // size = 2
		{
			BYTE m_Cmd;		// 0 = 0x85
			BYTE m_code;	// 1 = DELETE_ERR_*
		} DeleteBad;

		struct // size = var	// refill char list after delete.
		{
			BYTE m_Cmd;		// 0 = 0x86
			NWORD m_len;
			BYTE m_count;
			CEventCharDef m_char[MAX_CHARS_PER_ACCT];
		} CharList2;

		struct // size = 66
		{
			BYTE m_Cmd;			// 0 = 0x88
			NDWORD m_UID;		// 1-4 = 
			char m_text[60];	// 5-	
			BYTE m_mode;		// 0=normal, 4=poison, 0x40=attack, 0x80=hidden	
		} PaperDoll;

		struct // size = 7 + count * 5 // Equip stuff on corpse.
		{
			BYTE m_Cmd;		// 0 = 0x89
			NWORD m_len;
			NDWORD m_UID;

			struct // size = 5
			{
				BYTE m_layer;	// 0 = LAYER_TYPE, list must be null terminated ! LAYER_NONE.
				NDWORD m_UID;	// 1-4
			} items[ MAX_ITEMS_CONT ];
		} CorpEquip;	

		struct	// display a gump with some text on it.
		{
			BYTE m_Cmd; // 0x8b
			NWORD m_len; // 13 + len(m_unktext) + len(m_text)
			NDWORD m_UID; // Some uid which doesn't clash with any others that the client might have
			NWORD m_gump; // Signs and scrolls work well with this
			NWORD m_len_unktext; // Evidently m_unktext is either an array of bytes, or a non-nulled char array
			NWORD m_unk11; // who knows??
			char m_unktext[1]; // same
			char m_text[1]; // text to display
		} GumpTextDisp;

		struct	// size = 11
		{
			BYTE m_Cmd;			// 0 = 0x8C
			BYTE m_ip[4];		// 1 = struct in_addr 
			NWORD m_port;		// 5 = Port server is on
			NDWORD m_Account;	// 7-10 = customer id (sent to game server)
		} Relay;

		struct	// size = 19
		{
			BYTE m_Cmd; // 0 = 0x90
			NDWORD m_UID; // uid of the map item
			NWORD m_Gump_Corner; // always 0x139d....why? compass tile id in the corner.,
			NWORD m_x_ul; // upper left x coord.
			NWORD m_y_ul; // upper left y coord.
			NWORD m_x_lr; // lower right x coord.
			NWORD m_y_lr; // lower right y coord.
			NWORD m_xsize; // client width
			NWORD m_ysize; // client height
		} MapDisplay;	// Map1

		struct	// Needs God client ?
		{
#define MAX_ITEMS_MULTI (MAX_BUFFER - 11) / 12/*1279*/ 
			BYTE m_Cmd; // 0 = 0x92
			NWORD m_len; // 11 + (# of items * 12)
			NDWORD m_id; // ITEMID_TYPE of the multi item (> 0x4000)
			NDWORD m_tag; // Multi Item tag # (who needs it? version #???)
			struct
			{
				NWORD m_id; // ITEMID_TYPE
				NWORD m_dx; // signed!
				NWORD m_dy; // signed!
				NWORD m_dz; // signed!
				NDWORD m_flags; // visible = 1, invisible = 0
			} m_item[MAX_ITEMS_MULTI];
		} EditMultiMul;

		struct // size = 8 + 3 * MAX_NAME_SIZE
		{
			BYTE m_Cmd;			// 0 = 0x93
			NDWORD m_UID;		// 1-4 = book
			BYTE m_writable;	// 5 = 0 = non writable, 1 = writable.
			NWORD m_pages;		// 6-7 = number of pages.
			char m_title[ 2 * MAX_NAME_SIZE ];
			char m_author[ MAX_NAME_SIZE ];
		} BookOpen_v25;

		struct // size = 98 // Get a change to the book title.
		{
			BYTE m_Cmd;			// 0 = 0x93
			NDWORD m_UID;		// 1-4 = book
			BYTE m_writable;	// 5 = 0 = non writable, 1 = writable. (NOT USED HERE)
			BYTE m_NEWunk1;
			NWORD m_pages;		// 6-7 = number of pages. (NOT USED HERE)
			char m_title[ 2 * MAX_NAME_SIZE ];
			char m_author[ MAX_NAME_SIZE ];
		} BookOpen_v26;

		struct	// Needs God client ?
		{
			BYTE m_Cmd; // 0 = 0x94
			NWORD m_len; // 1-2 = 9 + strlen(m_name) + 1
			BYTE m_index; // 3 = skill to add/overwrite
			NDWORD m_tag; // 4-7 = goes into the skills.idx what is it? version #???
			BYTE m_button; // 8 = (0=no button, 1=button)
			char m_name[MAX_NAME_SIZE];// 9-? = zero term skill name
		} EditSkillsMul;

		struct // size = 9
		{
			BYTE m_Cmd;			// 0 = 0x95
			NDWORD m_UID;		// 1-4
			NWORD m_zero5;		// 5-6
			NWORD m_id;		// 7-8
		} DyeVat;

		struct	// makes the client's character walk 1 step  in what ever direction you tell it (0 - 7)
		{
			BYTE m_Cmd;		// 0 = 0x97
			BYTE m_dir;
		} WalkForce;	

		struct // size = 26	// preview a house/multi
		{
			BYTE m_Cmd;		// 0 = 0x99
			BYTE m_fAllowGround;	 // 1 = 1=ground only, 0=dynamic object
			NDWORD m_code;	// 2-5 = we sent this at target setup.
			BYTE m_zero6[12];	// 6-17
			NWORD m_id;		// 18-19 = The multi id to preview. (id-0x4000)
			BYTE m_zero20[6];
		} TargetMulti;

		struct // size = 16 // console prompt request.
		{
			BYTE m_Cmd;		// 0 = 0x9a
			NWORD m_len;	// 1-2 = length = 16
			BYTE m_unk3[12];
			char m_text[1];	// 3 = null terminated text? doesn't seem to work.
		} Prompt;

		struct // size = var	// vendor sell dialog
		{
			BYTE m_Cmd;		// 0 = 0x9e
			NWORD m_len;	// 1-2
			NDWORD m_UIDVendor;	// 3-6
			NWORD m_count;	// 7-8
			struct
			{
				NDWORD m_UID;
				NWORD m_id;
				NWORD m_color;
				NWORD m_amount;
				NWORD m_price;
				NWORD m_len;
				char m_text[1];
			} items[1];
		} VendOpenSell;

		struct	// size = 9	// update some change in stats.
		{
			BYTE m_Cmd;	// 0=0xa1 (str), 0xa2 (int), or 0xa3 (dex)
			NDWORD m_UID;	// 1-4
			NWORD m_max;	// 5-6
			NWORD m_val;	// 7-8
		} StatChng;

		struct // size = 3+var
		{
			BYTE m_Cmd;		// 0 = 0xA5
			NWORD m_len;
			char m_page[ 1 ];	// var size
		} Web;

		struct // size = 10 + var	// Read scroll
		{
			BYTE m_Cmd;			// 0=0xA6
			NWORD m_len;		// 1-2
			BYTE  m_type;		// 3 = form or gump = 0=tips,1=notice or 2 (SCROLL_TYPE)
			NDWORD m_scrollID;	// 4-7 = response type id
			NWORD m_lentext;	// 8
			char m_text[1];		// 10
		} Scroll;

		struct // size = 6+servers*40
		{
			BYTE m_Cmd;			// 0 = 0xA8
			NWORD m_len;		// 1-2
			BYTE m_unk3;		// 3=0x14 ?
			NWORD m_count;	// 4-5=num servers.

#define MAX_SERVERS 64
#define MAX_SERVER_NAME_SIZE MAX_NAME_SIZE

			struct	// size=40
			{
				NWORD m_count;	// 0=0 based enum
				char m_name[MAX_NAME_SIZE];	// 2
				NWORD m_zero32;	// 32 = 0 = unknown use.
				BYTE m_percentfull;	// 34 = 25 or 2e
				signed char m_timezone;	// 35 = 0x05=east coast or 0x08=west coast
				BYTE m_ip[4];		// 36-39 = struct in_addr  
			} m_serv[ MAX_SERVERS ];
		} ServerList;

		struct // size = 4+(5*2*MAX_NAME_SIZE)+1+(starts*63) // list available chars for your account.
		{
			BYTE m_Cmd;		// 0 = 0xa9
			NWORD m_len;	// 1-2 = var len
			BYTE m_count;	// 3=5 needs to be 5 for some reason.
			CEventCharDef m_char[MAX_CHARS_PER_ACCT];

			BYTE m_startcount;
			struct	// size = 63
			{
				BYTE m_id;
				char m_area[MAX_NAME_SIZE+1];
				char m_name[MAX_NAME_SIZE+1];
			} m_start[1];
		} CharList;

		struct // size = 5 = response to attack.
		{
			BYTE m_Cmd;		// 0= 0xaa
			NDWORD m_UID;	// 1= char attacked. 0=attack refused.
		} AttackOK;

		struct // size = 13 + var // Send a gump text entry dialog.
		{
			BYTE m_Cmd;							// 0 = 0xab
			NWORD m_len;						// 1-2	
			NDWORD m_dialogID;				// 3-6
			BYTE m_parentID;					// 7
			BYTE m_buttonID;					// 8 (7&8 to see where they came from?, like a context type thing?)
			NWORD m_textlen1;					// length of string #1
			char m_text1[256];				// line 1 text
			BYTE m_cancel;						// 0 = disable, 1=enable
			BYTE m_style;						// 0 = disable, 1=normal, 2=numerical
			NDWORD m_mask;						// format[1] = max length of textbox input, format[2] = max value of textbox input
			NWORD m_textlen2;					// length of string #2
			char m_text2[256];				// line 2 text
		} GumpInputBox;

		struct // size = 14 + 30 + var // Unicode speech
		{
			BYTE m_Cmd;			// 0 = 0xAE
			NWORD m_len;		// 1-2 = var len size.
			NDWORD m_UID;		// 3-6 = UID num of speaker., 0xFFFFFFFF = noone.
			NWORD m_id;			// 7-8 = CREID_TYPE of speaker. 0xFFFF = none
			BYTE m_mode;		// 9 = TALKMODE_TYPE
			NWORD m_color;		// 10-11 = COLOR_TYPE.
			NWORD m_font;		// 12-13 = FONT_TYPE	
			char m_lang[4];		// language (same as before)
			char m_name[MAX_NAME_SIZE];	// 
			NCHAR m_utext[1];		// text (null terminated, ?=(msgsize-48)/2
		} SpeakUNICODE;

		struct // size = 13	// Death anim of a creature
		{
			BYTE m_Cmd;		// 0 = 0xaf
			NDWORD m_UID;	// 1-4
			NDWORD m_UIDCorpse; // 5-8
			NWORD m_DeathFlight;		// 9= in flight ?
			NWORD m_Death2Anim;	// 11= forward/backward,dir ?
		} CharDeath;

		struct // size = 21 + var // Send a gump menu dialog.
		{
			BYTE m_Cmd;		// 0 = 0xb0
			NWORD m_len;	// 1	
			NDWORD m_UID;	// 3
			NDWORD m_gump;	// 7-10
			NDWORD m_x;
			NDWORD m_y;
			NWORD m_lenCmds; // 19-20 = command section length.
			char m_commands[1];	// Format commands (var len) { %s }
			BYTE m_zeroterm;
			NWORD m_textlines;	// How many lines of text to follow.
			struct 
			{
				NWORD m_len;	// len of this line.
				NCHAR m_utext[1];	// UNICODE text. (not null term)
			} m_texts[1];
		} GumpDialog;

		struct
		{
			BYTE m_Cmd;			// 0 = 0xb2
			NWORD m_len;		// 1-2 = length of packet
			NWORD m_subcmd;		// 3-4 = 0x001a - already in this channel
			char m_lang[4];// 5-8 = unicode language code (null term....default = 'enu' (english?)
			NCHAR m_uname[1];		// 9-? = name in unicode (zero terminated) (may have  prefix, moderator, etc)
			NCHAR m_uname2[1];	// ?-? = name in unicode (also used for other things (passworded, etc)
		} ChatReq;

		struct
		{
#define MAX_ITEMS_PREVIEW (MAX_BUFFER - 16) / 10
			// MAX_ITEMS_PREVIEW = 1534
			BYTE m_Cmd; // 0 = 0xb4
			NWORD m_len; // 1-2 = 16 + (10 * m_count)
			BYTE m_fAllowGround; // 3 = 
			NDWORD m_code; // 4-7 = target id
			NWORD m_xOffset; // 8-9 = x targ offset
			NWORD m_yOffset; // 10-11 = y targ offset
			NWORD m_zOffset; // 12-13 = z targ offset
			NWORD m_count;	// 14-15 = how many items in this preview image
			struct
			{
				NWORD m_id;	// 0-1 = ITEMID_TYPE
				NWORD m_dx;	// 2-3 = delta x
				NWORD m_dy;	// 4-5 = delta y
				NWORD m_dz;	// 6-7 = delta z
				NWORD m_unk;	// 6-7 = ???
			} m_item[MAX_ITEMS_PREVIEW];
		} TargetItems;

		struct // size = 64	// response to pressing the chat button
		{
			BYTE m_Cmd;		// 0 = 0xb5
			BYTE unk1;		// 1 = 0 (???)
			NCHAR m_uname[31];// 2-63 - unicode name
		} Chat;

		struct // size = variable	// Put up a tooltip
		{
			BYTE m_Cmd;				// 0 = 0xb7
			NWORD m_len;			// 1-2
			NDWORD m_UID;			// 3-6
			NCHAR m_utext[512];		// zero terminated UNICODE string
		} ToolTip;

		struct	// size = 8 // Show Character Profile.
		{
			BYTE m_Cmd;		// 0=0xB8
			NWORD m_len;	// 1 - 2
			NDWORD m_UID;	// 3-6=uid
			char m_desc1[MAX_NAME_SIZE+1];	// ???Description 1 (not unicode!, name, zero terminated)
			char m_desc2[MAX_NAME_SIZE+1];	// ???Description 2 (not unicode!, guild?? fame/karma??? zero terminated)
			NCHAR m_utext[1024];			// ???Description 3 (unicode!, zero terminated)
		} CharProfile;
		
		struct // size = 3	// response to pressing the chat button
		{
			BYTE m_Cmd;		// 0 = 0xb9
			NWORD m_enable;	
		} ChatEnable;

		struct
		{
			BYTE m_Cmd;	// 0xba
			BYTE m_Active;	// 1/0
			NWORD m_x;
			NWORD m_y;
		} Arrow;

		struct	// Unknown set len = 3
		{
			BYTE m_Cmd; // 0=0xbc
			NWORD m_Unk;
		} Unk_bc;

		struct	// we get walk codes from the > 1.26.0 server to be sent back in the Walk messages.
		{
			// walk codes from the > 1.26.0 server to be sent back in the Walk messages
			BYTE m_Cmd;	// 0=0xbf
			NWORD m_len;	// 1 - 2 (len = 5 + (# of codes * 4)
			NWORD m_type;	// 3 - 4 EXTDATA_TYPE = 1=prime LIFO stack, 2=add to beginning of LIFO stack	
			BYTE m_data[1];	// 5 - ? OSI sends 10 of these random numbers out to start at this time
		} ExtData;

		struct	// Display a msg from table. (already present at the client side)
		{
			BYTE m_Cmd; // 0=0xc1
			NWORD m_len;		// 1-2 = var len size. (50)
			NDWORD m_UID;		// 3-6 = UID num of speaker. 01010101 = system
			NWORD m_id;			// 7-8 = CREID_TYPE of speaker. Used for text hieight (doesn't the damn client already know this?)
			BYTE m_mode;		// 9 = TALKMODE_TYPE
			NWORD m_color;		// 10-11 = COLOR_TYPE.
			NWORD m_font;		// 12-13 = FONT_TYPE	
			NDWORD m_index;		// = predefined message type (MSG_TYPE) 
			char m_name[MAX_NAME_SIZE+2]; // Name or other label (does OSI have this part actually working?)
		} SpeakTable;

	};
} PACK_NEEDED;

// Turn off structure packing.
#if defined _WIN32 && (!__MINGW32__)
#pragma pack()
#endif

#endif // _INC_GRAYPROTO_H
