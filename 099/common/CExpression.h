    // CExpression.h
    // Copyright Menace Software (www.menasoft.com).
    //
    
    #ifndef _INC_CEXPRSSION_H
    #define _INC_CEXPRSSION_H
    #if _MSC_VER >= 1000
    #pragma once
    #endif // _MSC_VER >= 1000
    
    #include "catom.h"
    
    #define _ISCSYMF(ch) ( isalpha(ch) || (ch)=='_')	// __iscsym or __iscsymf
    #define _ISCSYM(ch) ( isalnum(ch) || (ch)=='_')	// __iscsym or __iscsymf
    
    class CVarDefBase : public CMemDynamic	// A variable from GRAYDEFS.SCP or other.
    {
    	// Similar to CScriptKey
    private:
    #define EXPRESSION_MAX_KEY_LEN SCRIPT_MAX_SECTION_LEN
    	const CAtomRef m_aKey;	// the key for sorting/ etc.
    public:
    	CAtomRef GetKey() const
    	{
    		return( m_aKey );
    	}
    	CVarDefBase( CAtomRef Key ) :
    		m_aKey( Key )
    	{
    	}
    	virtual LPCTSTR GetValStr() const = 0;
    	virtual int GetValNum() const = 0;
    	virtual CVarDefBase * CopySelf() const = 0;
    };
    
    class CVarDefNum : public CVarDefBase
    {
    	// Simple number equiv.
    protected:
    	DECLARE_MEM_DYNAMIC;
    private:
    	int m_iVal;	// the assigned value.
    public:
    	int GetValNum() const
    	{
    		return( m_iVal );
    	}
    	void SetValNum( int iVal )
    	{
    		m_iVal = iVal;
    	}
    	LPCTSTR GetValStr() const;
    
    #if 0
    	bool r_LoadVal( CScript & s )
    	{
    		SetValNum( s.GetArgVal());
    		return( true );
    	}
    	bool r_WriteVal( LPCTSTR pKey, CGString & sVal, CTextConsole * pSrc = NULL )
    	{
    		UNREFERENCED_PARAMETER(pKey);
    		UNREFERENCED_PARAMETER(pSrc);
    		sVal.FormatVal( GetValNum());
    		return( true );
    	}
    #endif
    
    	virtual CVarDefBase * CopySelf() const
    	{
    		return new CVarDefNum( GetKey(), m_iVal );
    	}
    	CVarDefNum( CAtomRef Key, int iVal ) :
    		CVarDefBase( Key ),
    		m_iVal( iVal )
    	{
    	}
    	CVarDefNum( CAtomRef Key ) :
    		CVarDefBase( Key )
    	{
    		m_iVal = 0;
    	}
    };
    
    class CVarDefStr : public CVarDefBase
    {
    protected:
    	DECLARE_MEM_DYNAMIC;
    private:
    	CGString m_sVal;	// the assigned value. (What if numeric?)
    public:
    	LPCTSTR GetValStr() const
    	{
    		return( m_sVal );
    	}
    	int GetValNum() const;
    	void SetValStr( LPCTSTR pszVal )
    	{
    		m_sVal.Copy( pszVal );
    	}
    
    #if 0
    	bool r_LoadVal( CScript & s )
    	{
    		SetValStr( s.GetArgStr());
    		return( true );
    	}
    	bool r_WriteVal( LPCTSTR pKey, CGString & sVal, CTextConsole * pSrc = NULL )
    	{
    		UNREFERENCED_PARAMETER(pKey);
    		UNREFERENCED_PARAMETER(pSrc);
    		sVal = GetValStr();
    		return( true );
    	}
    #endif
    
    	virtual CVarDefBase * CopySelf() const
    	{
    		return new CVarDefStr( GetKey(), m_sVal );
    	}
    	CVarDefStr( CAtomRef Key, LPCTSTR pszVal ) :
    		CVarDefBase( Key ),
    		m_sVal( pszVal )
    	{
    	}
    	CVarDefStr( CAtomRef Key ) :
    		CVarDefBase( Key )
    	{
    	}
    };
    
    struct CVarDefArray : public CGObSortArray< CVarDefBase *, CAtomRef >
    {
    	// Sorted array
    protected:
    	int CompareKey( CAtomRef Key, CVarDefBase * pVar ) const;
    	int Add( CVarDefBase * pVar );
    public:
    	void Copy( const CVarDefArray * pArray );
    
    	CVarDefArray & operator = ( const CVarDefArray & array )
    	{
    		Copy( &array );
    		return( *this );
    	}
    
    	int FindValNum( int iVal ) const;
    	int FindValStr( LPCTSTR pVal ) const;
    
    	// Manipulate the list of Vars
    	CVarDefBase * FindKeyPtr( CAtomRef Key ) const;	// CAtomRef Key
    	LPCTSTR FindKeyStr( CAtomRef Key ) const;
    	int SetNumNew( CAtomRef Key, int iVal );
    	int SetNum( CAtomRef Key, int iVal );
    	int SetStr( CAtomRef Key, LPCTSTR pszVal );
    
    	CVarDefBase * GetParseKey( LPCTSTR & pArgs ) const;
    	bool GetParseVal( LPCTSTR & pArgs, long * plVal ) const;
    
    	bool r_LoadVal( CScript & s )
    	{
    		return SetStr( s.GetKey(), s.GetArgStr()) ? true : false;
    	}
    	void r_WriteKeys( CScript & s )
    	{
    		// Write with no prefix.
    		for ( int i=GetCount(); --i >= 0; )
    		{
    			const CVarDefBase * pVar = GetAt(i);
    			s.WriteKey( pVar->GetKey(), pVar->GetValStr());
    		}
    	}
    	void r_WriteTags( CScript & s );
    	void DumpKeys( CTextConsole * pSrc, LPCTSTR pszPrefix = NULL );
    };
    
    extern class CExpression
    {
    public:
    	CVarDefArray m_VarDefs;	 // Global Defined variables in sorted order.
    public:
    	// Evaluate using the stuff we know.
    	int GetSingle( LPCTSTR & pArgs );
    	int GetVal( LPCTSTR & pArgs );
    	int GetValMath( int lVal, LPCTSTR & pExpr );
    	int GetRangeVals( LPCTSTR & pExpr, int * piVals, int iMaxQty );
    	int GetRange( LPCTSTR & pArgs );
    
    	CExpression();
    	~CExpression();
    
    } g_Exp;
    
    // Numeric formulas
    extern int Calc_GetRandVal( int iqty );
    extern int Calc_GetLog2( UINT iVal );
    extern int Calc_GetSCurve( int iValDiff, int iVariance );
    extern int Calc_GetBellCurve( int iValDiff, int iVariance );
    
    extern DWORD ahextoi( LPCTSTR pArgs ); // Convert hex string to integer
    
    #define Exp_GetSingle( pa ) g_Exp.GetSingle( pa )
    #define Exp_GetVal( pa )	g_Exp.GetVal( pa )
    #define Exp_GetRange( pa )	g_Exp.GetRange( pa )
    
    inline int CVarDefStr::GetValNum() const
    {
    	LPCTSTR pszStr = m_sVal;
    	return( Exp_GetVal(pszStr));
    }
    
    inline LPCTSTR CVarDefNum::GetValStr() const
    {
    	TCHAR * pszTmp = Str_GetTemp();
    	sprintf( pszTmp, _TEXT("0%x"), m_iVal );
    	return( pszTmp );
    }
    
#endif	// _INC_CEXPRSSION_H