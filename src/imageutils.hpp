#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <filesystem>
#include <SDL.h>
#include <SDL_image.h>

inline int loadImageFromMem(void* mem, size_t length) {
	SDL_Event event;
	SDL_Renderer* renderer = NULL;
	SDL_Texture* texture = NULL;
	SDL_Window* window = NULL;

	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);


	SDL_RWops* rw = SDL_RWFromMem(mem, length);
	SDL_Surface* surface = IMG_Load_RW(rw, false);
	if (!surface) {
		std::cout << IMG_GetError();
		return 1;
	}

	window = SDL_CreateWindow("test", 50, 50, 0, 0, SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, 0);
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Point size;
	SDL_QueryTexture(texture, NULL, NULL, &size.x, &size.y);
	SDL_SetWindowSize(window, size.x / 5, size.y / 5);
	SDL_SetWindowBordered(window, SDL_TRUE);

	while (1) {
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				break;
			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
				break;
		}
	}
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

/*
int main(int argc, char** argv) {
	std::ifstream is("picture.png", std::ios::binary);
	if (!is) {
		return 1;
	}
	// get length of file:
	is.seekg(0, is.end);
	int length = is.tellg();
	is.seekg(0, is.beg);

	char x[1024];

	char* buffer = new char[length];

	std::cout << "Reading " << length << " characters...";

	is.read(buffer, length);

	if (is){
		std::cout << "all characters read successfully.";
	}	
	else{
		std::cout << "error: only " << is.gcount() << " could be read";
		return 1;
	}

	loadImageFromMem(buffer, length);

	delete[] buffer;
	
	return 0;
}
*/