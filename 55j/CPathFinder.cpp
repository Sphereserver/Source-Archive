//
// CPathFinder.cpp
// Copyright Menace Software (www.menasoft.com).
//
//    -Pretty good working Astar (A*) Pathfinder Algorithm Class.
//    -Based on: The A* Pathfinder base Class (http://home.das-netz.de/pat/a-star.htm) by
//		pat@das-netz.de Germany 25.07.97
//    -Based on SPATH.C http://www-cs-students.stanford.edu/~amitp/Articles/spath.zip

#include "graysvr.h"	// predef header.

// NOTE: This does not work yet !!!

#define POINTNODE long

struct CPathNode
{
	// path node structure
	const CPointMap m_pt;	// What is it's world coords ?
	const POINTNODE m_iNodeNum;	// integer node designation on the map. (equiv to m_pt)
	const int m_iDist;		// Dist from where i want to be.

	long m_f;		// the score for how good a path this is.
	int m_g;
	CPathNode *m_pParent;
	CPathNode *m_pChild[DIR_QTY]; // a node may have up to 8 children.
	CPathNode *m_pNextNode;  // for filing purposes in m_pOpen or m_pClosed

public:
	CPathNode( const CPointMap & pt, int iDist ) :
		m_iNodeNum( pt.GetPointSortIndex()), m_pt(pt), m_iDist(iDist)
	{
		m_pParent= NULL;
		memset(m_pChild,0,sizeof(m_pChild));
		m_pNextNode= NULL;
	}
	void AddChild( CPathNode* pOld )
	{
		int c = 0;
		for ( c = 0; c < DIR_QTY; c++)
		{
			if ( m_pChild[c] == NULL ) // Add Old to the list of pBestNode's Children (or Successors).
			{
				m_pChild[c] = pOld;
				return;
			}
		}
		ASSERT(c<DIR_QTY);
	}

	CPathNode* CheckNodeList(POINTNODE iNodeNum) const
	{
		// Is this m_iNodeNum in the list ?
		// Don't check this node because this node is not really valid.
		CPathNode *pTmp = m_pNextNode;
		while ( pTmp != NULL )
		{
			if ( pTmp->m_iNodeNum == iNodeNum )
				return (pTmp);
			pTmp = pTmp->m_pNextNode;
		}
		return(NULL);
	}

	void FreeNodeList()
	{
		CPathNode *pNode = m_pNextNode;
		while ( pNode != NULL )
		{
			CPathNode *pOldNode = pNode;
			pNode = pNode->m_pNextNode;
			delete pOldNode;
		}
	}
};

struct CPathStack
{      // the stack structure
	CPathNode *m_pNode;
	CPathStack *m_pNextStack;
};

class CPathFinder
{
private:
	CPathNode *m_pOpen;    // the node list pointers
	CPathNode *m_pClosed;
	CPathNode *m_pPath;		// pointer to the best path
	CPathStack m_Stack;

	bool m_fIsPath;	// have i found a path ?

	int m_iWidth; // tilemap data members, need to be initialisize
	int *m_piTileMap;  // pointer to the A* own tilemap data array

private:
	void FreeNodes(void);// frees lists memory

	// The A* Algorithm and it's stuff
	void FindPath( const CPointMap & spt, const CPointMap & dpt );
	CPathNode *ReturnBestNode(void);
	void GenerateSucc( CPathNode *pBestNode, const CPointMap & pt, const CPointMap & dpt );

	void Insert(CPathNode *pSuccessor);
	void PropagateDownChildren( CPathNode *pOld );
	void PropagateDown(CPathNode *pOld);
	void Push(CPathNode *pNode); // stack functions
	CPathNode *Pop(void);

public:

	// Modify only these 3 public member functions to support Your favorite Map
	CPathFinder( int iWidth );
	~CPathFinder();

	bool NewPath(const CPointMap & spt, const CPointMap & dpt);  // Must be called and be true
	// before getting the node entries. It frees the lists,
	// calls ::FindPath() and returns true if path is accessible

	bool GetReachedGoal(void) const; // Call and check this function before using these 3 following
	//void SetPathNextNode(void) { m_pPath=m_pPath->m_pParent; }
	CPointMap GetPathPoint(void) const { return m_pPath->m_pt; }

	int CanMoveHere( const CPointMap & pt ) const // returns 1 = true if we can move on it
	{
		//  NOTE:
		//   if m_piTileMap[equation]==0 it means free so just !(not) it.
		//  RETURN:
		//    1 if tile is free(nothing on it).
		// urn( ! m_piTileMap[ pt..GetPointSortIndex() ] );
		return(1);
	}
};

////////////////////////////////////////////////////////////////////////////////
//                           Constructor Destructor                           //
////////////////////////////////////////////////////////////////////////////////

CPathFinder::CPathFinder( int iWidth )
{
	m_fIsPath = false;
	m_pOpen = NULL;
	m_pClosed = NULL;
	m_pPath = NULL;

	// Sets the A* Tilemap append on CDXMap
	m_iWidth = iWidth+2;    // note the cols should be 2 more than the map size
	int iTotalTiles = m_iWidth*m_iWidth; // to see if we can go in all directions, we don't need a

	// special case to check these. (same for rows)
	m_piTileMap = new int[iTotalTiles];
	ASSERT(m_piTileMap);
}

////////////////////////////////////////////////////////////////////////////////

CPathFinder::~CPathFinder()
{
	FreeNodes();
	if ( m_piTileMap )
		delete [] m_piTileMap;
}

////////////////////////////////////////////////////////////////////////////////
//                             Public Member Functions                        //
////////////////////////////////////////////////////////////////////////////////

bool CPathFinder::NewPath( const CPointMap & spt, const CPointMap & dpt )
{
	FreeNodes(); // clear existing node lists
	if ( CanMoveHere(dpt) &&
		CanMoveHere(spt) &&
		spt.GetDist(dpt))
	{
		FindPath(spt,dpt);
		return (m_fIsPath=true);
	}
	else
		return (m_fIsPath=false);
}

////////////////////////////////////////////////////////////////////////////////

bool CPathFinder::GetReachedGoal(void) const
{
	// check it's return value before getting the node entries
	if ( !m_fIsPath )
		return true;  // this looks a little bit strange
	ASSERT(m_pPath);
	if ( m_pPath->m_pParent != NULL )  // but prevents illegal calls of ::SetPathNextNode()
		return false;             // or ::PathGet*()
	else
		return true;
}

////////////////////////////////////////////////////////////////////////////////
//								      Private Member Functions                        //
////////////////////////////////////////////////////////////////////////////////

void CPathFinder::FreeNodes(void)
{
	if ( m_pOpen != NULL )
	{
		m_pOpen->FreeNodeList();
		delete m_pOpen;
		m_pOpen = NULL;
	}
	if ( m_pClosed != NULL )
	{
		m_pClosed->FreeNodeList();
		delete m_pClosed;
		m_pClosed = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////
//                               A* Algorithm                                 //
////////////////////////////////////////////////////////////////////////////////

void CPathFinder::FindPath( const CPointMap & spt, const CPointMap & dpt )
{
	ASSERT(m_pOpen==NULL);
	m_pOpen = new CPathNode(spt,0);
	ASSERT(m_pOpen);
	ASSERT(m_pClosed==NULL);
	m_pClosed = new CPathNode(spt,0);
	ASSERT(m_pClosed);

	CPathNode *pNode = new CPathNode( dpt, dpt.GetDist(spt));
	ASSERT(pNode);
	pNode->m_g = 0;
	pNode->m_f = pNode->m_iDist;

	m_pOpen->m_pNextNode=pNode;        // make Open List point to first node

	POINTNODE iNodeNumDest = spt.GetPointSortIndex();

	CPathNode *pBestNode;
	while(true)
	{
		pBestNode = ReturnBestNode();
		ASSERT(pBestNode);
		if ( pBestNode->m_iNodeNum == iNodeNumDest)    // if we've found the end, break and finish
			break;

		for ( int i=0; i<DIR_QTY; i++ )
		{
			CPointMap pt( spt );
			pt.Move( (DIR_TYPE) i );
			if ( CanMoveHere( pt ))
			{
				GenerateSucc( pBestNode, pt, spt );
			}
		}
	}

	m_pPath = pBestNode;
}

////////////////////////////////////////////////////////////////////////////////

CPathNode * CPathFinder::ReturnBestNode(void)
{
	ASSERT(m_pOpen);
	CPathNode *pTmp = m_pOpen->m_pNextNode;   // point to first node on m_pOpen

	if ( pTmp == NULL )
	{
		throw -1;
#if 0
		char message[128];
		sprintf(message,"No more nodes on m_pOpen: Perhaps iNodeNum destination not reachable!");
		MessageBox(0, message, "Error A* Pathfinder", MB_OK | MB_ICONERROR);
		PostQuitMessage(0);
#endif
	}

	// Pick Node with lowest f, in this case it's the first node in list
	// because we sort the m_pOpen list wrt lowest f. Call it BESTNODE.
	m_pOpen->m_pNextNode = pTmp->m_pNextNode;    // Make m_pOpen point to nextnode or NULL.

	// Next take BESTNODE (or temp in this case) and put it on m_pClosed

	pTmp->m_pNextNode = m_pClosed->m_pNextNode;
	m_pClosed->m_pNextNode = pTmp;

	return(pTmp);
}

////////////////////////////////////////////////////////////////////////////////

void CPathFinder::GenerateSucc( CPathNode *pBestNode, const CPointMap & pt, const CPointMap & dpt )
{
	int g = pBestNode->m_g + 1;	    // g(pSuccessor)=g(pBestNode)+cost of getting from pBestNode to pSuccessor
	POINTNODE iNodeNum = pt.GetPointSortIndex();  // identification purposes

	CPathNode *pOld = m_pOpen->CheckNodeList(iNodeNum);
	if ( pOld != NULL ) // if equal to NULL then not in m_pOpen list, else it returns the Node in Old
	{
		pBestNode->AddChild(pOld);
		if ( g < pOld->m_g )  // if our new g value is < Old's then reset pOld's parent to point to pBestNode
		{
			pOld->m_pParent = pBestNode;
			pOld->m_g = g;
			pOld->m_f = g + pOld->m_iDist;
		}
		return;
	}

	pOld=m_pClosed->CheckNodeList(iNodeNum);
	if ( pOld != NULL ) // if equal to NULL then not in m_pOpen list, else it returns the Node in Old
	{
		pBestNode->AddChild(pOld);
		if ( g < pOld->m_g )  // if our new g value is < pOld's then reset pOld's parent to point to pBestNode
		{
			pOld->m_pParent = pBestNode;
			pOld->m_g = g;
			pOld->m_f = g + pOld->m_iDist;
			PropagateDown(pOld);  // Since we changed the g value of pOld, we need
			// to propagate this new value downwards, i.e.
			// do a Depth-First traversal of the tree!
		}
		return;
	}

	CPathNode *pSuccessor = new CPathNode( pt, pt.GetDist(dpt));
	ASSERT(pSuccessor);
	pSuccessor->m_pParent = pBestNode;
	pSuccessor->m_g = g;
	pSuccessor->m_f = g + pSuccessor->m_iDist;
	Insert(pSuccessor);     // Insert pSuccessor on m_pOpen list wrt f

	// Add pOld to the list of pBestNode's Children (or Successors).
	pBestNode->AddChild(pSuccessor);
}

////////////////////////////////////////////////////////////////////////////////

void CPathFinder::Insert(CPathNode *pSuccessor)
{
	ASSERT(m_pOpen);
	if ( m_pOpen->m_pNextNode == NULL )
	{
		m_pOpen->m_pNextNode = pSuccessor;
		return;
	}

	// insert into m_pOpen successor wrt f
	int f = pSuccessor->m_f;
	CPathNode *pTmp1 = m_pOpen;
	CPathNode *pTmp2 = m_pOpen->m_pNextNode;

	while ( (pTmp2 != NULL) && (pTmp2->m_f < f) )
	{
		pTmp1 = pTmp2;
		pTmp2 = pTmp2->m_pNextNode;
	}

	pSuccessor->m_pNextNode = pTmp2;
	pTmp1->m_pNextNode = pSuccessor;
}

////////////////////////////////////////////////////////////////////////////////

void CPathFinder::PropagateDownChildren( CPathNode *pOld )
{
	ASSERT(pOld);
	int g = pOld->m_g;            // alias.
	for ( int c = 0; c < DIR_QTY; c++)
	{
		CPathNode *pChild = pOld->m_pChild[c];
		if ( pChild == NULL )   // create alias for faster access.
			break;
		if ( g+1 < pChild->m_g)	 // there are no children, or that the g value of
		{							// the child is equal or better than the cost we're propagating
			pChild->m_g = g+1;
			pChild->m_f = pChild->m_g + pChild->m_iDist;
			pChild->m_pParent = pOld;           // reset parent to new path.
			Push(pChild);                 // Now the pChild's branch need to be
		}     // checked out. Remember the new cost must be propagated down.
	}
}

void CPathFinder::PropagateDown(CPathNode *pOld)
{
	PropagateDownChildren(pOld);
	while ( m_Stack.m_pNextStack != NULL )
	{
		PropagateDownChildren(Pop());
	}
}

////////////////////////////////////////////////////////////////////////////////
//                              STACK FUNCTIONS								  //
////////////////////////////////////////////////////////////////////////////////

void CPathFinder::Push(CPathNode *pNode)
{
	CPathStack *pTmp = new CPathStack;
	ASSERT(pTmp);
	pTmp->m_pNode = pNode;
	pTmp->m_pNextStack = m_Stack.m_pNextStack;
	m_Stack.m_pNextStack = pTmp;
}

CPathNode* CPathFinder::Pop(void)
{
	CPathStack *pTmpStk = m_Stack.m_pNextStack;
	ASSERT(pTmpStk);
	CPathNode *pTmp = pTmpStk->m_pNode;

	m_Stack.m_pNextStack = pTmpStk->m_pNextStack;
	delete pTmpStk;
	return(pTmp);
}

////////////////////////////////////////////////////////////////////////////////

#if 0

void CChar::NPC_FindPath( const CPointMap & pt, int iMaxDist )
{
	// Find the optimal path to this location.
	// Start walking it.
	// ARGS:
	//  iMaxDist = basically how smart we are about fguring out our terrain.


	CPathFinder path( iMaxDist*2, pt );

}

#endif
