#include "BoiBoy.h"

BoiBoy::BoiBoy()
{
	memory = new Memory();
	cpu = new CPUSharp();
}

void BoiBoy::Init()
{
	cpu->InitMem(memory);
}

int BoiBoy::Clock()
{
	int cycles = 0;

	cycles = cpu->Clock();

	return cycles;
}
