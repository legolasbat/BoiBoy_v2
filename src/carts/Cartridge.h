#include <cstdint>
#include <vector>

class Cartridge
{
public:

	Cartridge();

	void virtual Write(uint16_t add, uint8_t n) = 0;
	uint8_t virtual Read(uint16_t add) = 0;

private:

	std::vector<uint8_t> ROM;
	std::vector<uint8_t> RAM;
};

