#include "chip8.h"
#include <stdio.h>
#include <string>

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

int tickNum = 0;
bool waiting;
unsigned short pixel;

chip8::chip8() {
	printf("New chip8!\n");
}

chip8::~chip8() {

}

void chip8::initialise() {
	// Initialise register and memory once
	pc		= 0x200; // program counter starts at beginning of rom memory
	opcode  = 0; // reset opcode
	I		= 0; // reset index reg
	sp		= 0; // reset stack pointer

	// clear display
	for (int j = 0; j < 2048; ++j) {
		gfx[j] = 0;
	}
	// clear stack
	for (int j = 0; j < 16; ++j) {
		stack[j] = 0;
	}
	// clear registers V0-Vf
	for (int j = 0; j < 16; ++j) {
		V[j] = 0;
	}
	// clear memory
	for (int j = 0x200; j < 0xFFF; ++j) {
		memory[j] = 0;
	}

	// load fontset
	for (int j = 0; j < 80; ++j) {
		memory[j] = chip8_fontset[j];
	}

	// reset timers
	delay_timer = 0;
	sound_timer = 0;

}

bool chip8::loadGame(const char* romName) {
	// fopen


	FILE* rom = fopen(romName, "rb");
	if (rom == NULL) { 
		fputs("file error", stderr); 
		return false; 
	}	
	fseek(rom, 0, SEEK_END);
	long lSize = ftell(rom);
	rewind(rom);
	char *buffer = (char*)malloc(sizeof(char)*lSize);
	if (buffer == NULL) {
		fputs("mem error", stderr);
		return false;
	}
	size_t result = fread(buffer, 1, lSize, rom);
	if (result != lSize) {
		fputs("read err", stderr);
		return false;
	}
	if ((4096 - 512) > lSize) {
		for (int i = 0; i < lSize; ++i) {
			memory[i + 512] = buffer[i];
		}
	}
	else {
		printf("err: rom too big");
	}
	fclose(rom);
	free(buffer);
	return true;
}

void chip8::emulateCycle() {
	// Fetch opcode
	opcode = memory[pc] << 8 | memory[pc + 1];
	printf("tickNum: %d, pc: %X opcode: %X\n", ++tickNum, pc, opcode);
	//for(int x=0;x<16;++x){printf("%d", key[x]);}
	//printf("\n");

	// Decode opcode
	switch (opcode & 0xF000) {
	case 0x0000:
		switch (opcode & 0x000F) {
		case 0x0000: // 0x00E0: clears the screen
			for (int j = 0; j < 2048; ++j) {
				gfx[j] = 0;
				drawFlag = true;
				pc += 2;
			}
			break;

		case 0x000E: // 0x00EE: returns from subroutine
			--sp;
			pc = stack[sp];
			pc += 2;
			break;

		default:
			printf("Unkown opcode 0x0000: 0x%X\n", opcode);
		}
		break;

		// 0x1NNN: Jump to NNN
	case 0x1000:
		pc = opcode & 0x0FFF;
		break;

		// 0x2NNN: call subroutine at NNN
	case 0x2000:
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;


		// 0x3XNN: skip next instruction if VX == NN
	case 0x3000:
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
			pc += 4;
		} else {
			pc += 2;
		}
		break;

		// 0x4XNN: skip next instruction if VX != NN
	case 0x4000:
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
			pc += 4;
		} else {
			pc += 2;
		}
		break;

		// 0x5XY0: skip next instruction if VX == VY	
	case 0x5000:
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
			pc += 4;
		} else {
			pc += 2;
		}
		break;

		// 0x6XNN: set VX to NN
	case 0x6000:
		V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
		pc += 2;
		break;

		// 0x7XNN: Add NN to VX (carry flag not changed)
	case 0x7000:
		V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
		pc += 2;
		break;

		// 0x8XYZ: bitwise math with VX and VY, determined by Z
	case 0x8000:
		switch (opcode & 0x000F) {

			// 0x8XY0: set VX = VY
		case 0x0000:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			// 0x8XY1: set VX = VX OR VY
		case 0x0001:
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			// 0x8XY2: set VX = VX AND VY
		case 0x0002:
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			// 0x8XY3: set VX = VX XOR VY
		case 0x0003:
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			// 0x8XY4: Add VY to VX, setVF if carrying
		case 0x0004:
			if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0xF00) >> 8])){
				V[0xF] = 1;
			}
			else {
				V[0xF] = 0;
			}
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			// 0x8XY5: subtract VY from VX, set VF to 0 if borrow
		case 0x0005:
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
				V[0xF] = 0;
			}
			else {
				V[0xF] = 1;
			}
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			// 0x8XY6: stores VX LSB in VF, then shift VX r1
		case 0x0006:
			V[0xF] = (V[(opcode & 0x0F00) >> 8] & 0x1);
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;

			// 0x8XY7: VX = VY-VX, VF =0 if borrow, else 1
		case 0x0007:
			if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]){//borrow
				V[0xF] = 0;
			} else {
				V[0xF] = 1;
			}
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

			// 0x8XYE: store VX MSB in VF, shift VX l1
		case 0x000E:
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1; 
			pc += 2;
			break;

		default:
			printf("Unknown opcode 0x8XY0: 0x%X\n", opcode);
		}
		break;

		// 0x9XY0: skip next if VX != VY
	case 0x9000:
		(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) ? pc += 4 : pc += 2;//XXX ternary
		break;

		// 0xANNN: set I to address NNN
	case 0xA000:
		I = opcode & 0x0FFF;
		pc += 2;
		break;

		// 0xBNNN: jump to NNN plus Vo
	case 0xB000:
		pc = (opcode & 0x0FFF) + V[0];
		break;

		// 0xCXNN: set VX to (NN AND random number(0-255)) 
	case 0xC000:
		V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF) & (rand() % 0xFF);
		pc += 2;
		break;

		/* 0xDXYN: draw sprite at Vx,Vy, width of 8, height N
		each 8bit row is bit coded in mem location I
		I doesnt change
		VF set to 1 if pixels are turned off as sprite is drawn, else 0
		*/
	case 0xD000:
		pixel = 0;
		V[0xF] = 0;
		//printf("X val: %d", V[(opcode & 0x0F00) >> 8]);
		//printf("Y val: %d", V[(opcode & 0x00F0) >> 4]);
		for (int row = 0; row < (opcode & 0x000F); ++row) {
			pixel = memory[I + row];
			//printf("pixel value: %X\n", pixel);
			for (int col = 0; col < 8; ++col) {
				if ((pixel & (0x80 >> col)) != 0) {
					unsigned short pixelX = V[(opcode & 0x0F00) >> 8] + col;
					unsigned short pixelY = (row + V[(opcode & 0x00F0) >> 4]) * 64;
					unsigned short changedPixel = (pixelX + pixelY )%(64*32);
					if(gfx[changedPixel] == 1){
						V[0xF]=1;
					}
					gfx[changedPixel] ^= 1;
					//printf("Toggling pix: %d\n", changedPixel);
					//if (gfx[(((opcode & 0x0F00) >> 8) + col + ((((opcode & 0x00F0) >> 4) + col) * 64))] == 1){
					//	V[0xF] = 1;
					//	gfx[(((opcode & 0x0F00) >> 8) + col + ((((opcode & 0x00F0) >> 4) + col) * 64))] ^= 1;
					//}
				}
			}
		}

		drawFlag = true;
		pc += 2;
		break;

		// EXIJ
	case 0xE000:
		switch (opcode & 0x00FF) {
			// 0xEX9E: skip next if key in VX is pressed
		case 0x009E:
			(key[V[(opcode & 0xF00) >> 8]] != 0) ? pc += 4 : pc += 2;
			break;
			// 0xEXA1: skip next if key VX is not pressed
		case 0x00A1:
			(key[V[(opcode & 0xF00) >> 8]] == 0) ? pc += 4 : pc += 2;
			break;
		}
		break;

		// 0xFXIJ
	case 0xF000:
		switch (opcode & 0x00FF) {
			// 0xFX07: set VX to value of delay_timer
		case 0x0007:
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

			// 0xFX0A: wait for keypress, store in VX XXX need SDL input read
		case 0x000A:
			waiting = true;
			while (waiting) {
				for(int k = 0; k < 16; ++k){
					if(key[k] != 0){
						V[(opcode & 0x0F00)>>8] = 1;
						waiting = false;
					}
				}
			}
			break;

			// 0xFX15: set delay_timer to VX
		case 0x0015:
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

			// 0xFX18: set sound_timer to VX
		case 0x0018:
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

			// 0xFX1E: add VX to I, VF =1 if overflow
		case 0x001E:
			V[0xF] = (I + V[(opcode & 0x0F00) >> 8] > 0xFFF) ? 1 : 0;
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

			// 0xFX29: set I to location of sprite for char in VX XXX
		case 0x0029:
			I = V[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
			break;

			// 0xFX33: store BCD of VX in I+{0..2}
		case 0x0033:
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
			pc += 2;
			break;

			// 0xFX55: store V0 thru VX in I thru I+1+J I is unchanged?!XXX
		case 0x0055:
			for (int vNum = 0; vNum < (opcode & 0x0F00) >> 8; ++vNum) {
				memory[I + vNum] = V[vNum];
			}
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

			/* 0xFX65: fill V0 thru VX with memory values starting at I
			incrementing for each register
			*/
		case 0x0065:
			for (int vNum = 0; vNum <= ((opcode & 0x0F00) >> 8); ++vNum) {
				V[vNum] = memory[I + vNum];
			}
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;
		default:
			printf("Unkown opcode: 0x%X\n", opcode);
			
		}
		break;

	default:
		printf("Unkown opcode: 0x%X\n", opcode);

		}

		// Update timers
		if (delay_timer > 0){
			--delay_timer;
		}
		if (sound_timer > 0){
			if (sound_timer == 1){printf("beep!\n");}
			--sound_timer;
		}
}
bool chip8::getDrawFlag(){
	return drawFlag;
}
void chip8::resetDrawFlag(){
	drawFlag = false;
}
void chip8::setKeys() {

}

void chip8::printState(){
	printf("opcode: %X\n", opcode);
	for(int i =0;i<16;++i){
		printf("V[%d]: %d\n", i, V[i]);
	}
	printf("I: %d\n", I);
	printf("pc: %d\n", pc);
	for(int i =0;i<16;++i){
		printf("stack[%d]: %d\n", i, stack[i]);
	}
	printf("stack pointer: %d\n", sp);
	printf("drawFlag: %d\n=========\n\n", drawFlag);
}

void chip8::printGfx(){
	

}
