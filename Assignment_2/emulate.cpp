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
#include <csignal>
#include <algorithm>  // Used for ::toupper

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
    dadd,
    bit,
    bic,
    bis,
    xor_,
    and_
};

void signalHandler(int signum) {
    if (debug_mode) debugger();
    else exit(2);
}

bool emulate(uint8_t *mem, bool debug_mode_, uint16_t PC_init) {
    init_regfile();

    debug_mode = debug_mode_;

    // Set starting point
    regfile[PC] = PC_init;

    std::signal(SIGINT, signalHandler); 

    while (!HCF) {
        // Fetch

        std::cout << "\nFETCHING INST AT PC: " << std::hex << regfile[PC] << std::dec << " (Clock at " << cpu_clock << ")" << std::endl;

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

    debug_mode = false;

    return true;
}

// Note: Need to decide when to increment PC
void decode_execute() {
    // Check for HCF command
    if (mdr == 0) HCF = true;
    else {

        // Update SR_Union to have the currently contents of the SR
        sr_union.us_sr_reg = regfile[SR];

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

                if (emit_flag) put_operand(single.as, SINGLE);

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

    uint16_t  src_mode = -1;

    switch (type) {
        case SINGLE:
            // RETI does not need variables
            if (single.us_single == 0x1300) break;

            emit_flag = true;

            dst = matrix_decoder(single.as, single.reg, single.bw);

            std::cout << "\t\t\tUPDATING CLOCK BY: " << single_op_clock[single.opcode & 0x07][mode] << " (Single)" << std::endl;

            cpu_clock += single_op_clock[single.opcode & 0x07][mode];

            break;

        case JUMP:

            offset = jump.offset;

            std::cout << "\t\tOFFSET IS >>" << std::hex << offset << std::dec << "<<\n";

            offset *= 2; // Shift to the left

            std::cout << "\t\tOFFSET IS >>" << std::hex << offset << std::dec << "<<\n";

            // IF Yes, the jump is negative
            if (offset >= 0x0400) offset += 0xf800;   // Sign extend the negative

            std::cout << "\t\tOFFSET IS >>" << std::hex << offset << std::dec << "<<\n";
            // Else, already positive, leave as is

            std::cout << "\t\t\tUPDATING CLOCK BY: 2 (Jump)\n";
            cpu_clock += 2;

            break;

        case DOUBLE:
            emit_flag = true;

            src = matrix_decoder(dbl.as, dbl.src, dbl.bw);

            // Store the mode from the global 'mode' before it is overwritten by the dst mode
            src_mode = mode;

            if (src_mode > 8) src_mode = 6; // If using the constant generator, pretend it's immediate
            // TODO: Ask Dr Hughes about this?

            dst = matrix_decoder(dbl.ad, dbl.dst, dbl.bw);

            cpu_clock += double_op_clock[src_mode][mode];
            std::cout << "\t\t\tUPDATING CLOCK BY: " << double_op_clock[src_mode][mode] << " (Double with SRC: " << src_mode << " || DST: " << mode << " )" << std::endl;

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
uint32_t matrix_decoder(uint16_t asd, uint16_t regnum, uint16_t bw) {
    uint32_t return_val = 0;
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
            // Fetch the absolute address, store in mdr
            bus(regfile[PC], mdr, (bw ? READ_B : READ_W));

            eff_address = mdr;

            // Fetch the actual value
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

    std::cout << "\t\t\t(Matrix Decoder) READ: >>" << std::hex << return_val << std::dec << "<<\n";

    return return_val;
}

// bw --> B = 1, W = 0
void update_sr(bool bw) {
    // Global 'result' from last two operand

    std::cout << "\t\t\t\tUPDATING SR WITH RESULT: >>" << std::hex << result << "<<\n";

    sr_union.us_sr_reg = regfile[SR];

    sr_union.Z = 0;
    sr_union.N = 0;
    sr_union.C = 0;
    sr_union.V = 0;

    if (!result) sr_union.Z = 1;
    if (bw ? ((result & 0xff)>>7 ) : ((result & 0xffff)>>15 )) sr_union.N = 1;
    if (bw ? ((result & 0x1ff)>>8) : ((result & 0x1ffff)>>16)) sr_union.C = 1;
    // sr_union.V (Overflow) logic performed in ADD, ADDC, SUB, SUBC, and CMP

    regfile[SR] = sr_union.us_sr_reg;

    std::cout << "\t\t\t\tUPDATING SR TO: >>" << regfile[SR] << "<<\n" << std::dec;
}

void put_operand(uint16_t asd, INST_TYPE type) {
    uint16_t regnum = (type == SINGLE) ? single.reg : dbl.dst;
    uint16_t bw = (type == SINGLE) ? single.bw : dbl.bw;

    mdr = static_cast<uint16_t>(result);

    std::cout << "\t\t\t\tOPERATION RESULT IS: >>" << std::hex << mdr << std::dec << "<< (REGNUM: " << regnum << " || REGNUM: " << regnum << ")\n";

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
            mdr += (mem_array[mar+1] << 8);

            break;
        case READ_B:
            mdr = mem_array[mar];

            break;
        case WRITE_W:
            mem_array[mar] = (mdr & 0xff);
            mem_array[mar+1] = ((mdr >> 8) & 0xff);

            break;
        case WRITE_B:
            mem_array[mar] = (mdr & 0xff);

            break;
        default:
            std::cout << "[BUS] INVALID BUS INPUT - ENDING" << std::endl;
            exit(1);
            break;
    }
}

void debugger() {
    std::string input;
    std::string input_temp;
    uint16_t str_len = 0;
    uint16_t hex_loc = 0;
    uint16_t hex_data = 0;
    uint16_t dec_loc = 0;  // In decimal

    std::cout << "\n\tDEBUG MODE ENTERED\n";

    /* Functionality Required:
        1) Inspect memory location (MRnnnn)
        2) Change Memory location (MWnnnn)
        3) Inspect Register (RRnn)
        4) Change Register (RWnn nnnn)
        5) Continue
        6) EXIT (Normal ctrl-C signal)
    */

    // TODO: DO PROPER STOI ERROR CHECKING

    while (1) {
        std::cout << "\n\tDEBUGGER MENU: Please enter command from below\n"
            << "\t\t(Mnnnn)    Read memory location     (Enter location in HEX)\n"
            << "\t\t(Ennnn nn) Emit to memory location  (Enter location and value in HEX)\n"
            << "\t\t(Rnn nnnn) Write CPU Register       (Enter register number in DECIMAL and value in HEX)\n"
            << "\t\t(D)        Dump CPU Registers\n"
            << "\t\t(C)        Continue program by exiting debugging mode\n"
            << "\t\t(Exit)     Exit(2)                  (Normal ctrl-C call, terminates program)\n\n"
            << "\t\t\tNOTE #1: Include all trailing 0s, that just makes the parsing easier\n"
            << "\t\t\tNOTE #2: Memory locations store things LITTLE ENDIAN\n"
            << "\t\t\tNOTE #3: Update PC by reading and changing R00\n\n"
            << "\t\tInput: >> ";

        std::getline(std::cin, input);

        str_len = input.length();

        std::transform(input.begin(), input.end(),input.begin(), ::toupper);

        if (input == "EXIT") exit(2);

        if (input.find_first_not_of("0123456789abcdefABCDEF MERWC") != std::string::npos) {
            std::cout << "\n\tPlease enter a valid input\n";
            continue;
        }

        if (str_len >= 1) {
            switch (input[0]) {
                case 'M':
                    if (str_len != 5) {
                        std::cout << "\n\tPlease enter a valid input (Wrong length on M)\n";
                        continue;
                    }

                    // Parse location to read
                    input_temp = input.substr(1, 4);
                    hex_loc = std::stoi(input_temp, nullptr, 16);

                    std::cout << std::hex << "\n\tMemory Location: >>" << hex_loc << "<< contains >>" << (uint16_t)mem_array[hex_loc] << std::dec << "<<\n";

                    break;
                case 'E':
                    if (str_len != 8) {
                        std::cout << "\n\tPlease enter a valid input (Wrong length on E)\n";
                        continue;
                    }

                    // Parse location to read
                    input_temp = input.substr(1, 4);
                    hex_loc = std::stoi(input_temp, nullptr, 16);

                    // Parse the data to store
                    input_temp = input.substr(6, 2);
                    hex_data = std::stoi(input_temp, nullptr, 16);

                    mem_array[hex_loc] = hex_data;

                    std::cout << std::hex << "\n\tMemory Location: >>" << hex_loc << "<< updated to >>" << (uint16_t)mem_array[hex_loc] << std::dec << "<<\n";
                    break;
                case 'R':
                    if (str_len != 8) {
                        std::cout << "\n\tPlease enter a valid input (Wrong length on R)\n";
                        continue;
                    }

                    // Parse location to read
                    input_temp = input.substr(1, 2);
                    dec_loc = std::stoi(input_temp, nullptr, 10);

                    // Parse the data to store
                    input_temp = input.substr(4, 4);
                    hex_data = std::stoi(input_temp, nullptr, 16);

                    regfile[dec_loc] = hex_data;

                    std::cout << "\n\tRegister Number: " << std::dec << dec_loc << " updated to >>" << std::hex << (uint16_t)regfile[dec_loc] << std::dec << "<<\n";

                    break;
                case 'D':
                    std::cout << std::endl;
                    for (int i = 0; i < 16; i++) {
                        std::cout << "Regfile [" << std::dec << i << "]\t" << std::hex << regfile[i] << std::endl;
                    }

                    break;
                case 'C':
                    return;
                    break;
                default:
                    std::cout << "\n\tPlease enter a valid input\n";
                    break;
            }
        }
        else std::cout << "\n\tPlease enter a valid input\n";
    }
}

void emulation_error(std::string error_msg) {
    std::cout << "[EMULATION ERROR] - " << error_msg << std::endl;
    exit(1);
}

// ============ INSTRUCTIONS ============= //

// INST: One Operand
void rrc() {
    uint32_t even_dst = ((dst%2) ? dst - 1 : dst);

    std::cout << "RRC SR: >>" << std::hex << regfile[SR] << "<< || shift: >>" << even_dst << " | " << (even_dst >> 1) << "<<" << std::dec << "<<\n";

    result = (even_dst >> 1) + ((regfile[SR]&0x1)<<(single.bw ? 7 : 15));

    update_sr(single.bw);

    // Set carry bit to LSB of dst
    sr_union.C = (dst&0x0001) ? 1 : 0;
    regfile[SR] = sr_union.us_sr_reg;

    std::cout << "\t\t\t\tEXECUTING RRC (DST >>" << std::hex << dst << std::dec << "<<)\n";
}

void swpb() {
    if (!single.bw) result = (dst >> 8) + (dst << 8);
    else emulation_error("(swpb) Byte attempted on Word only instruction");

    std::cout << "\t\t\t\tEXECUTING SWPB (DST >>" << std::hex << dst << std::dec << "<<)\n";
}

void rra() {
    // RRA sets the MSB to itself after shifting (TODO: FIX)
    uint32_t even_dst = ((dst%2) ? dst - 1 : dst);
    result = (even_dst >> 1) + (even_dst << (single.bw ? 7 : 15))&0xff;

    std::cout << "\t\t\t\tEXECUTING RRA (DST >>" << std::hex << dst << std::dec << "<<)\n";

    update_sr(single.bw);

    // Set carry bit to the LSB of the unrotated value

    std::cout << "RRA CARRY SET TO: >>" << std::hex << regfile[SR] << std::dec << "<<\n";

    // Set carry bit to LSB of dst
    sr_union.C = (dst&0x0001) ? 1 : 0;
    regfile[SR] = sr_union.us_sr_reg;

    std::cout << "RRA SR: >>" << std::hex << regfile[SR] << std::dec << "<<\n";
}

void sxt() {
    result = (dst & 0xff) + (((dst & 0xff) >> 7) ? 0xff00 : 0);

    std::cout << "\t\t\t\tEXECUTING SXT (DST >>" << std::hex << dst << std::dec << "<<)\n";

    update_sr(single.bw);

    // Set carry bit to the opposite of the negative bit
    sr_union.C = (sr_union.N ? 0 : 1);
    regfile[SR] = sr_union.us_sr_reg;
}

void push() {  // NO SR
    // If byte instruction, sign extend
    result = dst;
    if (single.bw) result = (dst & 0xff) + (((dst & 0xff) >> 7) ? 0xff00 : 0);

    // Call bus directly due to special case
    mdr = result;

    std::cout << "\t\t\t\tEXECUTING PUSH (MDR >>" << std::hex << mdr << std::dec << "<<)\n";

    bus(regfile[SP], mdr, WRITE_W);

    regfile[SP] -= 2;

    emit_flag = false;
}

void call() {
    // Call bus directly due to special case

    std::cout << "\t\t\t\tEXECUTING CALL (DST >>" << std::hex << dst << std::dec << "<<)\n";

    mdr = regfile[PC];

    bus(regfile[SP], mdr, WRITE_W);
    regfile[SP] -= 2;

    // Store Source to Program counter
    regfile[PC] = dst;

    emit_flag = false;
}

void reti() {
    // Implement with stuff
    std::cout << "\t\t\t\tEXECUTING RETI (DST >>" << std::hex << dst << std::dec << "<<)\n";
    emit_flag = false;
}

// Example logic (not sure what this is for as of JUNE 27)
// ((src>>15 == dst>>15) && ((src>>15) != ((result&0xff)>>15))) ? true: false;

// INST: Two Operand (USE RESULT VARIABLE)
void mov() {        // Does not update SR (See manual)
    result = src;

    std::cout << "\t\t\t\tEXECUTING MOV (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";
}

void add() {
    result = src + dst;
    update_sr(dbl.bw);

    std::cout << "\t\t\t\tEXECUTING ADD (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // If SRC and DST have the same sign, and the result has the opposite sign. Set the overflow bit
    // Note: Overflow bit was reset in update_sr (No need to reset it again)
    if (((src < 0x8000 && dst < 0x8000)&&(result >= 0x8000))||((src >= 0x8000 && dst >= 0x8000)&&(result < 0x8000))) {
        sr_union.V = 1;
        regfile[SR] = sr_union.us_sr_reg;
    }
}

void addc() {
    result = src + dst + sr_union.C;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING ADDC (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // If SRC and DST have the same sign, and the result has the opposite sign. Set the overflow bit
    // Note: Overflow bit was reset in update_sr (No need to reset it again)
    if (((src < 0x8000 && dst < 0x8000)&&(result >= 0x8000))||((src >= 0x8000 && dst >= 0x8000)&&(result < 0x8000))) {
        sr_union.V = 1;
        regfile[SR] = sr_union.us_sr_reg;
    }
}

void subc() {
    // result = dst + ~src + 1 + sr_union.C; TODO: WHY is that ONE not there?

    std::cout << "SR UNION IS: " << std::hex << sr_union.C << std::dec << "\n";

    result = dst + ~src + 1 + sr_union.C;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING SUBC (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // If SRC and DST have opposite signs, and the result has the same sign as the destination. Set the overflow bit
    // Note: Overflow bit was reset in update_sr (No need to reset it again)
    if (((src < 0x8000 && dst >= 0x8000)&&(result < 0x8000))||((src >= 0x8000 && dst < 0x8000)&&(result >= 0x8000))) {
        sr_union.V = 1;
        regfile[SR] = sr_union.us_sr_reg;
    }
}

void sub() {
    result = dst + ~src + 1;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING SUB (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // If SRC and DST have opposite signs, and the result has the same sign as the destination. Set the overflow bit
    // Note: Overflow bit was reset in update_sr (No need to reset it again)
    if (((src < 0x8000 && dst >= 0x8000)&&(result < 0x8000))||((src >= 0x8000 && dst < 0x8000)&&(result >= 0x8000))) {
        sr_union.V = 1;
        regfile[SR] = sr_union.us_sr_reg;
    }
}

void cmp() {  // NO EMIT
    result = dst + ~src + 1;

    emit_flag = false;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING CMP (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // If SRC and DST have opposite signs, and the result has the same sign as the destination. Set the overflow bit
    // Note: Overflow bit was reset in update_sr (No need to reset it again)
    if (((src < 0x8000 && dst >= 0x8000)&&(result < 0x8000))||((src >= 0x8000 && dst < 0x8000)&&(result >= 0x8000))) {
        sr_union.V = 1;
        regfile[SR] = sr_union.us_sr_reg;
    }

    std::cout << "\t\t\t\tOPERATION RESULT IS: >>" << std::hex << result << std::dec << "<<\n";
}

void dadd() {
    // ??

    uint16_t dec_src = (src>>4)*10 + (src&0x0f);
    uint16_t dec_dst = (dst>>4)*10 + (dst&0x0f);
    std::cout << "\t\t\t\tEXECUTING DADD (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // Each nibble of the byte corresponds to a digit in the final answer
    // Must convert from binary to deciaml before doing arithmatic and convert back when finished

    update_sr(dbl.bw);

    // Set carry flag if the result is greater than 9999 (W) or 99 (B)
}

void bit() {  // NO EMIT
    result = dst & src;

    emit_flag = false;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING BIT (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // Set carry bit to the opposite of the negative bit
    sr_union.C = (sr_union.N ? 0 : 1);
    regfile[SR] = sr_union.us_sr_reg;

    std::cout << "\t\t\t\tOPERATION RESULT IS: >>" << std::hex << result << std::dec << "<<\n";
}

void bic() {
    result = dst & ~src;
    std::cout << "\t\t\t\tEXECUTING BIC (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";
}

void bis() {
    result = dst | src;
    std::cout << "\t\t\t\tEXECUTING BIS (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";
}

void xor_() {
    result = src ^ dst;
    update_sr(dbl.bw);

    // Set carry bit to the opposite of the negative bit
    sr_union.C = (sr_union.N ? 0 : 1);

    // If SRC and DST are negative, set overflow bit
    sr_union.V = (src >= (dbl.bw ? 0x0080 : 0x8000) && dst >= (dbl.bw ? 0x0080 : 0x8000)) ? 1 : 0;

    regfile[SR] = sr_union.us_sr_reg;

    std::cout << "\t\t\t\tEXECUTING XOR_ (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";
}

void and_() {
    result = src & dst;
    update_sr(dbl.bw);

    // Set carry bit to the opposite of the negative bit
    sr_union.C = (sr_union.N ? 0 : 1);
    regfile[SR] = sr_union.us_sr_reg;

    std::cout << "\t\t\t\tEXECUTING AND_ (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";
}
