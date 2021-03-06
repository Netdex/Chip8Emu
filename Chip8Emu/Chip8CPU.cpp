#include "stdafx.h"
#include "Chip8CPU.h"

unsigned char chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

unsigned char memory[4096];
unsigned char V[16];
unsigned short I;
unsigned short pc;
unsigned char delay_timer;
unsigned char sound_timer;
unsigned short stack[16];
unsigned short sp;

Chip8CPU::Chip8CPU()
{
}

Chip8CPU::~Chip8CPU()
{
}


void Chip8CPU::initialize()
{
	I = 0;
	pc = 0x200;
	sp = 0;

	for (int i = 0; i < 64 * 32; i++)
		gfx[i] = 0;
	draw_flag = true;
	for (int i = 0; i < 4096; i++)
	{	
		if (i < 0x1FF)
			memory[i] = chip8_fontset[i];
		else
			memory[i] = 0;
	}
	for (int i = 0; i < 16; i++){
		V[i] = 0;
		stack[i] = 0;
		key[i] = 0;
	}

	delay_timer = 0;
	sound_timer = 0;

	srand(time(NULL));
}

bool Chip8CPU::loadApplication(string filename)
{
	initialize();
	printf("Loading: %s\n", filename.c_str());

	// Open file
	ifstream file(filename, ios::in | ios::binary | ios::ate);
	streampos lSize;
	if (file.is_open())
	{
		// Check file size
		lSize = file.tellg();
		printf("File size: %d byte(s)\n", (int)lSize);
		file.seekg(0, ios::beg);
		// Allocate memory to contain the whole file
		char* buffer = new char[sizeof(char) * lSize];

		printf("Beginning to read file...\n");
		// Copy the file into the buffer
		file.read(buffer, lSize);
		printf("File read complete!\n");
		// Copy buffer to Chip8 memory
		if ((4096 - 0x200) > lSize)
		{
			printf("Copying ROM to memory...\n");
			for (int i = 0; i < lSize; ++i)
				memory[i + 0x200] = buffer[i];
			printf("ROM copy complete!\n");
		}
		else
			printf("Could not copy, ROM is larger than memory buffer!\n");
		// Close file, free buffer
		file.close();
		delete[] buffer;
		return true;
	}
	else
	{
		printf("Could not access file!\n");
		return false;
	}
}

void Chip8CPU::emulateCycle()
{
	short opcode = memory[pc] << 8 | memory[pc + 1];
	const unsigned short NNN = opcode & 0x0FFF;
	const unsigned short NN = opcode & 0x00FF;
	const unsigned short N = opcode & 0x000F;
	const unsigned short X = (opcode & 0x0F00) >> 8;
	const unsigned short Y = (opcode & 0x00F0) >> 4;

	switch (opcode & 0xF000)
	{
	case 0x0000:
		switch (N)
		{
		case 0x0000: // 0x00E0: Clears the screen
			for (int i = 0; i < 2048; ++i)
				gfx[i] = 0x0;
			draw_flag = true;
			pc += 2;
			break;
		case 0x000E: // 0x00EE: Returns from subroutine
			--sp;			// 16 levels of stack, decrease stack pointer to prevent overwrite
			pc = stack[sp];	// Put the stored return address from the stack back into the program counter					
			pc += 2;	
			break;
		default:
			printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
		}
		break;
	case 0x1000: // 0x1NNN: Jumps to address NNN
		pc = NNN;
		break;
	case 0x2000: // 0x2NNN: Calls subroutine at NNN
		stack[sp] = pc;			// Store current address in stack
		++sp;					// Increment stack pointer
		pc = NNN;	// Set the program counter to the address at NNN
		break;
	case 0x3000: // 0x3XNN: Skips the next instruction if VX equals NN
		if (V[X] == NN)
			pc += 4;
		else
			pc += 2;
		break;
	case 0x4000: // 0x4XNN: Skips the next instruction if VX doesn't equal NN
		if (V[X] != NN)
			pc += 4;
		else
			pc += 2;
		break;
	case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY
		if (V[X] == V[Y])
			pc += 4;
		else
			pc += 2;
		break;
	case 0x6000: // 0x6XNN: Sets VX to NN
		V[X] = NN;
		pc += 2;
		break;
	case 0x7000: // 0x7XNN: Adds NN to VX
		V[X] += NN;
		pc += 2;
		break;
	case 0x8000:
		switch (N)
		{
		case 0x0000: // 0x8XY0: Sets VX to the value of VY
			V[X] = V[Y];
			pc += 2;
			break;
		case 0x0001: // 0x8XY1: Sets VX to "VX OR VY"
			V[X] |= V[Y];
			pc += 2;
			break;
		case 0x0002: // 0x8XY2: Sets VX to "VX AND VY"
			V[X] &= V[Y];
			pc += 2;
			break;
		case 0x0003: // 0x8XY3: Sets VX to "VX XOR VY"
			V[X] ^= V[Y];
			pc += 2;
			break;
		case 0x0004: // 0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't					
			if (V[Y] > (0xFF - V[X]))
				V[0xF] = 1; //carry
			else
				V[0xF] = 0;
			V[X] += V[Y];
			pc += 2;
			break;
		case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
			if (V[Y] > V[X])
				V[0xF] = 0; // borrow flag
			else
				V[0xF] = 1;
			V[X] -= V[Y];
			pc += 2;
			break;
		case 0x0006: // 0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
			V[0xF] = V[X] & 0x1;
			V[X] >>= 1;
			pc += 2;
			break;
		case 0x0007: // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
			if (V[X] > V[Y])	// VY-VX
				V[0xF] = 0; // there is a borrow
			else
				V[0xF] = 1;
			V[X] = V[Y] - V[X];
			pc += 2;
			break;
		case 0x000E: // 0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
			V[0xF] = V[X] >> 7;
			V[X] <<= 1;
			pc += 2;
			break;
		default:
			printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
		}
		break;
	case 0x9000: // 0x9XY0: Skips the next instruction if VX doesn't equal VY
		if (V[X] != V[Y])
			pc += 4;
		else
			pc += 2;
		break;
	case 0xA000: // ANNN: Sets I to the address NNN
		I = NNN;
		pc += 2;
		break;
	case 0xB000: // BNNN: Jumps to the address NNN plus V0
		pc = (NNN) + V[0];
		break;

	case 0xC000: // CXNN: Sets VX to a random number and NN
		V[X] = (rand() % 0xFF) & (NN);
		pc += 2;
		break;
	case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
		// Each row of 8 pixels is read as bit-coded starting from memory location I; 
		// I value doesn't change after the execution of this instruction. 
		// VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, 
		// and to 0 if that doesn't happen
	{
		unsigned short pixel;
		V[0xF] = 0;
		for (int yline = 0; yline < N; yline++)
		{
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++)
			{
				if ((pixel & (0x80 >> xline)) != 0)
				{
					if (gfx[(V[X] + xline + ((V[Y] + yline) * 64))] > 0)
						V[0xF] = 1;
					gfx[V[X] + xline + ((V[Y] + yline) * 64)] ^= 1;
				}
			}
		}
		draw_flag = true;
		pc += 2;
	}
	break;
	case 0xE000:
		switch (NN)
		{
		case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
			if (key[V[X]] != 0)
				pc += 4;
			else
				pc += 2;
			break;
		case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
			if (key[V[X]] == 0)
				pc += 4;
			else
				pc += 2;
			break;
		default:
			printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
		}
		break;
	case 0xF000:
		switch (NN)
		{
		case 0x0007: // FX07: Sets VX to the value of the delay timer
			V[X] = delay_timer;
			pc += 2;
			break;
		case 0x000A: // FX0A: A key press is awaited, and then stored in VX		
		{
			bool keyPress = false;
			for (int i = 0; i < 16; ++i)
			{
				if (key[i] != 0)
				{
					V[X] = i;
					keyPress = true;
				}
			}
			// If we didn't receive a keypress, skip this cycle and try again.
			if (!keyPress)
				return;

			pc += 2;
		}
		break;
		case 0x0015: // FX15: Sets the delay timer to VX
			delay_timer = V[X];
			pc += 2;
			break;
		case 0x0018: // FX18: Sets the sound timer to VX
			sound_timer = V[X];
			pc += 2;
			break;
		case 0x001E: // FX1E: Adds VX to I
			if (I + V[X] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
				V[0xF] = 1;
			else
				V[0xF] = 0;
			I += V[X];
			pc += 2;
			break;
		case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
			I = V[X] * 0x5;
			pc += 2;
			break;
		case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2
			memory[I] = V[X] / 100;
			memory[I + 1] = (V[X] / 10) % 10;
			memory[I + 2] = (V[X] % 100) % 10;
			pc += 2;
			break;
		case 0x0055: // FX55: Stores V0 to VX in memory starting at address I					
			for (int i = 0; i <= (X); ++i)
				memory[I + i] = V[i];
			// On the original interpreter, when the operation is done, I = I + X + 1.
			I += (X) + 1;
			pc += 2;
			break;
		case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address I					
			for (int i = 0; i <= (X); ++i)
				V[i] = memory[I + i];
			// On the original interpreter, when the operation is done, I = I + X + 1.
			I += (X) + 1;
			pc += 2;
			break;
		default:
			printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
		}
		break;
	default:
		printf("Unknown opcode: 0x%X\n", opcode);
	}

	if (delay_timer > 0)
		--delay_timer;
	if (sound_timer > 0)
	{
		if (sound_timer == 1)
			Beep(500, 100);
		--sound_timer;
	}
}


