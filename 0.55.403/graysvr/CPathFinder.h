//
//	CPathFinder
//		pathfinding algorytm based on AStar (A*) algorithm
//		based on A* Pathfinder (Version 1.71a) by Patrick Lester, pwlester@policyalmanac.org
//
#ifndef PATH_FINDER_H
#define PATH_FINDER_H

#include "../graysvr/graysvr.h"

#define PATH_SIZE UO_MAP_VIEW_SIZE	// limit NPC view by one screen
#define	PATH_OBJS 1					// objects moving at once

class CPathFinder
{
	public:
		#define PATH_NOTFINISHED	0
		#define PATH_NOTSTARTED		0
		#define PATH_FOUND			1
		#define PATH_NONEXISTENT	2
		#define PATH_WALKABLE		0
		#define PATH_UNWALKABLE		1

		CPathFinder(CChar *pChar, CPointMap ptTarget);
		~CPathFinder();

		int FindPath(int pid = 1); // search the path
		void ReadPath(int currentX, int currentY, int pid = 1);	// return next step to m_x/yPath[pid]
		void ReadStep(int step = 1, int pid = 1);	// extract step by number

	protected:
		char m_walkability[PATH_SIZE][PATH_SIZE];
		int m_openList[PATH_SIZE*PATH_SIZE+2];		//1 dimensional array holding ID# of open list items
		int m_whichList[PATH_SIZE+1][PATH_SIZE+1];  //2 dimensional array used to record 
				//	x,y locations of open list
		int m_openX[PATH_SIZE*PATH_SIZE+2];
		int m_openY[PATH_SIZE*PATH_SIZE+2];
				//	x,y locations of parent list
		int m_parentX[PATH_SIZE+1][PATH_SIZE+1];
		int m_parentY[PATH_SIZE+1][PATH_SIZE+1];
				//	consts list
		int m_Fcost[PATH_SIZE*PATH_SIZE+2];		//1d array to store F cost of a cell on the open list
		int m_Gcost[PATH_SIZE+1][PATH_SIZE+1];	//2d array to store G cost for each cell.
		int m_Hcost[PATH_SIZE*PATH_SIZE+2];		//1d array to store H cost of a cell on the open list
		int m_pathLength[PATH_OBJS+1];     //stores length of the found path for critter
		int m_pathLocation[PATH_OBJS+1];   //stores current position along the chosen path for critter		
		int* m_pathBank[PATH_OBJS+1];

		int m_onClosedList;

		CChar		*m_pChar;
		CPointMap	m_ptTarget;

	public:	// path global vars
		int m_pathStatus[PATH_OBJS+1];
		int m_xPath[PATH_OBJS+1];
		int m_yPath[PATH_OBJS+1];

					//	since we are dealing with virtual screen,
					//	we should know the real POS in the world of our window mapping
		int m_realXoffset;
		int m_realYoffset;

	protected:
		void FillMap();	// prepares map with walkable statuses
		int ReadPathX(int pathLocation, int pid = 1);
		int ReadPathY(int pathLocation, int pid = 1);
};

#endif
