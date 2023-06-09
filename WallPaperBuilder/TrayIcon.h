#pragma once
#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <stdlib.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_timer.h>
namespace App
{
	class TrayIcon
	{
	public:
		TrayIcon(const SDL_SysWMinfo&  info, UINT callbackMessage = WM_USER + 1, const wchar_t* tip = L"My Tip")
		{
			NOTIFYICONDATA icon;
			m_id = info.info.win.window;
			icon.uCallbackMessage = callbackMessage;
			icon.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
			icon.hIcon = LoadIcon(NULL, IDI_INFORMATION);
			icon.cbSize = sizeof(icon);
			icon.hWnd = m_id;
			wcscpy_s(icon.szTip, tip);
			Shell_NotifyIcon(NIM_ADD, &icon);
		}

		~TrayIcon()
		{
			NOTIFYICONDATA icon;
			icon.uCallbackMessage = NULL;
			icon.uFlags = NIF_ICON;
			icon.hIcon = NULL;
			icon.uID = NULL;
			icon.hWnd = m_id;
			Shell_NotifyIcon(NIM_DELETE, &icon);
		}
	private:
		HWND m_id = 0;
	};

}