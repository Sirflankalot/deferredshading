#pragma once

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <iostream>
#include <sstream>

struct SDL_Manager {
	SDL_Window* mainWindow;
	SDL_GLContext mainContext;
	struct size_t {
		int width;
		int height;
		float ratio;
	} size;
	SDL_Manager() {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			std::ostringstream ss;
			ss << "Video initialization failed: " << SDL_GetError() << '\n';
			throw std::runtime_error(ss.str().c_str());
		}

		mainWindow =
			SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							 WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

		if (!mainWindow) {
			std::ostringstream ss;
			ss << "SDL2/OPENGL window creation failed: " << SDL_GetError() << '\n';
			throw std::runtime_error(ss.str().c_str());
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		#ifdef DLDEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
		#endif
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		mainContext = SDL_GL_CreateContext(mainWindow);

		SDL_GL_SetSwapInterval(0);

		glewExperimental = GL_TRUE;
		glewInit();

		this->refresh_size();
	}
	~SDL_Manager() {
		SDL_GL_DeleteContext(mainContext);
		SDL_DestroyWindow(mainWindow);
		SDL_Quit();
	}
	void refresh_size() {
		SDL_GetWindowSize(mainWindow, &(this->size.width), &(this->size.height));
		glViewport(0, 0, size.width, size.height);
		size.ratio = (float) size.width / (float) size.height;
	}
};