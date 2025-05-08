#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"

// Definición de las funciones de ejecución de instrucciones
void execute_hlt(uint32_t instruction);
void execute_adds(uint32_t instruction);
void execute_addis_0(uint32_t instruction);
void execute_addis_1(uint32_t instruction);
void execute_subs(uint32_t instruction);
void execute_subis_0(uint32_t instruction);
void execute_subis_1(uint32_t instruction);
void execute_ands(uint32_t instruction);
void execute_eor(uint32_t instruction);
void execute_orr(uint32_t instruction);
void execute_b(uint32_t instruction);
void execute_br(uint32_t instruction);
void execute_b_cond(uint32_t instruction);
void execute_lsl(uint32_t instruction);
void execute_lsr(uint32_t instruction);
void execute_stur(uint32_t instruction);
void execute_sturb(uint32_t instruction);
void execute_sturh(uint32_t instruction);
void execute_ldur(uint32_t instruction);
void execute_ldurb(uint32_t instruction);
void execute_ldurh(uint32_t instruction);
void execute_movz(uint32_t instruction);
void execute_add(uint32_t instruction);
void execute_mul(uint32_t instruction);
void execute_cbz(uint32_t instruction);
void execute_cbnz(uint32_t instruction);

// OPCODES
#define OPCODE_HLT      0x6A2  // HLT
#define OPCODE_ADDS     0x558  // ADDS (register)
#define OPCODE_ADDIS_0  0x588  // ADDS (immediate). SHIFT=000
#define OPCODE_ADDIS_1  0x58A  // ADDS (immediate). SHIFT=010
#define OPCODE_SUBS     0x758  // SUBS (register). CMP SI RD=XZR
#define OPCODE_SUBIS_0  0x788  // SUBIS (immediate). SHIFT=000. CMPI SI RD=XZR
#define OPCODE_SUBIS_1  0x78A  // SUBIS (immediate). SHIFT=010. CMPI SI RD=XZR
#define OPCODE_ANDS     0x750  // ANDS (shifted register)
#define OPCODE_EOR      0x650  // EOR (shifted register)
#define OPCODE_ORR      0x550  // ORR (shifted register)
#define OPCODE_B        0x5    // B
#define OPCODE_BR       0x6B0  // BR
#define OPCODE_BCOND    0x54   // B.COND
#define OPCODE_LSL      0x69B  // LSL (immediate)
#define OPCODE_LSR      0x69A  // LSR (immediate)
#define OPCODE_STUR     0x7C0  // STUR
#define OPCODE_STURB    0x1C0  // STURB
#define OPCODE_STURH    0x3C0  // STURH
#define OPCODE_LDUR     0x7C2  // LDUR
#define OPCODE_LDURB    0x1C2  // LDURB
#define OPCODE_LDURH    0x3C2  // LDURH
#define OPCODE_MOVZ     0x694  // MOVZ
#define OPCODE_ADD      0x458  // ADD (register)
#define OPCODE_ADDI_0   0x488  // ADD (immediate). SHIFT=000
#define OPCODE_ADDI_1   0x48A  // ADD (immediate). SHIFT=010
#define OPCODE_MUL      0x4D8  // MUL
#define OPCODE_CBZ      0xB4   // CBZ
#define OPCODE_CBNZ     0xB5   // CBNZ

void process_instruction()
{
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    printf("Instrucción: 0x%x\n", instruction);

    // Defino la recuperación de bits para diferentes tamaños de opcode
    uint32_t opcode_11 = (instruction >> 21) & 0x7FF;
    uint32_t opcode_6 = (instruction >> 26) & 0x3F;
    uint32_t opcode_8 = (instruction >> 24) & 0x7F;
    printf("Opcode 11 bits: 0x%x\n", opcode_11);
    printf("Opcode 6 bits: 0x%x\n", opcode_6);
    printf("Opcode 8 bits: 0x%x\n", opcode_8);
    
    switch(opcode_11) {
        case OPCODE_HLT:
            execute_hlt(instruction);
            return;
        case OPCODE_ADDS:
            printf("ADDS\n");
            execute_adds(instruction);
            return;
        case OPCODE_ADDIS_0:
            printf("ADDIS (SHIFT=000)\n");
            execute_addis_0(instruction);
            return;
        case OPCODE_ADDIS_1:
            printf("ADDIS (SHIFT=010)\n");
            execute_addis_1(instruction);
            return;
        case OPCODE_SUBS:
            printf("SUBS\n");
            execute_subs(instruction);
            return;
        case OPCODE_SUBIS_0:
            printf("SUBIS (SHIFT=000)\n");
            execute_subis_0(instruction);
            return;
        case OPCODE_SUBIS_1:
            printf("SUBIS (SHIFT=010)\n");
            execute_subis_1(instruction);
            return;
        case OPCODE_ANDS:
            printf("ANDS\n");
            execute_ands(instruction);
            return;
        case OPCODE_EOR:
            printf("EOR\n");
            execute_eor(instruction);
            return;
        case OPCODE_ORR:
            printf("ORR\n");
            execute_orr(instruction);
            return;
        case OPCODE_LSL:
            printf("LSL\n");
            execute_lsl(instruction);
            return;
        case OPCODE_LSR:
            printf("LSR\n");
            execute_lsr(instruction);
            return;
        case OPCODE_MOVZ:
            printf("MOVZ\n");
            execute_movz(instruction);
            return;
        case OPCODE_BR:
            printf("BR\n");
            execute_br(instruction);
            return;
        case OPCODE_STUR:
            printf("STUR\n");
            execute_stur(instruction);
            return;
        case OPCODE_STURB:
            printf("STURB\n");
            execute_sturb(instruction);
            return;
        case OPCODE_STURH:
            printf("STURH\n");
            execute_sturh(instruction);
            return;
        case OPCODE_LDUR:
            printf("LDUR\n");
            execute_ldur(instruction);
            return;
        case OPCODE_LDURB:
            printf("LDURB\n");
            execute_ldurb(instruction);
            return;
        case OPCODE_LDURH:
            printf("LDURH\n");
            execute_ldurh(instruction);
            return;
    }

    switch (opcode_6) {
        case OPCODE_B:
            printf("B\n");
            execute_b(instruction);
            return;
    }

    switch (opcode_8) {
        case OPCODE_CBZ:
            printf("CBZ\n");
            execute_cbz(instruction);
            break;
        case OPCODE_CBNZ:
            printf("CBNZ\n");
            execute_cbnz(instruction);
            break;
        case OPCODE_BCOND:
            printf("B.COND\n");
            execute_b_cond(instruction);
            break;
        default:
            printf("Instrucción 0x%x no reconocida\n", instruction);
            RUN_BIT = 0;
            break;
    }
}


// Implementación de las funciones de ejecución de instrucciones
void execute_hlt(uint32_t instruction)
{
    printf("HLT\n");
    RUN_BIT = 0;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_adds(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int rm = (instruction >> 16) & 0x1F;

    int64_t val_n = (rn == 31) ? 0 : CURRENT_STATE.REGS[rn];
    int64_t val_m = (rm == 31) ? 0 : CURRENT_STATE.REGS[rm];
    int64_t result = val_n + val_m;

    NEXT_STATE.FLAG_N = (result < 0);
    NEXT_STATE.FLAG_Z = (result == 0);

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = result;
    }
    
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_addis_0(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int imm = (instruction >> 10) & 0xFFF;

    int64_t val_n = (rn == 31) ? 0 : CURRENT_STATE.REGS[rn];
    int64_t result = val_n + imm;

    NEXT_STATE.FLAG_N = (result < 0);
    NEXT_STATE.FLAG_Z = (result == 0);

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_addis_1(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int imm = (instruction >> 10) & 0xFFF;

    int64_t val_n = (rn == 31) ? 0 : CURRENT_STATE.REGS[rn];
    int64_t result = val_n + (imm << 12);

    NEXT_STATE.FLAG_N = (result < 0);
    NEXT_STATE.FLAG_Z = (result == 0);

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_subs(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int rm = (instruction >> 16) & 0x1F;

    int64_t val_n = (rn == 31) ? 0 : CURRENT_STATE.REGS[rn];
    int64_t val_m = (rm == 31) ? 0 : CURRENT_STATE.REGS[rm];
    int64_t result = val_n - val_m;

    NEXT_STATE.FLAG_N = (result < 0);
    NEXT_STATE.FLAG_Z = (result == 0);

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_subis_0(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int imm = (instruction >> 10) & 0xFFF;

    int64_t val_n = (rn == 31) ? 0 : CURRENT_STATE.REGS[rn];
    int64_t result = val_n - imm;

    NEXT_STATE.FLAG_N = (result < 0);
    NEXT_STATE.FLAG_Z = (result == 0);

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_subis_1(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int imm = (instruction >> 10) & 0xFFF;

    int64_t val_n = (rn == 31) ? 0 : CURRENT_STATE.REGS[rn];
    int64_t result = val_n - (imm << 12);

    NEXT_STATE.FLAG_N = (result < 0);
    NEXT_STATE.FLAG_Z = (result == 0);

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_ands(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int rm = (instruction >> 16) & 0x1F;

    int64_t val_n = (rn == 31) ? 0 : CURRENT_STATE.REGS[rn];
    int64_t val_m = (rm == 31) ? 0 : CURRENT_STATE.REGS[rm];
    int64_t result = val_n & val_m;

    NEXT_STATE.FLAG_N = (result < 0);
    NEXT_STATE.FLAG_Z = (result == 0);

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_eor(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int rm = (instruction >> 16) & 0x1F;

    int64_t val_n = (rn == 31) ? 0 : CURRENT_STATE.REGS[rn];
    int64_t val_m = (rm == 31) ? 0 : CURRENT_STATE.REGS[rm];
    int64_t result = val_n ^ val_m;

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_orr(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int rm = (instruction >> 16) & 0x1F;

    int64_t val_n = (rn == 31) ? 0 : CURRENT_STATE.REGS[rn];
    int64_t val_m = (rm == 31) ? 0 : CURRENT_STATE.REGS[rm];
    int64_t result = val_n | val_m;

    NEXT_STATE.FLAG_N = (result < 0);
    NEXT_STATE.FLAG_Z = (result == 0);

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

int64_t sign_extend(int32_t value, int bits) {
    int64_t mask = (int64_t)1 << (bits - 1);
    return (int64_t)(value ^ mask) - mask;
}

void execute_b(uint32_t instruction)
{
    int imm26 = instruction & 0x3FFFFFF;
    int64_t offset = sign_extend(imm26 << 2, 28);

    if (instruction != 31) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_br(uint32_t instruction)
{ 
    uint32_t rn = instruction >> 5 & 0x1F;

    if (rn != 31) {
        NEXT_STATE.PC = CURRENT_STATE.REGS[rn];
    } else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_b_cond(uint32_t instruction)
{
    int cond = instruction & 0xF;
    int imm19 = (instruction >> 5) & 0x7FFFF;

    int64_t offset = sign_extend(imm19 << 2, 21);

    // Verifica la condición unicamente para BEQ, BNE, BGT, BLT, BGE, BLE
    switch (cond) {
        case 0x0: // BEQ
            if (CURRENT_STATE.FLAG_Z) {
                printf("Cond: BEQ\n");
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
            }
            break;
        case 0x1:  // BNE
            if (!CURRENT_STATE.FLAG_Z) {
                printf("Cond: BNE\n");
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
            }
            break;
        case 0xC:  // BGT
            if (!CURRENT_STATE.FLAG_Z && !CURRENT_STATE.FLAG_N) {       // asumimos flag V=0 por enunciado.
                printf("Cond: BGT\n");
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
            }
            break;
        case 0xB:  // BLT
            if (CURRENT_STATE.FLAG_N) {                                 // asumimos flag V=0 por enunciado.
                printf("Cond: BLT\n");
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
            }
            break;
        case 0xA:  // BGE
            if (!CURRENT_STATE.FLAG_N) {                                // asumimos flag V=0 por enunciado.
                printf("Cond: BGE\n");
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
            }
            break;
        case 0xD:  // BLE
            if (!(!CURRENT_STATE.FLAG_Z && !CURRENT_STATE.FLAG_N)) {    // asumimos flag V=0 por enunciado.
                printf("Cond: BLE\n");
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
            }
            break;
        default:
            printf("No cumple ninguna condición\n");
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }
}

void execute_lsl(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int imms = (instruction >> 10) & 0x3F;

    int shift = 63 - imms;

    int64_t val_n = (rn == 31) ? 0 : CURRENT_STATE.REGS[rn];
    int64_t result = val_n << shift;

    NEXT_STATE.FLAG_N = (result < 0);
    NEXT_STATE.FLAG_Z = (result == 0);

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}


void execute_lsr(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int immr = (instruction >> 16) & 0x3F;

    int64_t val_n = (rn == 31) ? 0 : CURRENT_STATE.REGS[rn];
    int64_t result = val_n >> immr;

    NEXT_STATE.FLAG_N = (result < 0);
    NEXT_STATE.FLAG_Z = (result == 0);

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_stur(uint32_t instruction) {
    int rt = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int imm9 = (instruction >> 12) & 0x1FF;
    imm9 = (imm9 << 23) >> 23;

    uint64_t addr = CURRENT_STATE.REGS[rn] + imm9;

    mem_write_32(addr, (uint32_t)CURRENT_STATE.REGS[rt] & 0xFFFFFFFF);
    mem_write_32(addr + 4, (uint32_t)(CURRENT_STATE.REGS[rt] >> 32));

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_sturb(uint32_t instruction) {
    int rt = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int imm9 = (instruction >> 12) & 0x1FF;
    imm9 = (imm9 << 23) >> 23;

    uint64_t addr = CURRENT_STATE.REGS[rn] + imm9;

    uint32_t data = CURRENT_STATE.REGS[rt] & 0xFF;
    mem_write_32(addr, data);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_sturh(uint32_t instruction) {
    int rt = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int imm9 = (instruction >> 12) & 0x1FF;
    imm9 = (imm9 << 23) >> 23;

    uint64_t addr = CURRENT_STATE.REGS[rn] + imm9;
    uint64_t half = CURRENT_STATE.REGS[rt] & 0xFFFF;

    uint32_t value = mem_read_32(addr);
    value = (value & 0xFFFF0000) | half;
    mem_write_32(addr, value);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_ldur(uint32_t instruction) {
    int rt = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int imm9 = (instruction >> 12) & 0x1FF;
    imm9 = (imm9 << 23) >> 23;

    uint64_t addr = CURRENT_STATE.REGS[rn] + imm9;

    uint64_t start = mem_read_32(addr);
    uint64_t end = mem_read_32(addr + 4);
    NEXT_STATE.REGS[rt] = (end << 32) | start;
    
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_ldurb(uint32_t instruction) {
    int rt = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int imm9 = (instruction >> 12) & 0x1FF;
    imm9 = (imm9 << 23) >> 23;

    uint64_t addr = CURRENT_STATE.REGS[rn] + imm9;
    uint64_t word = mem_read_32(addr & ~0x3);
    uint8_t byte = (word >> ((addr & 0x3) * 8)) & 0xFF;

    NEXT_STATE.REGS[rt] = (uint64_t)byte;

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_ldurh(uint32_t instruction) {
    int rt = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int imm9 = (instruction >> 12) & 0x1FF;
    imm9 = (imm9 << 23) >> 23;

    uint64_t addr = CURRENT_STATE.REGS[rn] + imm9;
    uint64_t word = mem_read_32(addr & ~0x3);
    uint16_t half = (word >> ((addr & 0x3) * 8)) & 0xFFFF;

    NEXT_STATE.REGS[rt] = (uint64_t)half;

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_movz(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int imm = (instruction >> 5) & 0xFFFF;

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = imm;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_add(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int rm = (instruction >> 16) & 0x1F;

    int64_t val_n = (rn == 31) ? 0 : CURRENT_STATE.REGS[rn];
    int64_t val_m = (rm == 31) ? 0 : CURRENT_STATE.REGS[rm];
    int64_t result = val_n + val_m;

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_mul(uint32_t instruction)
{
    int rd = instruction & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int rm = (instruction >> 16) & 0x1F;

    int64_t val_n = (rn == 31) ? 0 : CURRENT_STATE.REGS[rn];
    int64_t val_m = (rm == 31) ? 0 : CURRENT_STATE.REGS[rm];
    int64_t result = val_n * val_m;

    if (rd != 31) {
        NEXT_STATE.REGS[rd] = result;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void execute_cbz(uint32_t instruction)
{
    int rt = instruction & 0x1F;
    int32_t imm = (instruction >> 5) & 0x7FFFF;

    // Verifica si el imm es positivo o negativo y hace el sign-extend
    if (imm & (1 << 18)) {
        imm |= ~0x7FFFF;
    }

    int64_t offset = ((int64_t)imm << 2);

    if (CURRENT_STATE.REGS[rt] == 0) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}

void execute_cbnz(uint32_t instruction)
{
    int rt = instruction & 0x1F;
    int32_t imm = (instruction >> 5) & 0x7FFFF;

    // Verifica si el imm es positivo o negativo y hace el sign-extend
    if (imm & (1 << 18)) {
        imm |= ~0x7FFFF;
    }

    int64_t offset = ((int64_t)imm << 2);

    if (CURRENT_STATE.REGS[rt] != 0) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}