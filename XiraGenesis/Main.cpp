/**********************************
**   Xira Genesis Main Source    **
**     11/3/12 Gary Boyd         **
**********************************/

#include "Game.h"

#define DISP_WIDTH	800
#define DISP_HEIGHT 600
#define LIMIT_FPS	60		//1 for unlimited
#ifdef _DEBUG
#define GFXPACKFILE	"Data/gdat.zip"
#else
#define GFXPACKFILE "Data/gdat.gpk"
#endif

bool Initialize();
bool CreateConsoleHandle();
bool InitializeDedicated();
void DedicatedLoopThread();

bool IsDedicated;
bool GameRunning = true;

int FPS[8] = {0,0,3,0,0,0,0,0};			//[0] Avg FPS, [1] Frame Counter, [2] FPS Switch Counter, [3-7] FPS to average

Scene *CurrentScene;
GameState GameScene;
ServerState ServerScene;
TitleState TitleScene;
ALLEGRO_STATE IOState;

int main(int argc, char *argv[])
{
	if(argc >= 2)
	{
		if(std::string(argv[1]) == "-dedx")
		{
			CreateConsoleHandle();
			InitializeDedicated();
			CurrentScene = &ServerScene;
			
			CurrentScene->Init();
			
			boost::thread ServerThread(DedicatedLoopThread);

			while(GameRunning)
			{
				string Input;
				getline(cin, Input);
			
				CurrentScene->ProcessConsoleInput(Input);

				if(NextScene == SCENE_EXITGAME)
				{
					GameRunning = false;
				}
			}
			ServerThread.join();

			enet_deinitialize();

			return 0;
		}
	}

	IsDedicated = false;
	//Some allegro objects
	ALLEGRO_DISPLAY *display = NULL;		//Contains user display data
	ALLEGRO_EVENT_QUEUE *EventQ = NULL;		//Contains event data
	ALLEGRO_BITMAP *WinIco = NULL;

	//Initialize allegro library
	if(!Initialize())
		return -1;

	al_store_state(&IOState, ALLEGRO_STATE_NEW_FILE_INTERFACE);

	//Initialize PhysFS file system
	if(!PHYSFS_init(NULL))
	{
		al_show_native_message_box(NULL, "Exception", "An error has occured.", "Could not intitialize FS.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return -1;
	}

	if(!PHYSFS_mount(GFXPACKFILE, "/", 1))
	{
		al_show_native_message_box(NULL, "Exception", "An error has occured.", "Could not load data file", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return -1;
	}

	al_set_physfs_file_interface();
	

	//Setup the display
	al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_DIRECT3D);
	if(!(display = al_create_display(DISP_WIDTH, DISP_HEIGHT)))
	{
		al_show_native_message_box(NULL, "Exception", "An error has occured.", "Initializing 51", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return -1;
	}
	WinIco = al_load_bitmap("mci.png");
	if(WinIco != NULL)
		al_set_display_icon(display, WinIco);
	al_set_window_title(display, "Xira Genesis");

	//Lock the backbuffer as the drawing surface
	al_set_target_bitmap(al_get_backbuffer(display));

	//Create a timer for FPS
	ALLEGRO_TIMER *FPSTimer = NULL;
	FPSTimer = al_create_timer(1.0);		//It will send a tick to the event queue every second
	al_start_timer(FPSTimer);

	//Limit FPS?
	ALLEGRO_TIMER *FPSLimiter = NULL;
	FPSLimiter = al_create_timer(1.0 / LIMIT_FPS);
	if(LIMIT_FPS > 1)
	{
		al_start_timer(FPSLimiter);
	}

	ALLEGRO_TIMER *HSTick = NULL;
	HSTick = al_create_timer(0.5);
	al_start_timer(HSTick);

	//Setup the event queue for input
	EventQ = al_create_event_queue();
	al_register_event_source(EventQ, al_get_keyboard_event_source());
	al_register_event_source(EventQ, al_get_mouse_event_source());
	al_register_event_source(EventQ, al_get_display_event_source(display));
	al_register_event_source(EventQ, al_get_timer_event_source(FPSTimer));
	al_register_event_source(EventQ, al_get_timer_event_source(FPSLimiter));
	al_register_event_source(EventQ, al_get_timer_event_source(HSTick));

	//Load a font for displaying text
	GameFont = al_load_ttf_font("TerminusBold-4.38.ttf", 11, NULL);

	//Set up the blender
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);

	//Our game loop
	bool KeyIsDown = false;
	bool NeedsTick = true;		//For FPS Limiter
	ALLEGRO_EVENT Cur_Event;

	bool WindowOpen = true;

	NextScene = SCENE_NOCHANGE;
	CurrentScene = &TitleScene;
	CurrentScene->Init();

	ENetEvent NetEvent;

	while(GameRunning)
	{
		Sleep(2);
		//Process our events

		if(CurrentScene->GetHost() != NULL)
		{
			if(enet_host_service(CurrentScene->GetHost(), &NetEvent, 1) > 0)
			{
				CurrentScene->ProcessNetEvent(NetEvent);
			}
		}

		if(al_get_next_event(EventQ, &Cur_Event))
		{
			if(Cur_Event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			{
				//The window has been closed
				GameRunning = false;
			}
			else if(Cur_Event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN)
			{
				WindowOpen = true;
			}
			else if(Cur_Event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT)
			{
				WindowOpen = false;
			}
			else if(Cur_Event.type == ALLEGRO_EVENT_KEY_CHAR)
			{
				CurrentScene->HandleConsoleInput(Cur_Event.keyboard.unichar);
			}
			else if(Cur_Event.type == ALLEGRO_EVENT_KEY_DOWN)
			{
				CurrentScene->ProcessInput(Cur_Event.keyboard.keycode, true);
			}
			else if(Cur_Event.type == ALLEGRO_EVENT_KEY_UP)
			{
				CurrentScene->ProcessInput(Cur_Event.keyboard.keycode, false);
			}
			else if(Cur_Event.type == ALLEGRO_EVENT_TIMER)
			{
				//Check if the FPSLimiter is on and handle
				if(Cur_Event.timer.source == FPSLimiter)
				{
					NeedsTick = true;
				}

				if(Cur_Event.timer.source == HSTick)
				{
					CurrentScene->HSTick();
				}

				//Check if our FPS Timer has ticked
				if(Cur_Event.timer.source == FPSTimer)
				{
					//It has...
					//Dont worry about it
					FPS[FPS[2]] = FPS[1];
					FPS[1] = 0;
					FPS[0] = (FPS[3] + FPS[4] + FPS[5] + FPS[6] + FPS[7]) / 5;
					FPS[2]++;
					if(FPS[2] == 8) FPS[2] = 3;
				}
			}
		}
		
		if(NeedsTick)
		{
			if(LIMIT_FPS > 1) NeedsTick = false;
			FPS[1]++;

			switch(NextScene)
			{
			case SCENE_NOCHANGE:
				break;
			case SCENE_GAMESCENE:
				CurrentScene = &GameScene;
				CurrentScene->Init();
				NextScene = SCENE_NOCHANGE;
				break;
			case SCENE_SERVERSCENE:
				CurrentScene = &ServerScene;
				CurrentScene->Init();
				NextScene = SCENE_NOCHANGE;
				break;
			case SCENE_TITLESCENE:
				CurrentScene = &TitleScene;
				CurrentScene->Init();
				NextScene = SCENE_NOCHANGE;
				break;
			case SCENE_EXITGAME:
				GameRunning = false;
				break;
			default:
				break;
			};

			if(!CurrentScene->DoLogic())
				GameRunning = false;

			if(1)
			{
				//Draw the scene5
				al_clear_to_color(al_map_rgb(0, 0, 0));	//Clear the backbuffer-+

				CurrentScene->Draw();
		
#ifdef _DEBUG
				al_draw_textf(GameFont, al_map_rgb(255, 255, 255), 5, 5, NULL, "FPS: %d", FPS[0]);		//Draw the FPS
#endif

				al_flip_display();							//Flip the backbuffer to the display
			}
		}

	}
	
	//Its only polite to clean up after yourself
	al_shutdown_primitives_addon();
	al_destroy_font(GameFont);
	al_destroy_timer(FPSTimer);
	al_destroy_timer(HSTick);
	al_destroy_timer(FPSLimiter);
	al_destroy_display(display);
	enet_deinitialize();

	return 0;
}

bool Initialize()
{
	if(!al_init())
	{
		al_show_native_message_box(NULL, "Exception", "An error has occured.", "Initializing 01", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return false;
	}

	if(!al_init_image_addon())
	{
		al_show_native_message_box(NULL, "Exception", "An error has occured.", "Initializing 02", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return false;
	}

	al_init_font_addon();

	if(!al_init_ttf_addon())
	{
		al_show_native_message_box(NULL, "Exception", "An error has occured.", "Initializing 03", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return false;
	}

	if(!al_install_keyboard())
	{
		al_show_native_message_box(NULL, "Exception", "An error has occured.", "Initializing 21", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return false;
	}

	if(!al_install_mouse())
	{
		al_show_native_message_box(NULL, "Exception", "An error has occured.", "Initializing 22", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return false;
	}
	
	if(!al_init_primitives_addon())
	{
		al_show_native_message_box(NULL, "Exception", "An error has occured.", "Initializing 23", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return false;
	}

	if(enet_initialize() != 0)
	{
		al_show_native_message_box(NULL, "Exception", "An error has occured.", "Error Initializing network.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return false;
	}

	return true;
}

bool InitializeDedicated()
{
	IsDedicated = true;
	std::cout << "Entering Xira Genesis dedicated server mode.\n";
	
	if(!al_init())
	{
		cout << "Could not initialize server.\n";
		return false;
	}

	al_store_state(&IOState, ALLEGRO_STATE_NEW_FILE_INTERFACE);

	if(!PHYSFS_init(NULL))
	{
		cout << "Could not initialize file system\n";
		return false;
	}

	if(!PHYSFS_mount(GFXPACKFILE, "/", 1))
	{
		cout << "Could not load data file\n";
		return false;
	}

	al_set_physfs_file_interface();
	
	if(enet_initialize() != 0)
	{
		cout << "Could not initialize networking\n";
		return false;
	}

	return true;
}

bool CreateConsoleHandle()
{
	AllocConsole();
	
	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
	FILE* hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, NULL, _IONBF, 128);
	*stdin = *hf_in;

	return true;
}

void DedicatedLoopThread()
{
	bool NeedsTick = true;
	ENetEvent NetEvent;

	ALLEGRO_TIMER *HSTick;
	HSTick = al_create_timer(0.5);
	al_start_timer(HSTick);

	ALLEGRO_TIMER *FPSLimiter = NULL;
	FPSLimiter = al_create_timer(1.0 / LIMIT_FPS);
	if(LIMIT_FPS > 1)
	{
		al_start_timer(FPSLimiter);
	}

	ALLEGRO_EVENT_QUEUE *EventQ;
	EventQ = al_create_event_queue();


	al_register_event_source(EventQ, al_get_timer_event_source(HSTick));
	al_register_event_source(EventQ, al_get_timer_event_source(FPSLimiter));

	ALLEGRO_EVENT Cur_Event;

	while(GameRunning)
	{
		if(CurrentScene->GetHost() != NULL)
		{
			if(enet_host_service(CurrentScene->GetHost(), &NetEvent, 1) > 0)
			{
				CurrentScene->ProcessNetEvent(NetEvent);
			}
		}

		if(al_get_next_event(EventQ, &Cur_Event))
		{

			if(Cur_Event.type == ALLEGRO_EVENT_TIMER)
			{
				if(Cur_Event.timer.source == FPSLimiter)
				{
					NeedsTick = true;
				}
				if(Cur_Event.timer.source == HSTick)
				{
					CurrentScene->HSTick();
				}
			}
		}

		if(NeedsTick)
		{
			NeedsTick = false;
			CurrentScene->DoLogic();
		}
		Sleep(2);
	}
}