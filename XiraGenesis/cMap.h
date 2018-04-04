#ifndef _CMAP_H_
#define _CMAP_H_

#include "Game.h"

#define MAPTYPE_SURFACE 1
#define MAPTYPE_CAVE	3
#define MAPTYPE_HOUSE	4
#define MAPTYPE_VOID	19

#define PATH_TILES		48

#define PATHTYPE_DIRT	0
#define PATHTYPE_BRICK	1
#define PATHTYPE_WATER	2

class cMap
{
public:
	cMap();
	~cMap();

	vector<cMob> Mobs;
	vector<int> PlayerRefs;
	vector<cBigObject> BigObjects;
	vector<cObj> Objects;

	void PlayerEnteredMap(int PlayerID);
	void PlayerLeftMap(int PlayerID);
	bool RemoveObject(int Ref);
	void AddObject(cObj Object, int X, int Y);
	int FindObjectRef(int ObjID, int X, int Y);

	char Tile[12][11][3];
	int Exit[12][11];
	int ExitDestX[12][11];
	int ExitDestY[12][11];
	string ExitMessage[12][11];

	int WorldMapID;
	int WorldMaxSize;
	int WorldX;
	int WorldY;
	int MapType;
	string Creator;
	int ExitSide;

	void SetType(int Type);

	bool Load(int ID);
	void Save(int ID);

	void AddExit(int X, int Y, string Message, int MapID, int DestX, int DestY);
private:
};

#endif