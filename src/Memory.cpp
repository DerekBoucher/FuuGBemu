#include "Memory.hpp"

Memory::Memory() { 

}

Memory::~Memory() {

}

void Memory::ReadRom(uBYTE data[MAX_CART_SIZE]) {
    memcpy(cartridge, data, MAX_CART_SIZE);

    // Set the rom/ram banks to 1 by default
    currentRamBank = 0x01;
    currentRomBank = 0x01;

    // Read the cart attributes from the header
    switch (cartridge[CART_HEADER_ATTRIBUTES]) {
    case 0x00:
        attributes[romOnly] = true;
        break;
    case 0x01:
        attributes[mbc1] = true;
        break;
    case 0x02:
        attributes[mbc1] = true;
        attributes[ram] = true;
        break;
    case 0x03:
        attributes[mbc1] = true;
        attributes[ram] = true;
        attributes[battery] = true;
        break;
    case 0x05:
        attributes[mbc2] = true;
        break;
    case 0x06:
        attributes[mbc2] = true;
        attributes[battery] = true;
        break;
    case 0x08:
        attributes[romOnly] = true;
        attributes[ram] = true;
        break;
    case 0x09:
        attributes[romOnly] = true;
        attributes[ram] = true;
        attributes[battery] = true;
        break;
    case 0x0B:
        printf("Unsupported cart type: %x\n", cartridge[CART_HEADER_ATTRIBUTES]);
        exit(EXIT_FAILURE);
        break;
    case 0x0C:
        printf("Unsupported cart type: %x\n", cartridge[CART_HEADER_ATTRIBUTES]);
        exit(EXIT_FAILURE);
        break;
    case 0x0D:
        printf("Unsupported cart type: %x\n", cartridge[CART_HEADER_ATTRIBUTES]);
        exit(EXIT_FAILURE);
        break;
    case 0x0F:
        attributes[mbc3] = true;
        attributes[timer] = true;
        attributes[battery] = true;
        break;
    case 0x10:
        attributes[mbc3] = true;
        attributes[timer] = true;
        attributes[battery] = true;
        attributes[ram] = true;
        break;
    case 0x11:
        attributes[mbc3] = true;
        break;
    case 0x12:
        attributes[mbc3] = true;
        attributes[ram] = true;
        break;
    case 0x13:
        attributes[mbc3] = true;
        attributes[ram] = true;
        attributes[battery] = true;
        break;
    case 0x15:
        attributes[mbc4] = true;
        break;
    case 0x16:
        attributes[mbc4] = true;
        attributes[ram] = true;
        break;
    case 0x17:
        attributes[mbc4] = true;
        attributes[ram] = true;
        attributes[battery] = true;
        break;
    case 0x19:
        attributes[mbc5] = true;
        break;
    case 0x1A:
        attributes[mbc5] = true;
        attributes[ram] = true;
        break;
    case 0x1B:
        attributes[mbc5] = true;
        attributes[ram] = true;
        attributes[battery] = true;
        break;
    case 0x1C:
        attributes[mbc5] = true;
        attributes[rumble] = true;
        break;
    case 0x1D:
        attributes[mbc5] = true;
        attributes[rumble] = true;
        attributes[ram] = true;
        break;
    case 0x1E:
        attributes[mbc5] = true;
        attributes[rumble] = true;
        attributes[ram] = true;
        attributes[battery] = true;
        break;
    case 0xFC:
        printf("Unsupported cart type: %x\n", cartridge[CART_HEADER_ATTRIBUTES]);
        exit(EXIT_FAILURE);
        break;
    case 0xFD:
        printf("Unsupported cart type: %x\n", cartridge[CART_HEADER_ATTRIBUTES]);
        exit(EXIT_FAILURE);
        break;
    case 0xFE:
        printf("Unsupported cart type: %x\n", cartridge[CART_HEADER_ATTRIBUTES]);
        exit(EXIT_FAILURE);
        break;
    case 0xFF:
        printf("Unsupported cart type: %x\n", cartridge[CART_HEADER_ATTRIBUTES]);
        exit(EXIT_FAILURE);
        break;
    default:
        break;
    }

    // Determine rom bank count and size
    switch (cartridge[CART_HEADER_ROMINFO]) {
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
        printf("unable to determine rom bank / size for cart: %x\n", cartridge[CART_HEADER_ROMINFO]);
        exit(EXIT_FAILURE);
        break;
    }

    // Determine ram bank count and size
    switch (cartridge[CART_HEADER_RAMINFO]) {
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
        printf("unable to determine ram banks / size for cartridge: %x\n", cartridge[CART_HEADER_RAMINFO]);
        exit(EXIT_FAILURE);
        break;
    }
}

void Memory::closeBootRom() {
    if (!bootRomClosed) {
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
            rom[addr] = data;
        }
    }
    else if ((addr >= 0xA000) && (addr < 0xC000) && !dmaTransferInProgress) // External RAM
    {
        translatedAddr = addr - 0xA000;
        if (attributes[ramEnabled])
        {
            if (attributes[romOnly])
            {
                rom[addr] = data;
            }
            else if (attributes[mbc1])
            {
                if (attributes[romRamMode])
                {
                    rom[translatedAddr + (0xA000 * currentRamBank)] = data;
                }
                else
                {
                    rom[addr] = data;
                }
            }
        }
    }
    else if ((addr >= 0xC000) && (addr < 0xD000) && !dmaTransferInProgress) // Work RAM 0
    {
        rom[addr] = data;
    }
    else if ((addr >= 0xD000) && (addr < 0xE000) && !dmaTransferInProgress) // Work RAM 1
    {
        rom[addr] = data;
    }
    else if ((addr >= 0xE000) && (addr < 0xFE00) && !dmaTransferInProgress) //Echo of Work RAM, typically not used
    {
        rom[addr] = data;
        rom[addr - 0x2000] = data;
    }
    else if ((addr >= 0xFE00) && (addr < 0xFEA0) && !dmaTransferInProgress) //OAM RAM
    {
        uBYTE mode = getStatMode();

        if (mode == 0 || mode == 1)
        {
            rom[addr] = data;
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
            data = (data & 0xF0) | (rom[addr] & 0x0F);
            rom[addr] = data;
        }
        else if (addr == 0xFF01) // Serial Transfer Data
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF02) // Serial Transfer Control Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF04) // Divider Register
        {
            rom[addr] = 0x00;
        }
        else if (addr == 0xFF05) // Timer Counter Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF06) // Timer Modulo Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF07) // Timer Controller Register
        {
            rom[addr] = data;
            switch (rom[addr] & 0x03)
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
            rom[addr] = data | 0xE0;
        }
        else if (addr == 0xFF10) // Channel 1 Sweep Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF11) // Channel 1 Sound length/wave pattern duty Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF12) // Channel 1 Volume Envelope Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF13) // Channel 1 Frequency lo Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF14) // Channel 1 Freqency hi Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF16) // Channel 2 Sound length/wave pattern duty Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF17) // Channel 2 Volume Envelope Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF18) // Channel 2 Frequency lo Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF19) // Channel 2 Freqency hi Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF1A) // Channel 3 Sound On/Off Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF1B) // Channel 3 Sound Length Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF1C) // Channel 3 Select Output Level Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF1D) // Channel 3 Frequency lo Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF1E) // Channel 3 Frequency hi Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF20) // Channel 4 Sound length/wave pattern duty Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF21) // Channel 4 Volume Envelope Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF22) // Channel 4 Polynomial Counter Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF23) // Channel 4 Counter/Consecutive Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF24) // Channel Control Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF25) // Selection of Sound Output Terminal
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF26) // Sound On/Off
        {
            data = (data & 0x80) | (rom[addr] & 0x7F); // Only bit 7 is writable
            rom[addr] = data;
        }
        else if ((addr >= 0xFF30) && (addr < 0xFF40)) // Wave Pattern RAM
        {
            if (rom[0xFF1A] & 0x80) // Only accessible if CH3 bit 7 is reset
            {
                rom[addr] = data;
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
            rom[addr] = data;
        }
        else if (addr == 0xFF41) // STAT Register
        {
            uBYTE temp = rom[addr] & 0x07;
            data |= 0x80;
            data = data & 0xF8;
            data |= temp;
            rom[addr] = data;
        }
        else if (addr == 0xFF42) // Scroll Y Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF43) // Scroll X Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF44) // LY Register
        {
            rom[addr] = 0;
        }
        else if (addr == 0xFF45) // LY Compare Register
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF46) // Request for dma transfer
        {
            dmaTransfer(data);
        }
        else if (addr == 0xFF47) // BG Palette Data
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF48) // Object Palette 0 Data
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF49) // Object Palette 1 Data
        {
            rom[addr] = data;
        }
        else if (addr == 0xFF50)
        {
            rom[addr] = data;
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
        rom[addr] = data;
    }
    else if ((addr == 0xFFFF) && !dmaTransferInProgress) // Interrupt Enable Register
    {
        rom[addr] = data;
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
            if (attributes[romRamMode])
            {
                if (romSize > 0x100000)
                {
                    switch (currentRomBank)
                    {
                    case 0x00:
                        return cartridge[addr];
                        break;
                    case 0x01:
                        return cartridge[addr * 0x20];
                        break;
                    case 0x02:
                        return cartridge[addr * 0x40];
                        break;
                    case 0x03:
                        return cartridge[addr * 0x60];
                        break;
                    default:
                        return cartridge[addr];
                        break;
                    }
                }
                else
                {
                    return cartridge[addr];
                }
            }
            else
            {
                return cartridge[addr];
            }
        }
    }
    else if ((addr >= 0x4000) && (addr < 0x8000) && !dmaTransferInProgress) // Cart ROM Bank n
    {
        translatedAddr = addr - 0x4000;
        return cartridge[translatedAddr + (0x4000 * currentRomBank)];
    }
    else if ((addr >= 0x8000) && (addr < 0xA000) && !dmaTransferInProgress) // Video RAM
    {
        uBYTE mode = getStatMode();

        if (mode == 3)
            return rom[addr];
        else
            return 0xFF;
    }
    else if ((addr >= 0xA000) && (addr < 0xC000) && !dmaTransferInProgress) // External RAM
    {
        translatedAddr = addr - 0xA000;
        if (attributes[ramEnabled])
        {
            if (attributes[romOnly])
            {
                return rom[addr];
            }
            if (attributes[mbc1])
            {
                if (attributes[romRamMode])
                {
                    return rom[translatedAddr + (0xA000 * currentRamBank)];
                }
                else
                {
                    return rom[addr];
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
        return rom[addr];
    }
    else if ((addr >= 0xD000) && (addr < 0xE000) && !dmaTransferInProgress) // Work RAM 1
    {
        return rom[addr];
    }
    else if ((addr >= 0xE000) && (addr < 0xFE00) && !dmaTransferInProgress) // Echo of Work RAM
    {
        return rom[addr];
    }
    else if ((addr >= 0xFE00) && (addr < 0xFEA0) && !dmaTransferInProgress) //OAM RAM
    {
        uBYTE mode = getStatMode();

        if (mode == 0 || mode == 1)
            return rom[addr];
        else
            return 0xFF;
    }
    else if ((addr >= 0xFEA0) && (addr < 0xFF00) && !dmaTransferInProgress) // Not Usable
    {
        return 0xFF;
    }
    else if ((addr >= 0xFF00) && (addr < 0xFF80) && !dmaTransferInProgress) // I/O Registers
    {
        return rom[addr];
    }
    else if ((addr >= 0xFF80) && (addr < 0xFFFE)) // HRAM
    {
        return rom[addr];
    }
    // Interrupt Enable Register 0xFFFF
    return rom[addr];
}

uBYTE Memory::DmaRead(uWORD addr)
{
    return rom[addr];
}

void Memory::DmaWrite(uWORD addr, uBYTE data)
{
    if (addr == 0xFF41)
    {
        data |= 0x80;
    }
    rom[addr] = data;
}

void Memory::UpdateTimers(int cycles)
{
    uBYTE TAC = rom[TAC_ADR];

    // Update the divider register
    dividerRegisterCounter += cycles;
    if (dividerRegisterCounter >= 256)
    {
        rom[DIV_ADR]++;
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
            if (rom[TIMA_ADR] == 0xFF)
            {
                rom[TIMA_ADR] = rom[TIM_MOD_ADR];
                RequestInterupt(TIMER_OVERFLOW_INT);
            }
            else
            {
                rom[TIMA_ADR]++;
            }
        }
    }
}

void Memory::changeRomBank(uWORD addr, uBYTE data)
{
    if (attributes[romOnly])
    {
        currentRomBank = 0x01;
        return;
    }

    if (attributes[mbc1])
    {
        currentRomBank = (data & 0x1F);

        if (currentRomBank > romBankCount)
            currentRomBank &= (romBankCount - 1);

        if (currentRomBank == 0x00 ||
            currentRomBank == 0x20 ||
            currentRomBank == 0x40 ||
            currentRomBank == 0x60)
            currentRomBank += 0x01;
    }
    else if (attributes[mbc2])
    {
        if ((addr & 0x10) == 0x10)
        {
            currentRomBank = data & 0x0F;

            if (currentRomBank == 0x00 ||
                currentRomBank == 0x20 ||
                currentRomBank == 0x40 ||
                currentRomBank == 0x60)
                currentRomBank += 0x01;
        }
    }
    else if (attributes[mbc3] || attributes[mbc5])
    {
        currentRomBank = data & 0x7F;

        if (currentRomBank == 0x00 ||
            currentRomBank == 0x20 ||
            currentRomBank == 0x40 ||
            currentRomBank == 0x60)
            currentRomBank += 0x01;
    }
}

void Memory::toggleRam(uWORD addr, uBYTE data)
{
    if (attributes[mbc1] ||
        attributes[mbc3] ||
        attributes[mbc5] ||
        attributes[romOnly])
    {
        if ((data & 0x0F) == 0x0A)
            attributes[ramEnabled] = true;
        else
            attributes[ramEnabled] = false;
    }
    else if (attributes[mbc2])
    {
        if ((addr & 0x10) == 0x00)
        {
            if ((data & 0x0F) == 0x0A)
                attributes[ramEnabled] = true;
            else
                attributes[ramEnabled] = false;
        }
    }
}

void Memory::changeMode(uBYTE data)
{
    if (attributes[mbc1])
    {
        if (data & 0x01)
        {
            attributes[romRamMode] = true;
        }
        else
        {
            attributes[romRamMode] = false;
        }
    }
}

void Memory::changeRamBank(uBYTE data)
{
    if (attributes[mbc1])
    {
        if (attributes[romRamMode])
        {
            currentRamBank = data & 0x03;
        }
        else
        {
            currentRamBank = 0x01;
            currentRomBank |= ((data & 0x03) << 5);
            if (currentRomBank > romBankCount)
                currentRomBank &= (romBankCount - 1);
        }
    }
}

void Memory::RequestInterupt(int code)
{
    uBYTE IF = rom[0xFF0F];

    switch (code)
    {
    case VBLANK_INT:
        IF |= 0x01;
        rom[0xFF0F] = IF;
        break;
    case LCDC_INT:
        IF |= 0x02;
        rom[0xFF0F] = IF;
        break;
    case TIMER_OVERFLOW_INT:
        IF |= 0x04;
        rom[0xFF0F] = IF;
        break;
    case SER_TRF_INT:
        IF |= 0x08;
        rom[0xFF0F] = IF;
        break;
    case CONTROL_INT:
        IF |= 0x10;
        rom[0xFF0F] = IF;
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
        rom[0xFE00 + i] = rom[(data << 8) | i];
    }
}

uBYTE Memory::getStatMode()
{
    return rom[0xFF41] & 0x03;
}
