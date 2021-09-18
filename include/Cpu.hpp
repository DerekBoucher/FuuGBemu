#ifndef CPU_H
#define CPU_H

#include "Memory.hpp"

#include <stdio.h>

#define Z_FLAG 7
#define N_FLAG 6
#define H_FLAG 5
#define C_FLAG 4

#define CPU_FLAG_BIT_SET(int) flagSet(int)
#define CPU_FLAG_BIT_TEST(int) flagTest(int)
#define CPU_FLAG_BIT_RESET(int) flagReset(int)

#define INTERUPT_EN_REGISTER_ADR 0xFFFF
#define INTERUPT_FLAG_REG 0xFF0F
#define TIMER_DIV_REG 0xFF04
#define JOYPAD_INPUT_REG 0xFF00

#define VBLANK_INTERUPT_VECTOR 0x0040
#define LCDC_INTERUPT_VECTOR 0x0048
#define TIMER_OVER_INTERUPT_VECTOR 0x0050
#define SER_TRF_INTERUPT_VECTOR 0x0058
#define CONTROL_INTERUPT_VECTOR 0x0060

class Cpu
{
public:
    Cpu();
    Cpu(Cpu &) = delete;
    ~Cpu();

    bool Halted;
    bool Paused;

    void Pause();
    void CheckInterupts();
    void Halt();
    void SetMemory(Memory* memory);
    int ExecuteNextOpCode();

    enum opCode {
        NOP = 0x00,             //No instruction
        LD_16IMM_BC = 0x01,     //Load immediate 16-bit value nn into BC
        LD_A_adrBC = 0x02,      //Load Contents of A into the location pointed by BC
        INC_BC = 0x03,          //Increment BC
        INC_B = 0x04,           //Increment B
        DEC_B = 0x05,           //Decrement B
        LD_8IMM_B = 0x06,       //Load immediate 8 bit value into B
        RLC_A = 0x07,           //Rotate A reg left with carry
        LD_SP_adr = 0x08,       //Load Stack Pointer address into a given address location
        ADD_BC_HL = 0x09,       //Add the contents of BC to HL (BC remains the same)
        LD_adrBC_A = 0x0A,      //Load the contents of the address pointed by BC into reg A
        DEC_BC = 0x0B,          //Decrement BC
        INC_C = 0x0C,           //Increment C
        DEC_C = 0x0D,           //Decrement C
        LD_8IMM_C = 0x0E,       //Load immediate 8bit value into reg C
        RRC_A = 0x0F,           //Rotate reg A to the right with carry
        STOP = 0x10,            //Stop processor
        LD_16IMM_DE = 0x11,     //Load immediate 16bit value into reg pair DE
        LD_A_adrDE = 0x12,      //Load contents of reg A into address pointed by DE
        INC_DE = 0x13,          //Increment DE
        INC_D = 0x14,           //Increment D
        DEC_D = 0x15,           //Decrement D
        LD_8IMM_D = 0x16,       //Load immediate 8-bit value into reg D
        RL_A = 0x17,            //Rotate reg A to the left
        RJmp_IMM = 0x18,        //Jump to address (PC + e) where e can be any value between -127 to +129 (the value of e is contained in the following 8 bit instruction)
        ADD_DE_HL = 0x19,       //Add contents of DE to HL
        LD_adrDE_A = 0x1A,      //Load contents of memoryUnit location pointed by DE into A
        DEC_DE = 0x1B,          //Decrement DE
        INC_E = 0x1C,           //Increment E
        DEC_E = 0x1D,           //Decrement E
        LD_8IMM_E = 0x1E,       // Load 8 bit immediate value into E
        RR_A = 0x1F,            // Rotate reg A to the right
        RJmp_NOTZERO = 0x20,    //Jump to address (PC+e) if last result was not zero. -127<e<129. e is specfied in the following instruction
        LD_16IMM_HL = 0x21,     // Load immediate 16 bit value into HL
        LDI_A_adrHL = 0x22,     // Load Contents of A into address pointed by HL. Increment HL afterwards
        INC_HL = 0x23,          //Increment HL
        INC_H = 0x24,           //Increment H
        DEC_H = 0x25,           //Decrement H
        LD_8IMM_H = 0x26,       //Load immediate 8bit value into H
        DAA = 0x27,             //Adjust A for BCD addition
        RJmp_ZERO = 0x28,       //Jump to address specified by (PC+e) if last result was zero. -127<e<129. e is specified in the following instruction
        ADD_HL_HL = 0x29,       //Add HL to itself
        LDI_adrHL_A = 0x2A,     //Load contents of address pointed by HL into A. Increment HL afterwards.
        DEC_HL = 0x2B,          //Decrement HL
        INC_L = 0x2C,           //Increment L
        DEC_L = 0x2D,           //Decrement L
        LD_8IMM_L = 0x2E,       //Load 8 bit immediate value into L
        CPL_A = 0x2F,           //Logical Compliment of A = A xor 0xFF
        RJmp_NOCARRY = 0x30,    //Jump to address specified by (PC+e) if last result did not generate a carry
        LD_16IM_SP = 0x31,      //Load 16 bit immediate value into SP
        LDD_A_adrHL = 0x32,     //Load contents of A into address pointed by HL, then decrement HL afterwards.
        INC_SP = 0x33,          //Increment SP
        INC_valHL = 0x34,       //Increment value pointed by HL
        DEC_valHL = 0x35,       //Decrement value pointed by HL
        LD_8IMM_adrHL = 0x36,   //Load immediate 8bit value into address pointed by HL
        SET_CARRY_FLAG = 0x37,  //Set Carry Flag
        RJmp_CARRY = 0x38,      //Jump to address (PC+e) if last result generated a Carry.
        ADD_SP_HL = 0x39,       //Add value of SP to value of HL
        LDD_adrHL_A = 0x3A,     //Load contents of address pointed by HL into A, then decrement HL afterwards.
        DEC_SP = 0x3B,          //Decrement SP
        INC_A = 0x3C,           //Increment A
        DEC_A = 0x3D,           //Decrement A
        LD_8IMM_A = 0x3E,       //Load 8 bit immediate value into A
        COMP_CARRY_FLAG = 0x3F, //Compliment carry flag
        LD_B_B = 0x40,          //Load B into B (Redundant)
        LD_C_B = 0x41,          //Load C into B
        LD_D_B = 0x42,          //Load D into B
        LD_E_B = 0x43,          //Load E into B
        LD_H_B = 0x44,          //Load H into B
        LD_L_B = 0x45,          //Load L into B
        LD_adrHL_B = 0x46,      //Load content pointed by HL into B
        LD_A_B = 0x47,          //Load A into B
        LD_B_C = 0x48,          //Load B into C
        LD_C_C = 0x49,          //Load C into C (Redundant)
        LD_D_C = 0x4A,          //Load D into C
        LD_E_C = 0x4B,          //Load E into C
        LD_H_C = 0x4C,          //Load H into C
        LD_L_C = 0x4D,          //Load L into C
        LD_adrHL_C = 0x4E,      //Load contents of address pointed by HL into C
        LD_A_C = 0x4F,          //Load A into C
        LD_B_D = 0x50,          //Load B into D
        LD_C_D = 0x51,          //Load C into D
        LD_D_D = 0x52,          //Load D into D (Redundant)
        LD_E_D = 0x53,          //Load E into D
        LD_H_D = 0x54,          //Load H into D
        LD_L_D = 0x55,          //Load L into D
        LD_adrHL_D = 0x56,      //Load contents of address pointed by HL into D
        LD_A_D = 0x57,          //Load A into D
        LD_B_E = 0x58,          //Load B into E
        LD_C_E = 0x59,          //Load C into E
        LD_D_E = 0x5A,          //Load D into E
        LD_E_E = 0x5B,          //Load E into E (Redundant)
        LD_H_E = 0x5C,          //Load H into E
        LD_L_E = 0x5D,          //Load L into E
        LD_adrHL_E = 0x5E,      //Load contents of address pointed by HL into E
        LD_A_E = 0x5F,          //Load A into E
        LD_B_H = 0x60,          //Load B into H
        LD_C_H = 0x61,          //Load C into H
        LD_D_H = 0x62,          //Load D into H
        LD_E_H = 0x63,          //Load E into H
        LD_H_H = 0x64,          //Load H into H
        LD_L_H = 0x65,          //Load L into H
        LD_adrHL_H = 0x66,      //Load contents of address pointed by HL into H
        LD_A_H = 0x67,          //Load A into H
        LD_B_L = 0x68,          //Load B into L
        LD_C_L = 0x69,          //Load C into L
        LD_D_L = 0x6A,          //Load D into L
        LD_E_L = 0x6B,          //Load E into L
        LD_H_L = 0x6C,          //Load H into L
        LD_L_L = 0x6D,          //Load L into L (Redundant)
        LD_adrHL_L = 0x6E,      //Load contents of address pointed by HL into L
        LD_A_L = 0x6F,          //Load A into L
        LD_B_adrHL = 0x70,      //Load B into address pointed by HL
        LD_C_adrHL = 0x71,      //Load C into address pointed by HL
        LD_D_adrHL = 0x72,      //Load D into address pointed by HL
        LD_E_adrHL = 0x73,      //Load E into address pointed by HL
        LD_H_adrHL = 0x74,      //Load H into address pointed by HL
        LD_L_adrHL = 0x75,      //Load L into address pointed by HL
        HALT = 0x76,            //Halt processor
        LD_A_adrHL = 0x77,      //Load A into address pointed by HL
        LD_B_A = 0x78,          //Load B into A
        LD_C_A = 0x79,          //Load C into A
        LD_D_A = 0x7A,          //Load D into A
        LD_E_A = 0x7B,          //Load E into A
        LD_H_A = 0x7C,          //Load H into A
        LD_L_A = 0x7D,          //Load L into A
        LD_adrHL_A = 0x7E,      //Load contents of address pointed by HL into A
        LD_A_A = 0x7F,          //Load A into A
        ADD_B_A = 0x80,         //Add B to A
        ADD_C_A = 0x81,         //Add C to A
        ADD_D_A = 0x82,         //Add D to A
        ADD_E_A = 0x83,         //Add E to A
        ADD_H_A = 0x84,         //Add H to A
        ADD_L_A = 0x85,         //Add L to A
        ADD_adrHL_A = 0x86,     //Add contents of address pointed by HL to A
        ADD_A_A = 0x87,         //Add A to A
        ADC_B_A = 0x88,         //Add B and CARRY flag to A
        ADC_C_A = 0x89,         //Add C and CARRY flag to A
        ADC_D_A = 0x8A,         //add D and CARRY flag to A
        ADC_E_A = 0x8B,         //Add E and CARRY flag to A
        ADC_H_A = 0x8C,         //Add H and CARRY flag to A
        ADC_L_A = 0x8D,         //Add L and CARRY flag to A
        ADC_adrHL_A = 0x8E,     //Add contents of address pointed by HL and CARRY flag to A
        ADC_A_A = 0x8F,         //Add A and CARRY flag to A
        SUB_B_A = 0x90,         //Subtract B from A (Result in A)
        SUB_C_A = 0x91,         //Subtract C from A
        SUB_D_A = 0x92,         //Subtract D from A
        SUB_E_A = 0x93,         //Subtract E from A
        SUB_H_A = 0x94,         //Subtract H from A
        SUB_L_A = 0x95,         //Subtract L from A
        SUB_adrHL_A = 0x96,     //Subtract value pointed by HL from A
        SUB_A_A = 0x97,         //Subtract A from A
        SBC_B_A = 0x98,         //Subtract B and Carry flag from A
        SBC_C_A = 0x99,         //Subtract C and CArry flag from A
        SBC_D_A = 0x9A,         //Subtract D and Carry flag from A
        SBC_E_A = 0x9B,         //Subtract E and CArry Flag from A
        SBC_H_A = 0x9C,         //Subtract H and Carry Flag from A
        SBC_L_A = 0x9D,         //Subtract L and Carry flag from A
        SBC_adrHL_A = 0x9E,     //Subtract value pointed by HL and carry flag from A
        SBC_A_A = 0x9F,         //Subtract A and carry flag from A
        AND_B_A = 0xA0,         //Logical B AND A
        AND_C_A = 0xA1,         //Logical C AND A
        AND_D_A = 0xA2,         //Logical D AND A
        AND_E_A = 0xA3,         //Logical E AND A
        AND_H_A = 0xA4,         //Logical H AND A
        AND_L_A = 0xA5,         //Logical L AND A
        AND_adrHL_A = 0xA6,     //Logical value pointed by HL AND A
        AND_A_A = 0xA7,         //Logical A AND A
        XOR_B_A = 0xA8,         //Logical B XOR A
        XOR_C_A = 0xA9,         //Logical C XOR A
        XOR_D_A = 0xAA,         //Logical D XOR A
        XOR_E_A = 0xAB,         //Logical E XOR A
        XOR_H_A = 0xAC,         //Logical H XOR A
        XOR_L_A = 0xAD,         //Logical L XOR A
        XOR_adrHL_A = 0xAE,     //Logical value pointed by HL XOR A
        XOR_A_A = 0xAF,         //Logical A XOR A
        OR_B_A = 0xB0,          //Logical B OR A
        OR_C_A = 0xB1,          //Logical C OR A
        OR_D_A = 0xB2,          //Logical D OR A
        OR_E_A = 0xB3,          //Logical E OR A
        OR_H_A = 0xB4,          //Logical H OR A
        OR_L_A = 0xB5,          //Logical L OR A
        OR_adrHL_A = 0xB6,      //Logical value pointed by HL OR A
        OR_A_A = 0xB7,          //Logical A OR A
        CMP_B_A = 0xB8,         //Compare B to A (Performs A - B, sets zero flag if equals 0, or else does nothing)
        CMP_C_A = 0xB9,         //Compare C to A
        CMP_D_A = 0xBA,         //Compare D to A
        CMP_E_A = 0xBB,         //Compare E to A
        CMP_H_A = 0xBC,         //Compare H to A
        CMP_L_A = 0xBD,         //Compare L to A
        CMP_adrHL_A = 0xBE,     //Compare value pointed by HL to A
        CMP_A_A = 0xBF,         //Compare A to A (Obsolete)
        RET_NOT_ZERO = 0xC0,    //Return if last result was not zero
        POP_BC = 0xC1,          //Pop stack into BC
        JMP_NOT_ZERO = 0xC2,    //Jump to absolute location if last result was not zero
        JMP = 0xC3,             //Jump to absolute value
        CALL_NOT_ZERO = 0xC4,   //Call routine at location if last result was not zero
        PUSH_BC = 0xC5,         //Push contents of BC into the stack
        ADD_IMM_A = 0xC6,       //Add immediate value to A
        RST_0 = 0xC7,           //Call routine at memoryUnit location 0x0000
        RET_ZERO = 0xC8,        //return from routine if last result was zero
        RETURN = 0xC9,          //Return from routine
        JMP_ZERO = 0xCA,        //Jump to absolute location if last result was zero
        EXT_OP = 0xCB,          //Extended operation (Requires more byte strings)
        CALL_ZERO = 0xCC,       //Call routine at location if last result was zero
        CALL = 0xCD,            //Call routine at location.
        ADC_8IMM_A = 0xCE,      //Add 8 bit immediate value and CARRY flag to A
        RST_8 = 0xCF,           //Call routine at memoryUnit location 0x0008
        RET_NOCARRY = 0xD0,     //Return from routine if last result did not generate a carry
        POP_DE = 0xD1,          //POP contents pointed by SP into DE
        JMP_NOCARRY = 0xD2,     //Jump to absolute location if last result did not generate a carry
        //        XX = 0xD3, //No operation
        CALL_NOCARRY = 0xD4, //Call routine at location if last result did not generate a carry
        PUSH_DE = 0xD5,      //Push contents of DE into the stack
        SUB_8IMM_A = 0xD6,   //Subtract immediate 8 bit value from A
        RST_10 = 0xD7,       //Call routine at memoryUnit location 0x0010
        RET_CARRY = 0xD8,    //REturn from routine if last result generated a carry
        RET_INT = 0xD9,      //Enable interrupts and return to calling routine
        JMP_CARRY = 0xDA,    //Jump to absolute location if last result generated a carry
        //            XX = 0xDB, //No operation
        CALL_CARRY = 0xDC, //Call routine at location if last result generated a carry
        //        XX = 0xDD, //No operation
        SBC_8IMM_A = 0xDE,   //Subtract 8 bit immediate value and CARRY flag from A
        RST_18 = 0xDF,       //Call routine at memoryUnit location 0x0018
        LDH_A_IMMadr = 0xE0, //Load contents of A into memoryUnit location pointed to by (0xFF00 + 8 bit immediate value)
        POP_HL = 0xE1,       //Pop stack into HL
        LDH_A_C = 0xE2,      //Load contents of A into memoryUnit location pointed to by (0xFF00 + C)
        //        XX = 0xE3, //No operation
        //        XX = 0xE4, //No operation
        PUSH_HL = 0xE5,     //Push contents of HL into the stack
        AND_8IMM_A = 0xE6,  //Logical 8 Bit immediate AND A
        RST_20 = 0xE7,      //Call routine at memoryUnit location 0x0020
        ADD_SIMM_SP = 0xE8, //Add signed 8 bit immediate value to SP
        JMP_adrHL = 0xE9,   //Jump to address pointed by HL
        LD_A_adr = 0xEA,    //Load A into specified 16 bit address
        //        XX = 0xEB, //No operation
        //        XX = 0xEC, //No operation
        //        XX = 0xED, //No operation
        XOR_8IMM_A = 0xEE,   //Logical 8 bit immediate XOR A
        RST_28 = 0xEF,       //Call routine at memoryUnit location 0x0028
        LDH_IMMadr_A = 0xF0, //Load Contents of memoryUnit at location (0xFF00 + 8bit immediate) into A
        POP_AF = 0xF1,       //Pop stack into AF
        LDH_C_A = 0xF2,      //Load contents of memoryUnit location pointed to by (0xFF00 + C) into A
        DISABLE_INT = 0xF3,  //Disable interupts
        //        XX = 0xF4, //No operation
        PUSH_AF = 0xF5,           //Push contents of AF into stack
        OR_8IMM_A = 0xF6,         //Logical 8 bit immediate OR A
        RST_30 = 0xF7,            //Call routine at memoryUnit location 0x0030
        LDHL_S_8IMM_SP_HL = 0xF8, // Add 8 bit immediate value to SP and then save result into HL
        LD_HL_SP = 0xF9,          //Load HL into SP
        LD_16adr_A = 0xFA,        //Load specified 16 bit address into A
        ENABLE_INT = 0xFB,        //Enable interrupts
        //        XX = 0xFC, //No operation
        //        XX = 0xFD, //No operation
        CMP_8IMM_A = 0xFE, //Compare 8 bit immediate to A
        RST_38 = 0xFF      //Call routine at memoryUnit location 0x0038
    };

    enum ExtendedOpCode
    {
        RLC_B = 0x00,       //Rotate B left. Old bit 7 to Carry Flag
        RLC_C = 0x01,       //Rotate C left. Old bit 7 to Carry Flag
        RLC_D = 0x02,       //Rotate D left. Old bit 7 to Carry Flag
        RLC_E = 0x03,       //Rotate E left. Old bit 7 to Carry Flag
        RLC_H = 0x04,       //Rotate H left. Old bit 7 to Carry Flag
        RLC_L = 0x05,       //Rotate L left. Old bit 7 to Carry Flag
        RLC_adrHL = 0x06,   //Rotate (HL) left. Old bit 7 to Carry Flag
        eRLC_A = 0x07,      //Rotate A left. Old bit 7 to Carry Flag
        RRC_B = 0x08,       //Rotate B right. Old bit 0 into carry flag
        RRC_C = 0x09,       //Rotate C right. old bit 0 into carry flag
        RRC_D = 0x0A,       //Rotate D right. old bit 0 into carry flag
        RRC_E = 0x0B,       //Rotate E right. old bit 0 into carry flag
        RRC_H = 0x0C,       //Rotate H right. old bit 0 into carry flag
        RRC_L = 0x0D,       //Rotate L right. old bit 0 into carry flag
        RRC_adrHL = 0x0E,   //Rotate (HL) right. old bit 0 into carry flag
        eRRC_A = 0x0F,      //Rotate A right. old bit 0 into carry flag
        RL_B = 0x10,        //Rotate B left through Carry Flag
        RL_C = 0x11,        //Rotate C left through Carry Flag
        RL_D = 0x12,        //Rotate D left through Carry Flag
        RL_E = 0x13,        //Rotate E left through Carry Flag
        RL_H = 0x14,        //Rotate H left through Carry Flag
        RL_L = 0x15,        //Rotate L left through Carry Flag
        RL_adrHL = 0x16,    //Rotate (HL) left through Carry Flag
        eRL_A = 0x17,       //Rotate A left through Carry Flag
        RR_B = 0x18,        // Rotate B right through carry flag
        RR_C = 0x19,        // Rotate C right through carry flag
        RR_D = 0x1A,        // Rotate D right through carry flag
        RR_E = 0x1B,        // Rotate E right through carry flag
        RR_H = 0x1C,        // Rotate H right through carry flag
        RR_L = 0x1D,        // Rotate L right through carry flag
        RR_adrHL = 0x1E,    // Rotate (HL) right through carry flag
        eRR_A = 0x1F,       // Rotate A right through carry flag
        SLA_B = 0x20,       //Shift B left into Carry. LSB of B set to 0
        SLA_C = 0x21,       //Shift C left into Carry. LSB of C set to 0
        SLA_D = 0x22,       //Shift D left into Carry. LSB of D set to 0
        SLA_E = 0x23,       //Shift E left into Carry. LSB of E set to 0
        SLA_H = 0x24,       //Shift H left into Carry. LSB of H set to 0
        SLA_L = 0x25,       //Shift L left into Carry. LSB of L set to 0
        SLA_adrHL = 0x26,   //Shift (HL) left into Carry. LSB of (HL) set to 0
        SLA_A = 0x27,       //Shift A left into Carry. LSB of A set to 0
        SRA_B = 0x28,       //Shift B right into carry. MSB does not change
        SRA_C = 0x29,       //Shift C right into carry. MSB does not change
        SRA_D = 0x2A,       //Shift D right into carry. MSB does not change
        SRA_E = 0x2B,       //Shift E right into carry. MSB does not change
        SRA_H = 0x2C,       //Shift H right into carry. MSB does not change
        SRA_L = 0x2D,       //Shift L right into carry. MSB does not change
        SRA_adrHL = 0x2E,   //Shift (HL) right into carry. MSB does not change
        SRA_A = 0x2F,       //Shift A right into carry. MSB does not change
        SWAP_B = 0x30,      //Swap upper & lower nibles of B
        SWAP_C = 0x31,      //Swap upper & lower nibles of C
        SWAP_D = 0x32,      //Swap upper & lower nibles of D
        SWAP_E = 0x33,      //Swap upper & lower nibles of E
        SWAP_H = 0x34,      //Swap upper & lower nibles of H
        SWAP_L = 0x35,      //Swap upper & lower nibles of L
        SWAP_adrHL = 0x36,  //Swap upper & lower nibles of (HL)
        SWAP_A = 0x37,      //Swap upper & lower nibles of A
        SRL_B = 0x38,       //Shift n Right into Carry. MSB set to 0
        SRL_C = 0x39,       //Shift n Right into Carry. MSB set to 0
        SRL_D = 0x3A,       //Shift n Right into Carry. MSB set to 0
        SRL_E = 0x3B,       //Shift n Right into Carry. MSB set to 0
        SRL_H = 0x3C,       //Shift n Right into Carry. MSB set to 0
        SRL_L = 0x3D,       //Shift n Right into Carry. MSB set to 0
        SRL_adrHL = 0x3E,   //Shift n Right into Carry. MSB set to 0
        SRL_A = 0x3F,       //Shift n Right into Carry. MSB set to 0
        BIT_1_B = 0x40,     //Test bit 1 in reg n
        BIT_1_C = 0x41,     //Test bit 1 in reg n
        BIT_1_D = 0x42,     //Test bit 1 in reg n
        BIT_1_E = 0x43,     //Test bit 1 in reg n
        BIT_1_H = 0x44,     //Test bit 1 in reg n
        BIT_1_L = 0x45,     //Test bit 1 in reg n
        BIT_1_adrHL = 0x46, //Test bit 1 in reg n
        BIT_1_A = 0x47,     //Test bit 1 in reg n
        BIT_2_B = 0x48,     //Test bit 2 in reg n
        BIT_2_C = 0x49,     //Test bit 2 in reg n
        BIT_2_D = 0x4A,     //Test bit 2 in reg n
        BIT_2_E = 0x4B,     //Test bit 2 in reg n
        BIT_2_H = 0x4C,     //Test bit 2 in reg n
        BIT_2_L = 0x4D,     //Test bit 2 in reg n
        BIT_2_adrHL = 0x4E, //Test bit 2 in reg n
        BIT_2_A = 0x4F,     //Test bit 2 in reg n
        BIT_3_B = 0x50,     //Test bit 3 in reg n
        BIT_3_C = 0x51,     //Test bit 3 in reg n
        BIT_3_D = 0x52,     //Test bit 3 in reg n
        BIT_3_E = 0x53,     //Test bit 3 in reg n
        BIT_3_H = 0x54,     //Test bit 3 in reg n
        BIT_3_L = 0x55,     //Test bit 3 in reg n
        BIT_3_adrHL = 0x56, //Test bit 3 in reg n
        BIT_3_A = 0x57,     //Test bit 3 in reg n
        BIT_4_B = 0x58,     //Test bit 4 in reg n
        BIT_4_C = 0x59,     //Test bit 4 in reg n
        BIT_4_D = 0x5A,     //Test bit 4 in reg n
        BIT_4_E = 0x5B,     //Test bit 4 in reg n
        BIT_4_H = 0x5C,     //Test bit 4 in reg n
        BIT_4_L = 0x5D,     //Test bit 4 in reg n
        BIT_4_adrHL = 0x5E, //Test bit 4 in reg n
        BIT_4_A = 0x5F,     //Test bit 4 in reg n
        BIT_5_B = 0x60,     //Test bit 5 in reg n
        BIT_5_C = 0x61,     //Test bit 5 in reg n
        BIT_5_D = 0x62,     //Test bit 5 in reg n
        BIT_5_E = 0x63,     //Test bit 5 in reg n
        BIT_5_H = 0x64,     //Test bit 5 in reg n
        BIT_5_L = 0x65,     //Test bit 5 in reg n
        BIT_5_adrHL = 0x66, //Test bit 5 in reg n
        BIT_5_A = 0x67,     //Test bit 5 in reg n
        BIT_6_B = 0x68,     //Test bit 6 in reg n
        BIT_6_C = 0x69,     //Test bit 6 in reg n
        BIT_6_D = 0x6A,     //Test bit 6 in reg n
        BIT_6_E = 0x6B,     //Test bit 6 in reg n
        BIT_6_H = 0x6C,     //Test bit 6 in reg n
        BIT_6_L = 0x6D,     //Test bit 6 in reg n
        BIT_6_adrHL = 0x6E, //Test bit 6 in reg n
        BIT_6_A = 0x6F,     //Test bit 6 in reg n
        BIT_7_B = 0x70,     //Test bit 7 in reg n
        BIT_7_C = 0x71,     //Test bit 7 in reg n
        BIT_7_D = 0x72,     //Test bit 7 in reg n
        BIT_7_E = 0x73,     //Test bit 7 in reg n
        BIT_7_H = 0x74,     //Test bit 7 in reg n
        BIT_7_L = 0x75,     //Test bit 7 in reg n
        BIT_7_adrHL = 0x76, //Test bit 7 in reg n
        BIT_7_A = 0x77,     //Test bit 7 in reg n
        BIT_8_B = 0x78,     //Test bit 8 in reg n
        BIT_8_C = 0x79,     //Test bit 8 in reg n
        BIT_8_D = 0x7A,     //Test bit 8 in reg n
        BIT_8_E = 0x7B,     //Test bit 8 in reg n
        BIT_8_H = 0x7C,     //Test bit 8 in reg n
        BIT_8_L = 0x7D,     //Test bit 8 in reg n
        BIT_8_adrHL = 0x7E, //Test bit 8 in reg n
        BIT_8_A = 0x7F,     //Test bit 8 in reg n
        RES_1_B = 0x80,     //Reset bit 1 in reg n
        RES_1_C = 0x81,     //Reset bit 1 in reg n
        RES_1_D = 0x82,     //Reset bit 1 in reg n
        RES_1_E = 0x83,     //Reset bit 1 in reg n
        RES_1_H = 0x84,     //Reset bit 1 in reg n
        RES_1_L = 0x85,     //Reset bit 1 in reg n
        RES_1_adrHL = 0x86, //Reset bit 1 in reg n
        RES_1_A = 0x87,     //Reset bit 1 in reg n
        RES_2_B = 0x88,     //Reset bit 2 in reg n
        RES_2_C = 0x89,     //Reset bit 2 in reg n
        RES_2_D = 0x8A,     //Reset bit 2 in reg n
        RES_2_E = 0x8B,     //Reset bit 2 in reg n
        RES_2_H = 0x8C,     //Reset bit 2 in reg n
        RES_2_L = 0x8D,     //Reset bit 2 in reg n
        RES_2_adrHL = 0x8E, //Reset bit 2 in reg n
        RES_2_A = 0x8F,     //Reset bit 2 in reg n
        RES_3_B = 0x90,     //Reset bit 3 in reg n
        RES_3_C = 0x91,     //Reset bit 3 in reg n
        RES_3_D = 0x92,     //Reset bit 3 in reg n
        RES_3_E = 0x93,     //Reset bit 3 in reg n
        RES_3_H = 0x94,     //Reset bit 3 in reg n
        RES_3_L = 0x95,     //Reset bit 3 in reg n
        RES_3_adrHL = 0x96, //Reset bit 3 in reg n
        RES_3_A = 0x97,     //Reset bit 3 in reg n
        RES_4_B = 0x98,     //Reset bit 4 in reg n
        RES_4_C = 0x99,     //Reset bit 4 in reg n
        RES_4_D = 0x9A,     //Reset bit 4 in reg n
        RES_4_E = 0x9B,     //Reset bit 4 in reg n
        RES_4_H = 0x9C,     //Reset bit 4 in reg n
        RES_4_L = 0x9D,     //Reset bit 4 in reg n
        RES_4_adrHL = 0x9E, //Reset bit 4 in reg n
        RES_4_A = 0x9F,     //Reset bit 4 in reg n
        RES_5_B = 0xA0,     //Reset bit 5 in reg n
        RES_5_C = 0xA1,     //Reset bit 5 in reg n
        RES_5_D = 0xA2,     //Reset bit 5 in reg n
        RES_5_E = 0xA3,     //Reset bit 5 in reg n
        RES_5_H = 0xA4,     //Reset bit 5 in reg n
        RES_5_L = 0xA5,     //Reset bit 5 in reg n
        RES_5_adrHL = 0xA6, //Reset bit 5 in reg n
        RES_5_A = 0xA7,     //Reset bit 5 in reg n
        RES_6_B = 0xA8,     //Reset bit 6 in reg n
        RES_6_C = 0xA9,     //Reset bit 6 in reg n
        RES_6_D = 0xAA,     //Reset bit 6 in reg n
        RES_6_E = 0xAB,     //Reset bit 6 in reg n
        RES_6_H = 0xAC,     //Reset bit 6 in reg n
        RES_6_L = 0xAD,     //Reset bit 6 in reg n
        RES_6_adrHL = 0xAE, //Reset bit 6 in reg n
        RES_6_A = 0xAF,     //Reset bit 6 in reg n
        RES_7_B = 0xB0,     //Reset bit 7 in reg n
        RES_7_C = 0xB1,     //Reset bit 7 in reg n
        RES_7_D = 0xB2,     //Reset bit 7 in reg n
        RES_7_E = 0xB3,     //Reset bit 7 in reg n
        RES_7_H = 0xB4,     //Reset bit 7 in reg n
        RES_7_L = 0xB5,     //Reset bit 7 in reg n
        RES_7_adrHL = 0xB6, //Reset bit 7 in reg n
        RES_7_A = 0xB7,     //Reset bit 7 in reg n
        RES_8_B = 0xB8,     //Reset bit 8 in reg n
        RES_8_C = 0xB9,     //Reset bit 8 in reg n
        RES_8_D = 0xBA,     //Reset bit 8 in reg n
        RES_8_E = 0xBB,     //Reset bit 8 in reg n
        RES_8_H = 0xBC,     //Reset bit 8 in reg n
        RES_8_L = 0xBD,     //Reset bit 8 in reg n
        RES_8_adrHL = 0xBE, //Reset bit 8 in reg n
        RES_8_A = 0xBF,     //Reset bit 8 in reg n
        SET_1_B = 0xC0,     //Set bit 1 in reg n
        SET_1_C = 0xC1,     //Set bit 1 in reg n
        SET_1_D = 0xC2,     //Set bit 1 in reg n
        SET_1_E = 0xC3,     //Set bit 1 in reg n
        SET_1_H = 0xC4,     //Set bit 1 in reg n
        SET_1_L = 0xC5,     //Set bit 1 in reg n
        SET_1_adrHL = 0xC6, //Set bit 1 in reg n
        SET_1_A = 0xC7,     //Set bit 1 in reg n
        SET_2_B = 0xC8,     //Set bit 2 in reg n
        SET_2_C = 0xC9,     //Set bit 2 in reg n
        SET_2_D = 0xCA,     //Set bit 2 in reg n
        SET_2_E = 0xCB,     //Set bit 2 in reg n
        SET_2_H = 0xCC,     //Set bit 2 in reg n
        SET_2_L = 0xCD,     //Set bit 2 in reg n
        SET_2_adrHL = 0xCE, //Set bit 2 in reg n
        SET_2_A = 0xCF,     //Set bit 2 in reg n
        SET_3_B = 0xD0,     //Set bit 3 in reg n
        SET_3_C = 0xD1,     //Set bit 3 in reg n
        SET_3_D = 0xD2,     //Set bit 3 in reg n
        SET_3_E = 0xD3,     //Set bit 3 in reg n
        SET_3_H = 0xD4,     //Set bit 3 in reg n
        SET_3_L = 0xD5,     //Set bit 3 in reg n
        SET_3_adrHL = 0xD6, //Set bit 3 in reg n
        SET_3_A = 0xD7,     //Set bit 3 in reg n
        SET_4_B = 0xD8,     //Set bit 4 in reg n
        SET_4_C = 0xD9,     //Set bit 4 in reg n
        SET_4_D = 0xDA,     //Set bit 4 in reg n
        SET_4_E = 0xDB,     //Set bit 4 in reg n
        SET_4_H = 0xDC,     //Set bit 4 in reg n
        SET_4_L = 0xDD,     //Set bit 4 in reg n
        SET_4_adrHL = 0xDE, //Set bit 4 in reg n
        SET_4_A = 0xDF,     //Set bit 4 in reg n
        SET_5_B = 0xE0,     //Set bit 5 in reg n
        SET_5_C = 0xE1,     //Set bit 5 in reg n
        SET_5_D = 0xE2,     //Set bit 5 in reg n
        SET_5_E = 0xE3,     //Set bit 5 in reg n
        SET_5_H = 0xE4,     //Set bit 5 in reg n
        SET_5_L = 0xE5,     //Set bit 5 in reg n
        SET_5_adrHL = 0xE6, //Set bit 5 in reg n
        SET_5_A = 0xE7,     //Set bit 5 in reg n
        SET_6_B = 0xE8,     //Set bit 6 in reg n
        SET_6_C = 0xE9,     //Set bit 6 in reg n
        SET_6_D = 0xEA,     //Set bit 6 in reg n
        SET_6_E = 0xEB,     //Set bit 6 in reg n
        SET_6_H = 0xEC,     //Set bit 6 in reg n
        SET_6_L = 0xED,     //Set bit 6 in reg n
        SET_6_adrHL = 0xEE, //Set bit 6 in reg n
        SET_6_A = 0xEF,     //Set bit 6 in reg n
        SET_7_B = 0xF0,     //Set bit 7 in reg n
        SET_7_C = 0xF1,     //Set bit 7 in reg n
        SET_7_D = 0xF2,     //Set bit 7 in reg n
        SET_7_E = 0xF3,     //Set bit 7 in reg n
        SET_7_H = 0xF4,     //Set bit 7 in reg n
        SET_7_L = 0xF5,     //Set bit 7 in reg n
        SET_7_adrHL = 0xF6, //Set bit 7 in reg n
        SET_7_A = 0xF7,     //Set bit 7 in reg n
        SET_8_B = 0xF8,     //Set bit 8 in reg n
        SET_8_C = 0xF9,     //Set bit 8 in reg n
        SET_8_D = 0xFA,     //Set bit 8 in reg n
        SET_8_E = 0xFB,     //Set bit 8 in reg n
        SET_8_H = 0xFC,     //Set bit 8 in reg n
        SET_8_L = 0xFD,     //Set bit 8 in reg n
        SET_8_adrHL = 0xFE, //Set bit 8 in reg n
        SET_8_A = 0xFF      //Set bit 8 in reg n
    };

private:
    union reg
    {
        uWORD data;
        struct
        {
            uBYTE lo;
            uBYTE hi;
        };

        void operator=(uWORD data)
        {
            this->data = data;
        }

        reg(int data)
        {
            this->data = data;
        }

        reg()
        {
            this->data = 0x0000;
        }
    };

    reg AF;
    reg BC;
    reg DE;
    reg HL;
    uWORD SP;
    uWORD PC;

    reg temp;

    bool IME;
    bool buggedHalt;
    int cyclesExecuted;
    int dividerRegisterCounter;
    Memory *memoryUnit;
    uBYTE byte;

    uWORD increment16BitRegister(uWORD);
    uWORD decrement16BitRegister(uWORD);
    uWORD add16BitRegister(uWORD, uWORD);
    uWORD twoComp_Word(uWORD);
    uBYTE increment8BitRegister(uBYTE);
    uBYTE decrement8BitRegister(uBYTE);
    uBYTE add8BitRegister(uBYTE, uBYTE);
    uBYTE add8BitRegister(uBYTE, uBYTE, bool);
    uBYTE sub8BitRegister(uBYTE, uBYTE);
    uBYTE sub8BitRegister(uBYTE, uBYTE, bool);
    uBYTE and8BitRegister(uBYTE, uBYTE);
    uBYTE xor8BitRegister(uBYTE, uBYTE);
    uBYTE or8BitRegister(uBYTE, uBYTE);
    uBYTE twoComp_Byte(uBYTE);
    uBYTE rotateReg(bool, bool, uBYTE);
    uBYTE rotateRegExt(bool, bool, uBYTE);
    uBYTE shiftReg(bool, bool, uBYTE);
    uBYTE swapReg(uBYTE);
    uBYTE resetBit(int, uBYTE);
    uBYTE setBit(int, uBYTE);
    uBYTE adjustDAA(uBYTE);
    void cmp8BitRegister(uBYTE, uBYTE);
    void flagSet(int);
    void testBit(int, uBYTE);
    void flagReset(int);
    bool flagTest(int);
    bool testBitInByte(uBYTE, int);
    bool testBitInWord(uWORD, int);
    bool checkCarryFromBit_Byte(int, uBYTE, uBYTE);
    bool checkCarryFromBit_Byte(int, uBYTE, uBYTE, uBYTE);
    bool checkCarryFromBit_Word(int, uWORD, uWORD);
    bool checkCarryFromBit_Word(int, uWORD, uWORD, uWORD);
    bool checkBorrowFromBit_Byte(int, uBYTE, uBYTE);
    bool checkBorrowFromBit_Byte(int, uBYTE, uBYTE, uBYTE);
    bool checkBorrowFromBit_Word(int, uWORD, uWORD);
    bool checkBorrowFromBit_Word(int, uWORD, uWORD, uWORD);
};

#endif
