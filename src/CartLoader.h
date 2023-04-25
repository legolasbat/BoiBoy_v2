//#include "carts/Cartridge.h"
//#include "carts/MBC0.h"


#include "Cartridge.h"
class CartLoader
{
public:
	Cartridge* LoadCartridge (const char* game) const;
};

