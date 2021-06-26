#ifndef DEFINES_H
#define DEFINES_H

typedef unsigned char uBYTE;
typedef unsigned short uWORD;
typedef char sBYTE;
typedef short sWORD;

#define SCALE_FACTOR 4

#define NATIVE_SIZE_X 160
#define NATIVE_SIZE_Y 144
#define EXT_SIZE_X 256
#define EXT_SIZE_Y 256

#define TILE_BYTE_LENGTH 16

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

#define CPU_FLAG_BIT_SET(int) CPU::flagSet(int)
#define CPU_FLAG_BIT_TEST(int) CPU::flagTest(int)
#define CPU_FLAG_BIT_RESET(int) CPU::flagReset(int)

#define INTERUPT_EN_REGISTER_ADR 0xFFFF
#define INTERUPT_FLAG_REG 0xFF0F
#define TIMER_DIV_REG 0xFF04
#define JOYPAD_INTERUPT_REG 0xFF00

#define VBLANK_INTERUPT_VECTOR 0x0040
#define LCDC_INTERUPT_VECTOR 0x0048
#define TIMER_OVER_INTERUPT_VECTOR 0x0050
#define SER_TRF_INTERUPT_VECTOR 0x0058
#define CONTROL_INTERUPT_VECTOR 0x0060

#define VBLANK_INT 0
#define LCDC_INT 1
#define TIMER_OVERFLOW_INT 2
#define SER_TRF_INT 3
#define CONTROL_INT 4

#define Z_KEY 90
#define X_KEY 88

#define Z_FLAG 7
#define N_FLAG 6
#define H_FLAG 5
#define C_FLAG 4

#define NUM_ATTRIBUTES 13
#define RAM_ENABLED 0
#define ROM_RAM_MODE 1
#define ROM_ONLY 2
#define RAM 3
#define MBC1 4
#define MBC2 5
#define MBC3 6
#define MBC4 7
#define MBC5 8
#define HUC1 9
#define BATTERY 10
#define TIMER 11
#define RUMBLE 12
#define ROM_BANK_SIZE 0x4000

#define wxID_CUSTOM_DEBUGGER 500

#define FRAME_TIME_MS 10

#define GET_VARIABLE_NAME(variable) (#variable)

#define OP_CODES                                    \
    X(NOP, "NOP", 0x00)                             \
    X(LD_16IMM_BC, "LD_16IMM_BC", 0x01)             \
    X(LD_A_adrBC, "LD_A_adrBC", 0x02)               \
    X(INC_BC, "INC_BC", 0x03)                       \
    X(INC_B, "INC_B", 0x04)                         \
    X(DEC_B, "DEC_B", 0x05)                         \
    X(LD_8IMM_B, "LD_8IMM_B", 0x06)                 \
    X(RLC_A, "RLC_A", 0x07)                         \
    X(LD_SP_adr, "LD_SP_adr", 0x08)                 \
    X(ADD_BC_HL, "ADD_BC_HL", 0x09)                 \
    X(LD_adrBC_A, "LD_adrBC_A", 0x0A)               \
    X(DEC_BC, "DEC_BC", 0x0B)                       \
    X(INC_C, "INC_C", 0x0C)                         \
    X(DEC_C, "DEC_C", 0x0D)                         \
    X(LD_8IMM_C, "LD_8IMM_C", 0x0E)                 \
    X(RRC_A, "RRC_A", 0x0F)                         \
    X(STOP, "STOP", 0x10)                           \
    X(LD_16IMM_DE, "LD_16IMM_DE", 0x11)             \
    X(LD_A_adrDE, "LD_A_adrDE", 0x12)               \
    X(INC_DE, "INC_DE", 0x13)                       \
    X(INC_D, "INC_D", 0x14)                         \
    X(DEC_D, "DEC_D", 0x15)                         \
    X(LD_8IMM_D, "LD_8IMM_D", 0x16)                 \
    X(RL_A, "RL_A", 0x17)                           \
    X(RJmp_IMM, "RJmp_IMM", 0x18)                   \
    X(ADD_DE_HL, "ADD_DE_HL", 0x19)                 \
    X(LD_adrDE_A, "LD_adrDE_A", 0x1A)               \
    X(DEC_DE, "DEC_DE", 0x1B)                       \
    X(INC_E, "INC_E", 0x1C)                         \
    X(DEC_E, "DEC_E", 0x1D)                         \
    X(LD_8IMM_E, "LD_8IMM_E", 0x1E)                 \
    X(RR_A, "RR_A", 0x1F)                           \
    X(RJmp_NOTZERO, "RJmp_NOTZERO", 0x20)           \
    X(LD_16IMM_HL, "LD_16IMM_HL", 0x21)             \
    X(LDI_A_adrHL, "LDI_A_adrHL", 0x22)             \
    X(INC_HL, "INC_HL", 0x23)                       \
    X(INC_H, "INC_H", 0x24)                         \
    X(DEC_H, "DEC_H", 0x25)                         \
    X(LD_8IMM_H, "LD_8IMM_H", 0x26)                 \
    X(DAA, "DAA", 0x27)                             \
    X(RJmp_ZERO, "RJmp_ZERO", 0x28)                 \
    X(ADD_HL_HL, "ADD_HL_HL", 0x29)                 \
    X(LDI_adrHL_A, "LDI_adrHL_A", 0x2A)             \
    X(DEC_HL, "DEC_HL", 0x2B)                       \
    X(INC_L, "INC_L", 0x2C)                         \
    X(DEC_L, "DEC_L", 0x2D)                         \
    X(LD_8IMM_L, "LD_8IMM_L", 0x2E)                 \
    X(CPL_A, "CPL_A", 0x2F)                         \
    X(RJmp_NOCARRY, "RJmp_NOCARRY", 0x30)           \
    X(LD_16IM_SP, "LD_16IM_SP", 0x31)               \
    X(LDD_A_adrHL, "LDD_A_adrHL", 0x32)             \
    X(INC_SP, "INC_SP", 0x33)                       \
    X(INC_valHL, "INC_valHL", 0x34)                 \
    X(DEC_valHL, "DEC_valHL", 0x35)                 \
    X(LD_8IMM_adrHL, "LD_8IMM_adrHL", 0x36)         \
    X(SET_CARRY_FLAG, "SET_CARRY_FLAG", 0x37)       \
    X(RJmp_CARRY, "RJmp_CARRY", 0x38)               \
    X(ADD_SP_HL, "ADD_SP_HL", 0x39)                 \
    X(LDD_adrHL_A, "LDD_adrHL_A", 0x3A)             \
    X(DEC_SP, "DEC_SP", 0x3B)                       \
    X(INC_A, "INC_A", 0x3C)                         \
    X(DEC_A, "DEC_A", 0x3D)                         \
    X(LD_8IMM_A, "LD_8IMM_A", 0x3E)                 \
    X(COMP_CARRY_FLAG, "COMP_CARRY_FLAG", 0x3F)     \
    X(LD_B_B, "LD_B_B", 0x40)                       \
    X(LD_C_B, "LD_C_B", 0x41)                       \
    X(LD_D_B, "LD_D_B", 0x42)                       \
    X(LD_E_B, "LD_E_B", 0x43)                       \
    X(LD_H_B, "LD_H_B", 0x44)                       \
    X(LD_L_B, "LD_L_B", 0x45)                       \
    X(LD_adrHL_B, "LD_adrHL_B", 0x46)               \
    X(LD_A_B, "LD_A_B", 0x47)                       \
    X(LD_B_C, "LD_B_C", 0x48)                       \
    X(LD_C_C, "LD_C_C", 0x49)                       \
    X(LD_D_C, "LD_D_C", 0x4A)                       \
    X(LD_E_C, "LD_E_C", 0x4B)                       \
    X(LD_H_C, "LD_H_C", 0x4C)                       \
    X(LD_L_C, "LD_L_C", 0x4D)                       \
    X(LD_adrHL_C, "LD_adrHL_C", 0x4E)               \
    X(LD_A_C, "LD_A_C", 0x4F)                       \
    X(LD_B_D, "LD_B_D", 0x50)                       \
    X(LD_C_D, "LD_C_D", 0x51)                       \
    X(LD_D_D, "LD_D_D", 0x52)                       \
    X(LD_E_D, "LD_E_D", 0x53)                       \
    X(LD_H_D, "LD_H_D", 0x54)                       \
    X(LD_L_D, "LD_L_D", 0x55)                       \
    X(LD_adrHL_D, "LD_adrHL_D", 0x56)               \
    X(LD_A_D, "LD_A_D", 0x57)                       \
    X(LD_B_E, "LD_B_E", 0x58)                       \
    X(LD_C_E, "LD_C_E", 0x59)                       \
    X(LD_D_E, "LD_D_E", 0x5A)                       \
    X(LD_E_E, "LD_E_E", 0x5B)                       \
    X(LD_H_E, "LD_H_E", 0x5C)                       \
    X(LD_L_E, "LD_L_E", 0x5D)                       \
    X(LD_adrHL_E, "LD_adrHL_E", 0x5E)               \
    X(LD_A_E, "LD_A_E", 0x5F)                       \
    X(LD_B_H, "LD_B_H", 0x60)                       \
    X(LD_C_H, "LD_C_H", 0x61)                       \
    X(LD_D_H, "LD_D_H", 0x62)                       \
    X(LD_E_H, "LD_E_H", 0x63)                       \
    X(LD_H_H, "LD_H_H", 0x64)                       \
    X(LD_L_H, "LD_L_H", 0x65)                       \
    X(LD_adrHL_H, "LD_adrHL_H", 0x66)               \
    X(LD_A_H, "LD_A_H", 0x67)                       \
    X(LD_B_L, "LD_B_L", 0x68)                       \
    X(LD_C_L, "LD_C_L", 0x69)                       \
    X(LD_D_L, "LD_D_L", 0x6A)                       \
    X(LD_E_L, "LD_E_L", 0x6B)                       \
    X(LD_H_L, "LD_H_L", 0x6C)                       \
    X(LD_L_L, "LD_L_L", 0x6D)                       \
    X(LD_adrHL_L, "LD_adrHL_L", 0x6E)               \
    X(LD_A_L, "LD_A_L", 0x6F)                       \
    X(LD_B_adrHL, "LD_B_adrHL", 0x70)               \
    X(LD_C_adrHL, "LD_C_adrHL", 0x71)               \
    X(LD_D_adrHL, "LD_D_adrHL", 0x72)               \
    X(LD_E_adrHL, "LD_E_adrHL", 0x73)               \
    X(LD_H_adrHL, "LD_H_adrHL", 0x74)               \
    X(LD_L_adrHL, "LD_L_adrHL", 0x75)               \
    X(HALT, "HALT", 0x76)                           \
    X(LD_A_adrHL, "LD_A_adrHL", 0x77)               \
    X(LD_B_A, "LD_B_A", 0x78)                       \
    X(LD_C_A, "LD_C_A", 0x79)                       \
    X(LD_D_A, "LD_D_A", 0x7A)                       \
    X(LD_E_A, "LD_E_A", 0x7B)                       \
    X(LD_H_A, "LD_H_A", 0x7C)                       \
    X(LD_L_A, "LD_L_A", 0x7D)                       \
    X(LD_adrHL_A, "LD_adrHL_A", 0x7E)               \
    X(LD_A_A, "LD_A_A", 0x7F)                       \
    X(ADD_B_A, "ADD_B_A", 0x80)                     \
    X(ADD_C_A, "ADD_C_A", 0x81)                     \
    X(ADD_D_A, "ADD_D_A", 0x82)                     \
    X(ADD_E_A, "ADD_E_A", 0x83)                     \
    X(ADD_H_A, "ADD_H_A", 0x84)                     \
    X(ADD_L_A, "ADD_L_A", 0x85)                     \
    X(ADD_adrHL_A, "ADD_adrHL_A", 0x86)             \
    X(ADD_A_A, "ADD_A_A", 0x87)                     \
    X(ADC_B_A, "ADC_B_A", 0x88)                     \
    X(ADC_C_A, "ADC_C_A", 0x89)                     \
    X(ADC_D_A, "ADC_D_A", 0x8A)                     \
    X(ADC_E_A, "ADC_E_A", 0x8B)                     \
    X(ADC_H_A, "ADC_H_A", 0x8C)                     \
    X(ADC_L_A, "ADC_L_A", 0x8D)                     \
    X(ADC_adrHL_A, "ADC_adrHL_A", 0x8E)             \
    X(ADC_A_A, "ADC_A_A", 0x8F)                     \
    X(SUB_B_A, "SUB_B_A", 0x90)                     \
    X(SUB_C_A, "SUB_C_A", 0x91)                     \
    X(SUB_D_A, "SUB_D_A", 0x92)                     \
    X(SUB_E_A, "SUB_E_A", 0x93)                     \
    X(SUB_H_A, "SUB_H_A", 0x94)                     \
    X(SUB_L_A, "SUB_L_A", 0x95)                     \
    X(SUB_adrHL_A, "SUB_adrHL_A", 0x96)             \
    X(SUB_A_A, "SUB_A_A", 0x97)                     \
    X(SBC_B_A, "SBC_B_A", 0x98)                     \
    X(SBC_C_A, "SBC_C_A", 0x99)                     \
    X(SBC_D_A, "SBC_D_A", 0x9A)                     \
    X(SBC_E_A, "SBC_E_A", 0x9B)                     \
    X(SBC_H_A, "SBC_H_A", 0x9C)                     \
    X(SBC_L_A, "SBC_L_A", 0x9D)                     \
    X(SBC_adrHL_A, "SBC_adrHL_A", 0x9E)             \
    X(SBC_A_A, "SBC_A_A", 0x9F)                     \
    X(AND_B_A, "AND_B_A", 0xA0)                     \
    X(AND_C_A, "AND_C_A", 0xA1)                     \
    X(AND_D_A, "AND_D_A", 0xA2)                     \
    X(AND_E_A, "AND_E_A", 0xA3)                     \
    X(AND_H_A, "AND_H_A", 0xA4)                     \
    X(AND_L_A, "AND_L_A", 0xA5)                     \
    X(AND_adrHL_A, "AND_adrHL_A", 0xA6)             \
    X(AND_A_A, "AND_A_A", 0xA7)                     \
    X(XOR_B_A, "XOR_B_A", 0xA8)                     \
    X(XOR_C_A, "XOR_C_A", 0xA9)                     \
    X(XOR_D_A, "XOR_D_A", 0xAA)                     \
    X(XOR_E_A, "XOR_E_A", 0xAB)                     \
    X(XOR_H_A, "XOR_H_A", 0xAC)                     \
    X(XOR_L_A, "XOR_L_A", 0xAD)                     \
    X(XOR_adrHL_A, "XOR_adrHL_A", 0xAE)             \
    X(XOR_A_A, "XOR_A_A", 0xAF)                     \
    X(OR_B_A, "OR_B_A", 0xB0)                       \
    X(OR_C_A, "OR_C_A", 0xB1)                       \
    X(OR_D_A, "OR_D_A", 0xB2)                       \
    X(OR_E_A, "OR_E_A", 0xB3)                       \
    X(OR_H_A, "OR_H_A", 0xB4)                       \
    X(OR_L_A, "OR_L_A", 0xB5)                       \
    X(OR_adrHL_A, "OR_adrHL_A", 0xB6)               \
    X(OR_A_A, "OR_A_A", 0xB7)                       \
    X(CMP_B_A, "CMP_B_A", 0xB8)                     \
    X(CMP_C_A, "CMP_C_A", 0xB9)                     \
    X(CMP_D_A, "CMP_D_A", 0xBA)                     \
    X(CMP_E_A, "CMP_E_A", 0xBB)                     \
    X(CMP_H_A, "CMP_H_A", 0xBC)                     \
    X(CMP_L_A, "CMP_L_A", 0xBD)                     \
    X(CMP_adrHL_A, "CMP_adrHL_A", 0xBE)             \
    X(CMP_A_A, "CMP_A_A", 0xBF)                     \
    X(RET_NOT_ZERO, "RET_NOT_ZERO", 0xC0)           \
    X(POP_BC, "POP_BC", 0xC1)                       \
    X(JMP_NOT_ZERO, "JMP_NOT_ZERO", 0xC2)           \
    X(JMP, "JMP", 0xC3)                             \
    X(CALL_NOT_ZERO, "CALL_NOT_ZERO", 0xC4)         \
    X(PUSH_BC, "PUSH_BC", 0xC5)                     \
    X(ADD_IMM_A, "ADD_IMM_A", 0xC6)                 \
    X(RST_0, "RST_0", 0xC7)                         \
    X(RET_ZERO, "RET_ZERO", 0xC8)                   \
    X(RETURN, "RETURN", 0xC9)                       \
    X(JMP_ZERO, "JMP_ZERO", 0xCA)                   \
    X(EXT_OP, "EXT_OP", 0xCB)                       \
    X(CALL_ZERO, "CALL_ZERO", 0xCC)                 \
    X(CALL, "CALL", 0xCD)                           \
    X(ADC_8IMM_A, "ADC_8IMM_A", 0xCE)               \
    X(RST_8, "RST_8", 0xCF)                         \
    X(RET_NOCARRY, "RET_NOCARRY", 0xD0)             \
    X(POP_DE, "POP_DE", 0xD1)                       \
    X(JMP_NOCARRY, "JMP_NOCARRY", 0xD2)             \
    X(D3, "0xD3", 0xD3)                             \
    X(CALL_NOCARRY, "CALL_NOCARRY", 0xD4)           \
    X(PUSH_DE, "PUSH_DE", 0xD5)                     \
    X(SUB_8IMM_A, "SUB_8IMM_A", 0xD6)               \
    X(RST_10, "RST_10", 0xD7)                       \
    X(RET_CARRY, "RET_CARRY", 0xD8)                 \
    X(RET_INT, "RET_INT", 0xD9)                     \
    X(JMP_CARRY, "JMP_CARRY", 0xDA)                 \
    X(DB, "0xDB", 0xDB)                             \
    X(CALL_CARRY, "CALL_CARRY", 0xDC)               \
    X(DD, "0xDD", 0xDD)                             \
    X(SBC_8IMM_A, "SBC_8IMM_A", 0xDE)               \
    X(RST_18, "RST_18", 0xDF)                       \
    X(LDH_A_IMMadr, "LDH_A_IMMadr", 0xE0)           \
    X(POP_HL, "POP_HL", 0xE1)                       \
    X(LDH_A_C, "LDH_A_C", 0xE2)                     \
    X(E3, "0xE3", 0xE3)                             \
    X(E4, "0xE4", 0xE4)                             \
    X(PUSH_HL, "PUSH_HL", 0xE5)                     \
    X(AND_8IMM_A, "AND_8IMM_A", 0xE6)               \
    X(RST_20, "RST_20", 0xE7)                       \
    X(ADD_SIMM_SP, "ADD_SIMM_SP", 0xE8)             \
    X(JMP_adrHL, "JMP_adrHL", 0xE9)                 \
    X(LD_A_adr, "LD_A_adr", 0xEA)                   \
    X(EB, "0xEB", 0xEB)                             \
    X(EC, "0xEC", 0xEC)                             \
    X(ED, "0xED", 0xED)                             \
    X(XOR_8IMM_A, "XOR_8IMM_A", 0xEE)               \
    X(RST_28, "RST_28", 0xEF)                       \
    X(LDH_IMMadr_A, "LDH_IMMadr_A", 0xF0)           \
    X(POP_AF, "POP_AF", 0xF1)                       \
    X(LDH_C_A, "LDH_C_A", 0xF2)                     \
    X(DISABLE_INT, "DISABLE_INT", 0xF3)             \
    X(F4, "0xF4", 0xF4)                             \
    X(PUSH_AF, "PUSH_AF", 0xF5)                     \
    X(OR_8IMM_A, "OR_8IMM_A", 0xF6)                 \
    X(RST_30, "RST_30", 0xF7)                       \
    X(LDHL_S_8IMM_SP_HL, "LDHL_S_8IMM_SP_HL", 0xF8) \
    X(LD_HL_SP, "LD_HL_SP", 0xF9)                   \
    X(LD_16adr_A, "LD_16adr_A", 0xFA)               \
    X(ENABLE_INT, "ENABLE_INT", 0xFB)               \
    X(FC, "0xFC", 0xFC)                             \
    X(FD, "0xFD", 0xFD)                             \
    X(CMP_8IMM_A, "CMP_8IMM_A", 0xFE)               \
    X(RST_38, "RST_38", 0xFF)

#endif
