#ifndef _GAME_H_
#define _GAME_H_

#define ALLEGRO_STATICLINK
#include "enet/enet.h"
#include <physfs.h>
#include <stdio.h>

#ifdef _DEBUG
#include <vld.h>
#endif

#include <stdio.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>

#include <iterator>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <random>

#include <allegro5/allegro.h>
#include <allegro5/allegro_physfs.h>
#include <allegro5/allegro_direct3d.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

//#define BOOST_ALL_NO_LIB
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

using namespace std;

#define GAMEWINDOW_X	224
#define GAMEWINDOW_Y	12
#define MAPWINDOW_X		656
#define MAPWINDOW_Y		12
#define STATWINDOW_X	656
#define STATWINDOW_Y	158
#define OTHERWINDOW_X	20
#define OTHERWINDOW_Y	12

#define OSStr(name) ostringstream name(ostringstream::out)

#include "PacketIDs.h"

#include "cSprite.h"
#include "cParticle.h"
#include "ParticleTypes.h"
#include "cButton.h"
#include "cGfxEvent.h"
#include "cCommand.h"
#include "cObj.h"
#include "cBigObject.h"
#include "cMob.h"
#include "cMap.h"
#include "cPlayer.h"
#include "cLight.h"

#include "Scene.h"
#include "TitleState.h"
#include "ServerState.h"
#include "GameState.h"

#define SCENE_NOCHANGE 0
#define SCENE_GAMESCENE 1
#define SCENE_SERVERSCENE 2
#define SCENE_TITLESCENE 3
#define SCENE_EXITGAME 4

extern int NextScene;
extern ALLEGRO_FONT *GameFont;
extern ALLEGRO_STATE IOState;
extern bool IsDedicated;

#endif