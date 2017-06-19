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
    dadc,
    bit,
    bic,
    bis,
    xor_,
    and_
};

bool emulate(char *mem, bool debug_mode, uint16_t PC_init) {
    init_regfile();

    // Set starting point
    regfile[PC] = PC_init;

    while (!HCF) {
        // Fetch
        bus(regfile[PC], mdr, READ_W);

        // Maybe move this
        regfile[PC] += 2;

        // Decode & Execute
        decode_execute();
    }
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

                addressing_mode_fetcher(SINGLE);

                // EXECUTE ONE OP THROUGH FUNCTION TABLE
                single_ptr[single.opcode & 0x07];

                // Update status register
                update_sr(single.bw, SINGLE);

                put_operand(single.as, SINGLE);

                break;
            case JUMP:   // JUMP enumerated as 1
                jump.us_jump = mdr;

                addressing_mode_fetcher(JUMP);

                // CALL FUNCTION TABLE FOR JUMP
                // EXECUTE JUMP THROUGH JUMP MATRIX

                if (jmp_matrix[jump.opcode & 0x07][sr_union.Z][sr_union.N][sr_union.C][sr_union.V]) regfile[PC] += offset;

                break;
            default:  // TWO OPERAND
                dbl.us_double = mdr;

                addressing_mode_fetcher(DOUBLE);

                // EXECUTE TWO OP THROUGH FUNCTION TABLE
                double_ptr[dbl.opcode];

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

            offset = offset << 1;

            // IF: Then negative
            if (offset > 0x0400) offset = 0x0400 - offset;
            // Else, already positive, leave as is

            break;

        case DOUBLE:
            emit_flag = true;
            src = matrix_decoder(dbl.as, dbl.src, dbl.bw);
            // The MOV command does not need to read the destination
            // > just get the effective address
            if (dbl.opcode != 0x4) dst = matrix_decoder(dbl.ad, dbl.dst, dbl.bw);
            else {
                // Calculate eff_address some other way? 
            }

            break;
                
        default:
            std::cout << "[ADDR MODE FETCHER] THIS IS BROKEN\n";
            exit(1);
            break;
    }
}

// (bw ? READ_B : READ_W)

// bw --> B = 1, W = 0
uint16_t matrix_decoder(uint8_t asd, uint8_t regnum, bool bw) {
    uint16_t return_val = 0;

    switch (mode = src_dst_matrix[asd][regnum]) {
        case REG_DIRECT:
            return_val = regfile[regnum];
            break;

        case INDEXED:
            // Fetch the base address, store in mdr
            bus(regfile[PC], mdr, (bw ? READ_B : READ_W));

            // Fetch the actual value, store in mdr
            bus(eff_address = (mdr + regfile[regnum]), mdr, (bw ? READ_B : READ_W));
            return_val = mdr;
            break;

        case ABSOLUTE:
            bus(eff_address = regfile[PC], mdr, (bw ? READ_B : READ_W));
            return_val = mdr;
            break;

        case INDIRECT:
            bus(eff_address = regfile[regnum], mdr, (bw ? READ_B : READ_W));
            return_val = mdr;
            break;

        case INDIRECT_AI:
            bus(eff_address = regfile[regnum], mdr, (bw ? READ_B : READ_W));
            regfile[regnum] += 2;
            return_val = mdr;
            break;

        case IMMEDIATE:
            bus(regfile[PC], mdr, (bw ? READ_B : READ_W));
            break;

        default:  // Constant generator, denoted by 0xC#
            return_val = mode & 0x0f;  // Get rid of the upper nibble
            if (return_val == 0x0f) return_val = -1;
            break;

    return return_val;
    }
}

// bw --> B = 1, W = 0
void update_sr(bool bw, INST_TYPE type) {
    // Global 'result' from last two operand
    sr_union.us_sr_reg = regfile[SR];

    sr_union.Z = (!result) ? true : false;
    sr_union.N = result&0xff>>(bw ? 7 : 15);
    sr_union.C = (result>>(bw ? 8 : 16)) ? true : false;
    sr_union.V = 0;  // Logic performed in ADD, ADDC, SUB, SUBC, and CMP

    regfile[SR] = sr_union.us_sr_reg;
}

void put_operand(uint8_t asd, INST_TYPE type) {
    uint8_t regnum = (type == SINGLE) ? single.reg : dbl.dst;
    bool bw = (type == SINGLE) ? single.bw : dbl.bw;

    mdr = result;

    switch (mode = src_dst_matrix[asd][regnum]) {
        case REG_DIRECT:
            regfile[regnum] = result;
            break;

        case INDEXED:
        case ABSOLUTE:
            bus(eff_address, mdr, (bw ? WRITE_B : WRITE_W));
            break;

        case INDIRECT:
        case INDIRECT_AI:
            if (type == SINGLE) bus(eff_address, mdr, (bw ? WRITE_B : WRITE_W));
            else emulation_error("(Put Operand) Invalid INDIRECT dst");
            break;
        case IMMEDIATE:
            // Only the push inst can do this
            if (type == SINGLE && single.opcode == 0x120) bus(eff_address, mdr, (bw ? WRITE_B : WRITE_W));
            else emulation_error("(Put Operand) Invalid Immediate dst");
            break;

        default:
            emulation_error("(Put Operand) Invalid Constant Generator dst");
            break;
    }
}

// Note: Memory is little-endian (LSB goes in first memory location)
void bus(uint16_t mar, uint16_t &mdr, int ctrl) {
    switch (ctrl) {
        case READ_W:
            mdr = mem_array[mar];
            mdr += mem_array[mar+1] << 8;

            regfile[PC] += 2;
            break;
        case READ_B:
            mdr = mem_array[mar];

            regfile[PC] += 2;
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

// INST: One Operand
void rrc() {
    result = (dst >> 1) + (sr_union.C << (single.bw ? 7 : 15));
}

void swpb() {
    if (!single.bw) result = (dst >> 8) + (dst << 8);
    else emulation_error("(swpb) Byte attempted on Word only instruction");

    put_operand(single.as, SINGLE);
}

void rra() {
    result = (dst >> 1) + (dst << (single.bw ? 7 : 15));
}

void sxt() {
    if (!single.bw) result = (dst & 0xff) + (((dst & 0xff) >> 7) ? 0xff00 : 0);
    else emulation_error("(sxt) Byte attempted on Word only instruction");
}

void push() {
    // Stack?!
}

void call() {
    // Implement with interrupts
}

void reti() {
    // Implement with stuff
}


// Example logic
// ((src>>15 == dst>>15) && ((src>>15) != ((result&0xff)>>15))) ? true: false;

// INST: Two Operand (USE RESULT VARIABLE)
void mov() {
    result = src;
    update_sr(dbl.bw, DOUBLE);
}

void add() {
    result = src + dst;
    update_sr(dbl.bw, DOUBLE);
}

void addc() {
    result = src + dst + sr_union.C;
    update_sr(dbl.bw, DOUBLE);
}

void subc() {
    result = dst - src - sr_union.C;
    update_sr(dbl.bw, DOUBLE);
}

void sub() {
    result = dst - src;
    update_sr(dbl.bw, DOUBLE);
}

void cmp() {  // NO EMIT
    result = dst - src;

    emit_flag = false;
    update_sr(dbl.bw, DOUBLE);
}

void dadc() {
    // ??
    update_sr(dbl.bw, DOUBLE);
}

void bit() {  // NO EMIT
    result = dst & src;

    emit_flag = false;
    update_sr(dbl.bw, DOUBLE);
}

void bic() {
    result = dst & ~src;
    update_sr(dbl.bw, DOUBLE);
}

void bis() {
    result = dst | src;
    update_sr(dbl.bw, DOUBLE);
}

void xor_() {
    result = src ^ dst;
    update_sr(dbl.bw, DOUBLE);
}

void and_() {
    result = src & dst;
    update_sr(dbl.bw, DOUBLE);
}
