    // CClient.h
    //
    
    #ifndef _INC_CCLIENT_H
    #define _INC_CCLIENT_H
    #if _MSC_VER >= 1000
    #pragma once
    #endif // _MSC_VER >= 1000
    
    enum CV_TYPE
    {
        CV_ADD,
        CV_ADDITEM,
        CV_ADDNPC,
        CV_ADMIN,
        CV_ARROWQUEST,
        CV_BANKSELF,
        CV_CAST,
        CV_CHARLIST,
        CV_CLIENTS,
        CV_EVERBTARG,
        CV_EXTRACT,
        CV_GHOST,
        CV_GMPAGE,
        CV_GOTARG,
        CV_HELP,
        CV_INFO,
        CV_INFORMATION,
        CV_ITEMMENU,
        CV_LAST,
        CV_LINK,
        CV_LOGIN,
        CV_LOGOUT,
        CV_MENU,
        CV_MIDI,
        CV_MIDILIST,
        CV_MUSIC,
        CV_NUDGE,
        CV_NUKE,
        CV_NUKECHAR,
        CV_ONECLICK,
        CV_PAGE,
        CV_REPAIR,
        CV_RESEND,
        CV_RESYNC,
        CV_SAVE,
        CV_SCROLL,
        CV_SELF,
        CV_SHOWSKILLS,
        CV_SKILLMENU,
        CV_STATIC,
        CV_SUMMON,
        CV_SYNC,
        CV_SYSMESSAGE,
        CV_TELE,
        CV_TILE,
        CV_UNEXTRACT,
        CV_VERSION,
        CV_WEBLINK,
        CV_QTY,
    };
    
    enum CC_TYPE
    {
        CC_ALLMOVE,
        CC_ALLSHOW,
        CC_CLIENTVER,
        CC_CLIENTVERSION,
        CC_CLIVER,
        CC_DEBUG,
        CC_DETAIL,
        CC_GM,                          // (R/W)
        CC_HEARALL,
        CC_LISTEN,
        CC_PRIVHIDE,
        CC_PRIVSHOW,
        CC_TARG,
        CC_TARGP,
        CC_TARGPROP,
        CC_TARGPRV,
        CC_TARGTXT,
        CC_QTY,
    };
    
    class CPartyDef : public CGObListRec
    {
        // a list of characters in the party.
    protected:
        DECLARE_MEM_DYNAMIC;
    private:
        CCharRefArray m_Chars;
        CGTypedArray< bool, bool> m_fLootFlags;
    
    private:
        bool SendMemberMsg( CChar * pCharDest, const CExtData * pExtData, int iLen );
        void SendAll( const CExtData * pExtData, int iLen );
        void SendRemoveList( CChar * pCharRemove, CGrayUID uidAct );
    
    public:
        CPartyDef( CChar * pCharInvite, CChar * pCharAccept );
    
        static bool AcceptEvent( CChar * pCharAccept, CGrayUID uidInviter );
        static bool DeclineEvent( CChar * pCharDecline, CGrayUID uidInviter );
        static void MessageClient( CClient * pClient, CGrayUID uidSrc, const NCHAR * pText, int ilenmsg );
    
        bool IsInParty( const CChar * pChar ) const
        {
                int i = m_Chars.FindChar( pChar );
                return( i >= 0 );
        }
        bool IsPartyMaster( const CChar * pChar ) const
        {
                int i = m_Chars.FindChar( pChar );
                return( i == 0 );
        }
    
        bool Disband( CGrayUID uidMaster );
        int AttachChar( CChar * pChar );
        int DetachChar( CChar * pChar );
    
        void SetLootFlag( CChar * pChar, bool fSet );
        bool GetLootFlag( const CChar * pChar );
    
        void MessageAll( CGrayUID uidSrc, const NCHAR * pText, int ilenmsg );
        bool MessageMember( CGrayUID uidDst, CGrayUID uidSrc, const NCHAR * pText, int ilenmsg );
        void SysMessageAll( LPCTSTR pText );
    
        void SendAddList( CGrayUID uid, CChar * pCharDest );
    
        bool RemoveChar( CGrayUID uid, CGrayUID uidAct );
        void AcceptMember( CChar * pChar );
    };
    
    #endif      // _INC_CCLIENT_H