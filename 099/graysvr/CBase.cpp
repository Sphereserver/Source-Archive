    // CBase.cpp
    // Copyright Menace Software (www.menasoft.com).
    // base classes.
    //
    #include "graysvr.h"        // predef header.
    
    ///////////////////////////////////////////////////////////////
    // -CBaseBaseDef
    
    // 
    
    enum OBC_TYPE
    {
        OBC_ARMOR,
        OBC_ATT,
        OBC_BASEID,
        OBC_CAN,
        OBC_DAM,
        OBC_DEFNAME,
        OBC_DEFNAME2,
        OBC_HEIGHT,
        OBC_INSTANCES,
        OBC_NAME,
        OBC_RESOURCES,
        OBC_QTY,
    };
    
    LPCTSTR const CBaseBaseDef::sm_szLoadKeys =
    {
        "ARMOR",
        "ATT",
        "BASEID",
        "CAN",
        "DAM",
        "DEFNAME",
        "DEFNAME2",
        "HEIGHT",
        "INSTANCES",
        "NAME",
        "RESOURCES",
        NULL,
    };
    
    void CBaseBaseDef::r_DumpLoadKeys( CTextConsole * pSrc )
    {
        r_DumpKeys(pSrc,sm_szLoadKeys);
    }
    
    bool CBaseBaseDef::r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pChar )
    {
        switch ( FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
        {
        case OBC_ARMOR:
        case OBC_ATT:   // from script
        case OBC_DAM:
                sVal.Format( "%d,%d", m_attackBase, m_attackBase+m_attackRange );
                return( true );
        case OBC_BASEID:
                sVal = g_Cfg.ResourceGetName( GetResourceID());
                return( true );
        case OBC_CAN:
                sVal.FormatHex( m_Can );
                return( true );
        case OBC_HEIGHT:
                sVal.FormatVal( GetHeight());
                return( true );
        case OBC_INSTANCES:
                sVal.FormatVal( GetRefInstances());
                return( true );
        case OBC_NAME:
                sVal = GetName();
                return( true );
        case OBC_RESOURCES:
                {
                        TCHAR szTmp;
                        m_BaseResources.WriteKeys( szTmp );
                        sVal = szTmp;
                }
                return( true );
        }
        return( false );
    }
    
    bool CBaseBaseDef::r_LoadVal( CScript & s )
    {
        switch ( FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
        {
        case OBC_ARMOR:
        case OBC_ATT:   //from script
        case OBC_DAM:
                {
                        int piVal;
                        int iQty = Str_ParseCmds( s.GetArgStr(), piVal, COUNTOF(piVal));
                        m_attackBase = piVal;
                        if ( iQty > 1 )
                        {
                                m_attackRange = piVal - m_attackBase;
                        }
                        else
                        {
                                m_attackRange = 0;
                        }
                }
                return( true );
        case OBC_BASEID:
                return( false );
        case OBC_CAN:
                m_Can = s.GetArgVal() | ( m_Can & ( CAN_C_INDOORS|CAN_C_EQUIP|CAN_C_USEHANDS|CAN_C_NONHUMANOID ));
                return( true );
    
        case OBC_DEFNAME:
        case OBC_DEFNAME2:
                return SetResourceName( s.GetArgStr());
    
        case OBC_INSTANCES:
                return( false );
        case OBC_NAME:
                SetTypeName( s.GetArgStr());
                return( true );
    
        case OBC_RESOURCES:
                m_BaseResources.Load( s.GetArgStr());
                return( true );
        }
        return( CScriptObj::r_LoadVal(s));
    }
    
    void CBaseBaseDef::CopyBasic( const CBaseBaseDef * pBase )
    {
        // do not copy CResourceLink
    
        if ( m_sName.IsEmpty()) // Base type name. should set this itself most times. (don't overwrite it!)
                m_sName = pBase->m_sName;
    
        m_wDispIndex = pBase->m_wDispIndex;
    
        m_Height = pBase->m_Height;
        // m_BaseResources = pBase->m_BaseResources;    // items might not want this.
        m_attackBase = pBase->m_attackBase;
        m_attackRange = pBase->m_attackRange;
        m_Can = pBase->m_Can;
    }
    
    void CBaseBaseDef::CopyTransfer( CBaseBaseDef * pBase )
    {
        CResourceLink::CopyTransfer( pBase );
    
        m_sName = pBase->m_sName;
        m_BaseResources = pBase->m_BaseResources;
    
        CopyBasic( pBase );
    }