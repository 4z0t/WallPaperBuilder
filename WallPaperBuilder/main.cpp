
#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <stdlib.h>
#include <lua.hpp>
#include  <SDL2/SDL_syswm.h>
#include <SDL2/SDL_timer.h>
#include "WallPaper.h"
#include "LuaTemplates.h"
#include <functional>
#undef main
#define FPS 60
#define frameDelay (1000 / FPS)
#undef main



SDL_Renderer* global_renderer = nullptr;
SDL_Window* wallpaper_window = nullptr;

//template<typename int>
//void DrawRect(int x, int y, int w, int h)
//{
//	if (!global_renderer) return;
//
//	SDL_Rect rect{ x,y,w,h };
//	SDL_SetRenderDrawColor(global_renderer, 255, 255, 255, 255);
//	SDL_RenderDrawRect(global_renderer, &rect);
//	SDL_RenderFillRect(global_renderer, &rect);
//}

void DrawRect(float x, float y, float w, float h)
{
	if (!global_renderer) return;

	SDL_FRect rect{ x,y,w,h };
	SDL_SetRenderDrawColor(global_renderer, 255, 255, 255, 255);
	SDL_RenderDrawRectF(global_renderer, &rect);
	SDL_RenderFillRectF(global_renderer, &rect);
}

void DrawLine(float x1, float y1, float x2, float y2)
{
	if (!global_renderer) return;

	SDL_SetRenderDrawColor(global_renderer, 255, 255, 255, 255);
	SDL_RenderDrawLineF(global_renderer, x1, y1, x2, y2);
}


int Lua_DrawLine(lua_State* l)
{
	float x1 = luaL_checknumber(l, 1);
	float y1 = luaL_checknumber(l, 2);
	float x2 = luaL_checknumber(l, 3);
	float y2 = luaL_checknumber(l, 4);
	DrawLine(x1, y1, x2, y2);
	return 0;
}

int Lua_DrawRect(lua_State* l)
{
	float x = luaL_checknumber(l, 1);
	float y = luaL_checknumber(l, 2);
	float w = luaL_checknumber(l, 3);
	float h = luaL_checknumber(l, 4);
	DrawRect(x, y, w, h);
	return 0;
}

int Lua_GetWallpaperWindowSize(lua_State* l)
{
	if (!wallpaper_window) return 0;
	int w, h;
	SDL_GetWindowSize(wallpaper_window, &w, &h);

	lua_pushnumber(l, w);
	lua_pushnumber(l, h);

	return 2;
}

inline void RegisterFunction(lua_State* l, const char* name, lua_CFunction func)
{
	lua_pushcfunction(l, func);
	lua_setglobal(l, name);
}

void FPSCap(Uint32 starting_tick) {
	if (frameDelay > SDL_GetTicks() - starting_tick) {
		SDL_Delay(frameDelay - (SDL_GetTicks() - starting_tick));
	}
}

class FnClass {
public:
	static int Call(int a, int b)
	{
		std::cout << a << b << std::endl;;
		return a * 2 + b;
	}
};

int main(int argc, char* argv[])
{
	using namespace std;
	lua_State* L;
	L = luaL_newstate();
	luaL_openlibs(L);

	uint32_t startingTick = 0;


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
	wallpaper_window = wall.GetWindow();


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


	//RegisterFunction(L, "Test", LuaFunc<void(int)>(PrintInt));
	//RegisterFunction(L, "DoubleInt", Lua_WrapFunction(DoubleInt));




	RegisterFunction(L, "DoubleInt", Lua_FunctionWrapper<FnClass, int, int>::Function);
	RegisterFunction(L, "DrawRect", Lua_DrawRect);
	RegisterFunction(L, "DrawLine", Lua_DrawLine);
	RegisterFunction(L, "GetWindowSize", Lua_GetWallpaperWindowSize);

	if (luaL_dofile(L, "main.lua"))
	{
		cout << "error" << endl;
		return 1;
	}

	Lua_CallFunction(L, "Main");


	while (isRunning) {

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
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					std::cout << "pressed" << std::endl;
					animation = !animation;

				}
				else if (e.button.button == SDL_BUTTON_RIGHT)
				{
					if (luaL_dofile(L, "main.lua"))
					{
						std::cout << "reload failed: " <<
							lua_tostring(L, -1) << std::endl;
						break;
					}
					std::cout << "reloaded" << std::endl;
					Lua_CallFunction(L, "Main");
				}
				break;

			}
		}

		double delta = (double)(SDL_GetTicks() - startingTick) / 1000;
		startingTick = SDL_GetTicks();


		if (animation)
		{
			wall.Clear();
			Lua_CallFunction(L, "OnUpdate", delta);
			wall.Update();
			Lua_CallFunction(L, "OnFrame", delta);
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
