
#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <stdlib.h>
#include <lua.hpp>
#include  <SDL2/SDL_syswm.h>
#include "WallPaper.h"
#undef main
#define FPS 60
#define frameDelay (1000 / FPS)
#undef main



SDL_Renderer* global_renderer = nullptr;

void DrawRect(int x, int y, int w, int h)
{
	if (!global_renderer) return;

	/*std::cout << "drawing rect " << x << " " <<
		y << " " <<
		w << " " <<
		h << std::endl;*/

	SDL_Rect rect{x,y,w,h};
	SDL_SetRenderDrawColor(global_renderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(global_renderer, &rect);
	SDL_RenderFillRect(global_renderer, &rect);
}

int Lua_DrawRect(lua_State* l)
{
	double x = luaL_checknumber(l, 1);
	double y = luaL_checknumber(l, 2);
	double w = luaL_checknumber(l, 3);
	double h = luaL_checknumber(l, 4);
	DrawRect(x, y, w, h);
	return 0;
}

void FPSCap(Uint32 starting_tick) {
	if (frameDelay > SDL_GetTicks() - starting_tick) {
		SDL_Delay(frameDelay - (SDL_GetTicks() - starting_tick));
	}
}

int main(int argc, char* argv[])
{

	using namespace std;
	lua_State* L;
	L = luaL_newstate();
	luaL_openlibs(L);






	uint32_t startingTick;
	int endTick;

	App::Window ui(
		"title",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		300,
		200,
		SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE
	);

	App::WallPaper wall;

	global_renderer = wall.GetRenderer();


	bool isHidden = true;


	HWND id = 0;
	{
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);

		NOTIFYICONDATA icon;
		if (SDL_GetWindowWMInfo(ui.GetWindow(), &info))
		{
			id = info.info.win.window;
			icon.uCallbackMessage = WM_USER + 1;
			icon.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
			icon.hIcon = LoadIcon(NULL, IDI_INFORMATION);
			icon.cbSize = sizeof(icon);
			icon.hWnd = id;
			wcscpy_s(icon.szTip, L"AAA");

			bool success = Shell_NotifyIcon(NIM_ADD, &icon);
		}
	}

	bool animation = true;

	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
	SDL_Event e;
	bool isRunning = true;



	lua_pushcfunction(L, Lua_DrawRect);
	lua_setglobal(L, "DrawRect");

	if (luaL_dofile(L, "main.lua"))
	{
		cout << "error" << endl;
		return 1;
	}

	lua_getglobal(L, "Main");
	lua_call(L, 0, 1);


	while (isRunning) {
		// Get the number of milliseconds since the SDL library initialization.
		startingTick = SDL_GetTicks();


		//--- USE THIS WHEN: You have an application window and want to be able to close it.
		while (SDL_PollEvent(&e) != 0)
		{
			switch (e.type)
			{
			case SDL_SYSWMEVENT:
				if (e.syswm.msg->msg.win.msg == WM_USER + 1)
				{
					//std::cout << "WM_USER" << std::endl;
					if (LOWORD(e.syswm.msg->msg.win.lParam) == WM_RBUTTONDOWN && isHidden)
					{
						ui.Show();
						ui.Restore();
						ui.Clear();
						isHidden = false;
					}
					else if (LOWORD(e.syswm.msg->msg.win.lParam) == WM_LBUTTONDOWN && isHidden)
					{
						animation = !animation;
					}
				}
				break;
			case SDL_QUIT:
				std::cout << "QUIT" << std::endl;
				isRunning = false;
				break;
			case SDL_WINDOWEVENT:
				switch (e.window.event)
				{
				case SDL_WINDOWEVENT_MINIMIZED:
					ui.Hide();
					isHidden = true;
					break;
				case SDL_WINDOWEVENT_CLOSE:
					isRunning = false;
					break;
				default:
					break;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				std::cout << "pressed" << std::endl;
				animation = !animation;
				break;

			}
		}
		if (animation)
		{
			wall.Clear();
			lua_getglobal(L, "OnUpdate");
			lua_call(L, 0, 1);
			wall.Update();
			lua_getglobal(L, "OnFrame");
			lua_call(L, 0, 1);
			wall.Render();
		}


		FPSCap(startingTick);
	}

	{
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);

		NOTIFYICONDATA icon;
		if (SDL_GetWindowWMInfo(ui.GetWindow(), &info))
		{
			icon.uCallbackMessage = NULL;
			icon.uFlags = NIF_ICON;
			icon.hIcon = NULL;
			icon.uID = NULL;
			icon.hWnd = id;
			wcscpy_s(icon.szTip, L"Test tip");
			bool success = Shell_NotifyIcon(NIM_DELETE, &icon);
		}
	}

	global_renderer = nullptr;
	lua_close(L);


	return 0;
}
