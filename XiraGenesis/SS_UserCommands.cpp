#include "Game.h"

//TODO: Come up with a way to update equipped object references when inventory changes.

void ServerState::UserCmd_Help(int PlayerID)
{
	if(GetWordCount(PacketString) > 1)
	{
		for(size_t x = 0; x < CommandList.size(); x++)
		{
			if(CheckParam(StrToLower(CommandList[x].Name), PacketString, 1))
			{
				OSStr(Msg);
				Msg << "Command: <C>" << CommandList[x].Name;
				SendMessage(PlayerList[PlayerID].Peer, Msg.str());
				for(size_t c = 0; c < CommandList[x].LongDesc.size(); c++)
				{
					SendMessage(PlayerList[PlayerID].Peer, CommandList[x].LongDesc[c]);
				}
				return;
			}
		}
		SendMessage(PlayerList[PlayerID].Peer, "Unknown help <Y>topic<W>.");
	}
	else
	{
		OSStr(Msg);
		bool HasGeneralCmd = false;
		bool HasSkillCmd = false;

		Msg << "<C>Commands <W>for <P>Xira Genesis<W>:";
		SendMessage(PlayerList[PlayerID].Peer, Msg.str());

		for(size_t x = 0; x < CommandList.size(); x++)
		{
			if(CommandList[x].Type == CMDTYPE_GENERAL)
			{
				Msg.str("");
				Msg << "<C>" << CommandList[x].Name << " <W>- " << CommandList[x].ShortDesc;
				SendMessage(PlayerList[PlayerID].Peer, Msg.str());
				HasGeneralCmd = true;
			}
		}
		if(!HasGeneralCmd)
			SendMessage(PlayerList[PlayerID].Peer, "<R>None");

		SendMessage(PlayerList[PlayerID].Peer, "  ");
		
		SendMessage(PlayerList[PlayerID].Peer, "Available <G>skills <W>are:");
		for(size_t x = 0; x < CommandList.size(); x++)
		{
			if(CommandList[x].Type == CMDTYPE_SKILL)
			{
				Msg.str("");
				Msg << "<C>" << CommandList[x].Name << " <W>- " << CommandList[x].ShortDesc;
				SendMessage(PlayerList[PlayerID].Peer, Msg.str());
				HasSkillCmd = true;
			}
		}
		if(!HasSkillCmd)
			SendMessage(PlayerList[PlayerID].Peer, "<R>None");
	}
}

void ServerState::UserCmd_Say(int PlayerID)
{
	ostringstream Message(ostringstream::out);
	Message << "<C>" << PlayerList[PlayerID].Name << ": <W>";
	for(int x = 0; x < GetWordCount(PacketString) - 1; x++)
	{
		Message << GetWord(PacketString, x+1) << " ";
	}
	SendMessage(NULL, Message.str());
}

void ServerState::UserCmd_Whois(int PlayerID)
{
	for(unsigned int x = 0; x < PlayerList.size(); x++)
	{
		ostringstream OS(ostringstream::out);
		OS << PlayerList[x].Name << " <P>[Lv <C>" << (int)PlayerList[x].Level << "<P>]";
		SendMessage(PlayerList[PlayerID].Peer, OS.str());
	}
}

void ServerState::UserCmd_Terraform(int PlayerID)
{
	if(CheckParam("exit", PacketString, 1))
	{
		CreateExit(PlayerID, "has just <G>opened up <W>a new <Y>pathway<W>!");
		return;
	}
	else if(CheckParam("landscape", PacketString, 1))
	{
		if(CheckParam("wilderness", PacketString, 2))
		{
			TerraformLandscape(PlayerID, MAPTYPE_SURFACE);
			return;
		}
	}
	else if(CheckParam("cave", PacketString, 1))
	{
		int DIR = 0;
		if(CheckParam("north", PacketString, 2))
			DIR = MOVEDIR_UP;
		else if(CheckParam("east", PacketString, 2))
			DIR = MOVEDIR_RIGHT;
		else if(CheckParam("south", PacketString, 2))
			DIR = MOVEDIR_DOWN;
		else if(CheckParam("west", PacketString, 2))
			DIR = MOVEDIR_LEFT;

		if(DIR > 0)
		{
			CreateCave(PlayerID, DIR);
		}
	}
	else if(CheckParam("bigobj", PacketString, 1))
	{
		cBigObject NewObj;
		NewObj.Type = BIGOBJ_HOUSE;
		NewObj.X = PlayerList[PlayerID].X;
		NewObj.Y = PlayerList[PlayerID].Y - 1;
		NewObj.XOff = -16;
		NewObj.YOff = -32;
		Maps[PlayerList[PlayerID].MapID].BigObjects.push_back(NewObj);

		for(size_t x = 0; x < Maps[PlayerList[PlayerID].MapID].BigObjects.size(); x++)
		SendMapBigObjects(PlayerList[PlayerID].MapID);
		return;
	}
	else if(CheckParam("path", PacketString, 1))
	{
		bool Rolling = CheckParam("rolling", PacketString, 3);
		int XMod = 0;
		int YMod = 0;
		if(CheckParam("north", PacketString, 2))
			YMod--;
		else if(CheckParam("east", PacketString, 2))
			XMod++;
		else if(CheckParam("south", PacketString, 2))
			YMod++;
		else if(CheckParam("west", PacketString, 2))
			XMod--;

		int CreateX = PlayerList[PlayerID].X;
		int CreateY = PlayerList[PlayerID].Y;
		int MapID = PlayerList[PlayerID].MapID;

		if(XMod || YMod)
		{
			if(Maps[MapID].MapType != MAPTYPE_SURFACE)
			{
				SendMessage(PlayerList[PlayerID].Peer, "You must be <G>outside <W>to create a <Y>dirt path<W>.");
				return;
			}
			if(Maps[MapID].Tile[CreateX+XMod][CreateY+YMod] == 0)
			{
				SendMessage(PlayerList[PlayerID].Peer, "You cannot do that <B>here<W>.");
				return;
			}

			SendPlayerUsedSkill(PlayerID, -1);
			bool Done = false;
			while(!Done)
			{
				CreateX += XMod;
				CreateY += YMod;
				CreatePath(MapID, CreateX, CreateY);
				if(!Rolling)
				{
					Done = true;
				}
				else
				if(CreateX == 0 || Maps[MapID].Tile[CreateX+XMod][CreateY+YMod][0] != Maps[MapID].MapType
								|| Maps[MapID].Tile[CreateX+XMod][CreateY+YMod][1] > 0
								|| Maps[MapID].Tile[CreateX+XMod][CreateY+YMod][2] > 0)
				{
					Done = true;
				}
			}

			for(size_t x = 0; x < Maps[MapID].PlayerRefs.size(); x++)
			{
				int Ref = Maps[MapID].PlayerRefs[x];

				SendPlayerMap(PlayerList[Ref].Peer, MapID);
				SendMessage(PlayerList[Ref].Peer, "A path was created.");
			}
		}
		return;
	}

	SendMessage(PlayerList[PlayerID].Peer, "I dont understand. Try using '<C>help terraform<W>'.");
}

void ServerState::UserCmd_Build(int PlayerID)
{
	int DIR = 0;
	if(CheckParam("north", PacketString, 2))
		DIR = MOVEDIR_UP;
	else if(CheckParam("east", PacketString, 2))
		DIR = MOVEDIR_RIGHT;
	else if(CheckParam("south", PacketString, 2))
		DIR = MOVEDIR_DOWN;
	else if(CheckParam("west", PacketString, 2))
		DIR = MOVEDIR_LEFT;

	if(DIR > 0)
	{
		if(CheckParam("house", PacketString, 1))
		{
			CreateHouse(PlayerID, DIR);
			return;
		}
		else if(CheckParam("forge", PacketString, 1))
		{
			CreateForge(PlayerID, DIR);
			return;
		}
		else if(CheckParam("bookshelf", PacketString, 1))
		{
			BuildItem(PlayerID, BUILDITEM_BOOKSHELF, DIR);
			return;
		}
		else if(CheckParam("table", PacketString, 1))
		{
			BuildItem(PlayerID, BUILDITEM_TABLE, DIR);
			return;
		}
		else if(CheckParam("chair", PacketString, 1))
		{
			BuildItem(PlayerID, BUILDITEM_CHAIR, DIR);
			return;
		}
	}
	
	SendMessage(PlayerList[PlayerID].Peer, "<C>Build <W>what, now?");
}

void ServerState::UserCmd_Pickup(int PlayerID)
{
	int MapID = PlayerList[PlayerID].MapID;
	int SearchObjID = -1;
	int ObjRef;
	bool PickedUp = false;
	bool OneItem = false;


	if(CheckParam("all", PacketString, 1))
	{
		if(GetWordCount(PacketString) == 2)
			SearchObjID = -8;	//Search for anything
		else
			SearchObjID = CheckParamForItem(PacketString, 2);
	}
	else
	{
		SearchObjID = CheckParamForItem(PacketString, 1);
		OneItem = true;
	}

	while((ObjRef = Maps[MapID].FindObjectRef(SearchObjID, PlayerList[PlayerID].X, PlayerList[PlayerID].Y)) >= 0)
	{
		int ESlot = -1;
		if((ESlot = PlayerList[PlayerID].FindEmptySlot()) < 0)
		{
			SendMessage(PlayerList[PlayerID].Peer, "You have <R>no room <W>in your inventory.");
			return;
		}
		PlayerList[PlayerID].Inventory[ESlot] = Maps[MapID].Objects[ObjRef];
		Maps[MapID].RemoveObject(ObjRef);
		SendMapObjects(MapID);

		ostringstream Msg(ostringstream::out);
		Msg << "You picked up the <C>" << GameItems[PlayerList[PlayerID].Inventory[ESlot].ID].Name;
		SendMessage(PlayerList[PlayerID].Peer, Msg.str());
		PickedUp = true;
		if(OneItem)
			break;
	}

	if(!PickedUp)
		SendMessage(PlayerList[PlayerID].Peer, "You are <R>not <W>near that.");
}

void ServerState::UserCmd_Drop(int PlayerID)
{
	bool Dropped = false;
	
	if(GetWord(PacketString, 1) == "all")
	{
		if(GetWordCount(PacketString) > 2)
		{
			int ObjID = CheckParamForItem(PacketString, 2);
			int ObjRef = -1;
			while((ObjRef = FindPlayerObjRefByName(PlayerID, PacketString, 2)) >= 0)
			{
				ostringstream Msg(ostringstream::out);
				Msg << "You <R>dropped <C>" << PlayerList[PlayerID].Inventory[ObjRef].Name;
				SendMessage(PlayerList[PlayerID].Peer, Msg.str());
				PlayerDropObject(PlayerID, ObjRef);
				Dropped = true;
			}
		}
		else
		{
			int InvSize = PlayerList[PlayerID].Inventory.size();
			for(int x = 0; x < PlayerList[PlayerID].Inventory.size(); x++)
			{
				if(!PlayerList[PlayerID].IsEquipped(x) && PlayerList[PlayerID].Inventory[x].ID >= 0)
				{
					ostringstream Msg(ostringstream::out);
					Msg << "You <R>dropped <C>" << GameItems[PlayerList[PlayerID].Inventory[x].ID].Name;
					SendMessage(PlayerList[PlayerID].Peer, Msg.str());

					PlayerDropObject(PlayerID, x);
					Dropped = true;
				}
			}
		}
	}
	else
	{
		int ObjID = CheckParamForItem(PacketString, 1);
		int ObjRef = FindPlayerObjRefByName(PlayerID, PacketString, 1);

		if(ObjRef >= 0)
		{
			ostringstream Msg(ostringstream::out);
			Msg << "You <R>dropped <C>" << PlayerList[PlayerID].Inventory[ObjRef].Name;
			SendMessage(PlayerList[PlayerID].Peer, Msg.str());
			PlayerDropObject(PlayerID, ObjRef);
			Dropped = true;
		}
		if(ObjRef == -2)
			SendMessage(PlayerList[PlayerID].Peer, "You must <C>unequip <W>that first.");
	}

	if(Dropped)
		return;

	SendMessage(PlayerList[PlayerID].Peer, "You <R>dont <W>have that.");
}

void ServerState::UserCmd_Forge(int PlayerID)
{
	bool CanForge = false;
	
	int X = PlayerList[PlayerID].X;
	int Y = PlayerList[PlayerID].Y;
	int MapID = PlayerList[PlayerID].MapID;

	if(Maps[MapID].Tile[X + 1][Y][2] == 14 || Maps[MapID].Tile[X - 1][Y][2] == 14 ||
		Maps[MapID].Tile[X][Y + 1][2] == 14 || Maps[MapID].Tile[X][Y - 1][2] == 14)
		CanForge = true;

	if(CanForge)
	{
		if(CheckParam("list", PacketString, 1))
		{
			SendMessage(PlayerList[PlayerID].Peer, "List of <C>items <W>you can currently <Y>forge<W>:");
			ostringstream ItemMsg(ostringstream::out);
			for(unsigned int x = 0; x < GameItems.size(); x++)
			{
				ItemMsg << "<C>" << GameItems[x].Name;
				SendMessage(PlayerList[PlayerID].Peer, ItemMsg.str());
				ItemMsg.str("");
			}
		}
		else
		{
			int ObjectID = CheckParamForItem(PacketString, 1);
			if(ObjectID >= 0)
			{
				int OpenSlot = PlayerList[PlayerID].FindEmptySlot();
				if(OpenSlot < 0)
				{
					SendMessage(PlayerList[PlayerID].Peer, "You have <R>no room <W>in your inventory.");
					return;
				}

				PlayerList[PlayerID].Inventory[OpenSlot] = GameItems[ObjectID];
					
				ostringstream Msg(ostringstream::out);
				Msg << "You slave over the <R>hot <Y>forge <W>and finish your new <C>" << GameItems[ObjectID].Name;
				SendMessage(PlayerList[PlayerID].Peer, Msg.str());
			}
		}
	}
	else
	{
		SendMessage(PlayerList[PlayerID].Peer, "You must be standing near a <Y>forge <W>to do that.");
	}
}

void ServerState::UserCmd_Inventory(int PlayerID)
{
	vector<string> InventoryList;

	int ID;
	int Quan;
	for(size_t x = 0; x < PlayerList[PlayerID].Inventory.size(); x++)
	{
		ID = PlayerList[PlayerID].Inventory[x].ID;
		Quan = PlayerList[PlayerID].Inventory[x].Quantity;
		if(ID >= 0)
		{
			OSStr(OS);
			OS << Quan << "x <C>" << GameItems[ID].Name;
			if(PlayerList[PlayerID].IsEquipped(x))
			{
				OS << " [<G>E<W>]";
			}
			InventoryList.push_back(OS.str());
		}
	}

	if(InventoryList.size() > 0)
	{
		SendMessage(PlayerList[PlayerID].Peer, "Current inventory:");

		for(size_t x = 0; x < InventoryList.size(); x++)
		{
			SendMessage(PlayerList[PlayerID].Peer, InventoryList[x]);
		}
	}
	else
	{
		SendMessage(PlayerList[PlayerID].Peer, "You have <R>nothing <W>in your inventory.");
	}
}

void ServerState::UserCmd_Equip(int PlayerID)
{
	int ObjRef = FindPlayerObjRefByName(PlayerID, PacketString, 1);
	if(ObjRef >= 0)
	{
		ostringstream Msg(ostringstream::out);
		if(PlayerList[PlayerID].Inventory[ObjRef].Type == OBJTYPE_ARMOR)
		{
			PCEquipItem(PlayerID, EQUIPSLOT_ARMOR, ObjRef);
			Msg << "You put on the <C>" << PlayerList[PlayerID].EquipmentSlot[EQUIPSLOT_ARMOR].Name << "<W>.";
		}
		else if(PlayerList[PlayerID].Inventory[ObjRef].Type == OBJTYPE_LEGS)
		{
			PCEquipItem(PlayerID, EQUIPSLOT_LEGS, ObjRef);
			Msg << "You put the <C>" << PlayerList[PlayerID].EquipmentSlot[EQUIPSLOT_LEGS].Name << "<W> on.";
		}
		else if(PlayerList[PlayerID].Inventory[ObjRef].Type == OBJTYPE_HELM)
		{
			PCEquipItem(PlayerID, EQUIPSLOT_HELM, ObjRef);
			Msg << "You put on the <C>" << PlayerList[PlayerID].EquipmentSlot[EQUIPSLOT_HELM].Name << "<W>.";
		}
		else if(PlayerList[PlayerID].Inventory[ObjRef].Type == OBJTYPE_BOOTS)
		{
			PCEquipItem(PlayerID, EQUIPSLOT_BOOTS, ObjRef);
			Msg << "You put the <C>" << PlayerList[PlayerID].EquipmentSlot[EQUIPSLOT_BOOTS].Name << "<W> on.";
		}
		else
		{
			SendMessage(PlayerList[PlayerID].Peer, "You can not <C>equip <W>that.");
			return;
		}
		SendUpdateEquip(NULL, PlayerID);
		
		SendMessage(PlayerList[PlayerID].Peer, Msg.str());
	}
	else
	{
		SendMessage(PlayerList[PlayerID].Peer, "You <R>don't <W>have that in your inventory.");
	}
}
void ServerState::UserCmd_UnEquip(int PlayerID)
{
	int OpenSlot;
	if((OpenSlot = PlayerList[PlayerID].FindEmptySlot()) == -1)
	{
		SendMessage(PlayerList[PlayerID].Peer, "You have no room in your <C>inventory <W>for that.");
	}
	OSStr(Msg);
	Msg << "You <C>unequip <W>your <Y>";
	if(CheckParam("armor", PacketString, 1))
	{
		Msg << PlayerList[PlayerID].EquipmentSlot[EQUIPSLOT_ARMOR].Name;
		Msg << "<W>.";
		SendMessage(PlayerList[PlayerID].Peer, Msg.str());
		PCUnequipItem(PlayerID, EQUIPSLOT_ARMOR);
	}
	else if(CheckParam("leggings", PacketString, 1))
	{
		Msg << PlayerList[PlayerID].EquipmentSlot[EQUIPSLOT_LEGS].Name;
		Msg << "<W>.";
		SendMessage(PlayerList[PlayerID].Peer, Msg.str());
		PCUnequipItem(PlayerID, EQUIPSLOT_LEGS);
	}
	else if(CheckParam("helmet", PacketString, 1))
	{
		Msg << PlayerList[PlayerID].EquipmentSlot[EQUIPSLOT_HELM].Name;
		Msg << "<W>.";
		SendMessage(PlayerList[PlayerID].Peer, Msg.str());
		PCUnequipItem(PlayerID, EQUIPSLOT_HELM);
	}
	else if(CheckParam("boots", PacketString, 1))
	{
		Msg << PlayerList[PlayerID].EquipmentSlot[EQUIPSLOT_BOOTS].Name;
		Msg << "<W>.";
		SendMessage(PlayerList[PlayerID].Peer, Msg.str());
		PCUnequipItem(PlayerID, EQUIPSLOT_BOOTS);
	}
	else if(CheckParam("all", PacketString, 1))
	{
		if(PlayerList[PlayerID].GetEmptySlotCount() >= EQUIPSLOT_MAX)
		{
			SendMessage(PlayerList[PlayerID].Peer, "You <C>unequip <Y>everything<W>.");
			for(int x = 0; x < EQUIPSLOT_MAX; x++)
			{
				PCUnequipItem(PlayerID, x);
			}
			return;
		}
		else
		{
			SendMessage(PlayerList[PlayerID].Peer, "You do not have enough <R>room <W>in your <C>inventory<W>.");
			return;
		}
	}
	else
	{
		SendMessage(PlayerList[PlayerID].Peer, "I dont understand.");
		SendMessage(PlayerList[PlayerID].Peer, "Try <C>help <Y>unequip<W>.");
		return;
	}


	SendUpdateEquip(NULL, PlayerID);
}

void ServerState::UserCmd_Summon(int PlayerID)
{
	int Map = PlayerList[PlayerID].MapID;
	int DestX = PlayerList[PlayerID].X;
	int DestY = PlayerList[PlayerID].Y;
	cMob NewMob;
	bool Summoned = false;

	if(GetWordCount(PacketString) < 2)
	{
		SendMessage(PlayerList[PlayerID].Peer, "<C>Summon <W>what?");
		return;
	}

	for(size_t x = 0; x < GameMobs.size(); x++)
	{
		if(CheckParam(GameMobs[x].Name, PacketString, 1))
		{
			NewMob = GameMobs[x];
			Summoned = true;
			break;
		}
	}

	if(!Summoned)
	{
		ostringstream os(ostringstream::out);
		os << "What is a <R>" << GetString(PacketString, 1, 0) << "<W>?";
		SendMessage(PlayerList[PlayerID].Peer, os.str());
		return;
	}

	NewMob.X = DestX;
	NewMob.Y = DestY;
	Maps[Map].Mobs.push_back(NewMob);

	ostringstream os(ostringstream::out);
	os << "<C>" << PlayerList[PlayerID].Name << " <W>summoned a <R>" << NewMob.Name;
	for(size_t x = 0; x < PlayerList.size(); x++)
	{
		if(PlayerList[x].MapID == Map)
		{
			SendAddMob(x, NewMob);
			SendMessage(PlayerList[x].Peer, os.str());
		}
	}
}

void ServerState::UserCmd_Join(int PlayerID)
{
	if(GetWordCount(PacketString) != 2)
	{
		SendMessage(PlayerList[PlayerID].Peer, "Join who?");
		return;
	}

	if(PlayerList[PlayerID].InParty)
	{
		SendMessage(PlayerList[PlayerID].Peer, "You are <R>already <W>in a <B>party<W>.");
		return;
	}

	int LeaderID = -1;
	for(size_t x = 0; x < PlayerList.size(); x++)
	{
		if(StrToLower(GetWord(PacketString, 1)) == StrToLower(PlayerList[x].Name))
		{
			LeaderID = x;
			break;
		}

	}
	if(LeaderID < 0)
	{
		SendMessage(PlayerList[PlayerID].Peer, "Who is that?");
		return;
	}
	if(LeaderID == PlayerID)
	{
		SendMessage(PlayerList[PlayerID].Peer, "You are already with yourself. <P>o.O");
		return;
	}
	if(PlayerList[LeaderID].MapID != PlayerList[PlayerID].MapID)
	{
		SendMessage(PlayerList[PlayerID].Peer, "You cannot <B>see <W>that person.");
		return;
	}
	int LeaderX = PlayerList[LeaderID].X;
	int LeaderY = PlayerList[LeaderID].Y;
	int PlayerX = PlayerList[PlayerID].X;
	int PlayerY = PlayerList[PlayerID].Y;

	if(PlayerX - 1 == LeaderX || PlayerX + 1 == LeaderX || PlayerX == LeaderX)
	{
		if(PlayerY - 1 == LeaderY || PlayerY + 1 == LeaderY || PlayerY == LeaderY)
		{
			if(PlayerX == LeaderX || PlayerY == LeaderY)
			{
				if(PlayerList[LeaderID].PartySize >= 2)
				{
					SendMessage(PlayerList[PlayerID].Peer, "That <B>party <W>is full!");
					return;
				}

				int PartyRef[3] = {PlayerList[LeaderID].Party[0], PlayerList[LeaderID].Party[1], PlayerList[LeaderID].Party[2]};
				int PartySize = PlayerList[LeaderID].PartySize;

				OSStr(Msg);
				Msg << "You <G>joined <C>" << PlayerList[LeaderID].Name << "<W>.";
				SendMessage(PlayerList[PlayerID].Peer, Msg.str());
				Msg.str("");
				Msg << "<C>" << PlayerList[PlayerID].Name << " <W>has joined your <B>party<W>.";
				SendMessage(PlayerList[LeaderID].Peer, Msg.str());

				int MySpot = 1;

				if(PartyRef[1] >= 0)
				{
					PartyRef[2] = PlayerList[PlayerID].ID;
					MySpot = 2;
				}
				else
				{
					PartyRef[1] = PlayerList[PlayerID].ID;
				}
				PartySize++;
				
				for(int x = 0; x < 3; x++)
				{
					int Ref = FindPlayerID(PartyRef[x]);
					if(Ref < 0)
						continue;

					PlayerList[Ref].PartySize = PartySize;
					PlayerList[Ref].Party[0] = PartyRef[0];
					PlayerList[Ref].Party[1] = PartyRef[1];
					PlayerList[Ref].Party[2] = PartyRef[2];
					PlayerList[Ref].InParty = true;
					if(Ref == FindPlayerID(PartyRef[2]) && LeaderID == FindPlayerID(PartyRef[0]))
						PlayerList[PlayerID].Moving = GetPlayerDir(FindPlayerID(PartyRef[1]), PlayerList[PlayerID].MapID, PlayerList[PlayerID].X, PlayerList[PlayerID].Y);
				}

				return;
			}
		}
	}

	SendMessage(PlayerList[PlayerID].Peer, "You must be standing beside them.");
}

void ServerState::UserCmd_Leave(int PlayerID)
{
	if(!PlayerList[PlayerID].InParty)
	{
		SendMessage(PlayerList[PlayerID].Peer, "You are not in a <B>party<W>.");
		return;
	}

	/* Old
	OSStr(Msg);
	Msg << "<C>" << PlayerList[PlayerID].Name << " <W>has left the <B>party<W>.";
	for(int x = 0; x <= PlayerList[PlayerID].PartySize; x++)
	{
		SendMessage(PlayerList[FindPlayerID(PlayerList[PlayerID].Party[x])].Peer, Msg.str());
	}*/
	
	int PartyRef[3] = {PlayerList[PlayerID].Party[0], PlayerList[PlayerID].Party[1], PlayerList[PlayerID].Party[2]};
	int PartySize = PlayerList[PlayerID].PartySize;

	int MySpot = 0;
	for(int x = 0; x < 3; x++)
	{
		if(PartyRef[x] == PlayerList[PlayerID].ID)
		{
			MySpot = x;
			break;
		}
	}

	PartyRef[MySpot] = -1;
	PartySize--;

	if(MySpot == 0)
	{
		PartyRef[0] = PartyRef[1];
		PartyRef[1] = PartyRef[2];
	}
	else if(MySpot == 1)
	{
		PartyRef[1] = PartyRef[2];
	}

	for(int x = 0; x < 3; x++)
	{
		int PlayerRef = FindPlayerID(PartyRef[x]);
		if(PlayerRef >= 0)
		{
			PlayerList[PlayerRef].Party[0] = PartyRef[0];
			PlayerList[PlayerRef].Party[1] = PartyRef[1];
			PlayerList[PlayerRef].Party[2] = PartyRef[2];
			PlayerList[PlayerRef].PartySize = PartySize;

			OSStr(Msg);
			Msg << "<C>" << PlayerList[PlayerID].Name << " <W>has left the <B>party<W>.";
			SendMessage(PlayerList[PlayerRef].Peer, Msg.str());
		}
	}
	PlayerList[PlayerID].Party[0] = PlayerList[PlayerID].ID;
	PlayerList[PlayerID].Party[1] = -1;
	PlayerList[PlayerID].Party[2] = -1;
	PlayerList[PlayerID].InParty = false;
	PlayerList[PlayerID].PartySize = 0;
}

void ServerState::UserCmd_PartyInfo(int PlayerID)
{
	OSStr(Msg);
	if(PlayerList[PlayerID].PartySize == 0)
	{
		Msg << "You are adventuring <B>alone<W>.";
		SendMessage(PlayerList[PlayerID].Peer, Msg.str());
		return;
	}

	for(int x = 0; x <= PlayerList[PlayerID].PartySize; x++)
	{
		int PlayerRef = FindPlayerID(PlayerList[PlayerID].Party[x]);
		if(PlayerRef < 0)
		{
			SendMessage(PlayerList[PlayerID].Peer, "I AM <C>ERROR<W>!");
			continue;
		}
		Msg << "<C>" << PlayerList[PlayerRef].Name << " <P>[Lv: " << PlayerList[PlayerID].Level << "]";
		SendMessage(PlayerList[PlayerID].Peer, Msg.str());
		Msg.str("");
	}

}

void ServerState::UserCmd_Battle(int PlayerID)
{
	PlayerGotoBattle(PlayerID, 0);
}

void ServerState::UserCmd_Time(int PlayerID)
{
	OSStr(Msg);
	Msg << "<C>";
	if(GameDay < 10)
		Msg << "0";
	Msg << GameDay << " <G>";
	if(GameHour < 10)
		Msg << "0";
	Msg << GameHour << "<W>:<G>";
	if(GameMinute < 10)
		Msg << "0";
	Msg << GameMinute;
	SendMessage(PlayerList[PlayerID].Peer, Msg.str());
}

void ServerState::UserCmd_Quit(int PlayerID)
{
	enet_peer_disconnect(PlayerList[PlayerID].Peer, 0);
}