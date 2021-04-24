#include "Cartridge.h"

Cartridge::Cartridge(FILE *input)
{
    currentRomBank = 0x01;
    currentRamBank = 0x01;

    for (int i = 0; i < NUM_ATTRIBUTES; i++)
    {
        attributes[i] = false;
    }

    fseek(input, 0x147, SEEK_SET);

    switch (fgetc(input))
    {
    case 0x00:
        attributes[ROM_ONLY] = true;
        break;
    case 0x01:
        attributes[MBC1] = true;
        break;
    case 0x02:
        attributes[MBC1] = true;
        attributes[RAM] = true;
        break;
    case 0x03:
        attributes[MBC1] = true;
        attributes[RAM] = true;
        attributes[BATTERY] = true;
        break;
    case 0x05:
        attributes[MBC2] = true;
        break;
    case 0x06:
        attributes[MBC2] = true;
        attributes[BATTERY] = true;
        break;
    case 0x08:
        attributes[ROM_ONLY] = true;
        attributes[RAM] = true;
        break;
    case 0x09:
        attributes[ROM_ONLY] = true;
        attributes[RAM] = true;
        attributes[BATTERY] = true;
        break;
    case 0x0B:
        printf("Unsupported cart type: %x", fgetc(input));
        wxExit();
        break;
    case 0x0C:
        printf("Unsupported cart type: %x", fgetc(input));
        wxExit();
        break;
    case 0x0D:
        printf("Unsupported cart type: %x", fgetc(input));
        wxExit();
        break;
    case 0x0F:
        attributes[MBC3] = true;
        attributes[TIMER] = true;
        attributes[BATTERY] = true;
        break;
    case 0x10:
        attributes[MBC3] = true;
        attributes[TIMER] = true;
        attributes[BATTERY] = true;
        attributes[RAM] = true;
        break;
    case 0x11:
        attributes[MBC3] = true;
        break;
    case 0x12:
        attributes[MBC3] = true;
        attributes[RAM] = true;
        break;
    case 0x13:
        attributes[MBC3] = true;
        attributes[RAM] = true;
        attributes[BATTERY] = true;
        break;
    case 0x15:
        attributes[MBC4] = true;
        break;
    case 0x16:
        attributes[MBC4] = true;
        attributes[RAM] = true;
        break;
    case 0x17:
        attributes[MBC4] = true;
        attributes[RAM] = true;
        attributes[BATTERY] = true;
        break;
    case 0x19:
        attributes[MBC5] = true;
        break;
    case 0x1A:
        attributes[MBC5] = true;
        attributes[RAM] = true;
        break;
    case 0x1B:
        attributes[MBC5] = true;
        attributes[RAM] = true;
        attributes[BATTERY] = true;
        break;
    case 0x1C:
        attributes[MBC5] = true;
        attributes[RUMBLE] = true;
        break;
    case 0x1D:
        attributes[MBC5] = true;
        attributes[RUMBLE] = true;
        attributes[RAM] = true;
        break;
    case 0x1E:
        attributes[MBC5] = true;
        attributes[RUMBLE] = true;
        attributes[RAM] = true;
        attributes[BATTERY] = true;
        break;
    case 0xFC:
        printf("Unsupported cart type: %x", fgetc(input));
        wxExit();
        break;
    case 0xFD:
        printf("Unsupported cart type: %x", fgetc(input));
        wxExit();
        break;
    case 0xFE:
        printf("Unsupported cart type: %x", fgetc(input));
        wxExit();
        break;
    case 0xFF:
        printf("Unsupported cart type: %x", fgetc(input));
        wxExit();
        break;
    default:
        break;
    }

    fseek(input, 0x148, SEEK_SET);
    switch (fgetc(input))
    {
    case 0x00:
        romBankCount = 1;
        romSize = 0x8000;
        break;
    case 0x01:
        romBankCount = 4;
        romSize = 0x10000;
        break;
    case 0x02:
        romBankCount = 8;
        romSize = 0x20000;
        break;
    case 0x03:
        romBankCount = 16;
        romSize = 0x40000;
        break;
    case 0x04:
        romBankCount = 32;
        romSize = 0x80000;
        break;
    case 0x05:
        romBankCount = 64;
        romSize = 0x100000;
        break;
    case 0x06:
        romBankCount = 128;
        romSize = 0x200000;
        break;
    case 0x07:
        romBankCount = 256;
        romSize = 0x400000;
        break;
    case 0x08:
        romBankCount = 512;
        romSize = 0x800000;
        break;
    case 0x52:
        romBankCount = 72;
        romSize = 0x120000;
        break;
    case 0x53:
        romBankCount = 80;
        romSize = 0x133334;
        break;
    case 0x54:
        romBankCount = 96;
        romSize = 0x180000;
        break;
    default:
        wxExit();
        break;
    }

    fseek(input, 0x149, SEEK_SET);
    switch (fgetc(input))
    {
    case 0x00:
        ramBankSize = 0x800;
        ramBankCount = 1;
        break;
    case 0x01:
        ramBankSize = 0x800;
        ramBankCount = 1;
        break;
    case 0x02:
        ramBankSize = 0x2000;
        ramBankCount = 1;
        break;
    case 0x03:
        ramBankSize = 0x2000;
        ramBankCount = 4;
        break;
    case 0x04:
        ramBankSize = 0x2000;
        ramBankCount = 16;
        break;
    case 0x05:
        ramBankSize = 0x2000;
        ramBankCount = 8;
        break;
    default:
        wxExit();
        break;
    }

    ramSize = ramBankCount * ramBankSize;

    m_Rom = new uBYTE[romSize];
    fseek(input, 0x0, SEEK_SET);
    fread(m_Rom, 0x1, romSize, input);
}

Cartridge::Cartridge(const Cartridge &other)
    : currentRamBank(other.currentRamBank),
      currentRomBank(other.currentRomBank),
      romBankCount(other.romBankCount),
      ramBankCount(other.ramBankCount),
      ramBankSize(other.ramBankSize),
      romSize(other.romSize),
      ramSize(other.ramSize)
{
    m_Rom = new uBYTE[romSize];
    memcpy(m_Rom, other.m_Rom, romSize);
    memcpy(attributes, other.attributes, NUM_ATTRIBUTES);
}

Cartridge::~Cartridge()
{
    delete[] m_Rom;
}
