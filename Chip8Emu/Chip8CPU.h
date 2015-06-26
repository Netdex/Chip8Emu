#pragma once
#include <cstdlib>
#include <algorithm>
#include "time.h"
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include "windows.h"
#include <thread>

using namespace std;

class Chip8CPU
{
public:
	unsigned char gfx[64 * 32];
	unsigned char key[16];
	bool draw_flag;

	Chip8CPU();
	~Chip8CPU();
	
	void emulateCycle();

	bool loadApplication(string filename);
private:
	void initialize();


};

