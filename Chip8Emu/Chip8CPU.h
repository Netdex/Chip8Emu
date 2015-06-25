#pragma once
#include <string>
class Chip8CPU
{
public:
	unsigned char key[16];
	bool draw_flag;

	Chip8CPU();
	~Chip8CPU();
	
	void emulateCycle();

	unsigned char* getGFX();
	bool loadApplication(std::string filename);
private:
	void clearScreen();
	void clearMemory();
	void clearRegisters();
	void clearStack();
	void clearKeys();
	void initialize();
	static bool exists_test(const std::string& name);

};

