#include "Game.h"

int NextScene;
ALLEGRO_FONT *GameFont = NULL;

//Todo: Fix colors

int Scene::GetIntFromPacket(ENetPacket *Packet, int Place)
{
	if(Packet->dataLength >= sizeof(int)*Place)
	{
		int r;
		memcpy(&r, Packet->data + (Place * sizeof(int)), sizeof(int));
		return r;
	}
	return -1;
}

void Scene::ConsoleDraw(int x, int y, int Lines, int LineOffset)
{
	int TextWidth = 0;

	if(ConsoleInput.length() > 0)
	{
		al_draw_text(GameFont, al_map_rgb(255, 255, 255), x, 8 + y + ((Lines+1) * GameFont->height), NULL, ConsoleInput.c_str());
	}

	if(Lines > ConsoleLine.size())
		Lines = ConsoleLine.size();

	int WriteLine;
	for(int DrawLine = 0; DrawLine < Lines; DrawLine++)
	{
		WriteLine = ConsoleLine.size() - DrawLine - 1 - LineOffset;
		for(size_t LineCol = 0; LineCol < ConsoleLine[WriteLine].size(); LineCol++)
		{
			if(LineCol > 0)
				TextWidth += al_get_text_width(GameFont, ConsoleLine[WriteLine][LineCol - 1].c_str());
			else
				TextWidth = 0;

			al_draw_textf(GameFont, ConsoleLineColors[WriteLine][LineCol], x + TextWidth, y + (Lines * GameFont->height) - (DrawLine * GameFont->height), NULL,
				"%s", ConsoleLine[WriteLine][LineCol].c_str());
		}
	}
}

void Scene::ConsoleWrite(string Str)
{
	if(IsDedicated)
	{
		string NewStr;
		for(int x = 0; x < Str.length(); x++)
		{
			if(Str.c_str()[x] == '<' && Str.c_str()[x+2] == '>')
			{
				x+=3;
			}
			if(x < Str.length())
			{
				NewStr += Str.c_str()[x];
			}
		}
		cout << NewStr << endl;
		return;
	}

	int LastOffset = 0;
	vector<ALLEGRO_COLOR> Colors;
	vector<string> Strings;
	Colors.push_back(al_map_rgb(255, 255, 255));
	for(size_t x = 0; x < Str.length(); x++)
	{
		if(Str.c_str()[x] == '<' && Str.length() >= x + 3)
		{
			if(Str.c_str()[x+2] == '>')
			{
				ALLEGRO_COLOR C = al_map_rgb(255, 255, 255);
				string S;
				switch(Str.c_str()[x+1])
				{
					case 'R':
						C = al_map_rgb(255, 25, 25);
						break;
					case 'G':
						C = al_map_rgb(25, 255, 25);
						break;
					case 'B':
						C = al_map_rgb(50, 50, 255);
						break;
					case 'P':
						C = al_map_rgb(255, 25, 255);
						break;
					case 'C':
						C = al_map_rgb(25, 255, 255);
						break;
					case 'Y':
						C = al_map_rgb(255, 255, 25);
						break;
				}
				S.assign(Str, LastOffset, x - LastOffset);
				Strings.push_back(S);
				Colors.push_back(C);
				x += 3;
				LastOffset = x;
			}
		}
	}

	string S;
	S.assign(Str, LastOffset, Str.length());
	Strings.push_back(S);

	ConsoleLine.push_back(Strings);
	ConsoleLineColors.push_back(Colors);
	Colors.clear();
	Strings.clear();
}

void Scene::HandleConsoleInput(char Key)
{
	if(Key >= 20 && Key < 128 && ConsoleInput.length() <= 115)
		ConsoleInput += Key;

	if(Key == 8 && ConsoleInput.length() > 0)
	{
		ConsoleInput.erase(ConsoleInput.length() - 1, 1);
	}
	if(Key == 13 && ConsoleInput.length() > 0)
	{
		ProcessConsoleInput();
		ConsoleInput.clear();
	}
}

int Scene::GetWordCount(string Str)
{
	vector<string> Tokens;
	istringstream ISS(Str);

	copy(istream_iterator<string>(ISS), istream_iterator<string>(), back_inserter<vector<string>>(Tokens));

	return Tokens.size();
}

string Scene::GetWord(string Str, int Word)
{
	vector<string> Tokens;
	istringstream ISS(Str);

	copy(istream_iterator<string>(ISS), istream_iterator<string>(), back_inserter<vector<string>>(Tokens));

	if(Tokens.size() == 0)
		return "ERROR";

	if(Word >= Tokens.size())
		Word = 0;

	return Tokens[Word];
}

int Scene::ParsePacket(ENetPacket *Packet)
{
	PacketString = "";
	for(unsigned int x = 0; x<Packet->dataLength; x++)
	{
		PacketString += Packet->data[x];
	}
	return 0;
}

string Scene::JoinStringArray(string *Array, char Sep, int First, int Last)
{
	ostringstream Ret(ostringstream::out);
	for(int x = First; x < Last; x++)
	{
		Ret << Array[x] << Sep;
	}
	Ret << Array[Last];

	return Ret.str();
}

string Scene::StrToLower(string Str)
{
	ostringstream Ret(ostringstream::out);
	for(int x = 0; x < Str.length(); x++)
	{
		Ret << (char)tolower(Str.c_str()[x]);
	}
	return Ret.str();
}

bool Scene::CheckParam(string Str, string Check, int Start)
{
	istringstream ISS(Str);
	istringstream ISSB(Check);
	vector<string> Tokens;
	vector<string> CheckTokens;

	copy(istream_iterator<string>(ISS), istream_iterator<string>(), back_inserter<vector<string>>(Tokens));
	copy(istream_iterator<string>(ISSB), istream_iterator<string>(), back_inserter<vector<string>>(CheckTokens));

	if(CheckTokens.size() == 0 || Tokens.size() == 0 || Start + Tokens.size() > CheckTokens.size())
		return false;

	int TokenCount = 0;
	int i = 0;
	for(unsigned int x = Start; x < Start + Tokens.size(); x++)
	{
		if(ShortStrCompare(StrToLower(Tokens[i]), StrToLower(CheckTokens[x])))
			TokenCount++;

		i++;
	}

	if(TokenCount == Tokens.size())
		return true;

	return false;
}

string Scene::GetString(string Str, int Start, int End)
{
	istringstream ISS(Str);
	ostringstream OSS(ostringstream::out);
	vector<string> Tokens;
	copy(istream_iterator<string>(ISS), istream_iterator<string>(), back_inserter<vector<string>>(Tokens));

	if(End == 0 || End > Tokens.size())
		End = Tokens.size();

	if(Start >= End)
		Start = 0;

	for(unsigned int x = Start; x < End; x++)
	{
		OSS << Tokens[x];
		if(x < End - 1)
			OSS << " ";
	}

	return OSS.str();
}

bool Scene::ShortStrCompare(string FullStr, string StrCmp)
{
	if(StrCmp.length() > FullStr.length())
		return false;

	int CharCheck = 0;
	for(unsigned int x = 0; x < StrCmp.length(); x++)
	{
		if(StrCmp.c_str()[x] == FullStr.c_str()[x])
		{
			CharCheck++;
		}
	}

	if(CharCheck == StrCmp.length())
		return true;

	return false;
}

ENetHost *Scene::GetHost()
{
	return Host;
}

int Scene::Rand(int Min, int Max)
{
	uniform_int_distribution<int> Dist(Min, Max);
	return Dist(RandGen);
}