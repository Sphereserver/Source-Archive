//
// CFragment.cpp
//
#include "graysvr.h"	// predef header.

#define IS_FAIL_OK (g_Serv.IsLoading() && g_World.m_iSaveVersion < 49)

//////////////////////////
// -CFragmentArray

bool CFragmentArray::r_LoadVal( CScript & s )
{
	// Load a ref to a frag.
	// RETURN: false = it failed.

	// "TOWN" and "REGION" are special !

	TCHAR * pszCmd = s.GetArgStr();

	bool fRet = false;
	TCHAR * ppBlocks[64];
	int iArgCount = ParseCmds( pszCmd, ppBlocks, COUNTOF(ppBlocks));

	for ( int i=0; i<iArgCount; i++ )
	{
		pszCmd = ppBlocks[i];

		if ( pszCmd[0] == '-' )
		{
			pszCmd ++;

			// remove a frag or all frags.
			if ( pszCmd[0] == '0' )
			{
				RemoveAll();
				fRet = true;
			}
			else
			{
				CFragmentDef * pFragDef = CFragmentDef::FindFragName( pszCmd, false );
				if ( pFragDef == NULL )
				{
					fRet = false;
					continue;
				}
				int iIndex = RemovePtr(pFragDef);
				fRet = ( iIndex >= 0 );
			}
		}
		else
		{
			// Add a single knowledge fragment or appropriate group item.

			if ( pszCmd[0] == '+' ) pszCmd ++;

			CFragmentDef * pFragDef = CFragmentDef::FindFragName( pszCmd, true );
			fRet = ( pFragDef != NULL );
			if ( !fRet )
			{
				if ( ! IS_FAIL_OK )
				{
					DEBUG_ERR(( "Char Bad Frag Block '%s'\n", pszCmd ));
				}
			}

			// Is it already in the list ?
			else if ( FindPtr(pFragDef) >= 0 )
			{
				DEBUG_ERR(( "Char Duplicate Frag '%s'\n", pszCmd ));
			}
			else
			{
				Add( pFragDef );
			}
		}
	}
	return( fRet );
}

void CFragmentArray::WriteFragList( CGString & sVal ) const
{
	TCHAR szVal[ MAX_SCRIPT_LINE_LEN ];
	int len = 0;
	for ( int j=0;j<GetCount(); j++ )
	{
		len += strcpylen( szVal+len, GetFragName( j ));
		if ( len >= MAX_SCRIPT_LINE_LEN-1 ) 
			break;
		szVal[ len ++ ] = ',';
	}
	if ( len ) len --;
	szVal[ len ] = '\0';
	sVal = szVal;
}

bool CFragmentArray::IsFragIn( const TCHAR * pszKey ) const
{
	CFragmentDef * pFragDef = CFragmentDef::FindFragName( pszKey, false );
	if ( pFragDef == NULL )
		return( false );

	// Is it already in the list ?
	if ( FindPtr(pFragDef) >= 0 )
		return( true );

	return( false );
}

void CFragmentArray::r_Write( CScript & s, const TCHAR * pszKey ) const
{
	for ( int j=0;j<GetCount(); j++ )
	{
		s.WriteKey( pszKey, GetFragName( j ));
	}
}

//////////////////////////////////////////////////////////////////////
// CFragmentDef

CFragmentDef::CFragmentDef( const TCHAR * pName, const TCHAR * pExtra )
{
	ASSERT( pName );
	ASSERT( pName[0] );

	if ( isdigit( pName[0] ))
	{
		// a ref into *SPEE.SCP with NO freindly name.
		m_wIndex = Exp_GetHex( pName );
	}
	else if ( pExtra && isdigit( pExtra[0] ))
	{
		// a ref into *SPEE.SCP with a freindly name.
		m_wIndex = Exp_GetHex( pExtra );
		m_sName = pName;
	}
	else
	{
		// A ref to a file in the Speech directory.
		m_wIndex = 0;	// default.
		if ( pExtra )
			m_sName = pExtra;
		else
			m_sName = pName;

		//DEBUG_MSG(( "Adding frag name '%s'\n", (LPCSTR) m_sName ));

		CFragmentFile * pFragFile = new CFragmentFile( pName );
		ASSERT(pFragFile);
		g_Serv.m_FragFiles.Add( pFragFile );
		m_ScriptLink.SetLinkUnk( pFragFile );
	}

	// Add it to the FragDefs array
	g_Serv.m_FragDefs.Add( this );
}

const TCHAR * CFragmentDef::GetFragName() const
{
	if ( ! m_sName.IsEmpty())	// has a friendly name ?
		return( m_sName );

	if ( IsDefFile())
	{
		// SPEE.SCP file.
		TCHAR * pszTmp = GetTempStr();
		sprintf( pszTmp, "0%x", GetDefIndex() );
		return( pszTmp );
	}

	DEBUG_ERR(( "Unnamed fragment?\n" ));
	return "0";	// this should not happen.
}

bool CFragmentDef::OpenFrag( CScriptLock & s )
{
	// ARGS:
	//  s = a limited life file structure we can use if we have to open this.
	//
	if ( m_ScriptLink.IsLinked())	// has already been loaded previously.
	{
		if ( m_ScriptLink.OpenLinkLock( s ))
		{
			return( true );
		}
		DEBUG_ERR(( "SeekFrag '%s' offset=0%x base=0%x FAILED\n", s.GetFilePath(), m_ScriptLink.GetBadLinkOffset(), GetFragName() ));
		// m_ScriptLink.InitLink();
	}

	if ( m_ScriptLink.IsBadLinked())
	{
		// Did the link fail before ? don't try again.
		return( false );
	}

	CScript * pScript;
	if ( IsDefFile())
	{
		// it is in SPEE.SCP or SPEE2.scp files.
		CGString sSection;
		sSection.Format( "%04X", GetDefIndex() );

		pScript = g_Serv.ScriptLock( s, SCPFILE_SPEE_2, sSection, OF_SORTED | OF_NONCRIT );
		if ( pScript == NULL )
		{
			m_ScriptLink.SetBadLinkOffset( -5 );
			return( false );
		}
	}
	else
	{
		pScript = m_ScriptLink.GetLinkScript();
		if ( pScript == NULL )
			return( false );

		// Look in the default dir first.
		if ( ! s.OpenCopy( *pScript, OF_READ | OF_NONCRIT ))
		{
			// Look in the speech dir.
			CGString sFileName = GetMergedFileName( g_Serv.m_sSpeechBaseDir, pScript->GetFilePath());
			if ( ! s.Open( sFileName, OF_READ | OF_NONCRIT ))
			{
				m_ScriptLink.SetBadLinkOffset( -6 );
				return( false );
			}
			pScript->SetFilePath(sFileName);
		}
	}

	m_ScriptLink.SetLink( pScript, s );
	return( true );
}

CFragmentDef * CFragmentDef::FindFragName( const TCHAR * pszName, bool fAdd )	// static private
{
	// PURPOSE:
	//  Find a frag with this ref name.
	//
	// RETURN: 
	//	pointer to CFragmentDef
	//  NULL = failed

	TCHAR szTmp[MAX_SCRIPT_LINE_LEN];
	strcpy( szTmp, pszName );
	TCHAR * ppCmds[3];
	int iArgs = ParseCmds( szTmp, ppCmds, COUNTOF( ppCmds ), ",:" );

	if ( iArgs <= 0 || ppCmds[0] == NULL || *ppCmds[0] == '\0' )
	{
		DEBUG_ERR(( "BAD Script fragment format %s\n", pszName ));
		return NULL;
	}

	WORD wIndex = 0;
	if ( isdigit( ppCmds[0][0] ))	// SPEE.SCP file.
	{
		wIndex = Exp_GetHex( pszName );
		if ( wIndex == 0 )
			return( NULL );
	}

	// Find it by it's friendly name.
	for ( int i=0; i<g_Serv.m_FragDefs.GetCount(); i++ )
	{
		CFragmentDef * pFragDef = g_Serv.m_FragDefs[i];
		ASSERT(pFragDef);
		if ( ! strcmpi( pFragDef->GetFragName(), ppCmds[0] ))
			return( pFragDef );
		if ( wIndex )
		{
			if ( pFragDef->GetDefIndex() == wIndex )
				return( pFragDef );
		}
	}

	// can't find it ? 
	if ( ! fAdd ) 
	{
		return( NULL );
	}

	// then add it to the list.
	CFragmentDef * pFragNew = new CFragmentDef( ppCmds[0], ppCmds[1] );


	// make sure the frag has a valid file !
	CScriptLock s;
	if ( ! pFragNew->OpenFrag(s))
	{
		// If not then try to prefix "job" or "e" to fix it ?
		if ( IS_FAIL_OK )
		{
			// Just get rid of it.
			g_Serv.m_FragDefs.DeleteOb( pFragNew );
			return( NULL );
		}
	}

	return( pFragNew );
}

