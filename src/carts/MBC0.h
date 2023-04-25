#include "Cartridge.h"

class MBC0 :
    public Cartridge
{
public:

	MBC0();

	void Write(uint16_t add, uint8_t n);
	uint8_t Read(uint16_t add);
};

