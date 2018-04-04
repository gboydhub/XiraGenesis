#include "Game.h"
#include <sstream>

void ServerState::Init()
{
	if(!IsDedicated)
	{
		ServerBG = al_load_bitmap("ServerBG.png");
		HUDImage = al_load_bitmap("HUDItems.png");
	}

	GameDay = 0;
	GameHour = 22;
	GameMinute = 0;
	NeedsSave = 0;

	for (int x = 0; x < 64; x++)
	{
		AssignedIDPool[x] = false;
	}

	NameOffset = 0;
	AssignedIDs = 0;
	Address.host = ENET_HOST_ANY;
	Address.port = 5410;
	Host = enet_host_create(&Address, 200, 2, 0, 0);
	if(Host == NULL)
	{
		ConsoleWrite("<R>Could not start server. <W>Try to start it manually.");
		return;
	}
	else
	{
		ConsoleWrite("<G>Server started.");
	}

	cMap NewMap;
	NewMap.SetType(MAPTYPE_SURFACE);
	NewMap.WorldMapID = WorldRefs.size();
	NewMap.WorldX = 200;
	NewMap.WorldY = 200;
	Maps.push_back(NewMap);
	vector<int> x;
	x.push_back(Maps.size()-1);

	ConsoleWrite("<G>Loading maps.");
	if(!IsDedicated)
	{
		Draw();
		al_flip_display();
	}

	LoadMaps();
	ConsoleWrite("<G>Done!");
	CreateItems();
	CreateMobs();
	CreateCommands();

#ifdef _DEBUG
	VLDEnable();
#endif
}

void ServerState::ProcessInput(int KeyCode, bool KeyDown)
{
}

//Handle player logic, etc.
bool ServerState::DoLogic()
{
	//Server logic

	if(!IsDedicated)
	{
		al_get_mouse_state(&MouseState);
		if(MouseState.buttons & 1)
		{
			if(MouseState.x >= 777 && MouseState.x <= 777 + 13
				&& MouseState.y >= 270 && MouseState.y <= 270 + 12)
			{
				int INCS = ConsoleLine.size() - 25;
				if(ConsoleOffset < INCS)
					ConsoleOffset++;
			}
			if(MouseState.x >= 777 && MouseState.x <= 777 + 13
				&& MouseState.y >= 561 && MouseState.y <= 561 + 12)
			{
				if(ConsoleOffset > 0)
					ConsoleOffset--;
			}

			if(MouseState.x >= 777 && MouseState.x <= 777 + 13
				&& MouseState.y >= 101 && MouseState.y <= 101 + 12)
			{
				if(NameOffset > 0)
					NameOffset--;
			}
			if(MouseState.x >= 777 && MouseState.x <= 777 + 13
				&& MouseState.y >= 246 && MouseState.y <= 246 + 12)
			{
				int INCS = PlayerList.size() - 13;
				if(NameOffset < INCS)
					NameOffset++;
			}
		}
	}

	//Game logic
	vector<int> MapIDs;

	for(unsigned int x = 0; x < PlayerList.size(); x++)
	{

		//MapIDs.push_back(PlayerList[x].MapID);
		if(PlayerList[x].Moving > 0 && PlayerList[x].TakingStep == 0)
		{
			switch(PlayerList[x].Moving)
			{
			case MOVEDIR_RIGHT:
				if(!CheckBlocking(PlayerList[x].X+1, PlayerList[x].Y, PlayerList[x].MapID) || (PlayerList[x].InParty && FindPlayerID(PlayerList[x].Party[0]) != x))
				{
					PlayerList[x].TakingStep = MOVEDIR_RIGHT;
					PlayerList[x].X++;
					PlayerList[x].XOff = -32;
				}
				break;
			case MOVEDIR_LEFT:
				if(!CheckBlocking(PlayerList[x].X-1, PlayerList[x].Y, PlayerList[x].MapID) || (PlayerList[x].InParty && FindPlayerID(PlayerList[x].Party[0]) != x))
				{
					PlayerList[x].TakingStep = MOVEDIR_LEFT;
					PlayerList[x].X--;
					PlayerList[x].XOff = 32;
				}
				break;
			case MOVEDIR_UP:
				if(!CheckBlocking(PlayerList[x].X, PlayerList[x].Y-1, PlayerList[x].MapID) || (PlayerList[x].InParty && FindPlayerID(PlayerList[x].Party[0]) != x))
				{
					PlayerList[x].TakingStep = MOVEDIR_UP;
					PlayerList[x].Y--;
					PlayerList[x].YOff = 32;
				}
				break;
			case MOVEDIR_DOWN:
				if(!CheckBlocking(PlayerList[x].X, PlayerList[x].Y+1, PlayerList[x].MapID) || (PlayerList[x].InParty && FindPlayerID(PlayerList[x].Party[0]) != x))
				{
					PlayerList[x].TakingStep = MOVEDIR_DOWN;
					PlayerList[x].Y++;
					PlayerList[x].YOff = -32;
				}
				break;
			}

			PlayerList[x].SpriteDir = PlayerList[x].Moving - 1;
			
			if(PlayerList[x].InParty && x == FindPlayerID(PlayerList[x].Party[0]) && PlayerList[x].TakingStep > 0)
			{
				int PartyRef[3] = {FindPlayerID(PlayerList[x].Party[0]), FindPlayerID(PlayerList[x].Party[1]), FindPlayerID(PlayerList[x].Party[2])};
				for(int p = 1; p <= PlayerList[x].PartySize; p++)
				{
					if(PartyRef[p] < 0)
						continue;
					
					PlayerList[PartyRef[p]].Moving = GetPlayerDir(PartyRef[p-1], PlayerList[PartyRef[p]].MapID, PlayerList[PartyRef[p]].X, PlayerList[PartyRef[p]].Y);
				}
			}
		}
		if(PlayerList[x].TakingStep > 0)
		{
			if(PlayerList[x].InParty && FindPlayerID(PlayerList[x].Party[0]) != x && PlayerList[x].Moving > 0)
			{
				PlayerList[x].Moving = 0;
			}
			switch(PlayerList[x].TakingStep)
			{
			case MOVEDIR_RIGHT:
				PlayerList[x].XOff+=2;
				if(PlayerList[x].XOff == 0)
					PlayerList[x].TakingStep = 0;
				break;
			case MOVEDIR_LEFT:
				PlayerList[x].XOff-=2;
				if(PlayerList[x].XOff == 0)
					PlayerList[x].TakingStep = 0;
				break;
			case MOVEDIR_UP:
				PlayerList[x].YOff-=2;
				if(PlayerList[x].YOff == 0)
					PlayerList[x].TakingStep = 0;
				break;
			case MOVEDIR_DOWN:
				PlayerList[x].YOff+=2;
				if(PlayerList[x].YOff == 0)
					PlayerList[x].TakingStep = 0;
				break; 
			}
			if(PlayerList[x].TakingStep == 0)
			{
				bool ChangingMaps = false;
				int NewMap = 0;
				int NewX = 0;
				int NewY = 0;
				string ExitMessage;
				if(Maps[PlayerList[x].MapID].Exit[PlayerList[x].X][PlayerList[x].Y] >= 0)
				{
					ChangingMaps = true;
					NewX = Maps[PlayerList[x].MapID].ExitDestX[PlayerList[x].X][PlayerList[x].Y];
					NewY = Maps[PlayerList[x].MapID].ExitDestY[PlayerList[x].X][PlayerList[x].Y];
					NewMap = Maps[PlayerList[x].MapID].Exit[PlayerList[x].X][PlayerList[x].Y];
					ExitMessage =  Maps[PlayerList[x].MapID].ExitMessage[PlayerList[x].X][PlayerList[x].Y];
				}
				else if(PlayerList[x].X == 0 && Maps[PlayerList[x].MapID].ExitSide != MOVEDIR_LEFT)
				{
					ChangingMaps = true;
					NewX = 10;
					NewY = PlayerList[x].Y;
					NewMap = FindWorldMapID(Maps[PlayerList[x].MapID].WorldX - 1, Maps[PlayerList[x].MapID].WorldY, Maps[PlayerList[x].MapID].WorldMapID);
					ExitMessage = "You moved west.";
				}
				else if(PlayerList[x].X == 11 && Maps[PlayerList[x].MapID].ExitSide != MOVEDIR_RIGHT)
				{
					ChangingMaps = true;
					NewX = 1;
					NewY = PlayerList[x].Y;
					NewMap = FindWorldMapID(Maps[PlayerList[x].MapID].WorldX + 1, Maps[PlayerList[x].MapID].WorldY, Maps[PlayerList[x].MapID].WorldMapID);
					ExitMessage = "You moved east.";
				}
				else if(PlayerList[x].Y == 0 && Maps[PlayerList[x].MapID].ExitSide != MOVEDIR_UP)
				{
					ChangingMaps = true;
					NewX = PlayerList[x].X;
					NewY = 9;
					NewMap = FindWorldMapID(Maps[PlayerList[x].MapID].WorldX, Maps[PlayerList[x].MapID].WorldY - 1, Maps[PlayerList[x].MapID].WorldMapID);
					ExitMessage = "You moved north.";
				}
				else if(PlayerList[x].Y == 10 && Maps[PlayerList[x].MapID].ExitSide != MOVEDIR_DOWN)
				{
					ChangingMaps = true;
					NewX = PlayerList[x].X;
					NewY = 1;
					NewMap = FindWorldMapID(Maps[PlayerList[x].MapID].WorldX, Maps[PlayerList[x].MapID].WorldY + 1, Maps[PlayerList[x].MapID].WorldMapID);
					ExitMessage = "You moved south.";
				}

				if(ChangingMaps == true && NewMap != -1)
				{
					SendPlayerClearGfx(x);
					TeleportPlayer(x, NewX, NewY, NewMap, ExitMessage);
					if(PlayerList[x].InParty && FindPlayerID(PlayerList[x].Party[0]) == x)
					{
						int PartyRef[3] = {FindPlayerID(PlayerList[x].Party[0]), FindPlayerID(PlayerList[x].Party[1]), FindPlayerID(PlayerList[x].Party[2])};
						for(int p = 1; p <= PlayerList[x].PartySize; p++)
						{
							if(PartyRef[p] >= 0)
							{
								PlayerList[PartyRef[p]].Moving = 0;
								PlayerList[PartyRef[p]].TakingStep = 0;
								PlayerList[PartyRef[p]].XOff = 0;
								PlayerList[PartyRef[p]].YOff = 0;
								SendPlayerClearGfx(PartyRef[p]);
								TeleportPlayer(PartyRef[p], NewX, NewY, NewMap, ExitMessage);
							}
						}
					}
				}
			}

			for(size_t pc = 0; pc < Maps[PlayerList[x].MapID].PlayerRefs.size(); pc++)
			{
				int PlayerRef = Maps[PlayerList[x].MapID].PlayerRefs[pc];
				SendPlayerLoc(PlayerList[PlayerRef].Peer, x);
			}
		}
	}
	
	if(MapIDs.size() > 0)
	{
		/*sort(MapIDs.begin(), MapIDs.end());
		MapIDs.erase(unique(MapIDs.begin(), MapIDs.end()));
		int MAP = 0;

		for(size_t x = 0; x < MapIDs.size(); x++)
		{
			MAP = MapIDs[x];
			for(size_t m = 0; m < Maps[MAP].Mobs.size(); m++)
			{
				if(Maps[MAP].Mobs[m].NeedsBlockCheck)
				{
					int NewX = Maps[MAP].Mobs[m].X;
					int NewY = Maps[MAP].Mobs[m].Y;

					switch(Maps[MAP].Mobs[m].TakingStep)
					{
					case MOVEDIR_UP:
						NewY--;
						break;
					case MOVEDIR_DOWN:
						NewY++;
						break;
					case MOVEDIR_LEFT:
						NewX--;
						break;
					case MOVEDIR_RIGHT:
						NewX++;
						break;
					}
					if(CheckBlocking(NewX, NewY, MAP))
					{
						Maps[MAP].Mobs[m].NeedsBlockCheck = false;
						Maps[MAP].Mobs[m].TakingStep = 0;
					}
				}

				if(Maps[MAP].Mobs[m].TakingStep)
				{
					switch(Maps[MAP].Mobs[m].TakingStep)
					{
					case MOVEDIR_UP:
						Maps[MAP].Mobs[m].YOff-=2;
						break;
					case MOVEDIR_DOWN:
						Maps[MAP].Mobs[m].YOff+=2;
						break;
					case MOVEDIR_RIGHT:
						Maps[MAP].Mobs[m].XOff+=2;
						break;
					case MOVEDIR_LEFT:
						Maps[MAP].Mobs[m].XOff-=2;
						break;
					}

					if(Maps[MAP].Mobs[m].XOff == 0 && Maps[MAP].Mobs[m].YOff == 0)
						Maps[MAP].Mobs[m].TakingStep = 0;
				}
			}
		}*/
	}
	return true;
}

void ServerState::Draw()
{
	al_draw_bitmap(ServerBG, 0, 0, NULL);
	for(int x = NameOffset; x < 13 + NameOffset; x++)
	{
		if((int)PlayerList.size() > x)
			al_draw_textf(GameFont, al_map_rgb(255, 255, 255), 648, 101 + (x * GameFont->height), NULL, "%s", PlayerList[x].Name.c_str());
	}
	al_draw_bitmap_region(HUDImage, 6, 0, 13, 12, 777, 101, NULL);
	al_draw_bitmap_region(HUDImage, 18, 0, 13, 12, 777, 246, NULL);
	al_draw_bitmap_region(HUDImage, 6, 0, 13, 12, 777, 270, NULL);
	al_draw_bitmap_region(HUDImage, 18, 0, 13, 12, 777, 561, NULL);
	ConsoleDraw(12, 260, 25, ConsoleOffset);
}

void ServerState::ResetState()
{
}

//Player input pakets
void ServerState::ProcessDataPacket(ENetPeer *From, ENetPacket *Packet)
{
	if(FindPlayerID(From) < 0)
		return;

	if(Packet->dataLength > 2)
	{
		int PlayerID = FindPlayerID(From);
		ostringstream OutMsg(ostringstream::out);
		switch(Packet->data[1])
		{
		case PACKETID_STARTMOVE:
			if(Packet->dataLength >= 3 && PlayerList[PlayerID].Moving == 0)
			{
				if(PlayerList[PlayerID].InBattle)
					break;

				if(PlayerList[PlayerID].UsingSkill > 0)
					break;

				//In a party, not the leader
				if(PlayerList[PlayerID].InParty && FindPlayerID(PlayerList[PlayerID].Party[0]) != PlayerID)
					break;

				PlayerList[PlayerID].Moving = Packet->data[2];
			}
			break;

		case PACKETID_CHANGEDIR:
			if(Packet->dataLength < 3)
				break;
			
			if(PlayerList[PlayerID].SpriteDir == Packet->data[2] - 1)
				break;

			if(PlayerList[PlayerID].InBattle || PlayerList[PlayerID].UsingSkill ||
				PlayerList[PlayerID].Moving)
				break;

			if(PlayerList[PlayerID].InParty && FindPlayerID(PlayerList[PlayerID].Party[0] != PlayerID))
				break;

			PlayerList[PlayerID].SpriteDir = Packet->data[2] - 1;
			for(size_t x = 0; x < Maps[PlayerList[PlayerID].MapID].PlayerRefs.size(); x++)
			{
				SendPlayerLoc(PlayerList[Maps[PlayerList[PlayerID].MapID].PlayerRefs[x]].Peer, PlayerID);
			}
			break;

		case PACKETID_ENDMOVE:
			PlayerList[PlayerID].Moving = 0;
			break;
		};
	}
}

//Player string commands
void ServerState::ProcessRecievedPacket(ENetPeer *From)
{
	int Words = GetWordCount(PacketString);
	if(Words == 0)
		return;

	if(CheckParam("login", PacketString, 0) && FindPlayerID(From) < 0)
	{
		ConsoleWrite("Recieved login packet.");
		if(Words != 3)
		{
			SendMessage(From, "<R>Invalid login attempt.");
		}
		else
		{
			if(FindPlayerID(GetWord(PacketString, 1)) >= 0)
			{
				SendMessage(From, "<C>Someone <W>with that name is <R>already <W>logged in!");
				return;
			}
			cPlayer NewPlayer;
			if(!NewPlayer.Load(GetWord(PacketString, 1)))
			{
				NewPlayer.Name = GetWord(PacketString, 1);
				NewPlayer.Password = GetWord(PacketString, 2);
			}
			else
			{
				if(NewPlayer.Password.compare(GetWord(PacketString, 2)) != 0)
				{
					ostringstream Msg(ostringstream::out);
					Msg << "<R>Incorrect password <W>for player <C>" << GetWord(PacketString, 1) << "<W>.";
					SendMessage(From, Msg.str());
					return;
				}
			}
			NewPlayer.HairStyle = 1;
			NewPlayer.HairR = 50;
			NewPlayer.HairG = 50;
			NewPlayer.HairB = 255;

			//Get new session ID
			for (int x = 0; x < 64; x++)
			{
				if (!AssignedIDPool[x])
				{
					NewPlayer.ID = x;
					NewPlayer.Party[0] = x;
					AssignedIDPool[x] = true;
					break;
				}
			}

			NewPlayer.Peer = From;
			PlayerList.push_back(NewPlayer);
			OSStr(Debug);
			Debug << "ID: " << NewPlayer.ID;
			SendMessage(NULL, Debug.str());
			
			char PlayersOnline = PlayerList.size();

			int DataPack[] = {PACKETID_DATAINT, PACKETID_LOGGEDIN, NewPlayer.ID};
			SendIntPacket(From, DataPack, 3);
			enet_host_flush(Host);

			ostringstream NewLoginMsg(ostringstream::out);
			NewLoginMsg << "<C>" << NewPlayer.Name << " <W>has logged in.";
			SendMessage(NULL, NewLoginMsg.str());
			
			Maps[NewPlayer.MapID].PlayerEnteredMap(FindPlayerID(From));
			SendGameTime();
			SendPlayerMap(From, NewPlayer.MapID);
			SendPlayerWorldMap(From, Maps[NewPlayer.MapID].WorldMapID, Maps[NewPlayer.MapID].WorldX, Maps[NewPlayer.MapID].WorldY);
			for(unsigned int x = 0; x < Maps[NewPlayer.MapID].PlayerRefs.size(); x++)
			{
				int PlayerRef = Maps[NewPlayer.MapID].PlayerRefs[x];
				SendAddPlayer(From, PlayerRef);
				SendAddPlayer(PlayerList[PlayerRef].Peer, FindPlayerID(From));
			}

			SendAddPlayer(From, FindPlayerID(From));
			SendMapObjects(From);

			ostringstream NewLogin(ostringstream::out);
			NewLogin << "<G>Logged in " << NewPlayer.Name;
			ConsoleWrite(NewLogin.str().c_str());

			Maps[NewPlayer.MapID].PlayerEnteredMap(PlayerList.size() - 1);
		}
	}
	else
	{
		int PlayerID = FindPlayerID(From);
		if(PlayerID < 0)
		{
			SendMessage(From, "<R>You are not logged in");
			return;
		}

		if(PlayerList[PlayerID].InBattle)
		{
			if(CheckParam("run", PacketString, 0) && PlayerList[PlayerID].InBattle)
			{
				PlayerBattleRun(PlayerID);
			}
			if(CheckParam("attack", PacketString, 0))
			{
				PlayerBattleAttack(PlayerID);
			}
			if(CheckParam("defend", PacketString, 0))
			{
				PlayerBattleDefend(PlayerID);
			}
			if(CheckParam("cast", PacketString, 0))
			{
				SendMessage(PlayerList[PlayerID].Peer, "You cast a spell!");
			}
			if(CheckParam("use", PacketString, 0))
			{
				SendMessage(PlayerList[PlayerID].Peer, "You use an item!");
			}
			return;
		}

		for(size_t x = 0; x < CommandList.size(); x++)
		{
			if(CheckParam(StrToLower(CommandList[x].Name), PacketString, 0))
			{
				PFN Func = CommandList[x].Function;
				(this->*Func)(PlayerID);
				return;
			}
		}
		
		SendMessage(From, "What are you trying to do?");
	}
}

void ServerState::SendMessage(ENetPeer *To, string Message)
{
	ENetPacket *OutPacket;
	OutPacket =  enet_packet_create(Message.c_str(), Message.length(), ENET_PACKET_FLAG_RELIABLE);
	if(To == NULL)
	{
		enet_host_broadcast(Host, 0, OutPacket);
	}
	else
	{
		enet_peer_send(To, 0, OutPacket);
	}
}

//Local commands
void ServerState::ProcessConsoleInput(string Input)
{
	ConsoleInput = Input;
	ProcessConsoleInput();
}

void ServerState::ProcessConsoleInput()
{
	int Words = GetWordCount(ConsoleInput);

	if(StrToLower(GetWord(ConsoleInput, 0)) == "quit")
	{
		NextScene = SCENE_EXITGAME;
	}
	else if(StrToLower(GetWord(ConsoleInput, 0)) == "echo")
	{
		string Output;
		for(int x = 1; x < Words; x++)
		{
			Output += GetWord(ConsoleInput, x);
			Output += " ";
		}
		ConsoleWrite(Output);
	}
	else if(StrToLower(GetWord(ConsoleInput, 0)) == "save")
	{
		ostringstream OutMsg(ostringstream::out);
		string LogFileName = "log.txt";

		if(Words > 1)
			LogFileName = GetWord(ConsoleInput, 1);

		OutMsg << "<G>Saving log file: <C>" << LogFileName;
		ConsoleWrite(OutMsg.str().c_str());
	}
	else if(StrToLower(GetWord(ConsoleInput, 0)) == "savemaps")
	{
		SaveMaps();
		ConsoleWrite("<G>Saved map data.");
	}
	else if(StrToLower(GetWord(ConsoleInput, 0)) == "loadmaps")
	{
		LoadMaps();
		ConsoleWrite("<G>Loaded <W>saved maps.");
	}
	else if(StrToLower(GetWord(ConsoleInput, 0)) == "kick")
	{
		int PlayerID = FindPlayerID(GetWord(ConsoleInput, 1));
		ostringstream OutMsg(ostringstream::out);
		if(PlayerID >= 0)
		{
			enet_peer_disconnect(PlayerList[PlayerID].Peer, 5);
			OutMsg << "<C>" << GetWord(ConsoleInput, 1) << " <W>has been kicked.";
		}
		else
		{
			OutMsg << "Could not find <C>" << GetWord(ConsoleInput, 1);
		}
		ConsoleWrite(OutMsg.str().c_str());
	}
	else
	{
		ConsoleWrite("<R>What?");
	}
}

int ServerState::FindPlayerID(ENetPeer *Peer)
{
	for(unsigned int x = 0; x < PlayerList.size(); x++)
	{
		if(PlayerList[x].Peer == Peer)
			return x;
	}

	return -1;
}

int ServerState::FindPlayerID(string Name)
{
	for(unsigned int x = 0; x < PlayerList.size(); x++)
	{
		if(PlayerList[x].Name == Name)
			return x;
	}

	return -1;
}

int ServerState::FindPlayerID(int ID)
{
	if(ID < 0)
		return -1;

	for(unsigned int x = 0; x < PlayerList.size(); x++)
	{
		if(PlayerList[x].ID == ID)
			return x;
	}

	return -1;
}

void ServerState::SendDataPacket(ENetPeer *To, const char *Data, int Size)
{
	ENetPacket *OutPacket = enet_packet_create(Data, Size, ENET_PACKET_FLAG_RELIABLE);
	if(To == NULL)
	{
		enet_host_broadcast(Host, 0, OutPacket);
	}
	else
	{
		enet_peer_send(To, 0, OutPacket);
	}
}

void ServerState::SendIntPacket(ENetPeer *To, const int *Data, int Size)
{
	ENetPacket *OutPacket = enet_packet_create(Data, Size*sizeof(int), ENET_PACKET_FLAG_RELIABLE);
	if(To == NULL)
	{
		enet_host_broadcast(Host, 0, OutPacket);
	}
	else
	{
		enet_peer_send(To, 0, OutPacket);
	}
}

int ServerState::CheckBlocking(int X, int Y, int MapID)
{
	if(X == -1 || X == 12 || Y == -1 || Y == 11)
		return 1;

	if(Maps[MapID].Tile[X][Y][0] == 0)
		return 1;

	//Forge tile
	if(Maps[MapID].Tile[X][Y][2] == 14)
		return 1;

	if(Maps[MapID].Tile[X][Y][2] == BUILDITEM_BOOKSHELF || Maps[MapID].Tile[X][Y][2] == BUILDITEM_TABLE)
		return 1;

	for(unsigned int x = 0; x < Maps[MapID].PlayerRefs.size(); x++)
	{
		int PlayerRef = Maps[MapID].PlayerRefs[x];
		if(PlayerList[PlayerRef].X == X && PlayerList[PlayerRef].Y == Y)
		{
			if(PlayerList[PlayerRef].InBattle)
				return 0;
			if(FindPlayerID(PlayerList[PlayerRef].Party[0]) != x)
				return 0;

			return 2;
		}
	}

	return false;
}

void ServerState::SendPlayerMap(ENetPeer *To, int MapID)
{
	char MapDataPack[398];
	MapDataPack[0] = (char)PACKETID_DATASTART;
	MapDataPack[1] = PACKETID_MAPDATA;
	for(int z = 0; z < 3; z++)
	{
		for(int x = 0; x < 12; x++)
		{
			for(int y = 0; y < 11; y++)
			{
				MapDataPack[(((x*11)+y)+(z*132)) + 2] = Maps[MapID].Tile[x][y][z];
			}
		}
	}
	SendDataPacket(To, MapDataPack, 398);

	int TypePacket[] = {PACKETID_DATAINT, PACKETID_SETMAPTYPE, Maps[MapID].MapType};
	SendIntPacket(To, TypePacket, 3);

	int MapIDPacket[] = {PACKETID_DATAINT, PACKETID_SETMAPID, MapID};
	SendIntPacket(To, MapIDPacket, 3);

	SendMapObjects(To);
	enet_host_flush(Host);
}

void ServerState::CreateExit(int PID, string SuccessMessage)
{
	bool InCorner = false;
	if(PlayerList[PID].X == 1 && PlayerList[PID].Y == 1)
	{
		InCorner = true;
	}
	else if (PlayerList[PID].X == 10 && PlayerList[PID].Y == 1)
	{
		InCorner = true;
	}
	else if (PlayerList[PID].X == 10 && PlayerList[PID].Y == 9)
	{
		InCorner = true;
	}
	else if (PlayerList[PID].X == 1 && PlayerList[PID].Y == 9)
	{
		InCorner = true;
	}
	if(InCorner)
	{
		SendMessage(PlayerList[PID].Peer, "You cannot do that in a corner.");
		return;
	}

	int PlayerX = PlayerList[PID].X;
	int PlayerY = PlayerList[PID].Y;
	int DestX = PlayerX;
	int DestY = PlayerY;
	int CurrentMapID = PlayerList[PID].MapID;
	int NewMapID = -1;

	int OnCreateType = Maps[CurrentMapID].MapType;
	if(Maps[CurrentMapID].MapType == MAPTYPE_SURFACE)
	{
		OnCreateType = MAPTYPE_VOID;
	}

	if(Maps[CurrentMapID].Tile[PlayerX + 1][PlayerY][0] == 0 && Maps[CurrentMapID].ExitSide != MOVEDIR_RIGHT)
	{
		NewMapID = FindWorldMapID(Maps[CurrentMapID].WorldX + 1, Maps[CurrentMapID].WorldY, Maps[CurrentMapID].WorldMapID);
		if(NewMapID == -1)
		{
			if(Maps[CurrentMapID].WorldMaxSize > 0 && WorldRefs[Maps[CurrentMapID].WorldMapID].size() >= Maps[CurrentMapID].WorldMaxSize)
			{
				SendMessage(PlayerList[PID].Peer, "You cannot <P>expand <W>any further.");
				return;
			}
			cMap NewMap;
			NewMap.SetType(OnCreateType);//Maps[CurrentMapID].MapType);
			NewMap.WorldMaxSize = Maps[CurrentMapID].WorldMaxSize;
			NewMap.Tile[0][PlayerY][0] = NewMap.MapType;
			NewMap.WorldX = Maps[CurrentMapID].WorldX + 1;
			NewMap.WorldY = Maps[CurrentMapID].WorldY;
			NewMap.WorldMapID = Maps[CurrentMapID].WorldMapID;
			WorldRefs[NewMap.WorldMapID].push_back(Maps.size());
			Maps.push_back(NewMap);
		}
		else
		{
			if(Maps[NewMapID].ExitSide == MOVEDIR_LEFT)
			{
				SendMessage(PlayerList[PID].Peer, "You shouldnt do that.");
				return;
			}
			Maps[NewMapID].Tile[0][PlayerY][0] = Maps[NewMapID].MapType;
			
			SendGfxEvent(NewMapID, GFXEVENT_TFE, 0, PlayerY);
			for(unsigned int m = 0; m < Maps[NewMapID].PlayerRefs.size(); m++)
			{
				int PlayerRef = Maps[NewMapID].PlayerRefs[m];

				ostringstream Msg(ostringstream::out);
				Msg << "<C>Someone<W> " << SuccessMessage;
				SendMessage(PlayerList[PlayerRef].Peer, Msg.str());
				SendPlayerMap(PlayerList[PlayerRef].Peer, PlayerList[PlayerRef].MapID);
			}
		}
		Maps[CurrentMapID].Tile[PlayerX+1][PlayerY][0] = Maps[CurrentMapID].MapType;
		DestX++;
	}
	else if(Maps[CurrentMapID].Tile[PlayerX - 1][PlayerY][0] == 0 && Maps[CurrentMapID].ExitSide != MOVEDIR_LEFT)//
	{
		NewMapID = FindWorldMapID(Maps[CurrentMapID].WorldX - 1, Maps[CurrentMapID].WorldY, Maps[CurrentMapID].WorldMapID);//
		if(NewMapID == -1)
		{
			if(Maps[CurrentMapID].WorldMaxSize > 0 && WorldRefs[Maps[CurrentMapID].WorldMapID].size() >= Maps[CurrentMapID].WorldMaxSize)
			{
				SendMessage(PlayerList[PID].Peer, "You cannot <P>expand <W>any further.");
				return;
			}
			cMap NewMap;
			NewMap.SetType(OnCreateType);//Maps[CurrentMapID].MapType);
			NewMap.WorldMaxSize = Maps[CurrentMapID].WorldMaxSize;
			NewMap.Tile[11][PlayerY][0] = NewMap.MapType;//
			NewMap.WorldX = Maps[CurrentMapID].WorldX - 1;//
			NewMap.WorldY = Maps[CurrentMapID].WorldY;//
			NewMap.WorldMapID = Maps[CurrentMapID].WorldMapID;
			WorldRefs[NewMap.WorldMapID].push_back(Maps.size());
			Maps.push_back(NewMap);
		}
		else
		{
			if(Maps[NewMapID].ExitSide == MOVEDIR_RIGHT)
			{
				SendMessage(PlayerList[PID].Peer, "You shouldnt do that.");
				return;
			}
			Maps[NewMapID].Tile[11][PlayerY][0] = Maps[NewMapID].MapType;//5
			SendGfxEvent(NewMapID, GFXEVENT_TFE, 11, PlayerY);
			for(unsigned int m = 0; m < Maps[NewMapID].PlayerRefs.size(); m++)
			{
				int PlayerRef = Maps[NewMapID].PlayerRefs[m];
				ostringstream Msg(ostringstream::out);
				Msg << "<C>Someone<W> " << SuccessMessage;
				SendMessage(PlayerList[PlayerRef].Peer, Msg.str());
				SendPlayerMap(PlayerList[PlayerRef].Peer, PlayerList[PlayerRef].MapID);
			}
		}
		Maps[CurrentMapID].Tile[PlayerX-1][PlayerY][0] = Maps[CurrentMapID].MapType;//6
		DestX--;
	}
	else if(Maps[CurrentMapID].Tile[PlayerX][PlayerY-1][0] == 0 && Maps[CurrentMapID].ExitSide != MOVEDIR_UP)//
	{
		NewMapID = FindWorldMapID(Maps[CurrentMapID].WorldX, Maps[CurrentMapID].WorldY - 1, Maps[CurrentMapID].WorldMapID);//
		if(NewMapID == -1)
		{
			if(Maps[CurrentMapID].WorldMaxSize > 0 && WorldRefs[Maps[CurrentMapID].WorldMapID].size() >= Maps[CurrentMapID].WorldMaxSize)
			{
				SendMessage(PlayerList[PID].Peer, "You cannot <P>expand <W>any further.");
				return;
			}
			cMap NewMap;
			NewMap.SetType(OnCreateType);//Maps[CurrentMapID].MapType);
			NewMap.WorldMaxSize = Maps[CurrentMapID].WorldMaxSize;
			NewMap.Tile[PlayerX][10][0] = NewMap.MapType;//
			NewMap.WorldX = Maps[CurrentMapID].WorldX;//
			NewMap.WorldY = Maps[CurrentMapID].WorldY - 1;//
			NewMap.WorldMapID = Maps[CurrentMapID].WorldMapID;
			WorldRefs[NewMap.WorldMapID].push_back(Maps.size());
			Maps.push_back(NewMap);
		}
		else
		{
			if(Maps[NewMapID].ExitSide == MOVEDIR_DOWN)
			{
				SendMessage(PlayerList[PID].Peer, "You shouldnt do that.");
				return;
			}
			Maps[NewMapID].Tile[PlayerX][10][0] = Maps[NewMapID].MapType;//5
		
			SendGfxEvent(NewMapID, GFXEVENT_TFE, PlayerX, 10);
			for(unsigned int m = 0; m < Maps[NewMapID].PlayerRefs.size(); m++)
			{
				int PlayerRef = Maps[NewMapID].PlayerRefs[m];
				ostringstream Msg(ostringstream::out);
				Msg << "<C>Someone<W> " << SuccessMessage;
				SendMessage(PlayerList[PlayerRef].Peer, Msg.str());
				SendPlayerMap(PlayerList[PlayerRef].Peer, PlayerList[PlayerRef].MapID);
			}
		}
		Maps[CurrentMapID].Tile[PlayerX][PlayerY-1][0] = Maps[CurrentMapID].MapType;//6
		DestY--;
	}
	else if(Maps[CurrentMapID].Tile[PlayerX][PlayerY+1][0] == 0 && Maps[CurrentMapID].ExitSide != MOVEDIR_DOWN)//
	{
		NewMapID = FindWorldMapID(Maps[CurrentMapID].WorldX, Maps[CurrentMapID].WorldY + 1, Maps[CurrentMapID].WorldMapID);//
		if(NewMapID == -1)
		{
			if(Maps[CurrentMapID].WorldMaxSize > 0 && WorldRefs[Maps[CurrentMapID].WorldMapID].size() >= Maps[CurrentMapID].WorldMaxSize)
			{
				SendMessage(PlayerList[PID].Peer, "You cannot <P>expand <W>any further.");
				return;
			}
			cMap NewMap;
			NewMap.SetType(OnCreateType);//Maps[CurrentMapID].MapType);
			NewMap.WorldMaxSize = Maps[CurrentMapID].WorldMaxSize;
			NewMap.Tile[PlayerX][0][0] = NewMap.MapType;//
			NewMap.WorldX = Maps[CurrentMapID].WorldX;//
			NewMap.WorldY = Maps[CurrentMapID].WorldY + 1;//
			NewMap.WorldMapID = Maps[CurrentMapID].WorldMapID;
			WorldRefs[NewMap.WorldMapID].push_back(Maps.size());
			Maps.push_back(NewMap);
		}
		else
		{
			if(Maps[NewMapID].ExitSide == MOVEDIR_UP)
			{
				SendMessage(PlayerList[PID].Peer, "You shouldnt do that.");
				return;
			}
			Maps[NewMapID].Tile[PlayerX][0][0] = Maps[NewMapID].MapType;//5
			
			SendGfxEvent(NewMapID, GFXEVENT_TFE, PlayerX, 0);
			for(unsigned int m = 0; m < Maps[NewMapID].PlayerRefs.size(); m++)
			{
				int PlayerRef = Maps[NewMapID].PlayerRefs[m];
				ostringstream Msg(ostringstream::out);
				Msg << "<C>Someone<W> " << SuccessMessage;
				SendMessage(PlayerList[PlayerRef].Peer, Msg.str());
				SendPlayerMap(PlayerList[PlayerRef].Peer, PlayerList[PlayerRef].MapID);
			}
		}
		Maps[CurrentMapID].Tile[PlayerX][PlayerY+1][0] = Maps[CurrentMapID].MapType;//6
		DestY++;
	}
	else
	{
		SendMessage(PlayerList[PID].Peer, "You <R>cant <W>do that here.");
		return;
	}
	
	for(size_t pc = 0; pc < Maps[CurrentMapID].PlayerRefs.size(); pc++)
	{
		int PlayerRef = Maps[CurrentMapID].PlayerRefs[pc];
		SendPlayerMap(PlayerList[PlayerRef].Peer, PlayerList[PID].MapID);
		ostringstream OS(ostringstream::out);
		OS << "<C>" << PlayerList[PID].Name << " <W>" << SuccessMessage;
		SendMessage(PlayerList[PlayerRef].Peer, OS.str());
	}

	for(size_t x = 0; x < PlayerList.size(); x++)
	{
		if(Maps[PlayerList[x].MapID].WorldMapID == Maps[PlayerList[PID].MapID].WorldMapID)
		{
			SendPlayerWorldMap(PlayerList[x].Peer, Maps[PlayerList[x].MapID].WorldMapID, Maps[PlayerList[x].MapID].WorldX, Maps[PlayerList[x].MapID].WorldY);
		}
	}
	
	SendGfxEvent(PlayerList[PID].MapID, GFXEVENT_TFE, DestX, DestY);
	
	SendPlayerUsedSkill(PID, -1);
}

int ServerState::FindWorldMapID(int X, int Y, int WorldID)
{
	for(size_t x = 0; x < WorldRefs[WorldID].size(); x++)
	{
		if(Maps[WorldRefs[WorldID][x]].WorldX == X && Maps[WorldRefs[WorldID][x]].WorldY == Y)
		{
			return WorldRefs[WorldID][x];
		}
	}

	return -1;
}

void ServerState::SendPlayerWorldMap(ENetPeer *To, int WorldID, int CenterX, int CenterY)
{
	int WorldMapData[31][31];
	for(int x = 0; x < 31; x++)
	{
		for(int y = 0; y < 31; y++)
		{
			WorldMapData[x][y] = 1;
		}
	}

	//Cycle through the maps
	for(unsigned int x = 0; x < Maps.size(); x++)
	{
		//If its on the same world
		if(Maps[x].WorldMapID == WorldID)
		{
			//And relevant to the current map
			if(Maps[x].WorldX > CenterX - 15 && Maps[x].WorldX < CenterX + 15 && Maps[x].WorldY > CenterY - 15 && Maps[x].WorldY < CenterY + 15)
			{
				//Figure out where it goes on the current map, relative to the center
				int PlotX = (Maps[x].WorldX - CenterX) + 15;
				int PlotY = (Maps[x].WorldY - CenterY) + 15;
				WorldMapData[PlotX][PlotY] = Maps[x].MapType-1;
			}
		}
	}

	//Generate the packet to send to the player
	int WorldMapPack[963];
	WorldMapPack[0] = PACKETID_DATAINT;
	WorldMapPack[1] = PACKETID_WORLDMAPDATA;
	for(int x = 0; x < 31; x++)
	{
		for(int y = 0; y < 31; y++)
		{
			WorldMapPack[2 + ((x * 30) + y)] = WorldMapData[x][y];
		}
	}

	SendIntPacket(To, WorldMapPack, 963);
}

void ServerState::CreateCave(int PID, int Direction)
{
	int CreateX = PlayerList[PID].X;
	int CreateY = PlayerList[PID].Y;
	int CreateMap = PlayerList[PID].MapID;

	switch(Direction)
	{
	case MOVEDIR_UP:
		CreateY--;
		break;
	case MOVEDIR_DOWN:
		CreateY++;
		break;
	case MOVEDIR_LEFT:
		CreateX--;
		break;
	case MOVEDIR_RIGHT:
		CreateX++;
		break;
	}
	if(CreateX < 2 || CreateY < 2 || CreateX > 9 || CreateY > 8)
	{
		SendMessage(PlayerList[PID].Peer, "You cant put that near the edge of a map.");
		return;
	}

	if(CheckBlocking(CreateX, CreateY, CreateMap) || CheckOverlay(CreateX, CreateY, CreateMap))
	{
		SendMessage(PlayerList[PID].Peer, "There is something in the way.");
		return;
	}

	if(Maps[CreateMap].MapType != MAPTYPE_SURFACE)
	{
		SendMessage(PlayerList[PID].Peer, "You must be on the surface to create a new cave.");
		return;
	}

	cMap NewMap;
	NewMap.SetType(MAPTYPE_CAVE);
	NewMap.WorldMapID = WorldRefs.size();
	NewMap.WorldX = 500;
	NewMap.WorldY = 500;
	NewMap.Tile[5][5][2] = 8;
	NewMap.AddExit(5, 5, "You head back to the <G>surface<W>.", CreateMap, CreateX, CreateY);
	Maps.push_back(NewMap);

	vector<int> x;
	x.push_back(Maps.size()-1);
	WorldRefs.push_back(x);

	Maps[CreateMap].Tile[CreateX][CreateY][2] = 7;
	Maps[CreateMap].AddExit(CreateX, CreateY, "You enter a <B>dark cave<W>.", Maps.size()-1, 5, 5);

	SendPlayerUsedSkill(PID, -1);
	SendGfxEvent(PlayerList[PID].MapID, GFXEVENT_TFCAVE, CreateX, CreateY);
		
	for(size_t pc = 0; pc < Maps[CreateMap].PlayerRefs.size(); pc++)
	{
		int PlayerRef = Maps[CreateMap].PlayerRefs[pc];
		SendPlayerMap(PlayerList[PlayerRef].Peer, PlayerList[PID].MapID);
		ostringstream OS(ostringstream::out);
		OS << "<C>" << PlayerList[PID].Name << "<W> waved his hands and a <Y>cave <W>rose from the <Y>ground<W>!";
		SendMessage(PlayerList[PlayerRef].Peer, OS.str());
	}
}

bool ServerState::CheckOverlay(int X, int Y, int MapID)
{
	if(Maps[MapID].Tile[X][Y][1] > 0)
		return true;

	return false;
}

void ServerState::CreateHouse(int PID, int Direction)
{
	int CreateX = PlayerList[PID].X;
	int CreateY = PlayerList[PID].Y;
	int CreateMap = PlayerList[PID].MapID;

	switch(Direction)
	{
	case MOVEDIR_UP:
		CreateY--;
		break;
	case MOVEDIR_DOWN:
		CreateY++;
		break;
	case MOVEDIR_LEFT:
		CreateX--;
		break;
	case MOVEDIR_RIGHT:
		CreateX++;
		break;
	}
	if(CreateX < 2 || CreateY < 2 || CreateX > 9 || CreateY > 8)
	{
		SendMessage(PlayerList[PID].Peer, "You cant put that near the edge of a map.");
		return;
	}

	if(CheckBlocking(CreateX, CreateY, CreateMap) || CheckOverlay(CreateX, CreateY, CreateMap))
	{
		SendMessage(PlayerList[PID].Peer, "There is something in the way.");
		return;
	}

	if(Maps[CreateMap].MapType != MAPTYPE_SURFACE)
	{
		SendMessage(PlayerList[PID].Peer, "You must be on the surface to create a new cave.");
		return;
	}
	
	cMap NewMap;
	NewMap.SetType(MAPTYPE_HOUSE);
	NewMap.WorldMapID = WorldRefs.size();
	NewMap.WorldMaxSize = 4;
	NewMap.WorldX = 500;
	NewMap.WorldY = 500;
	NewMap.Creator = PlayerList[PID].Name;
	NewMap.ExitSide = MOVEDIR_DOWN;
	NewMap.Tile[5][10][0] = NewMap.MapType;
	NewMap.Tile[6][10][0] = NewMap.MapType;
	NewMap.AddExit(5, 10, "You go back <G>outside<W>.", CreateMap, CreateX, CreateY);
	NewMap.AddExit(6, 10, "You go back <G>outside<W>.", CreateMap, CreateX, CreateY);
	Maps.push_back(NewMap);

	vector<int> x;
	x.push_back(Maps.size()-1);
	WorldRefs.push_back(x);

	Maps[CreateMap].Tile[CreateX][CreateY][2] = 11;
	Maps[CreateMap].AddExit(CreateX, CreateY, "You step through the <Y>doorway<W>.", Maps.size()-1, 5, 9);

	SendPlayerUsedSkill(PID, 0);
	SendGfxEvent(PlayerList[PID].MapID, GFXEVENT_BUHS, CreateX, CreateY);
	SendGfxEvent(PlayerList[PID].MapID, GFXEVENT_BUILDITEM, CreateX, CreateY);
		
	for(size_t pc = 0; pc < Maps[CreateMap].PlayerRefs.size(); pc++)
	{
		int PlayerRef = Maps[CreateMap].PlayerRefs[pc];
		SendPlayerMap(PlayerList[PlayerRef].Peer, PlayerList[PID].MapID);
		ostringstream OS(ostringstream::out);
		OS << "<C>" << PlayerList[PID].Name << "<W> works hard and <Y>builds<W> a <B>house <W>right before your eyes!";
		SendMessage(PlayerList[PlayerRef].Peer, OS.str());
	}

}

void ServerState::SaveMaps()
{
	for(unsigned int x = 0; x < Maps.size(); x++)
	{
		Maps[x].Save(x);
	}
}

void ServerState::LoadMaps()
{
	bool Loading = true;
	int CurrentFile = 0;
	cMap EmptyMap;
	int HighestWorldMapNum = -1;
	vector<int> EmptyIntVec;

	while(Loading)
	{
		if(Maps.size() == CurrentFile)
		{
			this->Maps.push_back(EmptyMap);
		}
		if(!Maps[CurrentFile].Load(CurrentFile) && CurrentFile > 0)
		{
			Loading = false;
			while((int)Maps.size() > CurrentFile)
				Maps.pop_back();
		}

		CurrentFile++;
	}

	for(size_t x = 0; x < Maps.size(); x++)
	{
		if(Maps[x].WorldMapID > HighestWorldMapNum)
			HighestWorldMapNum = Maps[x].WorldMapID;
	}

	for(int x = 0; x <= HighestWorldMapNum; x++)
	{
		WorldRefs.push_back(EmptyIntVec);
	}

	for(size_t x = 0; x < Maps.size(); x++)
	{
		int WorldID = Maps[x].WorldMapID;
		WorldRefs[WorldID].push_back(x);
	}
}

void ServerState::SendRoomDesc(ENetPeer *To, int MapID)
{
	int SeenHouses = 0;
	int SeenCaves = 0;
	int SeenLadders = 0;
	int SeenForge = 0;

	for(int x = 0; x < 12; x++)
	{
		for(int y = 0; y < 11; y++)
		{
			if(Maps[MapID].Tile[x][y][2] > 0)
			{
				if(Maps[MapID].Tile[x][y][2] == 3)
					SeenCaves++;
				else if(Maps[MapID].Tile[x][y][2] == 6)
					SeenHouses++;
				else if(Maps[MapID].Tile[x][y][2] == 5)
					SeenLadders++;
				else if(Maps[MapID].Tile[x][y][2] == 8)
					SeenForge++;
			}
		}
	}

	ostringstream Msg(ostringstream::out);
	vector<string> Exits;
	if(CheckForExit(MapID, MOVEDIR_UP))
	{
		Exits.push_back("north");
	}
	if(CheckForExit(MapID, MOVEDIR_RIGHT))
	{
		Exits.push_back("east");
	}
	if(CheckForExit(MapID, MOVEDIR_DOWN))
	{
		Exits.push_back("south");
	}
	if(CheckForExit(MapID, MOVEDIR_LEFT))
	{
		Exits.push_back("west");
	}

	if(Exits.size() > 0)
	{
		Msg << "Available <G>exits<W> are <C>";
		for(unsigned int x = 0; x < Exits.size(); x++)
		{
			if(x > 0)
			{
				Msg << "<W>, <C>";
			}
			Msg << Exits[x];
		}
		Msg << "<W>.";
		SendMessage(To, Msg.str());
		Msg.str("");
	}

	if(SeenCaves>0)
	{
		if(SeenCaves == 1)
		{
			Msg << "There is a <B>dark cave <W>here.";
		}
		else
		{
			Msg << "There are several <B>dark caves<W> here.";
		}
		SendMessage(To, Msg.str());
		Msg.str("");
	}

	if(SeenForge > 0)
	{
		SendMessage(To, "There is a <Y>forge <W>here.");
	}

	if(SeenHouses>0)
	{
		if(SeenHouses == 1)
		{
			Msg << "You see a cozy <Y>little house<W> ahead.";
		}
		else
		{
			Msg << "You see some <Y>houses <W>up ahead.";
		}
		SendMessage(To, Msg.str());
		Msg.str("");
	}
	if(SeenLadders>0)
	{
		if(SeenLadders == 1)
		{
			Msg << "There is a sturdy <Y>ladder <W>here that you can <C>climb<W>. It seems to lead to the <G>surface<W>.";
		}
		else
		{
			Msg << "There are several <Y>ladders <W>here that you can <C>climb<W>.";
		}
		SendMessage(To, Msg.str());
		Msg.str("");
	}

	if(Maps[MapID].ExitSide > 0)
	{
		Msg << "It seems you can go <C>";
		switch(Maps[MapID].ExitSide)
		{
		case MOVEDIR_UP:
			Msg << "north";
			break;
		case MOVEDIR_RIGHT:
			Msg << "east";
			break;
		case MOVEDIR_DOWN:
			Msg << "south";
			break;
		case MOVEDIR_LEFT:
			Msg << "west";
			break;
		}
		Msg << " <W>to go back <G>outside<W>.";
		SendMessage(To, Msg.str());
		Msg.str("");
	}
}

bool ServerState::CheckForExit(int MapID, int Direction)
{
	if(Direction == Maps[MapID].ExitSide)
		return false;

	if(Direction == MOVEDIR_UP)
	{
		for(int x = 1; x < 11; x++)
		{
			if(Maps[MapID].Tile[x][0][0] > 0)
				return true;
		}
	}
	else if(Direction == MOVEDIR_RIGHT)
	{
		for(int y = 1; y < 10; y++)
		{
			if(Maps[MapID].Tile[11][y][0] > 0)
				return true;
		}
	}
	else if(Direction == MOVEDIR_DOWN)
	{
		for(int x = 1; x < 11; x++)
		{
			if(Maps[MapID].Tile[x][10][0] > 0)
				return true;
		}
	}
	else if(Direction == MOVEDIR_LEFT)
	{
		for(int y = 1; y < 10; y++)
		{
			if(Maps[MapID].Tile[0][y][0] > 0)
				return true;
		}
	}
	return false;
}

void ServerState::SendAddName(ENetPeer *To, int PlayerID)
{
	int NNPacket[] = {PACKETID_DATAINT, PACKETID_NEXTNAMEID, PlayerList[PlayerID].ID};
	SendIntPacket(To, NNPacket, 3);

	int PacketLen = PlayerList[PlayerID].Name.length() + 3;
	char *Packet = new char[PacketLen];

	Packet[0] = (char)PACKETID_DATASTART;
	Packet[1] = (char)PACKETID_ADDNAME;
	Packet[2] = PlayerList[PlayerID].Name.length();
	for(int x = 0; x < PlayerList[PlayerID].Name.length(); x++)
	{
		Packet[3 + x] = PlayerList[PlayerID].Name[x];
	}

	SendDataPacket(To, Packet, PacketLen);
}

void ServerState::SendAddPlayer(ENetPeer *To, int PlayerID)
{
	int Packet[] = {PACKETID_DATAINT, PACKETID_ADDPLAYER, PlayerList[PlayerID].ID};
	SendIntPacket(To, Packet, 3);
	SendAddName(To, PlayerID);
	SendPlayerLoc(To, PlayerID);
	SendPlayerInfo(To, PlayerID);
	SendUpdateEquip(To, PlayerID);
}

void ServerState::SendRemovePlayer(ENetPeer *To, int PlayerID)
{
	int Packet[] = {PACKETID_DATAINT, PACKETID_PLAYERLEFT, PlayerList[PlayerID].ID};
	SendIntPacket(To, Packet, 3);
}

void ServerState::SendPlayerLoc(ENetPeer *To, int PlayerID)
{
	int Packet[] = {PACKETID_DATAINT, PACKETID_UPDATEPLAYERXY, PlayerList[PlayerID].ID, PlayerList[PlayerID].SpriteDir, (PlayerList[PlayerID].X*32)+PlayerList[PlayerID].XOff, (PlayerList[PlayerID].Y*32)+PlayerList[PlayerID].YOff};
	SendIntPacket(To, Packet, 6);
}

ServerState::~ServerState()
{
	SaveMaps();
}

void ServerState::SendPlayerInfo(ENetPeer *To, int PlayerID)
{
	int Packet[] = {PACKETID_DATAINT, PACKETID_PLAYERINFO, PlayerList[PlayerID].ID, PlayerList[PlayerID].Sex, PlayerList[PlayerID].SkinTone, PlayerList[PlayerID].HairStyle, PlayerList[PlayerID].HairR, PlayerList[PlayerID].HairG, PlayerList[PlayerID].HairB};
	SendIntPacket(To, Packet, 9);
}

void ServerState::PCEquipItem(int PlayerID, int EquipSlot, int InventorySlot)
{
	cObj Temp = PlayerList[PlayerID].Inventory[InventorySlot];
	PlayerList[PlayerID].Inventory[InventorySlot] = PlayerList[PlayerID].EquipmentSlot[EquipSlot];
	PlayerList[PlayerID].EquipmentSlot[EquipSlot] = Temp;
}

int ServerState::PCUnequipItem(int PlayerID, int EquipSlot)
{
	cObj NullObj;
	int OpenSlot = PlayerList[PlayerID].FindEmptySlot();
	if(OpenSlot < 0)
		return -1;

	PlayerList[PlayerID].Inventory[OpenSlot] = PlayerList[PlayerID].EquipmentSlot[EquipSlot];
	PlayerList[PlayerID].EquipmentSlot[EquipSlot] = NullObj;
	return 0;
}

void ServerState::SendUpdateEquip(ENetPeer *To, int PlayerID)
{
	if(To == NULL)
	{
		for(size_t pc = 0; pc < Maps[PlayerList[PlayerID].MapID].PlayerRefs.size(); pc++)
		{		
			for(int x = 0; x < EQUIPSLOT_MAX; x++)
			{
				int Packet[] = {PACKETID_DATAINT, PACKETID_CHANGEEQUIP, PlayerList[PlayerID].ID, x, PlayerList[PlayerID].EquipmentSlot[x].Index, PlayerList[PlayerID].EquipmentSlot[x].Type};
				
				int PlayerRef = Maps[PlayerList[PlayerID].MapID].PlayerRefs[pc];
				SendIntPacket(PlayerList[PlayerRef].Peer, Packet, 6);
			}
		}
	}
	else
	{
		for(int x = 0; x < EQUIPSLOT_MAX; x++)
		{
			int Packet[] = {PACKETID_DATAINT, PACKETID_CHANGEEQUIP, PlayerList[PlayerID].ID, x, PlayerList[PlayerID].EquipmentSlot[x].Index, PlayerList[PlayerID].EquipmentSlot[x].Type};
			SendIntPacket(To, Packet, 6);
		}
	}
}

void ServerState::PlayerDropObject(int PlayerID, int ObjRef)
{
	if(ObjRef < PlayerList[PlayerID].Inventory.size() && ObjRef >= 0)
	{
		Maps[PlayerList[PlayerID].MapID].AddObject(PlayerList[PlayerID].Inventory[ObjRef], PlayerList[PlayerID].X, PlayerList[PlayerID].Y);
		SendMapObjects(PlayerList[PlayerID].MapID);

		cObj NullObj;
		PlayerList[PlayerID].Inventory[ObjRef] = NullObj;
	}
}

void ServerState::BuildItem(int PlayerID, int ItemID, int Dir)
{
	int CreateX = PlayerList[PlayerID].X;
	int CreateY = PlayerList[PlayerID].Y;
	int CreateMap = PlayerList[PlayerID].MapID;

	switch(Dir)
	{
	case MOVEDIR_UP:
		CreateY--;
		break;
	case MOVEDIR_DOWN:
		CreateY++;
		break;
	case MOVEDIR_LEFT:
		CreateX--;
		break;
	case MOVEDIR_RIGHT:
		CreateX++;
		break;
	}

	if(CreateX < 2 || CreateY < 2 || CreateX > 9 || CreateY > 8)
	{
		SendMessage(PlayerList[PlayerID].Peer, "You cant put that near the edge of a map.");
		return;
	}

	if(CheckBlocking(CreateX, CreateY, CreateMap) || CheckOverlay(CreateX, CreateY, CreateMap))
	{
		SendMessage(PlayerList[PlayerID].Peer, "There is something in the way.");
		return;
	}

	Maps[CreateMap].Tile[CreateX][CreateY][2] = ItemID;

	OSStr(Msg);

	switch(ItemID)
	{
	case BUILDITEM_BOOKSHELF:
		Msg << PlayerList[PlayerID].Name << " assembles a <Y>bookshelf <W>and fills it up with all sorts of <C>books<W>.";
		break;

	case BUILDITEM_CHAIR:
		Msg << PlayerList[PlayerID].Name << " builds a small <Y>chair <W>for people to sit in.";
		break;

	case BUILDITEM_TABLE:
		Msg << PlayerList[PlayerID].Name << " builds a nice <Y>table<W>.";
		break;
	}

	SendPlayerUsedSkill(PlayerID, 0);
	SendGfxEvent(CreateMap, GFXEVENT_BUILDITEM, CreateX, CreateY);

	for(size_t x = 0; x < Maps[CreateMap].PlayerRefs.size(); x++)
	{
		SendMessage(PlayerList[Maps[CreateMap].PlayerRefs[x]].Peer, Msg.str());
		SendPlayerMap(PlayerList[Maps[CreateMap].PlayerRefs[x]].Peer, CreateMap);
	}
}

void ServerState::CreateForge(int PID, int Direction)
{
	int CreateX = PlayerList[PID].X;
	int CreateY = PlayerList[PID].Y;
	int CreateMap = PlayerList[PID].MapID;

	switch(Direction)
	{
	case MOVEDIR_UP:
		CreateY--;
		break;
	case MOVEDIR_DOWN:
		CreateY++;
		break;
	case MOVEDIR_LEFT:
		CreateX--;
		break;
	case MOVEDIR_RIGHT:
		CreateX++;
		break;
	}
	if(CreateX < 2 || CreateY < 2 || CreateX > 9 || CreateY > 8)
	{
		SendMessage(PlayerList[PID].Peer, "You cant put that near the edge of a map.");
		return;
	}

	if(CheckBlocking(CreateX, CreateY, CreateMap) || CheckOverlay(CreateX, CreateY, CreateMap))
	{
		SendMessage(PlayerList[PID].Peer, "There is something in the way.");
		return;
	}

	Maps[PlayerList[PID].MapID].Tile[CreateX][CreateY][2] = 14;
	SendPlayerUsedSkill(PID, 0);
	SendGfxEvent(PlayerList[PID].MapID, GFXEVENT_BUFO, CreateX, CreateY);
	SendGfxEvent(PlayerList[PID].MapID, GFXEVENT_BUILDITEM, CreateX, CreateY);
		
	for(size_t pc = 0; pc < Maps[PlayerList[PID].MapID].PlayerRefs.size(); pc++)
	{
		int PlayerRef = Maps[PlayerList[PID].MapID].PlayerRefs[pc];
		SendPlayerMap(PlayerList[PlayerRef].Peer, PlayerList[PID].MapID);
		ostringstream OS(ostringstream::out);
		OS << "<C>" << PlayerList[PID].Name << "<W> works hard and <Y>builds<W> a <B>forge <W>right before your eyes!";
		SendMessage(PlayerList[PlayerRef].Peer, OS.str());
	}

}

void ServerState::SendMapObjects(ENetPeer *To)
{
	int PlayerID = FindPlayerID(To);
	int MapID = PlayerList[PlayerID].MapID;
	int NumObjects = Maps[MapID].Objects.size();
	int Packet[] = {PACKETID_DATAINT, PACKETID_ADDMAPOBJ, 0, 0, 0, 0, 0};
	
	for(size_t x = 0; x < Maps[MapID].Objects.size(); x++)
	{
		Packet[2] = Maps[MapID].Objects[x].Type;
		Packet[3] = Maps[MapID].Objects[x].Index;
		Packet[4] = Maps[MapID].Objects[x].X;
		Packet[5] = Maps[MapID].Objects[x].Y;
		Packet[6] = Maps[MapID].Objects[x].ID;
		SendIntPacket(To, Packet, 7);
	}
}

void ServerState::SendMapObjects(int MapID)
{
	for(size_t pc = 0; pc < Maps[MapID].PlayerRefs.size(); pc++)
	{
		int PlayerRef = Maps[MapID].PlayerRefs[pc];
			SendMapObjects(PlayerList[PlayerRef].Peer);
	}
}

int ServerState::GetObjectID(string Name)
{
	for(unsigned int x = 0; x < GameItems.size(); x++)
	{
		if(StrToLower(GameItems[x].Name) == StrToLower(Name))
			return x;
	}
	return -1;
}

int ServerState::FindPlayerObjRefByName(int PlayerID, string Str, int Start)
{
	for(size_t x = 0; x < PlayerList[PlayerID].Inventory.size(); x++)
	{
		if(CheckParam(PlayerList[PlayerID].Inventory[x].Name, Str, Start))
		{
			return x;
		}
	}

	return -1;
}

int ServerState::CheckParamForItem(string Str, int Start)
{
	int Ret = -1;
	for(unsigned int x = 0; x < GameItems.size(); x++)
	{
		if(CheckParam(GameItems[x].Name, Str, Start))
		{
			Ret = x;
		}
	}
	return Ret;
}

class RecievedPacketNeedle
{
public:
	RecievedPacketNeedle(ServerState *SS, ENetPeer *P) { PeerHolder = P; Parent = SS;}
	void operator()()
	{
		Parent->ProcessRecievedPacket(PeerHolder);
	}
	ENetPeer *PeerHolder;
	ServerState *Parent;
};

void ServerState::ProcessNetEvent(ENetEvent NetEvent)
{
	ostringstream OutMsg(ostringstream::out);
	int PlayerID = -1;

	switch(NetEvent.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			OutMsg << "<B>New connection from: <C>" << NetEvent.peer->address.host << " [" << NetEvent.peer->connectID << "]";
			ConsoleWrite(OutMsg.str().c_str());
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			PlayerID = FindPlayerID(NetEvent.peer);
			if(PlayerID >= 0)
			{
				DisconnectPlayer(PlayerID);
			}
			break;

		case ENET_EVENT_TYPE_RECEIVE:
			if(NetEvent.packet->data[0] == 200)
			{
				if(NetEvent.packet->dataLength > 1)
				{
					ProcessDataPacket(NetEvent.peer, NetEvent.packet);
				}
			}
			else
			{
				ParsePacket(NetEvent.packet);

				OutMsg << "Recv[";
				int FoundPlayer = FindPlayerID(NetEvent.peer);
				if(FoundPlayer >= 0)
				{
					OutMsg << PlayerList[FoundPlayer].Name;
				}
				else
				{
					 OutMsg << NetEvent.peer->address.host;
				}
				OutMsg << "]: " << PacketString;
				ConsoleWrite(OutMsg.str().c_str());

				RecievedPacketNeedle Needle(this, NetEvent.peer);
				boost::thread PThread(Needle);
			}
			enet_packet_destroy(NetEvent.packet);
			break;
		};
}

void ServerState::TeleportPlayer(int PlayerID, int X, int Y, int Map, string Message)
{
	//Remove player from map, remove all map data from player
	int ClearMapPacket[] = {PACKETID_DATAINT, PACKETID_CLEARALLMAP};
	SendIntPacket(PlayerList[PlayerID].Peer, ClearMapPacket, 2);
	for(size_t x = 0; x < Maps[PlayerList[PlayerID].MapID].PlayerRefs.size(); x++)
	{
		SendRemovePlayer(PlayerList[Maps[PlayerList[PlayerID].MapID].PlayerRefs[x]].Peer, PlayerID);
	}
	
	//Update data
	Maps[PlayerList[PlayerID].MapID].PlayerLeftMap(PlayerID);
	PlayerList[PlayerID].MapID = Map;
	PlayerList[PlayerID].X = X;
	PlayerList[PlayerID].Y = Y;
	Maps[Map].PlayerEnteredMap(PlayerID);

	//Send player map data
	SendPlayerMap(PlayerList[PlayerID].Peer, Map);
	SendPlayerWorldMap(PlayerList[PlayerID].Peer, Maps[Map].WorldMapID, Maps[Map].WorldX, Maps[Map].WorldY);
	SendMapObjects(PlayerList[PlayerID].Peer);

	//Add player to his own data
	SendAddPlayer(PlayerList[PlayerID].Peer, PlayerID);

	//Add player to everyone on new maps data
	for(size_t x = 0; x < Maps[Map].PlayerRefs.size(); x++)
	{
		SendAddPlayer(PlayerList[Maps[Map].PlayerRefs[x]].Peer, PlayerID);
		//Also add everyone to players map
		SendAddPlayer(PlayerList[PlayerID].Peer, Maps[Map].PlayerRefs[x]);
	}

	//Send custom message, and room description to player
	SendMessage(PlayerList[PlayerID].Peer, Message);
	SendRoomDesc(PlayerList[PlayerID].Peer, PlayerList[PlayerID].MapID);
}

void ServerState::PlayerGotoBattle(int PlayerID, int BattleBG)
{
	int Packet[] = { PACKETID_DATAINT, PACKETID_GOTOBATTLE, BattleBG };

	int PartySize = PlayerList[PlayerID].PartySize + 1;
	int PartyRef[] = {FindPlayerID(PlayerList[PlayerID].Party[0]), FindPlayerID(PlayerList[PlayerID].Party[1]), FindPlayerID(PlayerList[PlayerID].Party[2])};
	
	
	cMob Slime;
	GenerateBattleMobs(PlayerID, Slime, Rand(1, 6));

	for(int x = 0; x < PartySize; x++)
	{
		if(PartyRef[x] < 0)
			continue;

		SendIntPacket(PlayerList[PartyRef[x]].Peer, Packet, 3);
		PlayerList[PartyRef[x]].InBattle = true;

		for(unsigned int pc = 0; pc < Maps[PlayerList[PartyRef[0]].MapID].PlayerRefs.size(); pc++)
		{
			int PlayerRef = Maps[PlayerList[PlayerID].MapID].PlayerRefs[pc];
			if(PlayerRef != PartyRef[x])
			{
				SendRemovePlayer(PlayerList[pc].Peer, PlayerID);
				SendRemovePlayer(PlayerList[PartyRef[x]].Peer, pc);
			}
		}
		SendRemovePlayer(PlayerList[PartyRef[x]].Peer, PartyRef[x]);

		SendClearMobs(PartyRef[x]);

		for(size_t m = 0; m < PlayerList[PlayerID].BattleMob.size(); m++)
		{
			SendAddMob(PartyRef[x], PlayerList[PlayerID].BattleMob[m]);
		}
		

		SendMessage(PlayerList[PlayerID].Peer, " ");
		SendMessage(PlayerList[PlayerID].Peer, " ");
		SendMessage(PlayerList[PlayerID].Peer, "You were <Y>attacked <W>by <R>Slimes<W>!");
		SendMessage(PlayerList[PlayerID].Peer, " ");
		SendMessage(PlayerList[PlayerID].Peer, "Available options are:");
		SendMessage(PlayerList[PlayerID].Peer, "<C>(A)ttack");
		SendMessage(PlayerList[PlayerID].Peer, "<C>(D)efend");
		SendMessage(PlayerList[PlayerID].Peer, "<C>(C)ast");
		SendMessage(PlayerList[PlayerID].Peer, "<C>(U)se");
		SendMessage(PlayerList[PlayerID].Peer, "<C>(R)un");
		SendMessage(PlayerList[PlayerID].Peer, "Eponick: <R>Health: 20/20   <C>Energy: 10/10   <G>Exp: 5/20");
	}

	for(int x = 0; x < PartySize; x++)
	{
		for(int y = 0; y < PartySize; y++)
		{
			SendAddPlayer(PlayerList[PartyRef[x]].Peer, PartyRef[y]);
			int PacketLoc[] = {PACKETID_DATAINT, PACKETID_UPDATEPLAYERXY, PlayerList[PartyRef[y]].ID, MOVEDIR_LEFT-1, 10 * 32, ((6 + (y*2))  * 32)};
			SendIntPacket(PlayerList[x].Peer, PacketLoc, 6);
		}
	}
}

void ServerState::PlayerExitBattle(int PlayerID)
{
	PlayerList[PlayerID].InBattle = false;
	TeleportPlayer(PlayerID, PlayerList[PlayerID].X, PlayerList[PlayerID].Y, PlayerList[PlayerID].MapID, "Got away safely.");

	int Packet[] = {PACKETID_DATAINT, PACKETID_EXITBATTLE};
	SendIntPacket(PlayerList[PlayerID].Peer, Packet, 2);

	SendClearMobs(PlayerID);
}

void ServerState::PlayerBattleAttack(int PlayerID)
{
}

void ServerState::PlayerBattleRun(int PlayerID)
{
	PlayerExitBattle(PlayerID);
}

void ServerState::PlayerBattleDefend(int PlayerID)
{
}

void ServerState::SendAddMob(int PlayerID, cMob Mob)
{
	int Packet[] = {PACKETID_DATAINT, PACKETID_ADDMOB, Mob.Type, Mob.Index, Mob.X * 32, Mob.Y * 32};
	SendIntPacket(PlayerList[PlayerID].Peer, Packet, 6);
}

void ServerState::SendMobLoc(int PlayerID, int MobRef, int X, int Y)
{
	int Packet[] = {PACKETID_DATAINT, PACKETID_UPDATEMOBXY, PlayerList[PlayerID].MapID, MobRef, X, Y};
	SendIntPacket(PlayerList[PlayerID].Peer, Packet, 6);
}

void ServerState::SendRemoveMob(int PlayerID, int MobRef)
{
	int Packet[] = {PACKETID_DATAINT, PACKETID_DELETEMOB, PlayerList[PlayerID].MapID, MobRef};
	SendIntPacket(PlayerList[PlayerID].Peer, Packet, 4);
}

void ServerState::SendClearMobs(int PlayerID)
{
	int Packet[] = {PACKETID_DATAINT, PACKETID_CLEARMOBS};
	SendIntPacket(PlayerList[PlayerID].Peer, Packet, 2);
}

void ServerState::GenerateBattleMobs(int PlayerID, cMob Mob, int NumberMobs)
{
	if(NumberMobs > 5)
		NumberMobs = 5;
	if(NumberMobs <= 0)
		NumberMobs = 1;

	PlayerList[PlayerID].BattleMob.clear();

	for(int x = 0; x < NumberMobs; x++)
	{
		Mob.X = (x % 2) + 1;
		Mob.Y = 6 + x;
		PlayerList[PlayerID].BattleMob.push_back(Mob);
	}
}

void ServerState::HSTick()
{
	if(NeedsSave >= 60)
	{
		NeedsSave = 0;
		ConsoleWrite("<G>Saving world.");
		SaveMaps();
		for(size_t x = 0; x < PlayerList.size(); x++)
		{
			PlayerList[x].Save();
		}
	}
	NeedsSave ++;

	if(PlayerList.size() < 1)
		return;

	for(size_t x = 0; x < PlayerList.size(); x++)
	{
		if(PlayerList[x].UsingSkill > 0)
			PlayerList[x].UsingSkill--;
	}

	/*vector<int> MapIDs;
	for(size_t x = 0; x < PlayerList.size(); x++)
	{
		MapIDs.push_back(PlayerList[x].MapID);
	}

	sort(MapIDs.begin(), MapIDs.end());
	MapIDs.erase(unique(MapIDs.begin(), MapIDs.end()));

	for(size_t x = 0; x < MapIDs.size(); x++)
	{
		for(size_t m = 0; m < Maps[MapIDs[x]].Mobs.size(); m++)
		{
			Maps[MapIDs[x]].Mobs[m].HSTick();
		}
	}*/

	GameMinute++;
	if(GameMinute == 60)
	{
		GameMinute = 0;
		GameHour++;
		if(GameHour == 24)
		{
			GameHour = 0;
			GameDay++;
		}
	}
	SendGameTime();
}

void ServerState::SendGameTime()
{
	int Packet[] = {PACKETID_DATAINT, PACKETID_GAMETIME, GameDay, GameHour, GameMinute};
	for(size_t x = 0; x < PlayerList.size(); x++)
	{
		SendIntPacket(PlayerList[x].Peer, Packet, 5);
	}
}

void ServerState::SendGfxEvent(int MapID, int EventID, int X, int Y)
{
	int WorldID = Maps[MapID].WorldMapID;
	int OrigX = Maps[MapID].WorldX;
	int OrigY = Maps[MapID].WorldY;

	int Packet[] = {PACKETID_DATAINT, PACKETID_GFXEVENT, EventID, X, Y, MapID};

	vector<int> MapsOccur;
	MapsOccur.push_back(MapID);
	int AddMap = -1;
	if((AddMap=FindWorldMapID(OrigX-1, OrigY, WorldID)) > -1)
		MapsOccur.push_back(AddMap);
	if((AddMap=FindWorldMapID(OrigX-2, OrigY, WorldID)) > -1)
		MapsOccur.push_back(AddMap);
	if((AddMap=FindWorldMapID(OrigX+1, OrigY, WorldID)) > -1)
		MapsOccur.push_back(AddMap);
	if((AddMap=FindWorldMapID(OrigX+2, OrigY, WorldID)) > -1)
		MapsOccur.push_back(AddMap);
	if((AddMap=FindWorldMapID(OrigX, OrigY-1, WorldID)) > -1)
		MapsOccur.push_back(AddMap);
	if((AddMap=FindWorldMapID(OrigX, OrigY-2, WorldID)) > -1)
		MapsOccur.push_back(AddMap);
	if((AddMap=FindWorldMapID(OrigX, OrigY+1, WorldID)) > -1)
		MapsOccur.push_back(AddMap);
	if((AddMap=FindWorldMapID(OrigX, OrigY+2, WorldID)) > -1)
		MapsOccur.push_back(AddMap);
	if((AddMap=FindWorldMapID(OrigX-1, OrigY-1, WorldID)) > -1)
		MapsOccur.push_back(AddMap);
	if((AddMap=FindWorldMapID(OrigX+1, OrigY-1, WorldID)) > -1)
		MapsOccur.push_back(AddMap);
	if((AddMap=FindWorldMapID(OrigX-1, OrigY+1, WorldID)) > -1)
		MapsOccur.push_back(AddMap);
	if((AddMap=FindWorldMapID(OrigX+1, OrigY+1, WorldID)) > -1)
		MapsOccur.push_back(AddMap);

	for(size_t m = 0; m < MapsOccur.size(); m++)
	{
		for(size_t x = 0; x < Maps[MapsOccur[m]].PlayerRefs.size(); x++)
		{
			SendIntPacket(PlayerList[Maps[MapsOccur[m]].PlayerRefs[x]].Peer, Packet, 6);
		}
	}

}

void ServerState::SendPlayerUsedSkill(int PlayerID, int ShowWeapon)
{
	PlayerList[PlayerID].UsingSkill = 4;
	int Packet[] = {PACKETID_DATAINT, PACKETID_PLUSEDSK, PlayerList[PlayerID].ID, ShowWeapon};
	for(size_t x = 0; x < Maps[PlayerList[PlayerID].MapID].PlayerRefs.size(); x++)
	{
		int PlayerRef = Maps[PlayerList[PlayerID].MapID].PlayerRefs[x];
		SendIntPacket(PlayerList[PlayerRef].Peer, Packet, 3);
	}
}

void ServerState::SendPlayerClearGfx(int PlayerID)
{
	int Packet[] = {PACKETID_DATAINT, PACKETID_CLEARGFXEV};
	SendIntPacket(PlayerList[PlayerID].Peer, Packet, 2);
}

int ServerState::GetPlayerDir(int PlayerID, int MapID, int SourceX, int SourceY, bool CheckBlock, bool StepAround)
{
	if(MapID != PlayerList[PlayerID].MapID)
		return -1;

	int PlayerMoving = 0;
	int PX = (PlayerList[PlayerID].X * 32) + PlayerList[PlayerID].XOff;
	int PY = (PlayerList[PlayerID].Y * 32) + PlayerList[PlayerID].YOff;
	PX /= 32;
	PY /= 32;
	if(PlayerList[PlayerID].Moving > 0)
		PlayerMoving = PlayerList[PlayerID].Moving;
	else if(PlayerList[PlayerID].TakingStep > 0)
		PlayerMoving = PlayerList[PlayerID].TakingStep;

	int CheckOrder[4] = {0, 0, 0, 0};

	if(PX < SourceX)
	{
		CheckOrder[0] = MOVEDIR_LEFT;
		CheckOrder[1] = MOVEDIR_UP;
		CheckOrder[2] = MOVEDIR_DOWN;
		CheckOrder[3] = MOVEDIR_RIGHT;
	}
	else if(PX > SourceX)
	{
		CheckOrder[0] = MOVEDIR_RIGHT;
		CheckOrder[1] = MOVEDIR_UP;
		CheckOrder[2] = MOVEDIR_DOWN;
		CheckOrder[3] = MOVEDIR_LEFT;
	}
	else if(PY < SourceY)
	{
		CheckOrder[0] = MOVEDIR_UP;
		CheckOrder[1] = MOVEDIR_LEFT;
		CheckOrder[2] = MOVEDIR_RIGHT;
		CheckOrder[3] = MOVEDIR_DOWN;
	}
	else if(PY > SourceY)
	{
		CheckOrder[0] = MOVEDIR_DOWN;
		CheckOrder[1] = MOVEDIR_RIGHT;
		CheckOrder[2] = MOVEDIR_LEFT;
		CheckOrder[3] = MOVEDIR_UP;
	}

	if(!CheckBlock)
	{
		return CheckOrder[0];
	}
	
	int Limit = 1;
	if(StepAround)
		Limit = 4;
	for(int x = 0; x < Limit; x++)
	{
		switch(CheckOrder[x])
		{
		case MOVEDIR_LEFT:
			if(!CheckBlocking(SourceX - 1, SourceY, MapID))
				return CheckOrder[x];
			break;
			
		case MOVEDIR_RIGHT:
			if(!CheckBlocking(SourceX + 1, SourceY, MapID))
				return CheckOrder[x];
			break;

		case MOVEDIR_UP:
			if(!CheckBlocking(SourceX, SourceY - 1, MapID))
				return CheckOrder[x];
			break;
			
		case MOVEDIR_DOWN:
			if(!CheckBlocking(SourceX, SourceY + 1, MapID))
				return CheckOrder[x];
			break;

		default:
			break;
		}
	}

	return -1;
}

int ServerState::GetPlayerDir(int PlayerID, int MapID, int X, int Y)
{
	return GetPlayerDir(PlayerID, MapID, X, Y, false, false);
}

void ServerState::DisconnectPlayer(int PlayerID)
{
	Maps[PlayerList[PlayerID].MapID].PlayerLeftMap(PlayerID);
	OSStr(OutMsg);
	PlayerList[PlayerID].Save();
	UserCmd_Leave(PlayerID);
	OutMsg << "<C>" << PlayerList[PlayerID].Name << " <W>has left us.";
	SendMessage(NULL, OutMsg.str());
	ConsoleWrite(OutMsg.str().c_str());
	for(size_t x = 0; x < Maps[PlayerList[PlayerID].MapID].PlayerRefs.size(); x++)
	{
		SendRemovePlayer(PlayerList[Maps[PlayerList[PlayerID].MapID].PlayerRefs[x]].Peer, PlayerID);
	}
	AssignedIDPool[PlayerList[PlayerID].ID] = false;
	PlayerList.erase(PlayerList.begin()+PlayerID);
}

void ServerState::CreatePath(int MapID, int X, int Y)
{
	Maps[MapID].Tile[X][Y][1] = 4;

	for(int tx = -1; tx < 2; tx++)
	{
		for(int ty = -1; ty < 2; ty++)
		{
			if(X + tx < 0 || Y + ty < 0 || X + tx > 11 || Y + ty > 10)
			{
				continue;
			}

			FixPathTile(MapID, X+tx, Y+ty, PATHTYPE_BRICK);
		}
	}
}

void ServerState::FixPathTile(int MapID, int X, int Y, int PathID)
{
	int TileGroup[3][3];

	for(int tx = -1; tx < 2; tx++)
	{
		for(int ty = -1; ty < 2; ty++)
		{
			TileGroup[tx+1][ty+1] = -1;
			if(X + tx < 0 || Y + ty < 0 || X + tx > 11 || Y + ty > 10)
			{
				continue;
			}

			if(Maps[MapID].Tile[X+tx][Y+ty][1] > 0)
				TileGroup[tx+1][ty+1] = Maps[MapID].Tile[X+tx][Y+ty][1]-1;
		}
	}

	//The lone tile
	int TileOut = -1;
	

	//						--Full Paths
	//Center Tile
	if(	TileGroup[0][0] >= 0 && TileGroup[1][0] >= 0 && TileGroup[2][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] >= 0 && TileGroup[1][2] >= 0 && TileGroup[2][2] >= 0 )
	{
		TileOut = 9;
	}
	//Left Tile
	else
	if(							TileGroup[1][0] >= 0 && TileGroup[2][0] >= 0 &&
		TileGroup[0][1] <  0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] >= 0 && TileGroup[2][2] >= 0 )
	{
		TileOut = 8;
	}
	//Right Tile
	else
	if(	TileGroup[0][0] >= 0 && TileGroup[1][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] <  0 &&
		TileGroup[0][2] >= 0 && TileGroup[1][2] >= 0 						 )
	{
		TileOut = 10;
	}
	//Top Tile
	else
	if(							TileGroup[1][0] <  0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] >= 0 && TileGroup[1][2] >= 0 && TileGroup[2][2] >= 0 )
	{
		TileOut = 1;
	}
	//Bottom Tile
	else
	if(	TileGroup[0][0] >= 0 && TileGroup[1][0] >= 0 && TileGroup[2][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] <  0						 )
	{
		TileOut = 17;
	}
	//Top Left
	else
	if(							TileGroup[1][0] <  0 && 
		TileGroup[0][1] <  0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] >= 0 && TileGroup[2][2] >= 0 )
	{
		TileOut = 0;
	}
	//Top Right
	else
	if(							TileGroup[1][0] <  0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] <  0 &&
		TileGroup[0][2] >= 0 && TileGroup[1][2] >= 0						 )
	{
		TileOut = 2;
	}
	//Bottom Left
	else
	if(							TileGroup[1][0] >= 0 && TileGroup[2][0] >= 0 &&
		TileGroup[0][1] <  0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] <  0						 )
	{
		TileOut = 16;
	}
	//Bottom Right
	else
	if(	TileGroup[0][0] >= 0 && TileGroup[1][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] <  0 &&
								TileGroup[1][2] <  0						 )
	{
		TileOut = 18;
	}
	//Split to two single paths v>
	else
	if(	TileGroup[0][0] >= 0 && TileGroup[1][0] >= 0 && TileGroup[2][0] <  0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] <  0 && TileGroup[1][2] >= 0 && TileGroup[2][2] <  0 )
	{
		TileOut = 36;
	}
	//Split to two single paths <v
	else
	if(	TileGroup[0][0] <  0 && TileGroup[1][0] >= 0 && TileGroup[2][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] <  0 && TileGroup[1][2] >= 0 && TileGroup[2][2] <  0 )
	{
		TileOut = 37;
	}
	//Split to two single paths ^>
	else
	if(	TileGroup[0][0] <  0 && TileGroup[1][0] >= 0 && TileGroup[2][0] <  0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] >= 0 && TileGroup[1][2] >= 0 && TileGroup[2][2] < 0 )
	{
		TileOut = 44;
	}
	//Split to two single paths <^
	else
	if(	TileGroup[0][0] <  0 && TileGroup[1][0] >= 0 && TileGroup[2][0] <  0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] <  0 && TileGroup[1][2] >= 0 && TileGroup[2][2] >= 0 )
	{
		TileOut = 45;
	}
	//Bottom Right Inner Corner on Edge
	else
	if(							TileGroup[1][0] <  0 && 
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] >= 0 && TileGroup[1][2] >= 0						 )
	{
		TileOut = 33;
	}
	//Bottom Left Inner Corner on Edge
	else
	if(							TileGroup[1][0] <  0 && 
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] >= 0 && TileGroup[2][2] >= 0 )
	{
		TileOut = 32;
	}
	//Top Left Inner Corner On Edge
	else
	if(							TileGroup[1][0] >= 0 && TileGroup[2][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] <  0						 )
	{
		TileOut = 40;
	}
	//Top Right Inner Corner On Edge
	else
	if(	TileGroup[0][0] >= 0 && TileGroup[1][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] <  0						 )
	{
		TileOut = 41;
	}
	//Down Left Inner Corner on Edge >
	else
	if(							TileGroup[1][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] < 0 &&
		TileGroup[0][2] >= 0 && TileGroup[1][2] >= 0						 )
	{
		TileOut = 43;
	}
	//Up Left Inner Corner on Edge
	else
	if(	TileGroup[0][0] >= 0 && TileGroup[1][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] <  0 &&
								TileGroup[1][2] >= 0						 )
	{
		TileOut = 35;
	}
	//Down Right Inner Corner on Edge
	else
	if(							TileGroup[1][0] >= 0 && 
		TileGroup[0][1] <  0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] >= 0 && TileGroup[2][2] >= 0 )
	{
		TileOut = 42;
	}
	//Up Right Inner Corner on Edge
	else
	if(							TileGroup[1][0] >= 0 && TileGroup[2][0] >= 0 &&
		TileGroup[0][1] <  0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] >= 0)
	{
		TileOut = 34;
	}
	//Inner Corner Top Left
	else
	if(							TileGroup[1][0] >= 0 && TileGroup[2][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] >= 0 && TileGroup[1][2] >= 0 && TileGroup[2][2] >= 0 )
	{
		TileOut = 14;
	}
	//Inner Corner Top Right
	else
	if(	TileGroup[0][0] >= 0 && TileGroup[1][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] >= 0 && TileGroup[1][2] >= 0 && TileGroup[2][2] >= 0 )
	{
		TileOut = 13;
	}
	//Inner Corner Bottom Left
	else
	if(	TileGroup[0][0] >= 0 && TileGroup[1][0] >= 0 && TileGroup[2][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] >= 0 && TileGroup[2][2] >= 0 )
	{
		TileOut = 6;
	}
	//Inner Corner Bottom Right
	else
	if(	TileGroup[0][0] >= 0 && TileGroup[1][0] >= 0 && TileGroup[2][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] >= 0 && TileGroup[1][2] >= 0						 )
	{
		TileOut = 5;
	}
	//Full Top T
	else
	if(	TileGroup[0][0] <  0 && TileGroup[1][0] >= 0 && TileGroup[2][0] <  0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] >= 0 && TileGroup[1][2] >= 0 && TileGroup[2][2] >= 0 )
	{
		TileOut = 29;
	}
	//Full Left T
	else
	if(	TileGroup[0][0] <  0 && TileGroup[1][0] >= 0 && TileGroup[2][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] <  0 && TileGroup[1][2] >= 0 && TileGroup[2][2] >= 0 )
	{
		TileOut = 30;
	}
	//Full Right T
	else
	if(	TileGroup[0][0] >= 0 && TileGroup[1][0] >= 0 && TileGroup[2][0] <  0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] >= 0 && TileGroup[1][2] >= 0 && TileGroup[2][2] <  0 )
	{
		TileOut = 21;
	}
	//Full Bottom T
	else
	if(	TileGroup[0][0] >= 0 && TileGroup[1][0] >= 0 && TileGroup[2][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
		TileGroup[0][2] <  0 && TileGroup[1][2] >= 0 && TileGroup[2][2] <  0 )
	{
		TileOut = 22;
	}
	
	//								--Single Paths--
	//The lone tile
	else
	if(							TileGroup[1][0] <  0 &&
		TileGroup[0][1] <  0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] <  0 &&
								TileGroup[1][2] <  0 						 )
	{
		TileOut = 38;
	}
	//4 Way Cross - Also the lone tile... Thats saving resources! =D
	else
	if(							TileGroup[1][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] >= 0						 )
	{
		TileOut = 7;
	}
	//Top Left Elbow
	else
	if(							TileGroup[1][0] <  0 &&
		TileGroup[0][1] <  0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] >= 0 						 )
	{
		TileOut = 3;
	}
	//Top Right Elbow
	else
	if(							TileGroup[1][0] <  0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] <  0 &&
								TileGroup[1][2] >= 0						 )
	{
		TileOut = 4;
	}
	//Bottom Left Elbow
	else
	if(							TileGroup[1][0] >= 0 && 
		TileGroup[0][1] <  0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] <  0						 )
	{
		TileOut = 11;
	}
	//Bottom Right Elbow
	else
	if(							TileGroup[1][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] <  0 &&
								TileGroup[1][2] <  0						 )
	{
		TileOut = 12;
	}
	//Horizontal Left End
	else
	if(							TileGroup[1][0] <  0 &&
		TileGroup[0][1] <  0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] <  0 						 )
	{
		TileOut = 24;
	}
	//Horizontal Middle
	else
	if(							TileGroup[1][0] <  0 && 
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] <  0						 )
	{
		TileOut = 25;
	}
	//Horizontal Right End
	else
	if(						    TileGroup[1][0] <  0 && 
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] <  0 &&
								TileGroup[1][2] <  0 						 )
	{
		TileOut = 26;
	}
	//Vertical Top
	else
	if(							TileGroup[1][0] <  0 &&
		TileGroup[0][1] <  0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] <  0 &&
								TileGroup[1][2] >= 0						 )
	{
		TileOut = 15;
	}
	//Vertical Middle
	else
	if(							TileGroup[1][0] >= 0 && 
		TileGroup[0][1] <  0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] <  0 &&
								TileGroup[1][2] >= 0						 )
	{
		TileOut = 23;
	}
	//Vertical Bottom
	else
	if(							TileGroup[1][0] >= 0 &&
		TileGroup[0][1] <  0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] <  0 &&
								TileGroup[1][2] <  0						 )
	{
		TileOut = 31;
	}
	//Left T
	else
	if(							TileGroup[1][0] >= 0 &&
		TileGroup[0][1] <  0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] >= 0						 )
	{
		TileOut = 19;
	}
	//Top T
	else
	if(							TileGroup[1][0] <  0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] >= 0						 )
	{
		TileOut = 20;
	}
	//Bottom T
	else
	if(							TileGroup[1][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] >= 0 &&
								TileGroup[1][2] <  0						 )
	{
		TileOut = 27;
	}
	//Right T
	else
	if(							TileGroup[1][0] >= 0 &&
		TileGroup[0][1] >= 0 && TileGroup[1][1] >= 0 &&	TileGroup[2][1] <  0 &&
								TileGroup[1][2] >= 0						 )
	{
		TileOut = 28;
	}

	if(TileOut >= 0)
		TileOut += (PathID * PATH_TILES);

	++TileOut;
	Maps[MapID].Tile[X][Y][1] = TileOut;
}

void ServerState::SendMapBigObjects(int MapID)
{
	for(size_t x = 0; x < Maps[MapID].PlayerRefs.size(); x++)
	{
		int Ref = Maps[MapID].PlayerRefs[x];
		int *Packet;
		Packet = new int[(Maps[MapID].BigObjects.size() * 3)+3];

		Packet[0] = PACKETID_DATAINT;
		Packet[1] = PACKETID_UPDATEBIGOBJ;
		Packet[2] = Maps[MapID].BigObjects.size();

		for(size_t o = 0; o < Maps[MapID].BigObjects.size(); o++)
		{
			Packet[3+(o*3)] = Maps[MapID].BigObjects[o].Type;
			Packet[4+(o*3)] = (Maps[MapID].BigObjects[o].X*32) + Maps[MapID].BigObjects[o].XOff;
			Packet[5+(o*3)] = (Maps[MapID].BigObjects[o].Y*32) + Maps[MapID].BigObjects[o].YOff;
		}
		SendIntPacket(PlayerList[Ref].Peer, Packet, (Maps[MapID].BigObjects.size() * 3) + 3);
		delete [] Packet;
	}
}

void ServerState::TerraformLandscape(int PlayerID, int MapType)
{
	int MapID = PlayerList[PlayerID].MapID;

	SendPlayerUsedSkill(PlayerID, -1);
	SendGfxEvent(MapID, GFXEVENT_TFLAND, PlayerList[PlayerID].X, PlayerList[PlayerID].Y);

	Maps[MapID].SetType(MapType);

	for(int x = 1; x < 11; x++)
	{
		for(int y = 1; y < 10; y++)
		{
			if(Rand(0, 100) < 15)
			{
				Maps[MapID].Tile[x][y][0] = Rand(17, 18);
			}
		}
	}
	for(size_t x = 0; x < Maps[MapID].PlayerRefs.size(); x++)
	{
		SendPlayerMap(PlayerList[Maps[MapID].PlayerRefs[x]].Peer, MapID);
		SendPlayerWorldMap(PlayerList[Maps[MapID].PlayerRefs[x]].Peer, Maps[MapID].WorldMapID, Maps[MapID].WorldX, Maps[MapID].WorldY);
	}
}