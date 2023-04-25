#include "CPUSharp.h"
#include "Memory.h"

class BoiBoy
{
public:
	BoiBoy();

	void Init();

	int Clock();

	// Press and Unpress control

	// Cartridge
	// PPU
	// SPU
	// Memory?

private:

	Memory* memory;
	CPUSharp* cpu;
};

