#include "Game.h"

#ifdef _DEBUG
#define CONNECTIP "127.0.0.1"
#else
#define CONNECTIP "184.14.53.147"
#endif

void GameState::Init()
{
	PlayerSprites	= al_load_bitmap("Players.png");
	HairStyles		= al_load_bitmap("HairStyles.png");
	Equipment		= al_load_bitmap("Equip.png");
	GameBG			= al_load_bitmap("GameBG.png");
	TileSheet		= al_load_bitmap("Tiles.png");
	HUDItems		= al_load_bitmap("HUDItems.png");
	Objects			= al_load_bitmap("Objects.png");
	Mobs			= al_load_bitmap("Mobs.png");
	BattleBG		= al_load_bitmap("BattleBGs.png");
	PathsBMP		= al_load_bitmap("Paths.png");
	MappiesBMP		= al_load_bitmap("Mappies.png");
	BigObjectsBMP	= al_load_bitmap("BigObjects.png");
	PEffectsBMP		= al_load_bitmap("PEffects.png");
	LightsBMP		= al_load_bitmap("Lights.png");
	WeaponsBMP		= al_load_bitmap("Weapons.png");

	WorldMapBMP		= al_create_bitmap(124, 124);
	BattleSceneBMP	= al_create_bitmap(384, 352);
	DayNightLayer	= al_create_bitmap(384, 352);
	MapImage		= al_create_bitmap(384, 352);
	
	GameTimeOverlay = al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f);
	GameDay = 0;
	GameHour = 0;
	GameMinute = 0;

	InBattle = -1;
	BattleSceneCounter = 0;
	ConsoleOffset = 0;
	
	for(int x = 0; x < 61; x++)
	{
		for(int y = 0; y < 61; y++)
		{
			WorldMap[x][y] = 0;
		}
	}

	ConsoleWrite("Connecting to server...");

	Connected = false;
	LoggedIn = false;
	if(enet_address_set_host(&Address, CONNECTIP) < 0)
		ConsoleWrite("Could not lookup address.");
	Address.port = 5410;

	Host = enet_host_create(NULL, 1, 2, 0, 0);
	if(Host == NULL)
	{
		ConsoleWrite("Could not create client host.");
	}
	Server = enet_host_connect(Host, &Address, 2, 0);
	if(Server == NULL || Host == NULL)
	{
		ConsoleWrite("<R>Could not find peer to server.");
	}

	CreateItems();
	
#ifdef _DEBUG
	VLDEnable();
#endif
}

void GameState::ProcessInput(int KeyCode, bool KeyDown)
{
	if(!LoggedIn || !Connected)
		return;

	if(KeyCode == ALLEGRO_KEY_LSHIFT || KeyCode == ALLEGRO_KEY_RSHIFT)
	{
		if(KeyDown)
		{
			ShiftMod++;
		}
		else
		{
			ShiftMod--;
		}
	}

	int MovePacket = PACKETID_STARTMOVE;
	if(ShiftMod)
		MovePacket = PACKETID_CHANGEDIR;

	if(KeyCode == ALLEGRO_KEY_UP)
	{
		if(KeyDown)
		{
			char PacketData[] = {PACKETID_DATASTART, MovePacket, MOVEDIR_UP};
			SendDataPacket(PacketData);
		}
		else
		{
			char PacketData[] = {PACKETID_DATASTART, PACKETID_ENDMOVE};
			SendDataPacket(PacketData);
		}
	}
	else if(KeyCode == ALLEGRO_KEY_DOWN)
	{
		if(KeyDown)
		{
			char PacketData[] = {PACKETID_DATASTART, MovePacket, MOVEDIR_DOWN};
			SendDataPacket(PacketData);
		}
		else
		{
			char PacketData[] = {PACKETID_DATASTART, PACKETID_ENDMOVE};
			SendDataPacket(PacketData);
		}
	}
	else if(KeyCode == ALLEGRO_KEY_LEFT)
	{
		if(KeyDown)
		{
			char PacketData[] = {PACKETID_DATASTART, MovePacket, MOVEDIR_LEFT};
			SendDataPacket(PacketData);
		}
		else
		{
			char PacketData[] = {PACKETID_DATASTART, PACKETID_ENDMOVE};
			SendDataPacket(PacketData);
		}
	}
	else if(KeyCode == ALLEGRO_KEY_RIGHT)
	{
		if(KeyDown)
		{
			char PacketData[] = {PACKETID_DATASTART, MovePacket, MOVEDIR_RIGHT};
			SendDataPacket(PacketData);
		}
		else
		{
			char PacketData[] = {PACKETID_DATASTART, PACKETID_ENDMOVE};
			SendDataPacket(PacketData);
		}
	}
}

bool GameState::DoLogic()
{
	al_get_mouse_state(&MouseState);
	if(MouseState.buttons & 1)
	{
		if(MouseState.x >= 777 && MouseState.x <= 777 + 13
			&& MouseState.y >= 387 && MouseState.y <= 387 + 12)
		{
			int INCS = ConsoleLine.size() - 15;
			if(ConsoleOffset < INCS)
				ConsoleOffset++;
		}
		if(MouseState.x >= 777 && MouseState.x <= 777 + 13
			&& MouseState.y >= 561 && MouseState.y <= 561 + 12)
		{
			if(ConsoleOffset > 0)
				ConsoleOffset--;
		}
	}

	for(size_t x = 0; x < GraphicsEvents.size(); x++)
	{
		GraphicsEvents[x].Tick();
		if(GraphicsEvents[x].IsDone)
			DelList.push_back(x);
	}
	if(DelList.size() > 0)
	{
		sort(DelList.begin(), DelList.end());
		for(size_t y = DelList.size(); y > 0; y--)
		{
			GraphicsEvents.erase(GraphicsEvents.begin() + DelList[y - 1]);
		}
		DelList.clear();
	}

	if(ShakeLife > 0)
	{
		ShakeLife--;
		ScreenOffsetX = Rand(-ShakeIntensity, ShakeIntensity);
		ScreenOffsetY = Rand(-ShakeIntensity, ShakeIntensity);
	}
	else
	{
		ScreenOffsetX = 0;
		ScreenOffsetY = 0;
		ShakeIntensity = 0;
	}
	return true;
}

void GameState::Draw()
{
	al_draw_bitmap(GameBG, 0, 0, NULL);

	ConsoleDraw(12, 380, 15, ConsoleOffset);
	al_draw_bitmap_region(HUDItems, 6, 0, 13, 12, 777, 387, NULL);
	al_draw_bitmap_region(HUDItems, 18, 0, 13, 12, 777, 561, NULL);

	if(!LoggedIn)
		return;

	if(InBattle >= 0)
	{
		al_draw_bitmap_region(BattleBG, InBattle * 384, 0, 384, 352, GAMEWINDOW_X + ScreenOffsetX, GAMEWINDOW_Y + ScreenOffsetY, NULL);
	}
	else
	{
		al_draw_bitmap(MapImage, GAMEWINDOW_X + ScreenOffsetX, GAMEWINDOW_Y + ScreenOffsetY, NULL);
		al_draw_bitmap(WorldMapBMP, MAPWINDOW_X, MAPWINDOW_Y, NULL);
	}

	for(size_t x = 0; x < Map.Mobs.size(); x++)
	{
		al_draw_bitmap_region(Mobs, Map.Mobs[x].Index * 32, Map.Mobs[x].Type * 32, 32, 32, Map.Mobs[x].X + GAMEWINDOW_X + ScreenOffsetX, Map.Mobs[x].Y + GAMEWINDOW_Y + ScreenOffsetY, NULL);
	}
	int DrawFrame = 0;
	for(unsigned int x = 0; x < PlayerList.size(); x++)
	{
		if(PlayerList[x].Visable)
		{
			if(PlayerList[x].ArmsUp > 0)
			{
				if(PlayerList[x].ArmsUp % 2 == 1)
				{
					DrawFrame = 256 + PlayerList[x].SpriteDir * 32;
				}
				else
				{
					DrawFrame = 384 + PlayerList[x].SpriteDir * 32;
				}
			}
			else
			{
				DrawFrame = (PlayerList[x].SpriteDir * 64) + (PlayerList[x].SpriteFrame * 32);
			}

			al_draw_bitmap_region(PlayerSprites, DrawFrame, PlayerList[x].Sex * 32, 32, 32, PlayerList[x].X+GAMEWINDOW_X+ScreenOffsetX, PlayerList[x].Y+GAMEWINDOW_Y+ScreenOffsetY, NULL);
			
			for(int y = 0; y < EQUIPSLOT_MAX; y++)
			{
				if(PlayerList[x].EquipmentSlot[y].Type != OBJTYPE_NONE)
				al_draw_bitmap_region(Equipment, DrawFrame + 32, PlayerList[x].EquipmentSlot[y].Index * 32, 32, 32, PlayerList[x].X + GAMEWINDOW_X+ScreenOffsetX, PlayerList[x].Y + GAMEWINDOW_Y+ScreenOffsetY, NULL);
			}
			if(PlayerList[x].EquipmentSlot[EQUIPSLOT_HELM].Type == OBJTYPE_NONE)
			{
				al_draw_tinted_bitmap_region(HairStyles, al_map_rgb(PlayerList[x].HairR, PlayerList[x].HairG, PlayerList[x].HairB), (PlayerList[x].SpriteDir * 32), PlayerList[x].HairStyle * 16, 32, 16, PlayerList[x].X + GAMEWINDOW_X + ScreenOffsetX, PlayerList[x].Y + GAMEWINDOW_Y + ScreenOffsetY, NULL);
			}
			if(PlayerList[x].ShowWeapon)
			{
				al_draw_bitmap_region(WeaponsBMP, DrawFrame, PlayerList[x].AttackWeapon * 32, 32, 32, PlayerList[x].X + GAMEWINDOW_X + ScreenOffsetX, PlayerList[x].Y + GAMEWINDOW_Y + ScreenOffsetY, NULL);
			}
		}
	}

	for(size_t x = 0; x < Map.BigObjects.size(); x++)
	{
		al_draw_bitmap_region(BigObjectsBMP, Map.BigObjects[x].Type * 64, 0, 64, 64, Map.BigObjects[x].X + GAMEWINDOW_X + ScreenOffsetX, Map.BigObjects[x].Y + GAMEWINDOW_Y + ScreenOffsetY, 0);
	}

	for(size_t x = 0; x < GraphicsEvents.size(); x++)
	{
		GraphicsEvents[x].Draw();
	}
	
	for(size_t x = 0; x < PEffectList.size(); x++)
	{
		PEffectList[x]->Tick();
		if(PEffectList[x]->GetMapID() == CurrentMapID)
			PEffectList[x]->Draw(ScreenOffsetX, ScreenOffsetY);

		
		if(PEffectList[x]->IsDead())
		{
			delete PEffectList[x];
			PEffectList.erase(PEffectList.begin() + x);
		}
	}

	al_set_target_bitmap(DayNightLayer);
	al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));
	al_draw_filled_rectangle(0, 0, 384, 352, GameTimeOverlay);
	
	if(GameHour >= 21 || GameHour <= 5)
	{
		al_set_blender(ALLEGRO_DEST_MINUS_SRC, ALLEGRO_ONE, ALLEGRO_ONE);
		for(size_t x = 0; x < LightList.size(); x++)
		{
			al_draw_tinted_scaled_bitmap(LightsBMP, LightList[x].ColorMask, 0, 0, 96, 96, LightList[x].X, LightList[x].Y, LightList[x].Width, LightList[x].Height, NULL);
		}
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
	}
	al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));

	al_draw_bitmap(DayNightLayer, GAMEWINDOW_X + ScreenOffsetX, GAMEWINDOW_Y + ScreenOffsetY, NULL);
	
	for(size_t x = 0; x < PlayerList.size(); x++)
	{
		if(PlayerList[x].Visable)
		{
			al_draw_text(GameFont, al_map_rgb(255, 255, 255), GAMEWINDOW_X + ScreenOffsetX + PlayerList[x].X+14, GAMEWINDOW_Y + ScreenOffsetY + PlayerList[x].Y + 32, ALLEGRO_ALIGN_CENTER, PlayerList[x].Name.c_str());
			al_draw_text(GameFont, al_map_rgb(255, 255, 255), GAMEWINDOW_X + ScreenOffsetX + PlayerList[x].X+14, GAMEWINDOW_Y + ScreenOffsetY + PlayerList[x].Y + 33, ALLEGRO_ALIGN_CENTER, PlayerList[x].Name.c_str());
			al_draw_text(GameFont, al_map_rgb(255, 255, 255), GAMEWINDOW_X + ScreenOffsetX + PlayerList[x].X+14,  GAMEWINDOW_Y + ScreenOffsetY + PlayerList[x].Y + 34, ALLEGRO_ALIGN_CENTER, PlayerList[x].Name.c_str());
			al_draw_text(GameFont, al_map_rgb(255, 255, 255), GAMEWINDOW_X + ScreenOffsetX + PlayerList[x].X+15,  GAMEWINDOW_Y + ScreenOffsetY + PlayerList[x].Y + 32, ALLEGRO_ALIGN_CENTER, PlayerList[x].Name.c_str());
			al_draw_text(GameFont, al_map_rgb(255, 255, 255), GAMEWINDOW_X + ScreenOffsetX + PlayerList[x].X+15,  GAMEWINDOW_Y + ScreenOffsetY + PlayerList[x].Y + 33, ALLEGRO_ALIGN_CENTER, PlayerList[x].Name.c_str());
			al_draw_text(GameFont, al_map_rgb(255, 255, 255), GAMEWINDOW_X + ScreenOffsetX + PlayerList[x].X+15,  GAMEWINDOW_Y + ScreenOffsetY + PlayerList[x].Y + 34, ALLEGRO_ALIGN_CENTER, PlayerList[x].Name.c_str());
			al_draw_text(GameFont, al_map_rgb(255, 255, 255), GAMEWINDOW_X + ScreenOffsetX + PlayerList[x].X+16,  GAMEWINDOW_Y + ScreenOffsetY + PlayerList[x].Y + 32, ALLEGRO_ALIGN_CENTER, PlayerList[x].Name.c_str());
			al_draw_text(GameFont, al_map_rgb(255, 255, 255), GAMEWINDOW_X + ScreenOffsetX + PlayerList[x].X+16,  GAMEWINDOW_Y + ScreenOffsetY + PlayerList[x].Y + 33, ALLEGRO_ALIGN_CENTER, PlayerList[x].Name.c_str());
			al_draw_text(GameFont, al_map_rgb(255, 255, 255), GAMEWINDOW_X + ScreenOffsetX + PlayerList[x].X+16,  GAMEWINDOW_Y + ScreenOffsetY + PlayerList[x].Y + 34, ALLEGRO_ALIGN_CENTER, PlayerList[x].Name.c_str());
			al_draw_text(GameFont, al_map_rgb(0, 0, 255), GAMEWINDOW_X + ScreenOffsetX + PlayerList[x].X+15,  GAMEWINDOW_Y + ScreenOffsetY + PlayerList[x].Y + 33, ALLEGRO_ALIGN_CENTER, PlayerList[x].Name.c_str());
		}
	}

	al_draw_text(GameFont, al_map_rgb(255, 255, 255), STATWINDOW_X + 40, STATWINDOW_Y + 5, NULL, "Eponick");
	al_draw_bitmap_region(HUDItems, 0, 26, 100, 11, STATWINDOW_X + 12, STATWINDOW_Y + 21, NULL);
	al_draw_bitmap_region(HUDItems, 0, 37, 100, 11, STATWINDOW_X + 12, STATWINDOW_Y + 36, NULL);
	al_draw_bitmap_region(HUDItems, 0, 48, 25, 11, STATWINDOW_X + 12, STATWINDOW_Y + 51, NULL);
	
	al_draw_bitmap_region(HUDItems, 0, 12, 103, 14, STATWINDOW_X + 11, STATWINDOW_Y + 20, NULL);
	al_draw_bitmap_region(HUDItems, 0, 12, 103, 14, STATWINDOW_X + 11, STATWINDOW_Y + 35, NULL);
	al_draw_bitmap_region(HUDItems, 0, 12, 103, 14, STATWINDOW_X + 11, STATWINDOW_Y + 50, NULL);

	if(BattleSceneCounter > 0)
	{
		if(BattleSceneCounter > 20 && BattleSceneCounter < 60)
		{
			BattleSceneAngle+=0.15f;
			BattleSceneYScale+=0.1f;
			BattleSceneXScale+=0.1f;
		}
		else if(BattleSceneCounter >= 60)
		{
			BattleSceneAngle+=0.15f;
			BattleSceneYScale-=0.1f;
			BattleSceneXScale-=0.1f;
		}
		al_draw_tinted_scaled_rotated_bitmap(BattleSceneBMP, al_map_rgba(255, 0, 0, 255), 192, 176, GAMEWINDOW_X + ScreenOffsetX + 192, GAMEWINDOW_Y + ScreenOffsetY + 176, BattleSceneXScale, BattleSceneYScale, BattleSceneAngle, NULL);
		BattleSceneCounter++;
		if(BattleSceneXScale <= 0.0f)
			BattleSceneCounter = 0;
	}
}

void GameState::ResetState()
{
}

void GameState::SendDataPacket(const char *Data)
{
	if(!Connected)
		return;
	ENetPacket *OutPacket = enet_packet_create(Data, sizeof(Data), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(Server, 0, OutPacket);
}

void GameState::ProcessConsoleInput()
{
	if(!Connected)
		return;

	ostringstream OS(ostringstream::out);
	OS << "<B>> <W>" << ConsoleInput.c_str();
	ConsoleWrite(OS.str().c_str());
	ENetPacket *OutPacket = enet_packet_create(ConsoleInput.c_str(), ConsoleInput.length(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(Server, 0, OutPacket);
}

void GameState::AddEmptyPlayer(int ID)
{
	cPlayer NewPlayer;
	NewPlayer.ID = ID;
	PlayerList.push_back(NewPlayer);
}

void GameState::ProcessDataPacket(ENetPacket *Packet)
{
	int PlayerID = -1;
	ostringstream WriteString(ostringstream::out);
	cObj NewObj;
	cMob NewMob;
	cBigObject NewBigObj;
	cLight NewLight;
	NewLight.ColorMask = al_map_rgba_f(1, 1, 1, 1);
	NewLight.Width = 96;
	NewLight.Height = 96;

	if(GetIntFromPacket(Packet, 0) == PACKETID_DATAINT)
	{
		switch(GetIntFromPacket(Packet, 1))
		{
		case PACKETID_LOGGEDIN:
			ConsoleWrite("Welcome to <P>Xira Genesis");
			LoggedIn = true;
			AddEmptyPlayer(GetIntFromPacket(Packet, 2));
			break;

		case PACKETID_GAMETIME:
			GameDay = GetIntFromPacket(Packet, 2);
			GameHour = GetIntFromPacket(Packet, 3);
			GameMinute = GetIntFromPacket(Packet, 4);
			UpdateGameTimeOverlay();
			break;

		case PACKETID_ADDMOB:
			NewMob.Type = GetIntFromPacket(Packet, 2);
			NewMob.Index = GetIntFromPacket(Packet, 3);
			NewMob.X = GetIntFromPacket(Packet, 4);
			NewMob.Y = GetIntFromPacket(Packet, 5);
			Map.Mobs.push_back(NewMob);
			break;

		case PACKETID_CLEARMOBS:
			Map.Mobs.clear();
			break;

		case PACKETID_GOTOBATTLE:
			InBattle = GetIntFromPacket(Packet, 2);
			GoToBattleScene();
			break;

		case PACKETID_EXITBATTLE:
			InBattle = -1;
			break;

		case PACKETID_UPDATEBIGOBJ:
			Map.BigObjects.clear();
			for(int x = 0; x < GetIntFromPacket(Packet, 2); x++)
			{
				NewBigObj.Type = GetIntFromPacket(Packet, 3+(x*3));
				NewBigObj.X = GetIntFromPacket(Packet, 4+(x*3));
				NewBigObj.Y = GetIntFromPacket(Packet, 5+(x*3));
				Map.BigObjects.push_back(NewBigObj);
			}
			break;

		case PACKETID_GFXEVENT:
			DoGfxEvent(GetIntFromPacket(Packet, 2), GetIntFromPacket(Packet, 3), GetIntFromPacket(Packet, 4), GetIntFromPacket(Packet, 5));
			break;

		case PACKETID_PLAYERINFO:
			PlayerID = FindPlayerID(GetIntFromPacket(Packet, 2));
			PlayerList[PlayerID].Sex = GetIntFromPacket(Packet, 3);
			PlayerList[PlayerID].SkinTone = GetIntFromPacket(Packet, 4);
			PlayerList[PlayerID].HairStyle = GetIntFromPacket(Packet, 5);
			PlayerList[PlayerID].HairR = GetIntFromPacket(Packet, 6);
			PlayerList[PlayerID].HairG = GetIntFromPacket(Packet, 7);
			PlayerList[PlayerID].HairB = GetIntFromPacket(Packet, 8);
			break;

		case PACKETID_SETSPRITE:
				break;

		case PACKETID_PLUSEDSK:
			PlayerID = FindPlayerID(GetIntFromPacket(Packet, 2));
			if(PlayerID >= 0)
			{
				PlayerList[PlayerID].ArmsUp = 4;
				PlayerList[PlayerID].AttackWeapon = GetIntFromPacket(Packet, 3);
				if(PlayerList[PlayerID].AttackWeapon >= 0)
				{
					PlayerList[PlayerID].ShowWeapon = true;
				}
			}
			break;

		case PACKETID_CLEARGFXEV:
			GraphicsEvents.clear();
			break;

		case PACKETID_ADDPLAYER:
				AddEmptyPlayer(GetIntFromPacket(Packet, 2));
				LightList.push_back(NewLight);
				ConsoleWrite(WriteString.str());
				break;

		case PACKETID_UPDATEPLAYERXY:
				PlayerID = FindPlayerID(GetIntFromPacket(Packet, 2));
				if(PlayerID >= 0)
				{
					PlayerList[PlayerID].SpriteDir = GetIntFromPacket(Packet, 3);
					PlayerList[PlayerID].X = GetIntFromPacket(Packet, 4);
					PlayerList[PlayerID].Y = GetIntFromPacket(Packet, 5);
					PlayerList[PlayerID].Visable = true;
					LightList[PlayerID].X = PlayerList[PlayerID].X - 32;
					LightList[PlayerID].Y = PlayerList[PlayerID].Y - 32;
				}
				break;

		case PACKETID_SETMAPTYPE:
			Map.MapType = GetIntFromPacket(Packet, 2);
			break;

		case PACKETID_SETMAPID:
			
			CurrentMapID = GetIntFromPacket(Packet, 2);
			break;

		case PACKETID_ADDMAPOBJ:
			CreateNewMapObj(GetIntFromPacket(Packet, 2), GetIntFromPacket(Packet, 3), GetIntFromPacket(Packet, 4),
				GetIntFromPacket(Packet, 5), GetIntFromPacket(Packet, 6));
			GenerateMapImage();
			break;

		case PACKETID_DELETEMAPOBJ:
			DeleteMapObjByID(GetIntFromPacket(Packet, 2));
			GenerateMapImage();
			break;

		case PACKETID_CLEARMAPOBJ:
			Map.Objects.clear();
			GenerateMapImage();
			break;

		case PACKETID_CHANGEEQUIP:
			PlayerID = FindPlayerID(GetIntFromPacket(Packet, 2));
			if(PlayerID >= 0)
			{
				PlayerList[PlayerID].EquipmentSlot[GetIntFromPacket(Packet, 3)].Index = GetIntFromPacket(Packet, 4);
				PlayerList[PlayerID].EquipmentSlot[GetIntFromPacket(Packet, 3)].Type = GetIntFromPacket(Packet, 5);
			}
			break;

		case PACKETID_PLAYERLEFT:
			PlayerID = GetIntFromPacket(Packet, 2);
			PlayerID = FindPlayerID(PlayerID);
			if(PlayerID >= 0)
			{
				PlayerList.erase(PlayerList.begin() + PlayerID);
				LightList.erase(LightList.begin() + PlayerID);
			}
			break;

		case PACKETID_NEXTNAMEID:
			NextNameID = GetIntFromPacket(Packet, 2);
			break;

		case PACKETID_CLEARALLMAP:
			PlayerList.clear();
			LightList.clear();
			break;

		case PACKETID_WORLDMAPDATA:
			ALLEGRO_BITMAP *Buff = al_get_target_bitmap();
			al_set_target_bitmap(WorldMapBMP);
			for(int x = 0; x < 31; x++)
			{
				for(int y = 0; y < 31; y++)
				{
					int MapType = GetIntFromPacket(Packet, 2 + ((x * 30) + y));
					al_draw_bitmap_region(MappiesBMP, MapType*4, 0, 4, 4, x * 4, y * 4, NULL);
				}
			}
			al_draw_bitmap_region(HUDItems, 0, 0, 6, 6, 59, 59, NULL);
			al_set_target_bitmap(Buff);
			break;
		}
	}
	else
	{
		if(Packet->dataLength >= 2)
		{
			string PlayerName;
			int PlayerLevel;
			int NameID;
			int PlayerExperience;
			int ps_x = 1;
			ostringstream OS(ostringstream::out);
			switch(Packet->data[1])
			{
			case PACKETID_PLAYERSTATS:
				while(Packet->data[ps_x] != 202)
				{
					ps_x++;
					PlayerName += Packet->data[ps_x];
				}
				PlayerLevel = (int)Packet->data[PlayerName.length() + 2];
				PlayerExperience = (int)Packet->data[PlayerName.length() + 3];
				OS << "Name: " << PlayerName << " Level: " << PlayerLevel << "Experience: " << PlayerExperience;
				ConsoleWrite(OS.str());
				break;

			case PACKETID_MAPDATA:
				for(int z = 0; z < 3; z++)
				{
					for(int x = 0; x < 12; x++)
					{
						for(int y = 0; y < 11; y++)
						{
							Map.Tile[x][y][z] = Packet->data[(((x*11)+y)+(z*132)) + 2];
						}
					}
				}
				Map.Objects.clear();

				GenerateMapImage();
				break;

			case PACKETID_ADDNAME:
				int NameSize = Packet->data[2];
				char *Name = new char[NameSize];
				for(int x = 0; x < NameSize; x++)
				{
					Name[x] = Packet->data[x + 3];
				}

				NameID = FindPlayerID(NextNameID);
				if(NameID >= 0 && NameID < PlayerList.size())
					PlayerList[NameID].Name = Name;
				break;
			}
		}
	}
}

int GameState::FindPlayerID(int ID)
{
	for(unsigned int x = 0; x < PlayerList.size(); x++)
	{
		if(PlayerList[x].ID == ID)
			return x;
	}
	return -1;
}

GameState::~GameState()
{
	if(Connected)
	{
		enet_peer_disconnect_now(Server, 0);
		enet_peer_reset(Server);
	}
}

void GameState::ProcessNetEvent(ENetEvent NetEvent)
{
	ostringstream OS(ostringstream::out);
		switch(NetEvent.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			Connected = true;
			LoggedIn = false;
			ConsoleWrite("<G>Successfully connected to server.");
			ConsoleWrite("[<P>Temporary<W>] Type: <C>login <Y>[UserName] [Password] <W>to log in.");
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			int RecvPack;
			if(NetEvent.packet->dataLength >= sizeof(int))
			{
				memcpy(&RecvPack, NetEvent.packet->data, sizeof(int));
			}

			if(NetEvent.packet->data[0] == PACKETID_DATASTART || RecvPack == PACKETID_DATAINT)
			{
				ProcessDataPacket(NetEvent.packet);
			}
			else
			{
				ParsePacket(NetEvent.packet);
				ConsoleWrite(PacketString.c_str());
			}
			enet_packet_destroy(NetEvent.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			if(NetEvent.data == 5)
			{
				ConsoleWrite("<R>You have been kicked from the server.");
			}
			else
			{
				ConsoleWrite("You have been <R>disconnected <W>from the server.");
			}
			Connected = false;
			break;
		}
}

void GameState::GoToBattleScene()
{
	ShakeLife = 0;
	ScreenOffsetX = 0;
	ScreenOffsetY = 0;
	BattleSceneCounter = 1;
	BattleSceneAngle = 0.0f;
	BattleSceneXScale = 1.0f;
	BattleSceneYScale = 1.0f;
	ALLEGRO_BITMAP *Buff = al_get_target_bitmap();
	al_set_target_bitmap(BattleSceneBMP);
	al_draw_bitmap_region(Buff, GAMEWINDOW_X + ScreenOffsetX, GAMEWINDOW_Y + ScreenOffsetY, 384, 352, 0, 0, 0);

	al_set_target_bitmap(Buff);
}

void GameState::HSTick()
{
	for(size_t x =0; x < PlayerList.size(); x++)
	{
		if(PlayerList[x].ArmsUp > 0)
			PlayerList[x].ArmsUp--;
		if(PlayerList[x].ArmsUp == 0)
			PlayerList[x].ShowWeapon = false;

		if(!PlayerList[x].InBattle)
		{
			PlayerList[x].SpriteCounter++;
			if(PlayerList[x].SpriteCounter >= 2)
			{
				PlayerList[x].SpriteCounter = 0;
				PlayerList[x].SpriteFrame++;
				if(PlayerList[x].SpriteFrame > 1)
					PlayerList[x].SpriteFrame = 0;
			}
		}
	}
}

void GameState::DoGfxEvent(int EventID, int X, int Y, int MapID)
{
	X *= 32;
	Y *= 32;
	X += GAMEWINDOW_X + ScreenOffsetX;
	Y += GAMEWINDOW_Y + ScreenOffsetY;
	if(EventID == GFXEVENT_TFE)
	{
		PEffectList.push_back(new PE_Fader(X, Y, 0, 0, 3, 0, 32, 32, 50, 0, PEffectsBMP, MapID, 0.0f));
		for(int x = 0; x < 100; x++)
		{
			PEffectList.push_back(new PE_Fader(Rand(X, X+29), Rand(Y, Y+26), 0.0f, -0.8f, 0, 0, 3, 8, Rand(40, 70), Rand(0, 40), PEffectsBMP, MapID, 0.0f));
		}
	}
	else if(EventID == GFXEVENT_TFCAVE)
	{
		ShakeScreen(5, 110);
		PEffectList.push_back(new PE_Changer(X, Y, 32, 0, 5 * 32, 0, 32, 32, 100, 0, TileSheet, MapID, 2));
	}
	else if(EventID == GFXEVENT_BUHS)
	{
		PEffectList.push_back(new PE_Changer(X, Y, 32, 0, 9 * 32, 0, 32, 32, 100, 0, TileSheet, MapID, 2));
	}
	else if(EventID == GFXEVENT_BUILDITEM)
	{
		int HitX = Rand(3, 30);
		int HitY = Rand(3, 30);
		PEffectList.push_back(new PE_Fader(X + HitX, Y + HitY, -0.5f, -0.5f, 35, 7, 3, 3, 60, 2, PEffectsBMP, MapID, 0.05f));
		PEffectList.push_back(new PE_Fader(X + HitX, Y + HitY, 0.5f, -0.5f, 35, 7, 3, 3, 60, 2, PEffectsBMP, MapID, 0.05f));
		PEffectList.push_back(new PE_Fader(X + HitX, Y + HitY, -0.5f, 0.5f, 35, 7, 3, 3, 60, 2, PEffectsBMP, MapID, 0.05f));
		PEffectList.push_back(new PE_Fader(X + HitX, Y + HitY, 0.5f, 0.5f, 35, 7, 3, 3, 60, 2, PEffectsBMP, MapID, 0.05f));

		HitX = Rand(3, 30);
		HitY = Rand(3, 30);
		PEffectList.push_back(new PE_Fader(X + HitX, Y + HitY, -0.5f, -0.5f, 35, 7, 3, 3, 70, 50, PEffectsBMP, MapID, 0.05f));
		PEffectList.push_back(new PE_Fader(X + HitX, Y + HitY, 0.5f, -0.5f, 35, 7, 3, 3, 70, 50, PEffectsBMP, MapID, 0.05f));
		PEffectList.push_back(new PE_Fader(X + HitX, Y + HitY, -0.5f, 0.5f, 35, 7, 3, 3, 70, 50, PEffectsBMP, MapID, 0.05f));
		PEffectList.push_back(new PE_Fader(X + HitX, Y + HitY, 0.5f, 0.5f, 35, 7, 3, 3, 70, 50, PEffectsBMP, MapID, 0.05f));
		
		HitX = Rand(3, 30);
		HitY = Rand(3, 30);
		PEffectList.push_back(new PE_Fader(X + HitX, Y + HitY, -0.5f, -0.5f, 35, 7, 3, 3, 50, 100, PEffectsBMP, MapID, 0.05f));
		PEffectList.push_back(new PE_Fader(X + HitX, Y + HitY, 0.5f, -0.5f, 35, 7, 3, 3, 50, 100, PEffectsBMP, MapID, 0.05f));
		PEffectList.push_back(new PE_Fader(X + HitX, Y + HitY, -0.5f, 0.5f, 35, 7, 3, 3, 50, 100, PEffectsBMP, MapID, 0.05f));
		PEffectList.push_back(new PE_Fader(X + HitX, Y + HitY, 0.5f, 0.5f, 35, 7, 3, 3, 50, 100, PEffectsBMP, MapID, 0.05f));
	}
	else if(EventID == GFXEVENT_BUFO)
	{
		PEffectList.push_back(new PE_Changer(X, Y, 32, 0, 12 * 32, 0, 32, 32, 100, 0, TileSheet, MapID, 2));
	}
}

void GameState::ShakeScreen(int Intensity, int Duration)
{
	this->ShakeIntensity = Intensity;
	this->ShakeLife = Duration;
}

void GameState::CreateNewMapObj(int Type, int Index, int X, int Y, int ID)
{
	cObj NewObj;
	NewObj.Type = Type;
	NewObj.Index = Index;
	NewObj.X = X;
	NewObj.Y = Y;
	NewObj.ID = ID;
	Map.Objects.push_back(NewObj);
}

void GameState::DeleteMapObjByID(int ID)
{
	for(size_t x = 0; x < Map.Objects.size(); x++)
	{
		if(Map.Objects[x].ID == ID)
		{
			Map.Objects.erase(Map.Objects.begin() + x);
			return;
		}
	}
}

void GameState::UpdateGameTimeOverlay()
{
	if(GameHour >= 23 || GameHour <= 4)
	{
		GameTimeOverlay = al_map_rgba_f(0.0f, 0.0f, 0.2f * 0.6f, 0.6f);
	}
	if(GameHour == 5)
		GameTimeOverlay = al_map_rgba_f(0.2f * 0.5f, 0.2f * 0.5f, 0.4f * 0.5f, 0.5f);
	if(GameHour == 6)
		GameTimeOverlay = al_map_rgba_f(0.5f * 0.3f, 0.5f * 0.3f, 0.7f * 0.3f, 0.3f);
	if(GameHour == 7)
		GameTimeOverlay = al_map_rgba_f(0.8f * 0.1f, 0.8f * 0.1f, 1.0f * 0.1f, 0.1f);
	if(GameHour >= 8 && GameHour <= 17)
		GameTimeOverlay = al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f);
	if(GameHour == 18)
		GameTimeOverlay = al_map_rgba_f(1.0f * 0.2f, 0.8f * 0.2f, 0.4f * 0.2f, 0.2f);
	if(GameHour == 19)
		GameTimeOverlay = al_map_rgba_f(0.8f * 0.3f, 0.5f * 0.3f, 0.1f * 0.3f, 0.3f);
	if(GameHour == 20)
		GameTimeOverlay = al_map_rgba_f(0.5f * 0.7f, 0.2f * 0.7f, 0.1f * 0.5f, 0.3f);
	if(GameHour == 21)
		GameTimeOverlay = al_map_rgba_f(0.3f * 0.6f, 0.1f * 0.6f, 0.15f * 0.6f, 0.45f);
	if(GameHour == 22)
		GameTimeOverlay = al_map_rgba_f(0.1f * 0.6f, 0.0f * 0.6f, 0.22f * 0.6f, 0.52f);
}

void GameState::GenerateMapImage()
{
	al_set_target_bitmap(MapImage);
	al_clear_to_color(al_map_rgb(0, 0, 0));

	for(int x = 0; x < 12; x++)
	{
		for(int y = 0; y < 11; y++)
		{
			for(int z = 0; z < 3; z++)
			{
				if(z < 1 || Map.Tile[x][y][z] > 0)
				{
					if(z == 1)
					{
						int Tile = Map.Tile[x][y][1];
						--Tile;
						int ImgX = Tile%8;
						int ImgY = Tile/8;
						al_draw_bitmap_region(PathsBMP, ImgX * 32, ImgY * 32, 32, 32, (x*32), (y*32), NULL);
					}
					else
					{
						al_draw_bitmap_region(TileSheet, Map.Tile[x][y][z]*32, 0, 32, 32, (x*32), (y*32), NULL);
					}
				}
			}
		}
	}

	for(unsigned int x = 0; x < Map.Objects.size(); x++)
	{	
		if(Map.Objects[x].Type > OBJTYPE_BEGINEQUIP && Map.Objects[x].Type < OBJTYPE_ENDEQUIP)
			al_draw_bitmap_region(Equipment, 0, Map.Objects[x].Index * 32, 32, 32, Map.Objects[x].X * 32, Map.Objects[x].Y * 32, NULL);
		else
			al_draw_bitmap_region(Objects, Map.Objects[x].Index * 32, Map.Objects[x].Type * 32, 32, 32, (Map.Objects[x].X * 32), (Map.Objects[x].Y * 32), NULL);
	}

	al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
}