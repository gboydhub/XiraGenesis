#include "Game.h"

cMap::cMap()
{
	for(int x = 0; x < 12; x++)
	{
		for(int y = 0; y < 11; y++)
		{
			Tile[x][y][0] = 1;
			Tile[x][y][1] = 0;
			Tile[x][y][2] = 0;
			Exit[x][y] = -1;
			ExitDestX[x][y] = 0;
			ExitDestY[x][y] = 0;
			ExitMessage[x][y] = "";
		}
	}
	for(int x = 0; x < 12; x++)
	{
		Tile[x][0][0] = 0;
		Tile[x][10][0] = 0;
	}
	for(int x = 1; x < 10; x++)
	{
		Tile[0][x][0] = 0;
		Tile[11][x][0] = 0;
	}
	WorldMapID = -1;
	WorldMaxSize = 0;
	WorldX = 0;
	WorldY = 0;
	ExitSide = 0;
	MapType = MAPTYPE_SURFACE;
	Creator = "NA";
}

void cMap::SetType(int Type)
{
	MapType = Type;
	for(int x = 1; x < 11; x++)
	{
		for(int y = 1; y < 10; y++)
		{
			Tile[x][y][0] = Type;
		}
	}

	for(int y = 2; y < 9; y++)
	{
		if(Tile[0][y][0] > 0)
			Tile[0][y][0] = Type;
		if(Tile[11][y][0] > 0)
			Tile[11][y][0] = Type;
	}
	for(int x = 2; x < 10; x++)
	{
		if(Tile[x][0][0] > 0)
			Tile[x][0][0] = Type;
		if(Tile[x][10][0] > 0)
			Tile[x][10][0] = Type;
	}
}

void cMap::AddExit(int X, int Y, string Message, int MapID, int DestX, int DestY)
{
	Exit[X][Y] = MapID;
	ExitMessage[X][Y] = Message;
	ExitDestX[X][Y] = DestX;
	ExitDestY[X][Y] = DestY;
}

bool cMap::Load(int ID)
{
	al_restore_state(&IOState);
	ostringstream FName(ostringstream::out);
	FName << "Data/Maps/" << ID << ".xgen";
	ALLEGRO_FILE *fp;
	fp = al_fopen(FName.str().c_str(), "rb");
	if(fp == NULL)
	{
		al_set_physfs_file_interface();
		return false;
	}

	int FID = al_fread32be(fp);
	if(FID != 1337)
	{
		al_set_physfs_file_interface();
		return false;
	}
	int NameLen = al_fread32be(fp);
	char *CName;
	CName = new char[NameLen];
	al_fread(fp, CName, NameLen);
	Creator.append(CName, NameLen);
	delete [] CName;

	WorldMapID = al_fread32be(fp);
	WorldMaxSize = al_fread32be(fp);
	WorldX = al_fread32be(fp);
	WorldY = al_fread32be(fp);
	MapType = al_fread32be(fp);
	ExitSide = al_fread32be(fp);

	for(int z = 0; z < 3; z++)
	{
		for(int x = 0; x < 12; x++)
		{
			for(int y = 0; y < 11; y++)
			{
				Tile[x][y][z] = al_fgetc(fp);
			}
		}
	}

	for(int x = 0; x < 12; x++)
	{
		for(int y = 0; y < 11; y++)
		{
			Exit[x][y] = al_fread32be(fp);
			ExitDestX[x][y] = al_fread32be(fp);
			ExitDestY[x][y] = al_fread32be(fp);
		}
	}

	for(int x = 0; x < 12; x++)
	{
		for(int y = 0; y < 11; y++)
		{
			int MSize = al_fread32be(fp);
			if(MSize > 0)
			{
				char *GetMsg;
				GetMsg = new char[MSize];
				al_fread(fp, GetMsg, MSize);
				ExitMessage[x][y].append(GetMsg, MSize);
				delete [] GetMsg;
			}
		}
	}

	int OSize = al_fread32be(fp);
	for(int x = 0; x < OSize; x++)
	{
		cObj NewObj;
		NewObj.ID = al_fread32be(fp);
		NewObj.Index = al_fread32be(fp);
		NewObj.Quantity = al_fread32be(fp);
		NewObj.Type = al_fread32be(fp);
		NewObj.X = al_fread32be(fp);
		NewObj.Y = al_fread32be(fp);
		Objects.push_back(NewObj);
	}

	al_fclose(fp);
	al_set_physfs_file_interface();
	return true;
}

void cMap::Save(int ID)
{
	al_restore_state(&IOState);
	ostringstream FName(ostringstream::out);
	FName << "Data/Maps/" << ID << ".xgen";
	ALLEGRO_FILE *fp;
	fp = al_fopen(FName.str().c_str(), "wb");

	int FID = 1337;
	al_fwrite32be(fp, FID);

	int NameLen = Creator.length();
	al_fwrite32be(fp, NameLen);
	al_fwrite(fp, Creator.c_str(), Creator.length());

	al_fwrite32be(fp, WorldMapID);
	al_fwrite32be(fp, WorldMaxSize);
	al_fwrite32be(fp, WorldX);
	al_fwrite32be(fp, WorldY);
	al_fwrite32be(fp, MapType);
	al_fwrite32be(fp, ExitSide);

	for(int z = 0; z < 3; z++)
	{
		for(int x = 0; x < 12; x++)
		{
			for(int y = 0; y < 11; y++)
			{
				al_fputc(fp, Tile[x][y][z]);
			}
		}
	}

	for(int x = 0; x < 12; x++)
	{
		for(int y = 0; y < 11; y++)
		{
			al_fwrite32be(fp, Exit[x][y]);
			al_fwrite32be(fp, ExitDestX[x][y]);
			al_fwrite32be(fp, ExitDestY[x][y]);
		}
	}

	for(int x = 0; x < 12; x++)
	{
		for(int y = 0; y < 11; y++)
		{
			int MSize = ExitMessage[x][y].length();
			al_fwrite32be(fp, MSize);
			if(MSize > 0)
				al_fwrite(fp, ExitMessage[x][y].c_str(), ExitMessage[x][y].length());
		}
	}

	al_fwrite32be(fp, Objects.size());
	for(unsigned int x = 0; x < Objects.size(); x++)
	{
		al_fwrite32be(fp, Objects[x].ID);
		al_fwrite32be(fp, Objects[x].Index);
		al_fwrite32be(fp, Objects[x].Quantity);
		al_fwrite32be(fp, Objects[x].Type);
		al_fwrite32be(fp, Objects[x].X);
		al_fwrite32be(fp, Objects[x].Y);
	}

	al_fclose(fp);
	al_set_physfs_file_interface();
}

cMap::~cMap()
{
}

bool cMap::RemoveObject(int Ref)
{
	if(Ref >= (int)Objects.size())
		return false;

	Objects.erase(Objects.begin() + Ref);
	return true;
}

void cMap::AddObject(cObj Object, int X, int Y)
{
	Object.X = X;
	Object.Y = Y;
	Objects.push_back(Object);
}

int cMap::FindObjectRef(int ObjID, int X, int Y)
{
	for(unsigned int x = 0; x < Objects.size(); x++)
	{
		if(Objects[x].X == X && Objects[x].Y == Y)
		{
			if(Objects[x].ID == ObjID || ObjID == -8)
				return x;
		}
	}

	return -1;
}

//TODO: Store IDs, not Refs
void cMap::PlayerEnteredMap(int PlayerID)
{
	PlayerRefs.push_back(PlayerID);
}

void cMap::PlayerLeftMap(int PlayerID)
{
	for(size_t x = 0; x < PlayerRefs.size(); x++)
	{
		if(PlayerID == PlayerRefs[x])
		{
			PlayerRefs.erase(PlayerRefs.begin() + x);
			return;
		}
	}
}