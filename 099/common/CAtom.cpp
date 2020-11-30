    // CAtom.cpp
    //
    
    #ifdef GRAY_MAKER
    #include "../graymaker/stdafx.h"
    #include "../graymaker/graymaker.h"
    #elif defined(GRAY_MAP)
    #include "../graymap/stdafx.h"
    #include "../graymap/graymap.h"
    #elif defined(GRAY_AGENT)
    #include "../grayagent/stdafx.h"
    #include "../grayagent/grayagent.h"
    #elif defined(_RIFF_PAD)
    #include "../riffpad/stdafx.h"
    #else
    #include "graycom.h"
    #endif
    
    #include "catom.h"
    
    struct CAtomManager : public CGObSortArray < CAtomDef *, LPCTSTR >
    {
    	// Create an alpha sorted string lookup table. CASE IGNORED !
    	int CompareKey( LPCTSTR pszKey, CAtomDef * pVal ) const
    	{
    		ASSERT(pszKey);
    		ASSERT(pVal);
    		return( strcmpi( pszKey, * ( static_cast <CGString*>( pVal ))));
    	}
    };
    
    static CAtomManager g_AtomManager;
    
    //*********************************
    // CAtomRef
    
    void CAtomRef::ClearRef()
    {
    	if ( m_pDef )
    	{
    		DEBUG_CHECK(m_pDef->m_iUseCount);
    		if ( ! --m_pDef->m_iUseCount )
    		{
    			g_AtomManager.DeleteOb(m_pDef);
    		}
    		m_pDef = NULL;
    	}
    }
    
    void CAtomRef::Copy( const CAtomRef & atom )
    {
    	// Copy's are fast.
    	if ( m_pDef == atom.m_pDef )
    		return;
    	ClearRef();
    	m_pDef = atom.m_pDef;
    	ASSERT(m_pDef);
    	m_pDef->m_iUseCount++;
    }
    
    void CAtomRef::SetStr( LPCTSTR pszText )
    {
    	ClearRef();
    	if ( pszText == NULL )
    		return;
    
    	// Find it in the atom table first.
    	int iCompareRes;
    	int index = g_AtomManager.FindKeyNear( pszText, iCompareRes );
    	if ( !iCompareRes )
    	{
    		// already here just increment useage.
    		m_pDef = g_AtomManager.GetAt( index );
    		ASSERT(m_pDef);
    		m_pDef->m_iUseCount++;
    	}
    	else
    	{
    		// Insertion sort.
    		m_pDef = new CAtomDef( pszText );
    		ASSERT(m_pDef);
    		g_AtomManager.AddPresorted( index, iCompareRes, m_pDef );
    	}
    }