// SIMPLE 6502 EMULATOR
//
// Complete opcode overview at https://www.pagetable.com/c64ref/6502/?tab=2
//
// CPU implementation
// ------------------
// all registers and flags implemented
// decimal mode, stack, timing/cycles and interrupt routines not implemented
//
// Opcode implementation table
// ---------------------------
// $00  BRK           works
// $81  STA ($vw,X)   under construction
// $85  STA  $vw      works
// $8D  STA  $vwxy    under construction
// $91  STA ($xy),Y   under construction
// $95  STA  $vx,X    works
// $99  STA  $vwxy,Y  under construction
// $9D  STA  $vwxy,X  under construction
// $A0  LDY #$xy      works
// $A1  LDA ($xy,X)   works
// $A2  LDX #$xy      works
// $A4  LDY  $xy      works
// $A5  LDA  $xy      works
// $A6  LDX  $xy      works
// $A9  LDA #$xy      works
// $AC  LDY  $vwxy    works
// $AD  LDA  $vwxy    works
// $AE  LDX  $vwxy    works
// $B1  LDA ($xy),Y   works
// $B4  LDY  $xy,X    works
// $B5  LDA  $xy,X    works
// $B6  LDX  $xy,Y    works
// $B9  LDA  $vwxy,Y  works
// $BC  LDY  $vwxy,X  works
// $BD  LDA  $vwxy,X  works
// $BE  LDX  $vwxy,Y  works
//
// Next opcodes to be implemented
// ------------------------------
// $84  STY  $xy      under construction
// $86  STX  $vw      under construction
// $8C  STY  $vwxy    under construction
// $8E  STX  $vwxy    under construction
// $94  STY  $vw,X    under construction
// $96  STX  $vw,Y    under construction
//
// Next features to be implemented
// -------------------------------
// basic debugging (set breakpoint/s)
// read program data from binary file
//
// Checks: do zeropage addresses properly wrap around?
// ------  when should flags be cleared?

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>                                         // for uint8_t and uint16_t data types
#include <stdlib.h>

#define SHOW_PROCESSED_DATA   true
#define SHOW_PROCESSOR_STATUS true

#define MEMORY_SIZE 65536                                   // 64 KB memory

#define FLAG_N 0x80                                         // N (negative) flag           1000 0000
#define FLAG_V 0x40                                         // V (overflow) flag           0100 0000
#define FLAG_U 0x20                                         // U (unused) flag             0010 0000
#define FLAG_B 0x10                                         // B (break command) flag      0001 0000
#define FLAG_D 0x08                                         // D (decimal mode) flag       0000 1000
#define FLAG_I 0x04                                         // I (interrupt disable) flag  0000 0100
#define FLAG_Z 0x02                                         // Z (zero) flag               0000 0010
#define FLAG_C 0x01                                         // C (carry) flag              0000 0001

typedef struct {                                            // define CPU registers
    uint8_t  A, X, Y;                                       // A (accumulator), X register, Y register
    uint8_t  SP;                                            // stack pointer
    uint16_t PC;                                            // program counter, 2 byte
    uint8_t  SR;                                            // status register, 1 bit for each flag:
                                                            // N (negative), V (overflow), U (undefined), B (break interrput),
                                                            // D (decimal mode), I (interrupt disable), Z (zero), C (carry)
} CPU6502;

void reset_cpu(CPU6502 *cpu);
void enter_code(uint8_t memory[MEMORY_SIZE]);
uint8_t get_byte(CPU6502* cpu, uint8_t memory[MEMORY_SIZE]);        // under construction
void execute_command(CPU6502 *cpu, uint8_t memory[MEMORY_SIZE]);
void show_cpu_status(CPU6502 cpu);
void show_memory_dump(uint16_t start, uint16_t end, uint8_t memory[MEMORY_SIZE]);

bool check_flag(uint8_t SR, uint8_t flag);
void update_flag(uint8_t *SR, uint8_t flag, bool set);

void lda(CPU6502 *cpu, uint8_t memory[MEMORY_SIZE], uint8_t opcode);
void ldx(CPU6502 *cpu, uint8_t memory[MEMORY_SIZE], uint8_t opcode);
void ldy(CPU6502 *cpu, uint8_t memory[MEMORY_SIZE], uint8_t opcode);

void sta(CPU6502* cpu, uint8_t memory[MEMORY_SIZE], uint8_t opcode);

int main() {
    CPU6502 cpu;                                            // define CPU
    uint8_t memory[MEMORY_SIZE] = {0};                      // define memory and fill with zeros

    printf("SIMPLE\n\n    ###   #####    ####    #####\n   ##  #  ##      ##  ##  #    ##\n  ##      ##      ##  ##       ##\n  #####   #####   ##  ##      ##\n  ##  ##      ##  ##  ##     ##\n  ##  ##  #   ##  ##  ##    ##  #\n   ####    ####    ####   #######\n\n                           EMULATOR\n\n");

    reset_cpu(&cpu);                                        // reset CPU to initial settings
    printf("Initial CPU status after reset:\n");
    show_cpu_status(cpu);

    enter_code(memory);                                     // load test scenario

    do {                                                    // main loop,
        execute_command(&cpu, memory);
        if(SHOW_PROCESSOR_STATUS) {
            show_cpu_status(cpu);
        }
    } while(!check_flag(cpu.SR, FLAG_B));                   // exited if B flag has been set

    if(SHOW_PROCESSED_DATA && !SHOW_PROCESSOR_STATUS) {
        printf("\n");
    }
    printf("B flag has been set, program terminated. Final CPU status:\n\n");
    show_cpu_status(cpu);
    show_memory_dump(0XEE, 0XEE, memory);
    return 0;
}

void reset_cpu(CPU6502 *cpu) {
    cpu->A  = 0x00;                                         // A, X, Y set to 0
    cpu->X  = 0x00;
    cpu->Y  = 0x00;
    cpu->SP = 0xFD;                                         // set stack pointer to standard value
    cpu->PC = 0xFFFC;                                       // set PC to reset vector
                                                            // (usually, values at FFFC/FFFD (low/high) would be loaded into PC)
    cpu->SR = 0x24;                                         // set default flags; 0x24 = 0010 0100: disables interrupts after reset
}

void execute_command(CPU6502 *cpu, uint8_t memory[MEMORY_SIZE]) {
    if(SHOW_PROCESSED_DATA) {
        printf(".%04X  ", cpu->PC);
    }
    uint8_t opcode = get_byte(cpu, memory);

    switch(opcode) {
        case 0x00:                                          // BRK
            update_flag(&(cpu->SR), FLAG_B, true);
            break;
        case 0xA1:                                          // LDA ($xy,X)
        case 0xA5:                                          // LDA  $xy
        case 0xA9:                                          // LDA #$xy
        case 0xAD:                                          // LDA  $vwxy
        case 0xB1:                                          // LDA ($xy),Y
        case 0xB5:                                          // LDA  $xy,X
        case 0xB9:                                          // LDA  $vwxy,Y
        case 0xBD:                                          // LDA  $vwxy,X
            lda(cpu, memory, opcode);
            break;
        case 0xA2:                                          // LDX #$xy
        case 0xA6:                                          // LDX  $xy
        case 0xAE:                                          // LDX  $vwxy
        case 0xB6:                                          // LDX  $xy,Y
        case 0xBE:                                          // LDX  $vwxy,Y
            ldx(cpu, memory, opcode);
            break;
        case 0xA0:                                          // LDY #$xy
        case 0xA4:                                          // LDY  $xy
        case 0xAC:                                          // LDY  $vwxy
        case 0xB4:                                          // LDY  $xy,X
        case 0xBC:                                          // LDY  $vwxy,X
            ldy(cpu, memory, opcode);
            break;
        case 0X81:                                          // STA ($vw,X)
        case 0X85:                                          // STA  $vw
        case 0X8D:                                          // STA  $vwxy
        case 0X91:                                          // STA ($vw),Y
        case 0x95:                                          // STA  $vx,X
        case 0X99:                                          // STA  $vwxy,Y
        case 0X9D:                                          // STA  $vwxy,X
            sta(cpu, memory, opcode);
            break;
        default:
            printf("Unknown opcode %02X.\n", opcode);
            break;
    }
    if(SHOW_PROCESSED_DATA && SHOW_PROCESSOR_STATUS) {
        printf("\n\n");
    } else {
        if(SHOW_PROCESSED_DATA) {
            printf("\n");
        }
    }
}

void show_cpu_status(CPU6502 cpu) {
    printf(" A: %02X  |   X: %02X  |   Y: %02X    |  NV-BDIZC\n", cpu.A, cpu.X, cpu.Y);
    printf("SP: %02X  |  SR: %02X  |  PC: %04X  |  %d%d%d%d%d%d%d%d\n\n", cpu.SP, cpu.SR, cpu.PC, check_flag(cpu.SR, FLAG_N), check_flag(cpu.SR, FLAG_V), check_flag(cpu.SR, FLAG_U), check_flag(cpu.SR, FLAG_B), check_flag(cpu.SR, FLAG_D), check_flag(cpu.SR, FLAG_I), check_flag(cpu.SR, FLAG_Z), check_flag(cpu.SR, FLAG_C));
}

bool check_flag(uint8_t SR, uint8_t flag) {                 // check if flag has been set (by doing a bitwise AND
    return (SR & flag) != 0;                                // and comparing to "NOT 0" to get a bool value)
}

void update_flag(uint8_t *SR, uint8_t flag, bool set) {     // update a flag
    if(set) {                                               // set flag by doing a bitwise OR with 1
        *SR |= flag;
    } else {                                                // clear flag by doing a bitwise AND with an inverted bit mask
        *SR &= ~flag;
    }
}

void lda(CPU6502 *cpu, uint8_t memory[MEMORY_SIZE], uint8_t opcode) {
    uint8_t temp_address_low, temp_address_high;
    uint16_t actual_word_address;
    
    switch(opcode) {
        case 0xA1:                                          // A1: LDA ($xy,X)
            // add X to one-byte address and get temp_address as a zeropage address as a pointer to low byte;
            // high byte of destination is stored at (temp_address + 1); if temp is FF and X=1, high byte will be at 00
            temp_address_low = get_byte(cpu, memory) + cpu->X;    // calculate temp address;
            temp_address_high = temp_address_low + 1;       // uint8_t data type guarantees address will "wrap around" $FF
            actual_word_address = memory[temp_address_low] | 
            (memory[(temp_address_high)] << 8);             // calculate two-byte address: shift wrapped high byte, blend with low byte
            cpu->A = memory[actual_word_address];
            break;
        case 0xA5:                                          // A5: LDA  $xy
            cpu->A = memory[get_byte(cpu, memory)];         // use operand as one-byte pointer
            break;
        case 0xA9:                                          // A9: LDA #$xy
            cpu->A = get_byte(cpu, memory);
            break;
        case 0xAD:                                          // AD: LDA  $vwxy
            // shift high byte to left and blend with low byte to form address:
            cpu->A = memory[get_byte(cpu, memory) | (get_byte(cpu, memory) << 8)];
            break;
        case 0xB1:                                          // B1: LDA ($xy),Y
            // "In indirect indexed addressing, the second byte of the instruction points to a memory location in page zero. The contents of this memory location is added to the contents of the Y index register, the result being the low order eight bits of the effective address. The carry from this addition is added to the contents of the next page zero memory location, the result being the high order eight bits of the effective address."
            temp_address_low = memory[get_byte(cpu, memory)] + cpu->Y;
            temp_address_high = temp_address_low + 1;       // uint8_t data type guarantees address will "wrap around" $FF
            actual_word_address = memory[temp_address_low] | 
            (memory[(temp_address_high)] << 8);             // calculate two-byte address: shift wrapped high byte, blend with low byte
            cpu->A = memory[actual_word_address];
            break;
        case 0xB5:                                          // B5: LDA  $xy,X
            cpu->A = memory[get_byte(cpu, memory) + cpu->X];       // uint8_t automatically wraps around
            break;
        case 0xB9:                                          // B9: LDA  $vwxy,Y
            // shift high byte to left and blend with low byte to form address:
            cpu->A = memory[(get_byte(cpu, memory) | (get_byte(cpu, memory) << 8)) + cpu->Y];
            break;
        case 0xBD:                                          // BD: LDA  $vwxy,X
            // shift high byte to left and blend with low byte to form address:
            cpu->A = memory[(get_byte(cpu, memory) | (get_byte(cpu, memory) << 8)) + cpu->X];
            break;
    }
    update_flag(&(cpu->SR), FLAG_Z, cpu->A == 0);           // set/clear Z flag depending on A == 0
    update_flag(&(cpu->SR), FLAG_N, cpu->A & 0x80);         // set/clear N flag depending on highest bit; 0x80 = 1000 0000
}

void ldx(CPU6502 *cpu, uint8_t memory[MEMORY_SIZE], uint8_t opcode) {
    switch(opcode) {
        case 0xA2:                                          // A2: LDX #$xy
            cpu->X = get_byte(cpu, memory);                 // copy current value to X
            break;
        case 0xA6:                                          // A6: LDX  $xy
            cpu->X = memory[get_byte(cpu, memory)];         // use operand as one-byte pointer
            break;
        case 0xAE:                                          // AE: LDX  %vwxy
            // shift high byte to left and blend with low byte to form address:
            cpu->X = memory[get_byte(cpu, memory) | (get_byte(cpu, memory) << 8)];
            break;
        case 0xB6:                                          // B6: LDX  $xy,Y
            cpu->X = memory[get_byte(cpu, memory) + cpu->Y];       // uint8_t automatically wraps around
            break;
        case 0xBE:                                          // BE: LDX  vwxy,Y
            // shift high byte to left and blend with low byte to form address:
            cpu->X = memory[(get_byte(cpu, memory) | (get_byte(cpu, memory) << 8)) + cpu->Y];
            break;
    }
    update_flag(&(cpu->SR), FLAG_Z, cpu->X == 0);           // set/clear Z flag depending on Y == 0
    update_flag(&(cpu->SR), FLAG_N, cpu->X & 0x80);         // set/clear N flag depending on highest bit; 0x80 = 1000 0000
}

void ldy(CPU6502 *cpu, uint8_t memory[MEMORY_SIZE], uint8_t opcode) {
    switch(opcode) {
        case 0xA0:                                          // A2: LDY #$xy
            cpu->Y = get_byte(cpu, memory);
            break;
        case 0xA4:                                          // A4: LDY  $xy
            cpu->Y = memory[get_byte(cpu, memory)];         // use operand as one-byte pointer
            break;
        case 0xAC:                                          // AC: LDY  $vwxy
            // shift high byte to left and blend with low byte to form address:
            cpu->Y = memory[get_byte(cpu, memory) | (get_byte(cpu, memory) << 8)];
            break;
        case 0xB4:                                          // B4: LDY  $xy,X
            cpu->Y = memory[get_byte(cpu, memory) + cpu->X];       // uint8_t automatically wraps around
            break;
        case 0xBC:                                          // BC: LDY  $vwxy,X
            // shift high byte to left and blend with low byte to form address:
            cpu->Y = memory[(get_byte(cpu, memory) | (get_byte(cpu, memory) << 8)) + cpu->X];
            break;
    }
    update_flag(&(cpu->SR), FLAG_Z, cpu->Y == 0);           // set/clear Z flag depending on Y == 0
    update_flag(&(cpu->SR), FLAG_N, cpu->Y & 0x80);         // set/clear N flag depending on highest bit; 0x80 = 1000 0000
}

void enter_code(uint8_t memory[MEMORY_SIZE]) {

    // Test case for LDA/LDX/LDY: program code

    memory[0xFFFC] = 0xA9;   // LDA #$FF
    memory[0xFFFD] = 0xFF;   //     A9 FF
    memory[0xFFFE] = 0xA5;   // LDA $22
    memory[0xFFFF] = 0x22;   //     A9 22
    memory[0x0000] = 0xAD;   // LDA $1234
    memory[0x0001] = 0x34;   //     AC 34 12
    memory[0x0002] = 0x12;   //
    memory[0x0003] = 0xA2;   // LDX #$05
    memory[0x0004] = 0x05;   //     A2 05
    memory[0x0005] = 0xBD;   // LDA $1234,X
    memory[0x0006] = 0x34;   //     BD 34 12
    memory[0x0007] = 0x12;   //
    memory[0x0008] = 0xB5;   // LDA $FF,X
    memory[0x0009] = 0xFF;   //     B5 FF
    memory[0x000A] = 0xA1;   // LDA ($02,X)
    memory[0x000B] = 0x02;   //     A1 02
    memory[0x000C] = 0xA0;   // LDY #$03
    memory[0x000D] = 0x03;   //     A0 03
    memory[0x000E] = 0xB9;   // LDA $3456,Y
    memory[0x000F] = 0x56;   //     B9 56 34
    memory[0x0010] = 0x34;   //
    memory[0x0011] = 0xB1;   // LDA $05,Y
    memory[0x0012] = 0x05;   //
    memory[0x0013] = 0xA6;   // LDX $00
    memory[0x0014] = 0x00;   //     A6 00
    memory[0x0015] = 0xAE;   // LDX $1234
    memory[0x0016] = 0x34;   //     AE 34 12
    memory[0x0017] = 0x12;   //
    memory[0x0018] = 0xB6;   // LDX $05,Y
    memory[0x0019] = 0x05;   //     B6 05
    memory[0x001A] = 0xBE;   // LDX $1231,Y
    memory[0x001B] = 0x31;   //     BE 31 12
    memory[0x001C] = 0x12;   //
    memory[0x001D] = 0xA4;   // LDY $03
    memory[0x001E] = 0x03;   //     A4 03
    memory[0x001F] = 0xAC;   // LDY $1234
    memory[0x0020] = 0x34;   //     AC 34 12
    memory[0x0021] = 0x12;   //
    memory[0x0022] = 0xB4;   // LDY $00,X
    memory[0x0023] = 0x01;   //
    memory[0x0024] = 0xBC;   // LDY $46CD,X
    memory[0x0025] = 0xCD;   //     BC CD 46
    memory[0x0026] = 0x46;   //
    memory[0x0027] = 0x95;   // STA $AA,X
    memory[0x0028] = 0xAA;   //     85 AA
    memory[0x0029] = 0x00;   // BRK

    printf("This code will test LDA, LDX, and LDY commands:\n\n");
    printf(".FFFC  A9 FF     LDA #$FF\n");
    printf(".FFFE  A5 22     LDA  $22\n");
    printf(".0000  AD 34 12  LDA  $1234\n");
    printf(".0003  A2 EE     LDX #$05\n");
    printf(".0005  BD 34 12  LDA  $1234,X\n");
    printf(".0008  B5 FF     LDA  $FF,X\n");
    printf(".000A  A1 02     LDA ($02,X)\n");      // with X=5, low byte is at 7 ("12"), high byte at 8 ("B5"), destination is B512
    printf(".000C  A0 89     LDY #$03\n");
    printf(".000E  B9 56 34  LDA  $3456,Y\n");     // with Y=03, this will be 3459
    printf(".0011  B1 05     LDA ($05),Y\n");      // 05 contains "BD", BD+03 = C0, so low byte in C0, high byte in C1
    printf(".0013  A6 00     LDX  $00\n");         // 00 has "AD"
    printf(".0015  AE 34 12  LDX  $1234\n");
    printf(".0018  B6 05     LDX  $05,Y\n");       // with Y=03, destination is 08
    printf(".001A  BE 31 12  LDX  $1231,Y\n");     // with Y=03, destination is 1234
    printf(".001D  A4 03     LDY  $03\n");         // 03 contains "A2"
    printf(".001F  AC 34 12  LDY  $1234\n");
    printf(".0022  B4 00     LDY  $01,X\n");       // with X=44, destination is 45
    printf(".0024  BC CD 46  LDY  $46CD,X\n");     // with X=44, destination is 4711
    printf(".0027  95 AA     STA  $AA,X\n");
    printf(".0029  00        BRK\n");

    // Test case for LDA/LDX/LDY: data

    memory[0x0022] = 0xB4;
    memory[0x0045] = 0x42;
    memory[0x00AD] = 0xAA;
    memory[0x00C0] = 0x11;
    memory[0x00C1] = 0x47;
    memory[0x1234] = 0x44;
    memory[0x1239] = 0x93;
    memory[0x3459] = 0x99;
    memory[0x4711] = 0xDA;
    memory[0xB512] = 0x77;

    printf("Contents of 0022 is %02X.  ", memory[0x0022]);
    printf("Contents of 1234 is %02X.  ", memory[0x1234]);
    printf("Contents of 1239 is %02X.  ", memory[0x1239]);
    printf("Contents of 0004 is %02X.\n", memory[0x0004]);
    printf("Contents of B512 is %02X.  ", memory[0xB512]);
    printf("Contents of 3459 is %02X.  ", memory[0x3459]);
    printf("Contents of 0001 is %02X.  ", memory[0x0001]);
    printf("Contents of 0008 is %02X.\n", memory[0x0008]);
    printf("Contents of 4711 is %02X.  ", memory[0x4711]);
    printf("Contents of 00B0 is %02X.  ", memory[0x00B0]);
    printf("Contents of 0008 is %02X.  ", memory[0x0008]);
    printf("Contents of 0003 is %02X.\n", memory[0x0003]);
    printf("Contents of 0045 is %02X.  ", memory[0x0045]);
    printf("\n\n");
}

uint8_t get_byte(CPU6502* cpu, uint8_t memory[MEMORY_SIZE]) {
    uint8_t byte = memory[cpu->PC];
    if(SHOW_PROCESSED_DATA) {
        printf("%02X ", byte);
    }
    cpu->PC++;
    return byte;
}

void show_memory_dump(uint16_t start, uint16_t end, uint8_t memory[MEMORY_SIZE]) {
    printf("       Memory dump from %04X to %04X\n", start, end);
    uint16_t location = start;
    int linebreak_counter = 0;

    // As uint16_t wraps around from FFFF to 0000, I cannot do this with a for loop
    while(1) {
        if(!(linebreak_counter % 10) || location == start) {
            printf("\n.%04X  ", location);
            linebreak_counter = 0;
        }
        printf("%02X ", memory[location]);
        if(location == end) {
            printf("\n\n");
            return;
        }
        location++;
        linebreak_counter++;
    }
}


void sta(CPU6502* cpu, uint8_t memory[MEMORY_SIZE], uint8_t opcode) {
    uint8_t temp_address_low, temp_address_high;
    uint16_t actual_word_address;

    switch(opcode) {
        case 0X81:                                                      // $81  STA ($vw,X)
            temp_address_low = get_byte(cpu, memory) + cpu->X;          // calculate temp address;
            temp_address_high = temp_address_low + 1;                   // uint8_t data type guarantees address will "wrap around" $FF
            actual_word_address = memory[temp_address_low] | 
            (memory[(temp_address_high)] << 8);                         // calculate two-byte address
            memory[actual_word_address] = cpu->A;
            break;
        case 0X85:                                                      // $85  STA  $vw
            memory[get_byte(cpu, memory)] = cpu->A;
            break;
        case 0X8D:                                                      // $8D  STA  $vwxy
            memory[get_byte(cpu, memory) | (get_byte(cpu, memory) << 8)] = cpu->A;
            break;
        case 0X91:                                                      // $91  STA ($vw),Y
            temp_address_low = memory[get_byte(cpu, memory)] + cpu->Y;
            temp_address_high = temp_address_low + 1;                   // uint8_t data type guarantees address will "wrap around" $FF
            actual_word_address = memory[temp_address_low] | 
            (memory[(temp_address_high)] << 8);                         // calculate two-byte address
            memory[actual_word_address] = cpu->A;
            break;
        case 0x95:                                                      // $95  STA  $vx,X
            memory[get_byte(cpu, memory) + cpu->X] = cpu->A;
            break;
        case 0X99:                                                      // $99  STA  $vwxy,Y
            memory[(get_byte(cpu, memory) | (get_byte(cpu, memory) << 8)) + cpu->Y] = cpu->A;
            break;
        case 0X9D:                                                      // $9D  STA  $vwxy,X
            memory[(get_byte(cpu, memory) | (get_byte(cpu, memory) << 8)) + cpu->X] = cpu->A;
            break;
    }
}