#ifndef _SERVERSTATE_H_
#define _SERVERSTATE_H_

#include "Game.h"
#include "BuildItemIDs.h"

class ServerState : public Scene
{
public:
	~ServerState();
	void Init();
	void ProcessInput(int KeyCode, bool KeyDown);
	bool DoLogic();
	void Draw();
	void ResetState();
	void ProcessConsoleInput();
	void ProcessConsoleInput(string Input);
	void ProcessNetEvent(ENetEvent Event);
	void ProcessRecievedPacket(ENetPeer *From);
	void ProcessDataPacket(ENetPeer *From, ENetPacket *Packet);
	void SendDataPacket(ENetPeer *To, const char *Data, int Size);
	void SendIntPacket(ENetPeer *To, const int *Data, int Size);
	void SendMessage(ENetPeer *To, string Message);
	void SendPlayerMap(ENetPeer *To, int MapID);
	void CreateExit(int PID, string SuccessMessage);
	void SendGameTime();
	int FindPlayerID(ENetPeer *Peer);
	int FindPlayerID(string Name);
	int FindPlayerID(int ID);
	void HSTick();
	void TerraformLandscape(int PlayerID, int MapType);

	int FindWorldMapID(int X, int Y, int WorldID);

private:
	ENetAddress Address;
	vector<cPlayer> PlayerList;
	vector<cMap>	Maps;
	vector<cCommand> CommandList;
	int AssignedIDs;

	vector<vector<int>> WorldRefs;


	//Network commands
	void DisconnectPlayer(int PlayerID);
	void SendMapObjects(ENetPeer *To);
	void SendMapObjects(int MapID);
	void SendUpdateEquip(ENetPeer *To, int PlayerID);
	void SendPlayerWorldMap(ENetPeer *To, int WorlID, int CenterX, int CenterY);
	void SendRoomDesc(ENetPeer *To, int MapID);
	void SendAddPlayer(ENetPeer *To, int PlayerID);
	void SendRemovePlayer(ENetPeer *To, int PlayerID);
	void SendPlayerLoc(ENetPeer *To, int PlayerID);
	void SendPlayerInfo(ENetPeer *To, int PlayerID);
	void TeleportPlayer(int PlayerID, int X, int Y, int Map, string Message);
	void SendAddMob(int PlayerID, cMob Mob);
	void SendMobLoc(int PlayerID, int MobRef, int X, int Y);
	void SendRemoveMob(int PlayerID, int MobRef);
	void SendClearMobs(int PlayerID);
	void SendGfxEvent(int MapID, int EventID, int X, int Y);
	void SendPlayerUsedSkill(int PlayerID, int ShowWeapon);
	void SendPlayerClearGfx(int PlayerID);
	void SendMapBigObjects(int MapID);
	void SendAddName(ENetPeer *To, int PlayerID);

	void PCEquipItem(int PlayerID, int EquipSlot, int InventorySlot);
	int PCUnequipItem(int PlayerID, int EquipSlot);

	//User Commands
	void UserCmd_Help(int PlayerID);
	void UserCmd_Say(int PlayerID);
	void UserCmd_Whois(int PlayerID);
	void UserCmd_Terraform(int PlayerID);
	void UserCmd_Build(int PlayerID);
	void UserCmd_Pickup(int PlayerID);
	void UserCmd_Drop(int PlayerID);
	void UserCmd_Forge(int PlayerID);
	void UserCmd_Inventory(int PlayerID);
	void UserCmd_Equip(int PlayerID);
	void UserCmd_UnEquip(int PlayerID);
	void UserCmd_Summon(int PlayerID);
	void UserCmd_Join(int PlayerID);
	void UserCmd_Leave(int PlayerID);
	void UserCmd_PartyInfo(int PlayerID);
	void UserCmd_Quit(int PlayerID);
	void UserCmd_Time(int PlayerID);

	void UserCmd_Battle(int PlayerID);

	//Local commands
	int CheckBlocking(int X, int Y, int MapID);
	bool CheckOverlay(int X, int Y, int MapID);
	void CreateCave(int PID, int Direction);
	void CreateHouse(int PID, int Direction);
	void CreateForge(int PID, int Direction);
	void CreatePath(int MapID, int X, int Y);
	void BuildItem(int PlayerID, int ItemID, int Dir);
	void FixPathTile(int MapID, int X, int Y, int PathID);
	void SaveMaps();
	void LoadMaps();
	bool CheckForExit(int MapID, int Direction);
	int GetObjectID(string Name);
	void PlayerDropObject(int PlayerID, int ObjRef);
	int CheckParamForItem(string Str, int Start);
	void GenerateBattleMobs(int PlayerID, cMob Mob, int NumberMobs);
	int GetPlayerDir(int PlayerID, int MapID, int SourceX, int SourceY, bool CheckBlock, bool StepAround);
	int GetPlayerDir(int PlayerID, int MapID, int SourceX, int SourceY);
	void CreateCommands();
	int FindPlayerObjRefByName(int PlayerID, string Str, int Start);

	//Battle commands
	void PlayerGotoBattle(int PlayerID, int BattleBG);
	void PlayerExitBattle(int PlayerID);

	void PlayerBattleRun(int PlayerID);
	void PlayerBattleAttack(int PlayerID);
	void PlayerBattleDefend(int PlayerID);
	
	ALLEGRO_BITMAP *ServerBG;
	ALLEGRO_BITMAP *HUDImage;
	ALLEGRO_MOUSE_STATE MouseState;

	int NameOffset;
	int ConsoleOffset;

	int GameDay;
	int GameHour;
	int GameMinute;

	int NeedsSave;

	bool AssignedIDPool[64];
};

#endif