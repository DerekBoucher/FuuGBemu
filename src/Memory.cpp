#include "Memory.h"

Memory::Memory(Cartridge *c)
{
    cart = c;
    memory = new uBYTE[0x10000];
    memset(memory, 0x0, 0x10000);

    dmaTransferInProgress = false;
    bootRomClosed = false;
    timerCounter = 1024;
    dmaCyclesCompleted = 0;
    translatedAddr = 0x0000;
    dividerRegisterCounter = 0;
}

Memory::Memory(const Memory &other)
    : timerCounter(other.timerCounter),
      dmaCyclesCompleted(other.dmaCyclesCompleted),
      bootRomClosed(other.bootRomClosed),
      dmaTransferInProgress(other.dmaTransferInProgress),
      translatedAddr(other.translatedAddr),
      cart(other.cart),
      dividerRegisterCounter(other.dividerRegisterCounter)
{
    memory = new uBYTE[0x10000];
    memcpy(memory, other.memory, 0x10000);
}

Memory::~Memory()
{
    delete[] memory;
    delete cart;
}

void Memory::closeBootRom()
{
    if (!bootRomClosed)
    {
        bootRomClosed = true;
    }
}

void Memory::Write(uWORD addr, uBYTE data)
{
    // Writing to memory takes 4 cycles
    UpdateTimers(4);

    if ((addr < 0x8000) && !dmaTransferInProgress) // Cart ROM
    {
        if (addr < 0x2000)
        {
            toggleRam(addr, data);
        }
        else if (addr < 0x4000)
        {
            changeRomBank(addr, data);
        }
        else if (addr < 0x6000)
        {
            changeRamBank(data);
        }
        else
        {
            changeMode(data);
        }
    }
    else if ((addr >= 0x8000) && (addr < 0xA000) && !dmaTransferInProgress) // Video RAM
    {
        uBYTE mode = getStatMode();

        if (mode == 0 || mode == 1 || mode == 2)
        {
            memory[addr] = data;
        }
    }
    else if ((addr >= 0xA000) && (addr < 0xC000) && !dmaTransferInProgress) // External RAM
    {
        translatedAddr = addr - 0xA000;
        if (cart->attributes[RAM_ENABLED])
        {
            if (cart->attributes[ROM_ONLY])
            {
                cart->m_Rom[addr] = data;
            }
            else if (cart->attributes[MBC1])
            {
                if (cart->attributes[ROM_RAM_MODE])
                {
                    cart->m_Rom[translatedAddr + (0xA000 * cart->currentRamBank)] = data;
                }
                else
                {
                    cart->m_Rom[addr] = data;
                }
            }
        }
    }
    else if ((addr >= 0xC000) && (addr < 0xD000) && !dmaTransferInProgress) // Work RAM 0
    {
        memory[addr] = data;
    }
    else if ((addr >= 0xD000) && (addr < 0xE000) && !dmaTransferInProgress) // Work RAM 1
    {
        memory[addr] = data;
    }
    else if ((addr >= 0xE000) && (addr < 0xFE00) && !dmaTransferInProgress) //Echo of Work RAM, typically not used
    {
        memory[addr] = data;
        memory[addr - 0x2000] = data;
    }
    else if ((addr >= 0xFE00) && (addr < 0xFEA0) && !dmaTransferInProgress) //OAM RAM
    {
        uBYTE mode = getStatMode();

        if (mode == 0 || mode == 1)
        {
            memory[addr] = data;
        }
    }
    else if ((addr >= 0xFEA0) && (addr < 0xFF00) && !dmaTransferInProgress) // Not Usable
    {
        return;
    }
    else if ((addr >= 0xFF00) && (addr < 0xFF80) && !dmaTransferInProgress) // I/O Registers
    {
        if (addr == 0xFF00) // Joypad register
        {
            data = (data & 0xF0) | (memory[addr] & 0x0F);
            memory[addr] = data;
        }
        else if (addr == 0xFF01) // Serial Transfer Data
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF02) // Serial Transfer Control Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF04) // Divider Register
        {
            memory[addr] = 0x00;
        }
        else if (addr == 0xFF05) // Timer Counter Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF06) // Timer Modulo Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF07) // Timer Controller Register
        {
            memory[addr] = data;
            switch (memory[addr] & 0x03)
            {
            case 0:
                this->timerCounter = 1024;
                break;
            case 1:
                this->timerCounter = 16;
                break;
            case 2:
                this->timerCounter = 64;
                break;
            case 3:
                this->timerCounter = 256;
                break;
            }
        }
        else if (addr == 0xFF0F) // Interrupt Flag Register
        {
            memory[addr] = data | 0xE0;
        }
        else if (addr == 0xFF10) // Channel 1 Sweep Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF11) // Channel 1 Sound length/wave pattern duty Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF12) // Channel 1 Volume Envelope Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF13) // Channel 1 Frequency lo Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF14) // Channel 1 Freqency hi Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF16) // Channel 2 Sound length/wave pattern duty Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF17) // Channel 2 Volume Envelope Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF18) // Channel 2 Frequency lo Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF19) // Channel 2 Freqency hi Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF1A) // Channel 3 Sound On/Off Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF1B) // Channel 3 Sound Length Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF1C) // Channel 3 Select Output Level Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF1D) // Channel 3 Frequency lo Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF1E) // Channel 3 Frequency hi Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF20) // Channel 4 Sound length/wave pattern duty Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF21) // Channel 4 Volume Envelope Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF22) // Channel 4 Polynomial Counter Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF23) // Channel 4 Counter/Consecutive Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF24) // Channel Control Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF25) // Selection of Sound Output Terminal
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF26) // Sound On/Off
        {
            data = (data & 0x80) | (memory[addr] & 0x7F); // Only bit 7 is writable
            memory[addr] = data;
        }
        else if ((addr >= 0xFF30) && (addr < 0xFF40)) // Wave Pattern RAM
        {
            if (memory[0xFF1A] & 0x80) // Only accessible if CH3 bit 7 is reset
            {
                memory[addr] = data;
            }
        }
        else if (addr == 0xFF40) // LCDC Register
        {
            uBYTE mode = getStatMode();
            if (!(data & 0x80))
            {
                if (mode != 1)
                {
                    data |= 0x80;
                }
            }
            memory[addr] = data;
        }
        else if (addr == 0xFF41) // STAT Register
        {
            uBYTE temp = memory[addr] & 0x07;
            data |= 0x80;
            data = data & 0xF8;
            data |= temp;
            memory[addr] = data;
        }
        else if (addr == 0xFF42) // Scroll Y Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF43) // Scroll X Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF44) // LY Register
        {
            memory[addr] = 0;
        }
        else if (addr == 0xFF45) // LY Compare Register
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF46) // Request for dma transfer
        {
            dmaTransfer(data);
        }
        else if (addr == 0xFF47) // BG Palette Data
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF48) // Object Palette 0 Data
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF49) // Object Palette 1 Data
        {
            memory[addr] = data;
        }
        else if (addr == 0xFF50)
        {
            memory[addr] = data;
            closeBootRom();
        }
        else if (addr == 0xFF51) // New DMA source, high
        {
            // Not Used in DMG
        }
        else if (addr == 0xFF52) // New DMA source, low
        {
            // Not Used in DMG
        }
        else if (addr == 0xFF53) // New DMA dest, high
        {
            // Not Used in DMG
        }
        else if (addr == 0xFF54) // New DMA dest, lo
        {
            // Not used in DMG
        }
        else if (addr == 0xFF55) // New DMA length/Mode/Start
        {
            // Not used in DMG
        }
        else if (addr == 0xFF56) // Infrared Communications Port
        {
            // Not used in DMG
        }
    }
    else if ((addr >= 0xFF80) && (addr < 0xFFFE)) // HRAM
    {
        memory[addr] = data;
    }
    else if ((addr == 0xFFFF) && !dmaTransferInProgress) // Interrupt Enable Register
    {
        memory[addr] = data;
    }
}

uBYTE Memory::Read(uWORD addr, bool debugRead)
{
    // Reading from memory takes 4 cycles
    if (!debugRead)
        UpdateTimers(4);

    if ((addr < 0x4000) && !dmaTransferInProgress) // Cart ROM Bank 0
    {
        if (!bootRomClosed && (addr < 0x100))
        {
            return bootRom[addr];
        }
        else
        {
            if (cart->attributes[ROM_RAM_MODE])
            {
                if (cart->romSize > 0x100000)
                {
                    switch (cart->currentRamBank)
                    {
                    case 0x00:
                        return cart->m_Rom[addr];
                        break;
                    case 0x01:
                        return cart->m_Rom[addr * 0x20];
                        break;
                    case 0x02:
                        return cart->m_Rom[addr * 0x40];
                        break;
                    case 0x03:
                        return cart->m_Rom[addr * 0x60];
                        break;
                    default:
                        return cart->m_Rom[addr];
                        break;
                    }
                }
                else
                {
                    return cart->m_Rom[addr];
                }
            }
            else
            {
                return cart->m_Rom[addr];
            }
        }
    }
    else if ((addr >= 0x4000) && (addr < 0x8000) && !dmaTransferInProgress) // Cart ROM Bank n
    {
        translatedAddr = addr - 0x4000;
        return cart->m_Rom[translatedAddr + (0x4000 * cart->currentRomBank)];
    }
    else if ((addr >= 0x8000) && (addr < 0xA000) && !dmaTransferInProgress) // Video RAM
    {
        uBYTE mode = getStatMode();

        if (mode == 3)
            return memory[addr];
        else
            return 0xFF;
    }
    else if ((addr >= 0xA000) && (addr < 0xC000) && !dmaTransferInProgress) // External RAM
    {
        translatedAddr = addr - 0xA000;
        if (cart->attributes[RAM_ENABLED])
        {
            if (cart->attributes[ROM_ONLY])
            {
                return cart->m_Rom[addr];
            }
            if (cart->attributes[MBC1])
            {
                if (cart->attributes[ROM_RAM_MODE])
                {
                    return cart->m_Rom[translatedAddr + (0xA000 * cart->currentRamBank)];
                }
                else
                {
                    return cart->m_Rom[addr];
                }
            }
        }
        else
        {
            return 0xFF;
        }
    }
    else if ((addr >= 0xC000) && (addr < 0xD000) && !dmaTransferInProgress) // Work RAM 0
    {
        return memory[addr];
    }
    else if ((addr >= 0xD000) && (addr < 0xE000) && !dmaTransferInProgress) // Work RAM 1
    {
        return memory[addr];
    }
    else if ((addr >= 0xE000) && (addr < 0xFE00) && !dmaTransferInProgress) // Echo of Work RAM
    {
        return memory[addr];
    }
    else if ((addr >= 0xFE00) && (addr < 0xFEA0) && !dmaTransferInProgress) //OAM RAM
    {
        uBYTE mode = getStatMode();

        if (mode == 0 || mode == 1)
            return memory[addr];
        else
            return 0xFF;
    }
    else if ((addr >= 0xFEA0) && (addr < 0xFF00) && !dmaTransferInProgress) // Not Usable
    {
        return 0xFF;
    }
    else if ((addr >= 0xFF00) && (addr < 0xFF80) && !dmaTransferInProgress) // I/O Registers
    {
        return memory[addr];
    }
    else if ((addr >= 0xFF80) && (addr < 0xFFFE)) // HRAM
    {
        return memory[addr];
    }
    // Interrupt Enable Register 0xFFFF
    return memory[addr];
}

uBYTE Memory::DmaRead(uWORD addr)
{
    return memory[addr];
}

void Memory::DmaWrite(uWORD addr, uBYTE data)
{
    if (addr == 0xFF41)
    {
        data |= 0x80;
    }
    memory[addr] = data;
}

void Memory::UpdateTimers(int cycles)
{
    uBYTE TAC = memory[0xFF07];

    // Update the divider register
    dividerRegisterCounter += cycles;
    if (dividerRegisterCounter >= 256)
    {
        memory[0xFF04]++;
        dividerRegisterCounter -= 256;
    }

    if (TAC & (1 << 2)) // Check if clock is enabled
    {
        timerCounter -= cycles;

        while (timerCounter <= 0)
        {
            int remainder = timerCounter;

            uBYTE frequency = (TAC & 0x03);
            switch (frequency)
            {
            case 0:
                timerCounter = 1024;
                break;
            case 1:
                timerCounter = 16;
                break;
            case 2:
                timerCounter = 64;
                break;
            case 3:
                timerCounter = 256;
                break;
            }

            timerCounter += remainder;

            // Timer Overflow
            if (memory[0xFF05] == 0xFF)
            {
                memory[0xFF05] = memory[0xFF06];
                RequestInterupt(2);
            }
            else
            {
                memory[0xFF05]++;
            }
        }
    }
}

void Memory::changeRomBank(uWORD addr, uBYTE data)
{
    if (cart->attributes[ROM_ONLY])
    {
        cart->currentRomBank = 0x01;
        return;
    }

    if (cart->attributes[MBC1])
    {
        cart->currentRomBank = (data & 0x1F);

        if (cart->currentRomBank > cart->romBankCount)
            cart->currentRomBank &= (cart->romBankCount - 1);

        if (cart->currentRomBank == 0x00 ||
            cart->currentRomBank == 0x20 ||
            cart->currentRomBank == 0x40 ||
            cart->currentRomBank == 0x60)
            cart->currentRomBank += 0x01;
    }
    else if (cart->attributes[MBC2])
    {
        if ((addr & 0x10) == 0x10)
        {
            cart->currentRomBank = data & 0x0F;

            if (cart->currentRomBank == 0x00 ||
                cart->currentRomBank == 0x20 ||
                cart->currentRomBank == 0x40 ||
                cart->currentRomBank == 0x60)
                cart->currentRomBank += 0x01;
        }
    }
    else if (cart->attributes[MBC3] || cart->attributes[MBC5])
    {
        cart->currentRomBank = data & 0x7F;

        if (cart->currentRomBank == 0x00 ||
            cart->currentRomBank == 0x20 ||
            cart->currentRomBank == 0x40 ||
            cart->currentRomBank == 0x60)
            cart->currentRomBank += 0x01;
    }
}

void Memory::toggleRam(uWORD addr, uBYTE data)
{
    if (cart->attributes[MBC1] ||
        cart->attributes[MBC3] ||
        cart->attributes[MBC5] ||
        cart->attributes[ROM_ONLY])
    {
        if ((data & 0x0F) == 0x0A)
            cart->attributes[RAM_ENABLED] = true;
        else
            cart->attributes[RAM_ENABLED] = false;
    }
    else if (cart->attributes[MBC2])
    {
        if ((addr & 0x10) == 0x00)
        {
            if ((data & 0x0F) == 0x0A)
                cart->attributes[RAM_ENABLED] = true;
            else
                cart->attributes[RAM_ENABLED] = false;
        }
    }
}

void Memory::changeMode(uBYTE data)
{
    if (cart->attributes[MBC1])
    {
        if (data & 0x01)
        {
            cart->attributes[ROM_RAM_MODE] = true;
        }
        else
        {
            cart->attributes[ROM_RAM_MODE] = false;
        }
    }
}

void Memory::changeRamBank(uBYTE data)
{
    if (cart->attributes[MBC1])
    {
        if (cart->attributes[ROM_RAM_MODE])
        {
            cart->currentRamBank = data & 0x03;
        }
        else
        {
            cart->currentRamBank = 0x01;
            cart->currentRomBank |= ((data & 0x03) << 5);
            if (cart->currentRomBank > cart->romBankCount)
                cart->currentRomBank &= (cart->romBankCount - 1);
        }
    }
}

void Memory::RequestInterupt(int code)
{
    uBYTE IF = memory[0xFF0F];

    switch (code)
    {
    case 0:
        IF |= 0x01;
        memory[0xFF0F] = IF;
        break;
    case 1:
        IF |= 0x02;
        memory[0xFF0F] = IF;
        break;
    case 2:
        IF |= 0x04;
        memory[0xFF0F] = IF;
        break;
    case 3:
        IF |= 0x08;
        memory[0xFF0F] = IF;
        break;
    case 4:
        IF |= 0x10;
        memory[0xFF0F] = IF;
        break;
    }
}

void Memory::UpdateDmaCycles(int cyclesToAdd)
{
    if (dmaTransferInProgress)
    {
        dmaCyclesCompleted += cyclesToAdd;
        if (dmaCyclesCompleted >= 160)
        {
            dmaTransferInProgress = false;
        }
    }
    else
    {
        dmaCyclesCompleted = 0;
    }
}

void Memory::dmaTransfer(uBYTE data)
{
    dmaTransferInProgress = true;

    // Check if source prefix is outside of allowed source addresses
    if (data > 0xF1)
    {
        data = 0xF1;
    }

    // Begin Transfer
    for (uBYTE i = 0; i < 0xA0; i++)
    {
        memory[0xFE00 + i] = memory[(data << 8) | i];
    }
}

uBYTE Memory::getStatMode()
{
    return memory[0xFF41] & 0x03;
}
