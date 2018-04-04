#ifndef _CPLAYER_H_
#define _CPLAYER_H_

#include "Game.h"

#define EQUIPSLOT_ARMOR		0
#define EQUIPSLOT_LEGS		1
#define EQUIPSLOT_BOOTS		2
#define EQUIPSLOT_HELM		3
#define EQUIPSLOT_MAX		4

struct sInvObject
{
	int ObjID;
	int Quantity;
};

class cPlayer
{
public:
	cPlayer();
	~cPlayer();

	bool Save();
	bool Load(string PlayerName);

	bool IsEquipped(int ItemID);

	ENetPeer *Peer;

	string Name;
	string Password;

	vector<cObj> Inventory;
	int MaxInventory;

	int GetEmptySlotCount();
	int FindEmptySlot();

	cObj EquipmentSlot[EQUIPSLOT_MAX];

	int FindObjectRef(int ObjID);
	int FindNotEquippedObjectRef(int ObjID);

	int ArmsUp;
	int AttackWeapon;
	bool ShowWeapon;

	int ID;

	int Moving;
	int TakingStep;
	int UsingSkill;
	
	int Level;
	int Experience;
	int Health;
	int MaxHealth;
	int Magic;
	int MaxMagic;
	int Energy;
	int MaxEnergy;
	int Attack;
	int MaxAttack;
	int Defense;
	int MaxDefense;
	
	int Sex;
	int SkinTone;
	int HairStyle;
	int HairR;
	int HairG;
	int HairB;

	int Party[3];
	int PartySize;
	bool InParty;

	bool Visable;

	int X;
	int Y;
	int XOff;
	int YOff;
	int MapID;

	int SpriteFrame;
	int SpriteCounter;
	int SpriteDir;

	bool InBattle;
	vector<cMob> BattleMob;
};

#endif