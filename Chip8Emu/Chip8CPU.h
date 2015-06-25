#pragma once
#include <string>
class Chip8CPU
{
public:
	unsigned char gfx[64 * 32];
	unsigned char key[16];
	bool draw_flag;

	Chip8CPU();
	~Chip8CPU();
	
	void emulateCycle();

	bool loadApplication(std::string filename);
private:
	void initialize();
	static bool exists_test(const std::string& name);

};

