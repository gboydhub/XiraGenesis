#include "Game.h"

cPlayer::cPlayer()
{
	Name = "ERROR";
	Password = "";
	Level = 0;
	Experience = 0;
	Moving = 0;
	X = 5;
	Y = 5;
	XOff = 0;
	YOff = 0;
	ID = -1;
	TakingStep = 0;
	UsingSkill = 0;
	ShowWeapon = false;
	AttackWeapon = 0;
	Sex = 0;
	SkinTone = 0;
	MapID = 0;
	Visable = false;
	HairStyle = 0;
	HairR = 0;
	HairG = 0;
	HairB = 0;
	SpriteFrame = 0;
	SpriteCounter = 0;
	SpriteDir = 0;
	ArmsUp = 0;

	Health = 20;
	MaxHealth = 20;
	Magic = 10;
	MaxMagic = 10;
	Energy = 10;
	MaxEnergy = 10;
	Attack = 5;
	MaxAttack = 5;
	Defense = 3;
	MaxDefense = 3;

	MaxInventory = 20;

	cObj NoObj;
	NoObj.ID = -1;
	for(int x = 0; x < MaxInventory; x++)
	{
		Inventory.push_back(NoObj);
	}

	InBattle = false;
	InParty = false;
	PartySize = 0;
	Party[0] = -1;
	Party[1] = -1;
	Party[2] = -1;
}

cPlayer::~cPlayer()
{
}

bool cPlayer::Save()
{
	al_restore_state(&IOState);
	ostringstream FName(ostringstream::out);
	FName << "Data/Players/" << Name << ".xgen";
	ALLEGRO_FILE *fp;
	if((fp = al_fopen(FName.str().c_str(), "wb")) == NULL)
	{
		al_set_physfs_file_interface();
		return false;
	}

	int FID = 1337;
	al_fwrite32be(fp, FID);
	al_fwrite32be(fp, Password.length());
	al_fwrite(fp, Password.c_str(), Password.length());

	al_fwrite32be(fp, Level);
	al_fwrite32be(fp, Experience);
	al_fwrite32be(fp, Sex);
	al_fwrite32be(fp, SkinTone);
	al_fwrite32be(fp, HairStyle);
	al_fwrite32be(fp, HairR);
	al_fwrite32be(fp, HairG);
	al_fwrite32be(fp, HairB);
	al_fwrite32be(fp, MapID);
	al_fwrite32be(fp, X);
	al_fwrite32be(fp, Y);

	for(int x = 0; x < EQUIPSLOT_MAX; x++)
	{
		al_fwrite32be(fp, EquipmentSlot[x].ID);
		al_fwrite32be(fp, EquipmentSlot[x].Type);
		al_fwrite32be(fp, EquipmentSlot[x].Index);
	}

	al_fwrite32be(fp, Inventory.size());
	for(unsigned int x = 0; x < Inventory.size(); x++)
	{
		al_fwrite32be(fp, Inventory[x].ID);
		al_fwrite32be(fp, Inventory[x].Quantity);
	}

	al_fclose(fp);
	al_set_physfs_file_interface();

	return true;
}

bool cPlayer::Load(string PlayerName)
{
	al_restore_state(&IOState);
	ostringstream FName(ostringstream::out);
	FName << "Data/Players/" << PlayerName << ".xgen";
	ALLEGRO_FILE *fp;
	if((fp = al_fopen(FName.str().c_str(), "rb")) == NULL)
	{
		al_set_physfs_file_interface();
		return false;
	}
	
	Name = PlayerName;

	int FID = al_fread32be(fp);
	if(FID != 1337)
	{
		al_set_physfs_file_interface();
		return false;
	}

	int PWLen = al_fread32be(fp);
	char *PW;
	PW = new char [PWLen];
	al_fread(fp, PW, PWLen);
	Password.append(PW, PWLen);
	delete [] PW;

	Level		= al_fread32be(fp);
	Experience	= al_fread32be(fp);
	Sex			= al_fread32be(fp);
	SkinTone	= al_fread32be(fp);
	HairStyle	= al_fread32be(fp);
	HairR		= al_fread32be(fp);
	HairG		= al_fread32be(fp);
	HairB		= al_fread32be(fp);
	MapID		= al_fread32be(fp);
	X			= al_fread32be(fp);
	Y			= al_fread32be(fp);

	for(int x = 0; x < EQUIPSLOT_MAX; x++)
	{
		EquipmentSlot[x].ID = al_fread32be(fp);
		EquipmentSlot[x].Type = al_fread32be(fp);
		EquipmentSlot[x].Index = al_fread32be(fp);
	}

	int ISize = al_fread32be(fp);
	for(int x = 0; x < ISize; x++)
	{
		cObj NewObj;
		int i = al_fread32be(fp);
		int q = al_fread32be(fp);
		NewObj.ID = i;
		NewObj.Quantity = q;
		Inventory.push_back(NewObj);
	}

	al_fclose(fp);
	al_set_physfs_file_interface();

	return true;
}

int cPlayer::GetEmptySlotCount()
{
	int Count = 0;
	for(size_t x = 0; x < Inventory.size(); x++)
	{
		if(Inventory[x].ID < 0)
			Count++;
	}
	return Count;
}

int cPlayer::FindEmptySlot()
{
	for(size_t x = 0; x < Inventory.size(); x++)
	{
		if(Inventory[x].ID < 0)
			return x;
	}
	return -1;
}

int cPlayer::FindObjectRef(int ObjID)
{
	if(ObjID < 0)
		return -1;

	for(unsigned int x = 0; x < Inventory.size(); x++)
	{
		if(Inventory[x].ID == ObjID)
			return x;
	}

	return -1;
}

int cPlayer::FindNotEquippedObjectRef(int ObjID)
{
	bool FoundItem = false;

	if(ObjID < 0)
		return -1;

	for(size_t x = 0; x < Inventory.size(); x++)
	{
		if(Inventory[x].ID == ObjID)
		{
			FoundItem = true;
			if(!IsEquipped(x))
				return x;
		}
	}

	if(FoundItem)
		return -2;

	return -1;
}

bool cPlayer::IsEquipped(int ItemID)
{
	for(int x = 0; x < EQUIPSLOT_MAX; x++)
	{
		if(EquipmentSlot[x].ID == ItemID)
			return true;
	}
	return false;
}