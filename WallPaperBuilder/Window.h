#pragma once
#include <windows.h>
#include <SDL2/SDL_image.h>


typedef   SDL_Window* (WindowCreationFunction)(const char* title,
	int x, int y, int w,
	int h, Uint32 flags);



namespace App
{
	class Window
	{
	public:
		Window(const char* title, int x, int y, int w,
			int h, Uint32 flags, WindowCreationFunction* create_window = &SDL_CreateWindow, bool fullscreen = false)
		{
			if (SDL_Init(SDL_INIT_VIDEO))
			{
				return;
			}

			if (fullscreen)
			{
				flags = SDL_WINDOW_FULLSCREEN;
			}

			m_window = (*create_window)(
				title,
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				w,
				h,
				flags
			);

			m_renderer = SDL_CreateRenderer(m_window, -1, 0);

			if (m_renderer)
			{
				SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
			}
		}

		void Update()
		{

		}

		void Show()
		{
			SDL_ShowWindow(m_window);
		}

		void Hide()
		{
			SDL_HideWindow(m_window);
		}

		void Restore()
		{
			SDL_RestoreWindow(m_window);
		}

		void Render()
		{
			SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
			SDL_RenderPresent(m_renderer);
		}

		void Clear()
		{
			SDL_RenderClear(m_renderer);
		}

		SDL_Window* const GetWindow()
		{
			return m_window;
		}

		~Window()
		{
			SDL_DestroyWindow(m_window);
			SDL_DestroyRenderer(m_renderer);
			SDL_Quit();
		}

	private:
		SDL_Window* m_window = nullptr;
		SDL_Renderer* m_renderer = nullptr;
	};
}



