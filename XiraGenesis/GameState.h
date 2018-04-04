#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_

#include "Game.h"

class GameState : public Scene
{
public:
	~GameState();
	void Init();
	void ProcessInput(int KeyCode, bool KeyDown);
	bool DoLogic();
	void Draw();
	void ResetState();
	void ProcessConsoleInput();
	void ProcessNetEvent(ENetEvent Event);
	void SendDataPacket(const char *Data);
	void ProcessDataPacket(ENetPacket *Packet);
	int FindPlayerID(int ID);
	void AddEmptyPlayer(int ID);
	void HSTick();
	void DoGfxEvent(int EventID, int X, int Y, int MapID);
	void ApplyEquipToSprite(ALLEGRO_BITMAP *Sprite, int EquipID);

	void CreateNewMapObj(int Type, int Index, int X, int Y, int ID);
	void DeleteMapObjByID(int ID);

	void ShakeScreen(int Intensity, int Duration);

private:
	ENetPeer *Server;
	ENetAddress Address;
	bool Connected;
	bool LoggedIn;

	vector<cPlayer> PlayerList;
	vector<cGfxEvent> GraphicsEvents;
	vector<int> DelList;
	vector<cParticle*> PEffectList;

	ALLEGRO_MOUSE_STATE MouseState;
	int ConsoleOffset;

	ALLEGRO_BITMAP *PlayerSprites;
	ALLEGRO_BITMAP *HairStyles;
	ALLEGRO_BITMAP *Equipment;
	ALLEGRO_BITMAP *GameBG;
	ALLEGRO_BITMAP *BattleBG;
	ALLEGRO_BITMAP *TileSheet;
	ALLEGRO_BITMAP *WorldMapBMP;
	ALLEGRO_BITMAP *HUDItems;
	ALLEGRO_BITMAP *Objects;
	ALLEGRO_BITMAP *Mobs;
	ALLEGRO_BITMAP *PathsBMP;
	ALLEGRO_BITMAP *MappiesBMP;
	ALLEGRO_BITMAP *BigObjectsBMP;
	ALLEGRO_BITMAP *PEffectsBMP;
	ALLEGRO_BITMAP *LightsBMP;
	ALLEGRO_BITMAP *DayNightLayer;
	ALLEGRO_BITMAP *WeaponsBMP;
	ALLEGRO_BITMAP *MapImage;

	vector<cLight> LightList;
	vector<string> NameList;
	int NextNameID;

	int ShiftMod;

	int WorldMap[61][61];
	cMap Map;

	int GameDay;
	int GameHour;
	int GameMinute;
	ALLEGRO_COLOR GameTimeOverlay;
	void UpdateGameTimeOverlay();

	int CurrentMapID;

	ALLEGRO_BITMAP *BattleSceneBMP;
	void GoToBattleScene();

	int InBattle;
	int BattleSceneCounter;
	float BattleSceneXScale;
	float BattleSceneYScale;
	float BattleSceneAngle;

	int ShakeLife;
	int ShakeIntensity;
	int ScreenOffsetX;
	int ScreenOffsetY;

	void GenerateMapImage();
};

#endif