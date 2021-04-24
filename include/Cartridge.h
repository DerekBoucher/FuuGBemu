#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "Defines.h"

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <wx/app.h>

class Cartridge
{
public:
    explicit Cartridge(FILE *input);
    Cartridge(const Cartridge &);
    ~Cartridge();

    bool attributes[NUM_ATTRIBUTES];

    uWORD currentRamBank;
    uWORD currentRomBank;
    uWORD romBankCount;
    uWORD ramBankCount;
    uWORD ramBankSize;

    uint64_t romSize;
    uint64_t ramSize;

    uBYTE *m_Rom;

    Cartridge &operator=(const Cartridge &other)
    {
        memcpy(this->attributes, other.attributes, NUM_ATTRIBUTES);
        this->currentRamBank = other.currentRamBank;
        this->currentRomBank = other.romBankCount;
        this->romBankCount = other.romBankCount;
        this->ramBankCount = other.ramBankCount;
        this->ramBankSize = other.ramBankSize;
        this->romSize = other.romSize;
        this->ramSize = other.ramSize;
        memcpy(this->m_Rom, other.m_Rom, other.romSize);
        return *this;
    }
};

#endif