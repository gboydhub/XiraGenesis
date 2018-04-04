#ifndef _SCENE_H_
#define _SCENE_H_

#include "Game.h"

class Scene
{
public:
	Scene() { Host = NULL; RandGen.seed(time(NULL));}
	virtual ~Scene() {};
	virtual void ProcessInput(int KeyCode, bool KeyDown) = 0;
	virtual void Init() = 0;
	virtual bool DoLogic() = 0;
	virtual void Draw() = 0;
	virtual void ResetState() = 0;
	virtual void ProcessConsoleInput() {};
	virtual void ProcessConsoleInput(string Input) {};
	virtual void ProcessPacket(ENetPeer *Peer, ENetPacket *Packet) {};
	virtual void ProcessNetEvent(ENetEvent Event) {}
	virtual void HSTick() {};
	
	void HandleConsoleInput(char Key);
	int GetWordCount(string Str);
	string GetWord(string Str, int Word);
	int ParsePacket(ENetPacket *Packet);

	ENetHost *GetHost();
	
	vector<cObj>	GameItems;
	vector<cMob>	GameMobs;
	void CreateItems();
	void CreateMobs();

protected:
	default_random_engine RandGen;
	int Rand(int Min, int Max);

	string JoinStringArray(string *Array, char Sep, int First, int Last);
	string StrToLower(string Str);
	bool CheckParam(string Str, string Check, int Start);
	bool ShortStrCompare(string FullStr, string StrCmp);
	string GetString(string Str, int Start, int End);
	int GetIntFromPacket(ENetPacket *Packet, int Place);
	void ConsoleDraw(int x, int y, int Lines, int LineOffset);
	void ConsoleWrite(string STR);
	vector<vector<string>> ConsoleLine;
	vector<vector<ALLEGRO_COLOR>> ConsoleLineColors;
	string ConsoleInput;
	string PacketString;

	ENetHost *Host;
};

#endif