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

#include "Include/emulate.h"
#include "Include/library.h"
#include "Include/single_inst.h"
#include "Include/double_inst.h"
#include "Include/debugger.h"

// Globals: Delcared extern in library.h

uint16_t mdr  = 0;

uint16_t cpu_clock = 0;

uint16_t regfile[16] = {};  // All initialized to 0

uint32_t src = 0;  // Not used in the case of single operand
uint32_t dst = 0;  // Used in the case of single operand
uint16_t offset = 0;  // Used for jump commands
uint32_t result = 0;

bool emit_flag = true;

uint16_t mode = 0;
uint32_t eff_address = 0;

single_overlay single;
jump_overlay jump;
double_overlay dbl;
sr_reg sr_union;

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
    uint16_t next_interrupt = 0;

    // Load devices into memory (Call function)

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
        sr_union.us_sr_reg = regfile[SR];  // Check if this is strictly necessary
//         sr_union.GIE = 1;

        if (sr_union.GIE && (interrupts[next_interrupt].time <= cpu_clock)) {
            std::cout << "\t\tLAYER 1\n";
            if ((mem_array[(2*interrupts[next_interrupt].dev)]&1)) {
                execute_interrupt(next_interrupt);
                next_interrupt++;
            }
        }
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

                std::cout << "\t\tFound JUMP instruction: JUMP.OPCODE >>" << (jump.opcode & 0x07) << "<<" << std::endl;

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

    std::cout << "\t\t\t(Matrix Decoder) READ VALUE: >>" << std::hex << return_val << std::dec << "<<\n";

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

    if (!(result&(bw ? 0xff : 0xffff))) sr_union.Z = 1;
    if (bw ? ((result & 0xff)>>7 ) : ((result & 0xffff)>>15 )) sr_union.N = 1;
    if (bw ? ((result & 0x1ff)>>8) : ((result & 0x1ffff)>>16)) sr_union.C = 1;
    // sr_union.V (Overflow) logic performed in ADD, ADDC, SUB, SUBC, and CMP

    regfile[SR] = sr_union.us_sr_reg;

    std::cout << "\t\t\t\tUPDATING SR TO: >>" << regfile[SR] << "<<\n" << std::dec;
}

void put_operand(uint16_t asd, INST_TYPE type) {
    uint16_t regnum = (type == SINGLE) ? single.reg : dbl.dst;
    uint16_t bw = (type == SINGLE) ? single.bw : dbl.bw;

    // Mode should be set already (DST was called last in both SINGLE and DOUBLE)
    // Maybe remove this
    mode = src_dst_matrix[asd][regnum];

    mdr = static_cast<uint16_t>(result);

    std::cout << "\t\t\t\tOPERATION RESULT IS: >>" << std::hex << mdr << std::dec << "<< (REGNUM: " << regnum << " || REGNUM: " << regnum << ")\n";

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
    if (mar <= 32) {
	// DO DEVICE STUFF

    }
    else {
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
}

void emulation_error(std::string error_msg) {
	std::cout << "[EMULATION ERROR] - " << error_msg << std::endl;
	exit(1);
}

// TEMPORARY
void execute_interrupt(uint16_t next_interrupt) {
	std::cout << "\n\n\n\tDOING INTERRUPT #" << next_interrupt
		<< "\n\tTIME " << interrupts[next_interrupt].time
		<< "\n\tDEV  " << interrupts[next_interrupt].dev
		<< "\n\tDATA " << interrupts[next_interrupt].data
		<< "\n\n\n";
	while(1);
}
