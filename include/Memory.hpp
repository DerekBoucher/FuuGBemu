#ifndef MEMORY_H
#define MEMORY_H

#define NATIVE_ROM_SIZE 0x10000
#define MAX_CART_SIZE 0x200000
#define BOOTROM_SIZE 0x100
#define NUM_ATTRIBUTES 13
#define CART_HEADER_ATTRIBUTES 0x147
#define CART_HEADER_ROMINFO 0x148
#define CART_HEADER_RAMINFO 0x149

typedef unsigned char uBYTE;
typedef unsigned short uWORD;

#include <iostream>
#include <map>
#include <string.h>
#include <memory>

#define VBLANK_INT 0
#define LCDC_INT 1
#define TIMER_OVERFLOW_INT 2
#define SER_TRF_INT 3
#define CONTROL_INT 4

#define LCDC_ADR 0xFF40
#define STAT_ADR 0xFF41
#define LY_ADR 0xFF44
#define OAM_ADR 0xFE00
#define SCR_X_ADR 0xFF43
#define SCR_Y_ADR 0xFF42
#define LYC_ADR 0xFF45
#define TAC_ADR 0xFF07
#define DIV_ADR 0xFF04
#define TIMA_ADR 0xFF05
#define TIM_MOD_ADR 0xFF06
#define IF_ADR 0xFF0F
#define JOYPAD_INPUT_REG 0xFF00

using namespace std;

class Memory {

    friend class Gameboy;
    friend class Apu;
    friend class SideNav;

public:
    Memory();
    ~Memory();

    int timerCounter;

    void Write(uWORD, uBYTE);
    void DmaWrite(uWORD, uBYTE);
    void RequestInterupt(int);
    void UpdateDmaCycles(int);
    void UpdateTimers(int);
    void ReadRom(uBYTE* data);
    uBYTE Read(uWORD, bool = false);
    uBYTE DmaRead(uWORD);
    void SetPostBootRomState();

    bool RequiresCh1LengthReload();
    bool RequiresCh2LengthReload();
    bool RequiresCh3LengthReload();
    bool RequiresCh4LengthReload();

    bool TriggerEventCh1();
    bool TriggerEventCh2();
    bool TriggerEventCh3();
    bool TriggerEventCh4();

private:
    void changeRomBank(uWORD, uBYTE);
    void changeRamBank(uBYTE);
    void toggleRam(uWORD, uBYTE);
    void changeMode(uBYTE);
    void dmaTransfer(uBYTE);
    void closeBootRom();
    void handleJoypadTranslation(uBYTE);
    uBYTE getStatMode();

    int dmaCyclesCompleted;
    int dividerRegisterCounter;
    bool bootRomClosed;
    bool dmaTransferInProgress;
    uWORD translatedAddr;

    // APU flags
    bool reloadCh1LengthTimer = false;
    bool reloadCh2LengthTimer = false;
    bool reloadCh3LengthTimer = false;
    bool reloadCh4LengthTimer = false;
    bool triggerEventCh1 = false;
    bool triggerEventCh2 = false;
    bool triggerEventCh3 = false;
    bool triggerEventCh4 = false;

    enum CartAttributes {
        ramEnabled,
        romRamMode,
        romOnly,
        ram,
        mbc1,
        mbc2,
        mbc3,
        mbc4,
        mbc5,
        huc1,
        battery,
        timer,
        rumble
    };

    map<CartAttributes, bool> attributes;

    uWORD currentRomBank;
    uWORD currentRamBank;
    uWORD romBankCount;
    uWORD ramBankCount;
    uWORD ramBankSize;

    uint64_t romSize;
    uint64_t ramSize;

    uBYTE joypadBuffer;

    uBYTE* rom;
    uBYTE* cartridge;
    uBYTE cart_ram[4][0x2000];

    const uBYTE bootRom[BOOTROM_SIZE] = {
        0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C,
        0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E, 0x11, 0x3E, 0x80, 0x32,
        0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E,
        0xFC, 0xE0, 0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A,
        0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B, 0xFE, 0x34,
        0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22,
        0x23, 0x05, 0x20, 0xF9, 0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21,
        0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
        0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0,
        0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04, 0x1E, 0x02, 0x0E, 0x0C,
        0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D,
        0x20, 0xF2, 0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62,
        0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06, 0x7B, 0xE2,
        0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15,
        0x20, 0xD2, 0x05, 0x20, 0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F,
        0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
        0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED,
        0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83,
        0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89,
        0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
        0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC,
        0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5,
        0xB9, 0xA5, 0x42, 0x3C, 0x21, 0x04, 0x01, 0x11, 0xA8, 0x00,
        0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
        0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86,
        0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50,
    };
};

#endif
