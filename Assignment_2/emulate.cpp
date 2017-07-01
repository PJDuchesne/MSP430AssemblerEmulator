/*
__/\\\\\\\\\\\\\_____/\\\\\\\\\\\__/\\\\\\\\\\\\____        
 _\/\\\/////////\\\__\/////\\\///__\/\\\////////\\\__       
  _\/\\\_______\/\\\______\/\\\_____\/\\\______\//\\\_      
   _\/\\\\\\\\\\\\\/_______\/\\\_____\/\\\_______\/\\\_     
    _\/\\\/////////_________\/\\\_____\/\\\_______\/\\\_    
     _\/\\\__________________\/\\\_____\/\\\_______\/\\\_   
      _\/\\\___________/\\\___\/\\\_____\/\\\_______/\\\__  
       _\/\\\__________\//\\\\\\\\\______\/\\\\\\\\\\\\/___
        _\///____________\/////////_______\////////////_____

-> Name:  emulate.cpp
-> Brief: Implementation for the emulate.cpp code that performs fetch, decode, and execute
-> Date: June 16, 2017    (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

// TO DO: Move instructions to their own separate files

#include <stdio.h>
#include <inttypes.h>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>

#include "Include/emulate.h"
#include "Include/library.h"

static single_overlay single;
static jump_overlay jump;
static double_overlay dbl;
static sr_reg sr_union;

static bool debug_mode;

void (*single_ptr[])(/* INPUTS HERE */) = {
    rrc,
    swpb,
    rra,
    sxt,
    push,
    call,
    reti
};

void (*double_ptr[])(/* INPUTS HERE */) = {
    mov,
    add,
    addc,
    subc,
    sub,
    cmp,
    dadd,
    bit,
    bic,
    bis,
    xor_,
    and_
};

bool emulate(uint8_t *mem, bool debug_mode_, uint16_t PC_init) {
    init_regfile();

    debug_mode = debug_mode_;

    // Set starting point
    regfile[PC] = PC_init;

    while (!HCF) {
        // Fetch

        std::cout << "\nFETCHING INST AT PC: " << regfile[PC] << std::endl;

        bus(regfile[PC], mdr, READ_W);

	std::cout << "\tInst: >>" << std::hex << mdr << "<<" << std::dec << std::endl;

        // Maybe move this
        regfile[PC] += 2;

        // Decode & Execute
        decode_execute();

	// TEMP
	dump_mem();

	outfile << "\n\n\n\n\n";

   	for (int i = 0; i < 16; i++) outfile << "Regfile [" << std::dec << i << "] " << std::hex << regfile[i] << std::endl;


    // Check for interrupt (IF GAI is set?)
    }
    system("aafire");
    return true;
}

// Note: Need to decide when to increment PC
void decode_execute() {
    // Check for HCF command
    if (mdr == 0) HCF = true;
    else {
        // Switch case on the 3 MSB of the opcode
        switch (mdr >> 13) {
            case SINGLE:   // SINGLE enumerated as 0
                single.us_single = mdr;

                // Double check extra characers
                if ((single.opcode>>3) != 0b100) {
                    std::cout << "[Emulate] INVALID ONE OPERAND COMMAND\n";
                    exit(1);
                }

                std::cout << "\t\tFound SINGLE instruction: SINGLE.OPCODE >>" << (single.opcode & 0x07) << "<<" << std::endl;

                addressing_mode_fetcher(SINGLE);

                // EXECUTE ONE OP THROUGH FUNCTION TABLE
                single_ptr[single.opcode & 0x07]();

                // Update status register
                update_sr(single.bw, SINGLE);

                put_operand(single.as, SINGLE);

                break;
            case JUMP:   // JUMP enumerated as 1
                jump.us_jump = mdr;

                std::cout << "\t\tFound DOUBLE instruction: JUMP.OPCODE >>" << (jump.opcode & 0x07) << "<<" << std::endl;

                addressing_mode_fetcher(JUMP);

                if (jmp_matrix[jump.opcode & 0x07][sr_union.Z][sr_union.N][sr_union.C][sr_union.V]) regfile[PC] += offset;

                std::cout << "\tPC Updated to: >>" <<  std::hex << regfile[PC] << "<<" << std::endl;

                break;
            default:  // TWO OPERAND
                dbl.us_double = mdr;

                std::cout << "\t\tFound DOUBLE instruction: DBL.OPCODE >>" << dbl.opcode << "<<" << std::endl;

                addressing_mode_fetcher(DOUBLE);

                // EXECUTE TWO OP THROUGH FUNCTION TABLE

                double_ptr[dbl.opcode-4]();

                // Some two operand commands dont emit, they just update SR
                if (emit_flag) put_operand(dbl.ad, DOUBLE);

                break;
        }
    }
}

void init_regfile() {
    // regfile[SP] = 0xffff  // Init SP?
    regfile[16] = -1;
    regfile[17] =  0;
    regfile[18] =  1;
    regfile[19] =  2;
    regfile[20] =  4;
    regfile[21] =  8;
}

void addressing_mode_fetcher(int type) {
    // Initialize these to something WRONG
    src = -1;
    dst = -1;

    switch (type) {
        case SINGLE:
            // RETI does not need variables
            if (single.us_single == 0x1300) break;

            dst = matrix_decoder(single.as, single.reg, single.bw);

            break;

        case JUMP:

            offset = jump.offset;

            offset *= 2; // Shift to the left

            // IF Yes, the jump is negative
            if (offset >= 0x0400) offset += 0xf800;   // Sign extend the negative

            std::cout << "\t\tOFFSET IS >>" << offset << "<<\n";
            // Else, already positive, leave as is

            break;

        case DOUBLE:
            emit_flag = true;

            src = matrix_decoder(dbl.as, dbl.src, dbl.bw);
            dst = matrix_decoder(dbl.ad, dbl.dst, dbl.bw);

	    std::cout << "SRC: " << src << " | DST: " << dst << "\n";

            break;
                
        default:
            std::cout << "[ADDR MODE FETCHER] THIS IS BROKEN\n";
            exit(1);
            break;
    }
}

// (bw ? READ_B : READ_W)

// bw --> B = 1, W = 0

// These are uint16_t to make it work.... (TODO: FIX)
uint16_t matrix_decoder(uint16_t asd, uint16_t regnum, uint16_t bw) {
    uint16_t return_val = 0;
    mode = src_dst_matrix[asd][regnum];

    std::cout << "\t\t\t(Matrix Decoder) ASD >>" << asd << "<< | REGNUM >>" << regnum << "<< | BW >>" << bw << "<< | MODE >>" << mode << "<<\n"; 

    switch (mode) {
        case REG_DIRECT:
            return_val = regfile[regnum];
            break;

        // Relative is simply indexed addressing mode with the PC as the register
        // In relative, the regnum is set to PC (0)
        case RELATIVE:
        case INDEXED:
            // Fetch the base address, store in mdr
            bus(regfile[PC], mdr, (bw ? READ_B : READ_W));

            // Fetch the actual value, store in mdr
	    eff_address = mdr + regfile[regnum];
            bus(eff_address, mdr, (bw ? READ_B : READ_W));
            return_val = mdr;
            break;

        case ABSOLUTE:
	    eff_address = regfile[PC];
            bus(eff_address, mdr, (bw ? READ_B : READ_W));
            return_val = mdr;
            break;

        case INDIRECT:
        case INDIRECT_AI:
	    eff_address = regfile[regnum];
            bus(eff_address, mdr, (bw ? READ_B : READ_W));
            regfile[regnum] += (mode == INDIRECT_AI ? 2 : 0);
            return_val = mdr;
            break;

        case IMMEDIATE:
            bus(regfile[PC], mdr, (bw ? READ_B : READ_W));

	    return_val = mdr;

            break;

        default:  // Constant generator, denoted by 0xC#
            return_val = mode & 0x0f;  // Get rid of the upper nibble
            if (return_val == 0x0f) return_val = -1;
            break;
    }
    // For modes other than the default, increment the program counter accordingly
    if (mode <= IMMEDIATE) regfile[PC] += addr_mode_PC_array[mode];

    std::cout << "\t\t\t(Matrix Decoder) READ: >>" << std::hex << mdr << std::dec << "<<\n";

    return return_val;
}

// bw --> B = 1, W = 0
void update_sr(bool bw, INST_TYPE type) {
    // Global 'result' from last two operand
    sr_union.us_sr_reg = regfile[SR];

    sr_union.Z = (!result) ? true : false;
    sr_union.N = bw ? ((result&0xff)>>7) : (result >> 15);
    sr_union.C = (result>>(bw ? 8 : 16)) ? true : false;
    sr_union.V = 0;  // Logic performed in ADD, ADDC, SUB, SUBC, and CMP

    regfile[SR] = sr_union.us_sr_reg;
}

void put_operand(uint16_t asd, INST_TYPE type) {
    uint16_t regnum = (type == SINGLE) ? single.reg : dbl.dst;
    uint16_t bw = (type == SINGLE) ? single.bw : dbl.bw;

    mdr = result;

    std::cout << "\t\t\t\tOPERATION RESULT IS: >>" << mdr << "<< (REGNUM: " << regnum << " || REGNUM: " << regnum << ")\n";

    // Mode should be set already (DST was called last in both SINGLE and DOUBLE)
    mode = src_dst_matrix[asd][regnum];

    switch (mode) {
        case REG_DIRECT:
            regfile[regnum] = result;
            break;

    // All 3 of these modes simply return to the effective address
    	case RELATIVE:
        case INDEXED:
        case ABSOLUTE:
            bus(eff_address, mdr, (bw ? WRITE_B : WRITE_W));
            break;

    // These two cases must do some error checking to prevent
    // two operand instructions from putting values to the destination
        case INDIRECT:
        case INDIRECT_AI:
            if (type == SINGLE) bus(eff_address, mdr, (bw ? WRITE_B : WRITE_W));
            else emulation_error("(Put Operand) Invalid INDIRECT dst");
            break;

        // Only the push instruction can do this
        case IMMEDIATE:
            if (type == SINGLE && single.opcode == 0x120) bus(eff_address, mdr, (bw ? WRITE_B : WRITE_W));
            else emulation_error("(Put Operand) Invalid Immediate dst");
            break;

        default:
            emulation_error("(Put Operand) Invalid Constant Generator dst");
            break;
    }

    std::cout << "\t\t\t\t\t(Put Operand) ASD: " << asd << " | REGNUM: " << regnum << " | MODE: " << mode << std::endl;
}

// Note: Memory is little-endian (LSB goes in first memory location)
void bus(uint16_t mar, uint16_t &mdr, int ctrl) {
    switch (ctrl) {
        case READ_W:
            mdr = mem_array[mar];
            mdr += mem_array[mar+1] << 8;

            break;
        case READ_B:
            mdr = mem_array[mar];

            break;
        case WRITE_W:
            mem_array[mar] = mdr & 0x0f;
            mem_array[mar+1] = (mdr >> 8) & 0x0f;

            break;
        case WRITE_B:
            mem_array[mar] = mdr & 0x0f;

            break;
        default:
            std::cout << "[BUS] INVALID BUS INPUT - ENDING" << std::endl;
            exit(1);
            break;
    }
}

void emulation_error(std::string error_msg) {
    std::cout << "[EMULATION ERROR] - " << error_msg << std::endl;
    exit(1);
}

// ============ INSTRUCTIONS ============= //

// INST: One Operand
void rrc() {
    result = (dst >> 1) + (sr_union.C << (single.bw ? 7 : 15));

    update_sr(single.as, SINGLE);

    // Set carry bit to the LSB of the rotated value
    regfile[SR] += dst&0x01;

    std::cout << "\t\t\t\tEXECUTING RRC (DST >>" << dst << "<<)\n";
}

void swpb() {
    if (!single.bw) result = (dst >> 8) + (dst << 8);
    else emulation_error("(swpb) Byte attempted on Word only instruction");

    std::cout << "\t\t\t\tEXECUTING SWPB (DST >>" << dst << "<<)\n";
}

void rra() {
    result = (dst >> 1) + (dst << (single.bw ? 7 : 15));

    std::cout << "\t\t\t\tEXECUTING RRA (DST >>" << dst << "<<)\n";

    update_sr(single.as, SINGLE);

    // Set carry bit to the LSB of the rotated value
    regfile[SR] += dst&0x01;
}

void sxt() {
    result = (dst & 0xff) + (((dst & 0xff) >> 7) ? 0xff00 : 0);

    std::cout << "\t\t\t\tEXECUTING SXT (DST >>" << dst << "<<)\n";

    update_sr(dbl.bw, DOUBLE);

    // Set carry bit to the opposite of the negative bit
    regfile[SR] += (regfile[SR] & 0x02) ? 0 : 0x02;
}

void push() {  // NO SR
    // If byte instruction, sign extend
    if (single.bw) result = (dst & 0xff) + (((dst & 0xff) >> 7) ? 0xff00 : 0);

    std::cout << "\t\t\t\tEXECUTING PUSH (DST >>" << dst << "<<)\n";

    // Call bus directly due to special case
    result = mdr;
    bus(regfile[SP], mdr, WRITE_W);
    regfile[SP] -= 2;
}

void call() {
    // Call bus directly due to special case

    std::cout << "\t\t\t\tEXECUTING CALL (DST >>" << dst << "<<)\n";

    mdr = regfile[PC];

    bus(regfile[SP], mdr, WRITE_W);
    regfile[SP] -= 2;

    // Store Source to Program counter
    regfile[PC] = dst;
    // Implement with interrupts
}

void reti() {
    // Implement with stuff
    std::cout << "\t\t\t\tEXECUTING RETI (DST >>" << dst << "<<)\n";
}

// Example logic (not sure what this is for as of JUNE 27)
// ((src>>15 == dst>>15) && ((src>>15) != ((result&0xff)>>15))) ? true: false;

// INST: Two Operand (USE RESULT VARIABLE)
void mov() {		// Does not update SR (See manual)
    result = src;

    std::cout << "\t\t\t\tEXECUTING MOV (SRC >>" << src << "<< || DST: >>" << dst << "<<)\n";
}

void add() {
    result = src + dst;
    update_sr(dbl.bw, DOUBLE);

    // If SRC and DST have the same sign, and the result has the opposite sign. Set the overflow bit
    // Note: Overflow bit was reset in update_sr
    regfile[SR] += (((src < 0x8000 && dst < 0x8000)&&(result >= 0x8000))||((src >= 0x8000 && dst >= 0x8000)&&(result < 0x8000))) ? 0x10 : 0;
    std::cout << "\t\t\t\tEXECUTING ADD (SRC >>" << src << "<< || DST: >>" << dst << "<<)\n";

    /* More visual method, but strictly slower
    sr_union.us_sr_reg = regfile[SR];
    sr_union.V = ((src < 0x8000 && dst < 0x8000)&&(result >= 0x8000))||((src >= 0x8000 && dst >= 0x8000)&&(result < 0x8000));
    regfile[SR] = sr_union.us_sr_reg;
    */
}

void addc() {
    result = src + dst + sr_union.C;
    update_sr(dbl.bw, DOUBLE);
    std::cout << "\t\t\t\tEXECUTING ADDC (SRC >>" << src << "<< || DST: >>" << dst << "<<)\n";

    // If SRC and DST have the same sign, and the result has the opposite sign. Set the overflow bit
    // Note: Overflow bit was reset in update_sr
    regfile[SR] += (((src < 0x8000 && dst < 0x8000)&&(result >= 0x8000))||((src >= 0x8000 && dst >= 0x8000)&&(result < 0x8000))) ? 0x10 : 0;
}

void subc() {
    result = dst - src - sr_union.C;
    update_sr(dbl.bw, DOUBLE);
    std::cout << "\t\t\t\tEXECUTING SUBC (SRC >>" << src << "<< || DST: >>" << dst << "<<)\n";

    // If SRC and DST have opposite signs, and the result has the same sign as the destination. Set the overflow bit
    // Note: Overflow bit was reset in update_sr
    regfile[SR] += (((src < 0x8000 && dst >= 0x8000)&&(result >= 0x8000))||((src >= 0x8000 && dst < 0x8000)&&(result < 0x8000))) ? 0x10 : 0;
}

void sub() {
    result = dst - src;
    update_sr(dbl.bw, DOUBLE);
    std::cout << "\t\t\t\tEXECUTING SUB (SRC >>" << src << "<< || DST: >>" << dst << "<<)\n";

    // If SRC and DST have opposite signs, and the result has the same sign as the destination. Set the overflow bit
    // Note: Overflow bit was reset in update_sr
    regfile[SR] += (((src < 0x8000 && dst >= 0x8000)&&(result >= 0x8000))||((src >= 0x8000 && dst < 0x8000)&&(result < 0x8000))) ? 0x10 : 0;
}

void cmp() {  // NO EMIT
    result = dst - src;

    emit_flag = false;
    update_sr(dbl.bw, DOUBLE);
    std::cout << "\t\t\t\tEXECUTING CMP (SRC >>" << src << "<< || DST: >>" << dst << "<<)\n";

    // If SRC and DST have opposite signs, and the result has the same sign as the destination. Set the overflow bit
    // Note: Overflow bit was reset in update_sr
    regfile[SR] += (((src < 0x8000 && dst >= 0x8000)&&(result >= 0x8000))||((src >= 0x8000 && dst < 0x8000)&&(result < 0x8000))) ? 0x10 : 0;
}

void dadd() {
    // ??

    uint16_t dec_src = (src>>4)*10 + (src&0x0f);
    uint16_t dec_dst = (dst>>4)*10 + (dst&0x0f);
    std::cout << "\t\t\t\tEXECUTING DADD (SRC >>" << src << "<< || DST: >>" << dst << "<<)\n";

    // Each nibble of the byte corresponds to a digit in the final answer
    // Must convert from binary to deciaml before doing arithmatic and convert back when finished

    update_sr(dbl.bw, DOUBLE);

    // Set carry flag if the result is greater than 9999 (W) or 99 (B)
}

void bit() {  // NO EMIT
    result = dst & src;

    emit_flag = false;
    update_sr(dbl.bw, DOUBLE);
    std::cout << "\t\t\t\tEXECUTING BIT (SRC >>" << src << "<< || DST: >>" << dst << "<<)\n";

    // Set carry bit to the opposite of the negative bit
    regfile[SR] += (regfile[SR] & 0x02) ? 0 : 0x02;
}

void bic() {
    result = dst & ~src;
    std::cout << "\t\t\t\tEXECUTING BIC (SRC >>" << src << "<< || DST: >>" << dst << "<<)\n";
}

void bis() {
    result = dst | src;
    std::cout << "\t\t\t\tEXECUTING BIS (SRC >>" << src << "<< || DST: >>" << dst << "<<)\n";
}

void xor_() {
    result = src ^ dst;
    update_sr(dbl.bw, DOUBLE);

    // Set carry bit to the opposite of the negative bit
    regfile[SR] += (regfile[SR] & 0x02) ? 0 : 0x02;

    // If SRC and DST are negative, set overflow bit
    regfile[SR] += (src >= (dbl.bw ? 0x0080 : 0x8000) && dst >= (dbl.bw ? 0x0080 : 0x8000)) ? 0x10 : 0;;

    std::cout << "\t\t\t\tEXECUTING XOR_ (SRC >>" << src << "<< || DST: >>" << dst << "<<)\n";
}

void and_() {
    result = src & dst;
    update_sr(dbl.bw, DOUBLE);

    // Set carry bit to the opposite of the negative bit
    regfile[SR] += (regfile[SR] & 0x02) ? 0 : 0x02;
    std::cout << "\t\t\t\tEXECUTING AND_ (SRC >>" << src << "<< || DST: >>" << dst << "<<)\n";
}
