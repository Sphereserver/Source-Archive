    // CAtom.h
    //
    
    #ifndef _INC_CATOM_H
    #define _INC_CATOM_H
    
    #include "carray.h"
    
    class CAtomDef : public CGString
    {
    	friend class CAtomRef;
    private:
    	int m_iUseCount;
    public:
    	CAtomDef( LPCTSTR pszStr ) :
    		CGString( pszStr )
    	{
    		m_iUseCount = 1;	
    	}
    };
    
    class CAtomRef
    {
    private:
    	CAtomDef * m_pDef;
    private:
    	void ClearRef();
    public:
    	DWORD GetIndex() const
    	{
    		return( (DWORD)( m_pDef ));
    	}
    	LPCTSTR GetStr() const
    	{
    		if ( m_pDef == NULL )
    			return( NULL );
    		return( *m_pDef );
    	}
    	void SetStr( LPCTSTR pszText );
    	void Copy( const CAtomRef & atom );
    
    	bool IsValid() const
    	{
    		return( m_pDef != NULL && ! m_pDef->IsEmpty());
    	}
        operator LPCTSTR() const       // as a C string
        {
    		return(  GetStr());
        }
    	bool operator == ( const CAtomRef & atom ) const
    	{
    		return( atom.m_pDef == m_pDef );
    	}
    	CAtomRef & operator = ( const CAtomRef & atom )
    	{
    		// Copy operator is quick.
    		Copy( atom );
    		return( *this );
    	}
    
    	~CAtomRef()
    	{
    		ClearRef();
    	}
    	CAtomRef()
    	{
    		m_pDef = NULL;
    	}
    	CAtomRef( const CAtomRef & atom )
    	{
    		m_pDef = NULL;
    		Copy( atom );
    	}
    	CAtomRef( LPCTSTR pszName )
    	{
    		m_pDef = NULL;
    		SetStr(pszName);
    	}
    };
    
    #endif // _INC_CATOM_H