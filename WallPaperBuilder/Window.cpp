#include "Window.hpp"

SDL_SysWMinfo App::Window::GetWMInfo()
{
	SDL_SysWMinfo info;
	SDL_GetWindowWMInfo(m_window, &info);
	return info;
}
