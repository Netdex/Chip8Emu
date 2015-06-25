// Chip8Emu.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SDL.h"
#include <stdio.h>
#include <iostream>
#include "Chip8CPU.h"

using namespace std;

const int PIXEL_WIDTH = 16;

const int WIDTH = 64 * PIXEL_WIDTH;
const int HEIGHT = 32 * PIXEL_WIDTH;

const string FILENAME = "IBM Logo.ch8";

SDL_Window* window = NULL;
SDL_Surface* screen = NULL;
SDL_Renderer* renderer = NULL;

bool init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		return false;
	}
	window = SDL_CreateWindow(FILENAME.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED
		, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	screen = SDL_GetWindowSurface(window);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	return true;
}

bool close()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	bool initcode = init();
	if (!initcode)
	{
		printf("Failed to initialize SDL!");
		exit(1);
	}

	printf("CHIP8 EMULATOR - GORDON GUAN\n");
	Chip8CPU cpu;
	bool loaded = cpu.loadApplication("D:\\Programming\\C++\\Chip8Emu\\Release\\" + FILENAME);
	if (!loaded)
	{
		printf("Failed to load Chip8 application!");
		exit(1);
	}

	bool quit = false;
	SDL_Event e;
	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			else if (e.type == SDL_KEYDOWN)
			{
				switch (e.key.keysym.sym)
				{
				case SDLK_1:
					cpu.key[0x01] = 1;
					break;
				case SDLK_2:
					cpu.key[0x02] = 1;
					break;
				case SDLK_3:
					cpu.key[0x03] = 1;
					break;
				case SDLK_4:
					cpu.key[0x0C] = 1;
					break;
				case SDLK_q:
					cpu.key[0x04] = 1;
					break;
				case SDLK_w:
					cpu.key[0x05] = 1;
					break;
				case SDLK_e:
					cpu.key[0x06] = 1;
					break;
				case SDLK_r:
					cpu.key[0x0D] = 1;
					break;
				case SDLK_a:
					cpu.key[0x07] = 1;
					break;
				case SDLK_s:
					cpu.key[0x08] = 1;
					break;
				case SDLK_d:
					cpu.key[0x09] = 1;
					break;
				case SDLK_f:
					cpu.key[0x0E] = 1;
					break;
				case SDLK_z:
					cpu.key[0x0A] = 1;
					break;
				case SDLK_x:
					cpu.key[0x00] = 1;
					break;
				case SDLK_c:
					cpu.key[0x0B] = 1;
					break;
				case SDLK_v:
					cpu.key[0x0F] = 1;
					break;
				}
			}
			else if (e.type = SDL_KEYUP)
			{
				switch (e.key.keysym.sym)
				{
				case SDLK_1:
					cpu.key[0x01] = 0;
					break;
				case SDLK_2:
					cpu.key[0x02] = 0;
					break;
				case SDLK_3:
					cpu.key[0x03] = 0;
					break;
				case SDLK_4:
					cpu.key[0x0C] = 0;
					break;
				case SDLK_q:
					cpu.key[0x04] = 0;
					break;
				case SDLK_w:
					cpu.key[0x05] = 0;
					break;
				case SDLK_e:
					cpu.key[0x06] = 0;
					break;
				case SDLK_r:
					cpu.key[0x0D] = 0;
					break;
				case SDLK_a:
					cpu.key[0x07] = 0;
					break;
				case SDLK_s:
					cpu.key[0x08] = 0;
					break;
				case SDLK_d:
					cpu.key[0x09] = 0;
					break;
				case SDLK_f:
					cpu.key[0x0E] = 0;
					break;
				case SDLK_z:
					cpu.key[0x0A] = 0;
					break;
				case SDLK_x:
					cpu.key[0x00] = 0;
					break;
				case SDLK_c:
					cpu.key[0x0B] = 0;
					break;
				case SDLK_v:
					cpu.key[0x0F] = 0;
					break;
				}
			}
		}

		if (cpu.draw_flag){
			SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			unsigned char* gfx = cpu.getGFX();

			for (int y = 0; y < 32; ++y)
			{
				for (int x = 0; x < 64; ++x)
				{
					unsigned char pixel = gfx[y * 64 + x];
					if (pixel != 0)
					{
						SDL_Rect fillRect = { x * PIXEL_WIDTH , y * PIXEL_WIDTH, PIXEL_WIDTH, PIXEL_WIDTH };
						SDL_RenderFillRect(renderer, &fillRect);
					}
				}
			}

			//Update screen
			SDL_RenderPresent(renderer);
			cpu.draw_flag = false;
		}
		cpu.emulateCycle();
		SDL_Delay(1);
	}
	close();
	return 0;
}

