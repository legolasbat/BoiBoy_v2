#include "PPU.h"

#include "Memory.h"

PPU::PPU() {
	for (int i = 0; i < 0xA0; i++) {
		OAM[i] = 0;
	}

	for (int i = 0; i < 10; i++) {
		spritesIndex[i] = 0;
	}

	for (int i = 0; i < 0x1800; i++) {
		tileData[i] = 0;
	}

	for (int i = 0; i < 0x400; i++) {
		tileMap0[i] = 0;
	}

	for (int i = 0; i < 0x400; i++) {
		tileMap1[i] = 0;
	}

	for (int i = 0; i < 256 * 256 * 3; i++) {
		fullScreen[i] = 0;
	}

	for (int i = 0; i < 160 * 144 * 4; i++) {
		frameBufferA[i] = 0;
	}
}

void PPU::Clock(int cycles) {
	if ((LCDC & 0x80) != 0x80) {
		return;
	}
	//std::cout << (int)LYWindow << std::endl;
	for (int i = 0; i < cycles; i += 1) {
		totalCycles++;
		if (mode == 1) {
			scanlineCycles++;
			cnt--;
		}
		else {
			cnt--;
		}


		if (cnt < 0) {
			switch (mode) {
			case 0:
				LY++;
				CheckLY();
				copy = false;
				// H-Blank -> OAM Scan
				if (LY < 144) {
					mode = 2;
					spriteFetch = false;
					STAT &= 0xFC;
					STAT |= 0x02;
					if ((STAT & 0x20) == 0x20)
						memory->IFReg |= 0x02;
					cnt += 20;
					break;
				}

				// H-Blank -> V-Blank
				mode = 1;
				STAT &= 0xFC;
				STAT |= 0x01;
				memory->IFReg |= 1;
				if ((STAT & 0x10) == 0x10)
					memory->IFReg |= 0x02;
				scanlineCycles = -cnt;
				cnt += 1140;
				break;

				// V-Blank -> OAM Scan
			case 1:
				totalCycles = 0;
				frameComplete = true;
				mode = 2;
				spriteFetch = false;
				STAT &= 0xFC;
				STAT |= 0x02;
				if ((STAT & 0x20) == 0x20)
					memory->IFReg |= 0x02;
				scanlineCycles = 0;
				LY = 0;
				LYWindow = -1;
				CheckLY();
				cnt += 20;
				break;

				// OAM Scan -> Drawing
			case 2:
				if ((LCDC & 0x20) == 0x20 && LY >= WY && WX >= 0 && WX <= 166 && ((LCDC & 0x01) == 0x01))
					LYWindow++;

				mode = 3;
				STAT &= 0xFC;
				STAT |= 0x03;
				cntOffset = 10 * spriteCont;
				cnt += 43 + cntOffset;
				drew = false;
				break;

				// Drawing -> H-Blank
			case 3:
				mode = 0;
				STAT &= 0xFC;
				if ((STAT & 0x08) == 0x08)
					memory->IFReg |= 0x02;
				cnt += 94 - (43 + cntOffset);
				break;
			}
		}

		switch (mode) {
		case 0:
			STAT &= 0xFC;
			if (!copy) {
				CopyBuffer(LY);
				copy = true;
			}
			break;
		case 1:
			STAT &= 0xFC;
			STAT |= 0x01;
			if (scanlineCycles >= 114) {
				scanlineCycles -= 114;
				LY++;
			}
			break;
		case 2:
			STAT &= 0xFC;
			STAT |= 0x02;
			if (!spriteFetch) {
				GetSpIndex();
				spriteFetch = true;
			}
			break;
		case 3:
			STAT &= 0xFC;
			STAT |= 0x03;
			if (!drew) {
				if (BGEnable) {
					GetBGRow(LY);
				}
				if (WNEnable) {
					GetWinRow(LY);
				}
				if (SPEnable) {
					GetSpRow(LY);
				}
				drew = true;
			}
			break;
		}
	}
}

void PPU::Write(uint16_t add, uint8_t n)
{
	if (mode != 3) {
		if (add >= 0x8000 && add < 0x9800) {
			tileData[add - 0x8000] = n;
			return;
		}

		if (add >= 0x9800 && add < 0x9C00) {
			tileMap0[add - 0x9800] = n;
			return;
		}

		if (add >= 0x9C00 && add < 0xA000) {
			tileMap1[add - 0x9C00] = n;
			return;
		}
	}

	if ((mode != 2 && mode != 3) || memory->DMA) {
		if (add >= 0xFE00 && add < 0xFEA0) {
			OAM[add - 0xFE00] = n;
			return;
		}
	}

	if ((add >= 0xFF40 && add < 0xFF46) || (add > 0xFF46 && add <= 0xFF4B)) {
		if (add == 0xFF40) {
			WriteLCDC(n);
			return;
		}
		if (add == 0xFF41) {
			STAT = n | 0x80;
			return;
		}
		if (add == 0xFF42) {
			SCY = n;
			return;
		}
		if (add == 0xFF43) {
			SCX = n;
			return;
		}
		if (add == 0xFF44) {
			std::cout << "Trying to writting in LY" << std::endl;
			// Read only
			return;
		}
		if (add == 0xFF45) {
			LYC = n;
			return;
		}
		if (add == 0xFF47) {
			BGP = n;
			return;
		}
		if (add == 0xFF48) {
			OBP0 = n;
			return;
		}
		if (add == 0xFF49) {
			OBP1 = n;
			return;
		}
		if (add == 0xFF4A) {
			WY = n;
			return;
		}
		if (add == 0xFF4B) {
			WX = n;
			return;
		}
	}
}

uint8_t PPU::Read(uint16_t add)
{
	uint8_t value = 0;

	if (add >= 0x8000 && add < 0x9800) {
		value = tileData[add - 0x8000];
	}
	else if (add >= 0x9800 && add < 0x9C00) {
		value = tileMap0[add - 0x9800];
	}
	else if (add >= 0x9C00 && add < 0xA000) {
		value = tileMap1[add - 0x9C00];
	}
	else if (add >= 0xFE00 && add < 0xFEA0) {
		value = OAM[add - 0xFE00];
	}
	else if ((add >= 0xFF40 && add < 0xFF46) || (add > 0xFF46 && add <= 0xFF4B)) {
		if (add == 0xFF40) {
			value = LCDC;
		}
		else if (add == 0xFF41) {
			value = STAT;
		}
		else if (add == 0xFF42) {
			value = SCY;
		}
		else if (add == 0xFF43) {
			value = SCX;
		}
		else if (add == 0xFF44) {
			value = LY;
		}
		else if (add == 0xFF45) {
			value = LYC;
		}
		else if (add == 0xFF47) {
			value = BGP;
		}
		else
		if (add == 0xFF48) {
			value = OBP0;
		}
		else if (add == 0xFF49) {
			value = OBP1;
		}
		else if (add == 0xFF4A) {
			value = WY;
		}
		else if (add == 0xFF4B) {
			value = WX;
		}
	}

	return value;
}

void PPU::WriteLCDC(uint8_t n) {
	LCDC = n;
	if ((LCDC & 0x80) != 0x80) {
		LY = 0;
		LYWindow = -1;
		cnt = 40;
		STAT &= 0xFC;
		mode = 0;
	}
	if ((LCDC & 0x4) == 0x4) {
		spriteHeight = 16;
	}
	else {
		spriteHeight = 8;
	}
	// If BG is disable, redraw
	if ((LCDC & 0x01) != 0x01) {
		LCDC &= 0xDF;
		drew = false;
	}
}

void PPU::GetSpIndex() {
	spriteCont = 0;

	for (int i = 0; i < 0xA0; i += 4) {
		if (LY + 16 >= OAM[i] && LY + 16 < OAM[i] + spriteHeight) {
			if (spriteCont == 10) {
				break;
			}
			else {
				spritesIndex[spriteCont++] = i;
			}
		}
	}
}

void PPU::CheckLY() {
	if (LY == LYC) {
		STAT |= 0x04;
		if ((STAT & 0x40) == 0x40)
			memory->IFReg |= 0x02;
	}
	else {
		STAT &= ~0x04;
	}
}

uint8_t* PPU::GetBackground() {
	if ((LCDC & 0x80) != 0x80)
		return fullScreen;

	uint16_t dataDir = ((LCDC & 0x10) == 0x10) ? 0x8000 : 0x8800;

	uint16_t mapDir = ((LCDC & 0x08) == 0x08) ? 0x9C00 : 0x9800;

	for (int r = 0; r < 256; r++) {
		for (int c = 0; c < 256; c++) {

			uint8_t index = Read(mapDir + (c / 8 + (r / 8) * 32));

			int offset = index * 0x10 + (r % 8) * 2;

			if (dataDir == 0x8800) {
				offset = 0x800 + (int8_t)index * 0x10 + (r % 8 * 2);
			}

			uint8_t low = Read(dataDir + offset);
			uint8_t high = Read(dataDir + offset + 1);

			uint8_t color = ((low >> (7 - (c % 8))) & 1) + ((high >> (7 - (c % 8))) & 1) * 2;

			uint32_t pal = pallete[color & 0x3];

			fullScreen[(r * 256 * 4) + (c * 4)] = (pal >> 24) & 0xFF; // R
			fullScreen[(r * 256 * 4) + (c * 4) + 1] = (pal >> 16) & 0xFF; // G
			fullScreen[(r * 256 * 4) + (c * 4) + 2] = (pal >> 8) & 0xFF; // B
			fullScreen[(r * 256 * 4) + (c * 4) + 3] = pal & 0xFF; // A
		}
	}

	return fullScreen;
}

void PPU::GetBGRow(uint8_t r) {
	uint16_t mapDir = ((LCDC & 0x08) == 0x08) ? 0x9C00 : 0x9800;

	uint16_t dataDir = ((LCDC & 0x10) == 0x10) ? 0x8000 : 0x8800;

	for (int c = 0; c < 256; c++) {

		uint8_t offY = r + SCY;
		uint8_t offX = c + SCX;

		uint8_t index = Read(mapDir + (offX / 8 + (offY / 8) * 32));

		int offset = index * 0x10 + (offY % 8 * 2);

		if (dataDir == 0x8800) {
			offset = 0x800 + (int8_t)index * 0x10 + (offY % 8 * 2);
		}

		uint8_t low = Read(dataDir + offset);
		uint8_t high = Read(dataDir + offset + 1);

		uint8_t color = ((low >> (7 - (offX % 8))) & 1) + ((high >> (7 - (offX % 8))) & 1) * 2;

		uint32_t pal = pallete[(BGP >> (2 * color)) & 0x3];
		if ((LCDC & 0x01) != 0x01) {
			pal = pallete[BGP & 0x3];
		}

		fullScreen[(r * 256 * 4) + (c * 4)] = (pal >> 24) & 0xFF; // R
		fullScreen[(r * 256 * 4) + (c * 4) + 1] = (pal >> 16) & 0xFF; // G
		fullScreen[(r * 256 * 4) + (c * 4) + 2] = (pal >> 8) & 0xFF; // B
	}
}

void PPU::GetWinRow(uint8_t r) {
	if (((LCDC & 0x20) != 0x20 || r < WY) && ((LCDC & 0x01) == 0x01))
		return;

	uint16_t mapDir = ((LCDC & 0x40) == 0x40) ? 0x9C00 : 0x9800;

	uint16_t dataDir = ((LCDC & 0x10) == 0x10) ? 0x8000 : 0x8800;

	for (int c = 0; c < 256; c++) {

		if (c < (WX - 7))
			continue;

		uint8_t offY = LYWindow;// ((r + LYWindow) - WY);
		uint8_t offX = (c - (WX - 7));

		uint8_t index = Read(mapDir + (offX / 8 + (offY / 8) * 32));

		int offset = index * 0x10 + (offY % 8 * 2);

		if (dataDir == 0x8800) {
			offset = 0x800 + (int8_t)index * 0x10 + (offY % 8 * 2);
		}

		uint8_t low = Read(dataDir + offset);
		uint8_t high = Read(dataDir + offset + 1);

		uint8_t color = ((low >> (7 - (offX % 8))) & 1) + ((high >> (7 - (offX % 8))) & 1) * 2;

		uint32_t pal = pallete[(BGP >> (2 * color)) & 0x3];
		if ((LCDC & 0x01) != 0x01) {
			pal = pallete[BGP & 0x3];
		}

		fullScreen[(r * 256 * 4) + (c * 4)] = (pal >> 24) & 0xFF; // R
		fullScreen[(r * 256 * 4) + (c * 4) + 1] = (pal >> 16) & 0xFF; // G
		fullScreen[(r * 256 * 4) + (c * 4) + 2] = (pal >> 8) & 0xFF; // B
	}
}

void PPU::GetSpRow(uint8_t r) {
	if ((LCDC & 0x02) != 0x02)
		return;

	uint16_t dataDir = 0x8000;

	uint8_t prevPosX = -1;

	for (int i = 0; i < spriteCont; i++) {
		uint8_t posY = OAM[spritesIndex[i]];
		uint8_t posX = OAM[spritesIndex[i] + 1];
		uint8_t index = OAM[spritesIndex[i] + 2];
		uint8_t attr = OAM[spritesIndex[i] + 3];

		// Check overlapping sprites
		if (prevPosX == posX) {
			prevPosX = posX;
			continue;
		}
		prevPosX = posX;

		// When 8x16, bit 0 in index is ignored
		if (spriteHeight == 16) {
			index &= 0xFE;
		}

		int offset = 0;

		if ((attr & 0x40) == 0x40) {
			offset = index * 0x10 + ((posY + 16 - r - 1) % spriteHeight * 2);
		}
		else {
			offset = index * 0x10 + ((r - posY + 16) % spriteHeight * 2);
		}

		uint8_t low = Read(dataDir + offset);
		uint8_t high = Read(dataDir + offset + 1);

		uint8_t color = 0;

		uint32_t BGpal = pallete[BGP & 0x3];

		for (int c = 0; c < 8; c++) {

			if ((attr & 0x20) == 0x20) {
				color = ((low >> (c % 8)) & 1) + ((high >> (c % 8)) & 1) * 2;
			}
			else {
				color = ((low >> (7 - (c % 8))) & 1) + ((high >> (7 - (c % 8))) & 1) * 2;
			}

			uint8_t spritePal = ((attr & 0x10) == 0x10) ? OBP1 : OBP0;

			uint32_t pal = pallete[(spritePal >> (2 * color)) & 0x3];

			int fPosX = (c + posX) - 8;
			if (fPosX < 0 || color == 0)
				continue;

			if ((attr & 0x80) == 0x80) {
				uint32_t currentBGpal = (uint32_t)fullScreen[(r * 256 * 4) + (fPosX * 4)] << 24; // R
				currentBGpal |= (uint32_t)fullScreen[(r * 256 * 4) + (fPosX * 4) + 1] << 16; // G
				currentBGpal |= (uint32_t)fullScreen[(r * 256 * 4) + (fPosX * 4) + 2] << 8; // B
				currentBGpal |= 0xFF;

				if (BGpal == currentBGpal) {
					fullScreen[(r * 256 * 4) + (fPosX * 4)] = (pal >> 24) & 0xFF; // R
					fullScreen[(r * 256 * 4) + (fPosX * 4) + 1] = (pal >> 16) & 0xFF; // G
					fullScreen[(r * 256 * 4) + (fPosX * 4) + 2] = (pal >> 8) & 0xFF; // B
				}
			}
			else {
				fullScreen[(r * 256 * 4) + (fPosX * 4)] = (pal >> 24) & 0xFF; // R
				fullScreen[(r * 256 * 4) + (fPosX * 4) + 1] = (pal >> 16) & 0xFF; // G
				fullScreen[(r * 256 * 4) + (fPosX * 4) + 2] = (pal >> 8) & 0xFF; // B
			}
		}
	}
}

uint8_t* PPU::GetScreen() {
	return frameBufferA;
}

int PPU::GetPitch()
{
	return WIDTH * sizeof(uint8_t) * 4;
}

void PPU::ToggleLayer(uint8_t index)
{
	if (index == 0) {
		BGEnable = !BGEnable;
	}
	else if (index == 1) {
		WNEnable = !WNEnable;
	}
	else {
		SPEnable = !SPEnable;
	}
}

void PPU::CopyBuffer(uint8_t r) {
	if ((LCDC & 0x80) != 0x80)
		return;

	for (int c = 0; c < 160; c++) {

		int yOffA = (r * 256 * 4);
		int xOffA = (c * 4);

		frameBufferA[(r * 160 * 4) + (c * 4)] = fullScreen[yOffA + xOffA];			// R
		frameBufferA[(r * 160 * 4) + (c * 4) + 1] = fullScreen[yOffA + xOffA + 1];	// G
		frameBufferA[(r * 160 * 4) + (c * 4) + 2] = fullScreen[yOffA + xOffA + 2];	// B
		frameBufferA[(r * 160 * 4) + (c * 4) + 3] = 0xFF;							// A
	}
	
}
