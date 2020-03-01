#ifndef chip8_h
#define chip8_h


class chip8 {
private:
	//current instruction
	unsigned short opcode;

	/* Memory, contains fonts/ram/graphics and rom
	0x000 - 0x1FF chip8 interpreter (has font in emus)
	0x050 - 0x0A0 built in 4x5 pixel font (0-F)
	0x200 - 0xFFF Program ROM and work ram
	*/
	unsigned char memory[4096];

	/* registers
	15 8 bit regs named V0,V1...VE
	1 register used as 'carry flag'
	*/
	unsigned char V[16];

	/* pointer registers (3 Byte Length)
	I: Index register
	pc: program counter
	*/
	unsigned short I;
	unsigned short pc;

	/* Timer registers
	count at 60Hz, when set above zero, will count down to 0

	system will buzz when sound timer is == 0
	*/
	unsigned char delay_timer;
	unsigned char sound_timer;

	/* stack
	stores pc values for when jumps and subroutines are called
	holds 16 values

	sp:stack pointer indicates next free slot in the stack
	*/
	unsigned short stack[16];
	unsigned short sp;

	bool drawFlag = false;	

	
public:
	/* chip8 graphics
	are a monochrome panel of 64 x 32 pixels (2048)
	*/
	unsigned char gfx[64 * 32];
	
	/* Input state
	stores the value of the HEX keypad (0x0-0xF)

	Layout:
	1 2 3 C
	4 5 6 D
	7 8 9 E
	A 0 B F
	*/
	bool key[16] = {false};


	chip8();
	~chip8();
	void initialise();
	bool getDrawFlag();
	void resetDrawFlag();
	bool loadGame(const char*);
	void emulateCycle();
	void setKeys();
	void printState();
	void printGfx();

};
#endif
