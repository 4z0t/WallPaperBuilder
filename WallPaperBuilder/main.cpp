
#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <stdlib.h>
#include <tuple>
#include <lua.hpp>
#include  <SDL2/SDL_syswm.h>
#include <SDL2/SDL_timer.h>
#include "WallPaper.hpp"
#include "LuaTemplates.hpp"
#include <functional>
#undef main
#define FPS 60
#define frameDelay (1000 / FPS)
#undef main



SDL_Renderer* global_renderer = nullptr;
SDL_Window* wallpaper_window = nullptr;


void SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if (!global_renderer) return;
	SDL_SetRenderDrawColor(global_renderer, r, g, b, a);
}

void DrawRect(float x, float y, float w, float h)
{
	if (!global_renderer) return;

	SDL_FRect rect{ x,y,w,h };
	SDL_RenderDrawRectF(global_renderer, &rect);
	SDL_RenderFillRectF(global_renderer, &rect);
}

void DrawLine(float x1, float y1, float x2, float y2)
{
	if (!global_renderer) return;

	SDL_SetRenderDrawColor(global_renderer, 255, 255, 255, 255);
	SDL_RenderDrawLineF(global_renderer, x1, y1, x2, y2);
}

std::tuple<int, int> GetWallpaperWindowSize()
{
	if (!wallpaper_window) return { 0,0 };
	int w, h;
	SDL_GetWindowSize(wallpaper_window, &w, &h);
	return { w,h };
}

void FPSCap(Uint32 starting_tick) {
	if (frameDelay > SDL_GetTicks() - starting_tick) {
		SDL_Delay(frameDelay - (SDL_GetTicks() - starting_tick));
	}
}

class Callable
{
public:
	int operator()(int a, int b)
	{
		return a * a + b;
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


	Lua::RegisterFunction(L, "DoubleInt", Lua::FunctionWrapper<Callable, int, int>::Function);
	Lua::RegisterFunction(L, "SetColor", Lua::CFunctionWrapper<functype(SetColor), Uint8, Uint8, Uint8, Uint8>::Function);
	Lua::RegisterFunction(L, "DrawRect", Lua::CFunctionWrapper<functype(DrawRect), float, float, float, float>::Function);
	Lua::RegisterFunction(L, "DrawLine", Lua::CFunctionWrapper<functype(DrawLine), float, float, float, float>::Function);
	Lua::RegisterFunction(L, "GetWindowSize", Lua::CFunctionWrapper<functype(GetWallpaperWindowSize)>::Function);

	if (luaL_dofile(L, "main.lua"))
	{
		cout << "error:" << lua_tostring(L, -1) << std::endl;
		return 1;
	}

	Lua::CallFunction(L, "Main");


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
					Lua::CallFunction(L, "Main");
				}
				break;

			}
		}

		double delta = (double)(SDL_GetTicks() - startingTick) / 1000;
		startingTick = SDL_GetTicks();


		if (animation)
		{
			wall.Clear();
			Lua::CallFunction(L, "OnUpdate", delta);
			wall.Update();
			Lua::CallFunction(L, "OnFrame", delta);
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
