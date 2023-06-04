#pragma once
#include "Window.h"

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
HWND _GetWallpaperWindow();


namespace App
{
	class WallPaper : public Window
	{
	public:
		WallPaper(Uint32 flags = 0)
			:Window("", 0, 0, 0, 0, flags, WallPaper::GetWallPaperWindow, true)
		{

		}
	private:
		static SDL_Window* GetWallPaperWindow(const char* title,
			int x, int y, int w,
			int h, Uint32 flags)
		{
			return SDL_CreateWindowFrom(_GetWallpaperWindow());
		}
	};
}



