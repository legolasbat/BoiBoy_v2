#include "CartLoader.h"

#include <fstream>
#include <iostream>
#include <vector>

Cartridge* CartLoader::LoadCartridge(const char* game) const
{
	std::vector<uint8_t> ROM;
	std::vector<uint8_t> RAM;

	std::ifstream ifs;
	ifs.open(game, std::ifstream::binary);
	if (ifs.is_open()) {
		// Title
		ifs.seekg(0x134);
		char t;
		ifs.read(&t, sizeof(char));
		while (t != 0) {
			std::cout << t;
			ifs.read(&t, sizeof(char));
		}
		std::cout << std::endl;

		// Cartridge Type
		uint8_t type = 0;
		ifs.seekg(0x147);
		ifs.read((char*)&type, sizeof(uint8_t));
		if (type == 0x00) {
			// MBC0
			std::cout << "ROM ONLY" << std::endl;
		}
		else if (type == 0x01) {
			std::cout << "MBC1" << std::endl;
		}
		else if (type == 0x02) {
			std::cout << "MBC1 + RAM" << std::endl;
		}
		else if (type == 0x03) {
			std::cout << "MBC1 + RAM + BATTERY" << std::endl;
		}
		else if (type == 0x05) {
			std::cout << "MBC2" << std::endl;
		}
		else if (type == 0x06) {
			std::cout << "MBC2 + BATTERY" << std::endl;
		}
		else {
			std::cout << "CARTRIDGE TYPE NOT HANDLED" << std::endl;
		}

		// ROM Size
		uint8_t RomSize;
		ifs.read((char*)&RomSize, sizeof(uint8_t));
		uint8_t nRomBanks = 0;
		nRomBanks = (0x8000 << RomSize) / 0x4000;
		if (nRomBanks == 2) {
			std::cout << "No ROM banking" << std::endl;
		}
		else if (nRomBanks <= 0x7F) {
			std::cout << (int)nRomBanks << " ROM Banks" << std::endl;
		}
		else {
			std::cout << "CARTRIDGE ROM SIZE NOT HANDLED" << std::endl;
		}

		// RAM Size
		uint8_t RamSize;
		ifs.read((char*)&RamSize, sizeof(uint8_t));
		uint8_t nRamBanks = 0;
		if (RamSize == 0x00) {
			std::cout << "No RAM" << std::endl;
			nRamBanks = 0;
		}
		else if (RamSize == 0x01) {
			std::cout << "Unused RAM" << std::endl;
			nRamBanks = 0;
		}
		else if (RamSize == 0x02) {
			std::cout << "1 Bank RAM" << std::endl;
			nRamBanks = 1;
		}
		else if (RamSize == 0x03) {
			std::cout << "4 Bank RAM" << std::endl;
			nRamBanks = 4;
		}
		else if (RamSize == 0x04) {
			std::cout << "16 Bank RAM" << std::endl;
			nRamBanks = 16;
		}
		else if (RamSize == 0x05) {
			std::cout << "8 Bank RAM" << std::endl;
			nRamBanks = 8;
		}
		else {
			std::cout << "CARTRIDGE RAM SIZE NOT HANDLED" << std::endl;
		}

		ROM.resize(nRomBanks * 0x4000);
		RAM.resize(nRamBanks * 0x8000);

		ifs.seekg(0);
		ifs.read((char*)ROM.data(), ROM.size());
	}
	ifs.close();

	if (ROM.size() != 0) {
		//return new MBC0();
	}

    return nullptr;
}
