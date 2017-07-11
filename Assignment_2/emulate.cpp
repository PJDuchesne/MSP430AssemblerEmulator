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
#include "Include/devices.h"

// Globals: Delcared extern in library.h
uint16_t mdr  = 0;

uint32_t cpu_clock = 0;

uint16_t regfile[16] = {};  // All initialized to 0

uint32_t src = 0;  // Not used in the case of single operand
uint32_t dst = 0;  // Used in the case of single operand
uint16_t offset = 0;  // Used for jump commands
uint32_t result = 0;

uint16_t next_interrupt = 0;

bool emit_flag = true;
bool temp_GIE_disable = false;

uint16_t mode = 0;
uint32_t eff_address = 0;

single_overlay single;
jump_overlay jump;
double_overlay dbl;
sr_reg *sr_union;

void (*single_ptr[])() = {
    rrc,
    swpb,
    rra,
    sxt,
    push,
    call,
    reti
};

void (*double_ptr[])() = {
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

/*
    Function: signalHandler
    Input:  signum: The number of the interrupt to catch
    Brief: This signal handler will catch the CTRL-C signal if
            the debug_mode flag is active, otherwise it will
            act as if CTRL-C was not handled specially
*/
void signalHandler(int signum) {
    if (debug_mode) debugger();
    else exit(2);
}

/*
    Function: emulate
    Input: PC_init: This is the initial value to set the program counter to 
    Brief: This is the core of the program, it calls fetch, decode, execute,
        and then checks for interrupts in a continuous loop until the program
        ends by deteching HCF. A number of errors or unique situations may
        also cause the program to cease.
*/
bool emulate(uint8_t *mem, uint16_t PC_init) {

    // Set up SR_REG union to be used throughout code
    sr_union = (sr_reg *)&regfile[SR];

    // Set starting point
    regfile[PC] = PC_init;

    // Set up signal handler for the debugger
    std::signal(SIGINT, signalHandler); 

    while (!HCF) {
        // Fetch
        std::cout << "\nFETCHING INST AT PC: " << std::hex << regfile[PC] << std::dec << " (Clock at " << cpu_clock << ")" << std::endl;

        bus(regfile[PC], mdr, READ_W);

        std::cout << "\tInst: >>" << std::hex << mdr << "<<" << std::dec << std::endl;

        // Increment PC for the INST
        regfile[PC] += WORD;

        // Decode & Execute
        decode_execute();

        update_device_statuses();

        if (!temp_GIE_disable) check_for_interrupts();
        else temp_GIE_disable = false;
    }

    // Dump memory and close output file
    dump_mem();
    dev_outfile.close();

    system("aafire");

    debug_mode = false;

    return true;
}

/*
    Function: decode_execute 
    Input: MDR (Globally): This is the current instruction to be decoded and executed
    Output: Result (Globally): This is (or isn't!) stored to the desired location at the end of the execution
            SR_REG (Globally): The SR_REG is updated (or isn't!) after desired times during the program
    Brief: This function takes the given MDR and extracts the opcode. It then determines which type of instruction
            is being run and calls the appropriate combination of 'addressing_mode_fetcher' to obtain the SRC, DST,
            and OFFSET values. Afterwards, it calls the appropriate instruction by either invoking a function table
            or a lookup table. Finally, 'put_operand' is called to place the operand in the correct location.
*/
void decode_execute() {
    // Check for HCF command
    if (mdr == 0) HCF = true;
    else {
        // Switch case on the 3 MSB of the opcode
        switch (mdr >> OPCODE_SEPARATION_SHIFT) {
            case SINGLE:   // SINGLE is enumerated as 0
                // Fill union for future use
                single.us_single = mdr;

                // Double check extra characers are correct
                if ((single.opcode>>3) != 0b100) {
                    std::cout << "[Emulate] INVALID ONE OPERAND COMMAND (>>" << std::hex << single.opcode << std::dec << "<<)\n";
                    exit(1);
                }

                std::cout << "\t\tFound SINGLE instruction: SINGLE.OPCODE >>" << (single.opcode & UNIQUE_OP_MASK) << "<<" << std::endl;

                // Fetches DST value for single operand
                addressing_mode_fetcher(SINGLE);

                // EXECUTE ONE OP THROUGH FUNCTION TABLE
                single_ptr[single.opcode & UNIQUE_OP_MASK]();

                if (emit_flag) put_operand(single.as, SINGLE);

                break;
            case JUMP:   // JUMP is enumerated as 1
                // Fill union for future use
                jump.us_jump = mdr;

                std::cout << "\t\tFound JUMP instruction: JUMP.OPCODE >>" << (jump.opcode & UNIQUE_OP_MASK) << "<<" << std::endl;

                // Fetches offset value for jump inst
                addressing_mode_fetcher(JUMP);

                // Execute jump command through lookup table
                if (jmp_matrix[jump.opcode & UNIQUE_OP_MASK][sr_union->Z][sr_union->N][sr_union->C][sr_union->V]) regfile[PC] += offset;

                std::cout << "\tPC Updated to: >>" <<  std::hex << regfile[PC] << "<<" << std::endl;

                break;
            default:  // TWO OPERAND
                // Fill union for future use
                dbl.us_double = mdr;

                std::cout << "\t\tFound DOUBLE instruction: DBL.OPCODE >>" << dbl.opcode << "<<" << std::endl;

                // Fetches SRC and DST values for double operand
                addressing_mode_fetcher(DOUBLE);

                // EXECUTE TWO OP THROUGH FUNCTION TABLE
                double_ptr[dbl.opcode - DOUBLE_OFFSET]();

                // Some two operand commands don't emit, they just update SR
                // This is accomplished by setting and checking the emit_flag
                if (emit_flag) put_operand(dbl.ad, DOUBLE);

                break;
        }
    }
}

/*
    Function: addressing_mode_fetcher
    Input: type: This is the enumerated type of instruction to fetch the modes of
    Output: SRC (Globally): The source data, used in two operation instructions
            DST (Globally): The destination data, used in both one and two operation instructions
            OFFSET (Globally): The jump offset, used with the jump command
    Brief: This function does a switch case on the given input type and then does one of three things:
            1) SINGLE: Obtains the destination value by calling the 'matrix_decoder'
            2) JUMP: Calculates the JUMP offset by having access to the jump union
            3) DOUBLE: Obtains both the source and destination values by calling the 'matrix_decoder' twice.
            -> In all 3 cases, the clock is incremented by the correct value for the instruction with
                the current addressing modes.
*/
void addressing_mode_fetcher(INST_TYPE type) {
    // Initialize these to something wrong for debugging
    src = -1;
    dst = -1;
    uint16_t src_mode = -1;

    switch (type) {
        case SINGLE:
            // RETI does not need variables
            if (single.us_single == RETI_OPCODE) break;

            // Instructions emit by default
            emit_flag = true;

            // Get the destination address (Name is irrelevent, SRC = DST in single operand)
            dst = matrix_decoder(single.as, single.reg, single.bw);

            // Update clock according to lookup table
            cpu_clock += single_op_clock[single.opcode & UNIQUE_OP_MASK][mode];

            break;

        case JUMP:
            // Fetch initial offset from union
            offset = jump.offset;

            std::cout << "\t\tOFFSET IS >>" << std::hex << offset << std::dec << "<<\n";

            // Shift to the left by one
            offset <<= 1;

            std::cout << "\t\tOFFSET IS >>" << std::hex << offset << std::dec << "<<\n";

            // Perform sign extend by checking if it is greater than or equal to
            // the largest possible value
            if (offset >= NEG_JUMP_CHECK) offset += JUMP_SIGN_EXTEND;
            // Else, already positive, leave as is

            std::cout << "\t\tOFFSET IS >>" << std::hex << offset << std::dec << "<<\n";

            // Clock always updates by 2 for jump instructions
            cpu_clock += JUMP_CLOCK_INC;

            break;

        case DOUBLE:
            // Instructions emit by default
            emit_flag = true;

            src = matrix_decoder(dbl.as, dbl.src, dbl.bw);

            // Store the mode from the global 'mode' before it is overwritten by the dst mode
            src_mode = mode;

            // If using the constant generator, pretend it's immediate
            // TODO: Ask Dr Hughes about this?
            if (src_mode > IMMEDIATE) src_mode = IMMEDIATE;

            dst = matrix_decoder(dbl.ad, dbl.dst, dbl.bw);

            // Update clock according to lookup table
            cpu_clock += double_op_clock[src_mode][mode];
            break;
                
        default:
            std::cout << "[ADDR MODE FETCHER] THIS IS BROKEN\n";
            exit(1);
            break;
    }
}

/*
    Function: matrix_decoder
    Input:  asd: The AS or AD value from the opcode
            regnum: The register number to act upon
            bw: Byte or word instruction flag
    Output: return_val: Either SRC or DST data for instructions
    Brief: This function decodes the 'src_dst_matrix' found in emulate.h and
            using the outputted mode in a switch case to return the proper value.
            Once the mode is known, it is a simple matter of calling the BUS or directly
            accessing the regfile to obtain the value.
*/

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
            // Relative address is a full word
            bus(regfile[PC], mdr, READ_W);

            // Set effective address to the location of the PC plus 
            std::cout << "\t\t\tMDR (RELATIVE OFFSET): >>" << std::hex << mdr << std::dec << "<<" << std::endl;
            eff_address = mdr + regfile[regnum];

            // Fetch the actual value, store in mdr
            bus(eff_address, mdr, (bw ? READ_B : READ_W));

            return_val = mdr;
            break;

        case ABSOLUTE:
            // Fetch the absolute address, store in mdr
            // Absolute address is a full word
            bus(regfile[PC], mdr, READ_W);

            eff_address = mdr;

            // Fetch the actual value, store in MDR
            bus(eff_address, mdr, (bw ? READ_B : READ_W));
            return_val = mdr;

            break;

        case INDIRECT:
        case INDIRECT_AI:
            // Set the effective address to the value stored in the given register
            eff_address = regfile[regnum];

            // Fetch the desired address, store in MDR
            bus(eff_address, mdr, READ_W);
            return_val = mdr;

            // Increment the register if in INDIRECT_AI mode
            regfile[regnum] += (mode == INDIRECT_AI ? (bw ? BYTE : WORD) : 0);

            break;

        case IMMEDIATE:
            // Fetch the value directly
            bus(regfile[PC], mdr, (bw ? READ_B : READ_W));

            return_val = mdr;

            break;

        default:  // Constant generator cases, denoted by 0xC#
            // Get rid of the upper nibble (0xC_ portion)
            return_val = mode & Nibble_Mask;  

            // Check if value is encoded for -1, denoted as its two's compliment, 0xf
            if (return_val == Nibble_Mask) return_val = -1;
            break;
    }
    // For modes other than the default, increment the program counter accordingly
    // Immediate is the highest emumerated state, therefore anything above is
    // the constant generator
    if (mode <= IMMEDIATE) regfile[PC] += addr_mode_PC_array[mode];

    std::cout << "\t\t\t(Matrix Decoder) READ VALUE: >>" << std::hex << return_val << std::dec << "<<\n";

    return return_val;
}

/*
    Function: update_sr
    Input:  bw: Byte or word instruction flag
    Output: sr_union (Globally): The SR_union is tied to regfile[SR] and updates it accordingly 
    Brief: This function performs the general status register updates that typically
            happen in instructions. All special cases required by inidividual instructions,
            including all overflow cases, are hardcoded into the instructions themselves.
*/
void update_sr(bool bw) {
    // Global 'result' from last instruction
    std::cout << "\t\t\t\tUPDATING SR WITH RESULT: >>" << std::hex << result << "<<\n";

    // First clear all values
    sr_union->Z = 0;
    sr_union->N = 0;
    sr_union->C = 0;
    sr_union->V = 0;

    // Only set values that should be set, everything else is left unset

    // Set ZERO flag if the result is zero
    if (!(result&(bw ? BYTE_MAX : WORD_MAX))) sr_union->Z = 1;

    // Set NEGATIVE flag is bit 8 or 16 is set (Byte or Word)
    if (bw ? ((result & BYTE_MAX)>>BYTE_N_REVEAL) : ((result & WORD_MAX)>>WORD_N_REVEAL)) sr_union->N = 1;

    // Set CARRY flag if bit 9 or 17 is set (Byte or Word)
    if (bw ? ((result & BYTE_C_CHECK)>>BYTE_C_REVEAL) : ((result & WORD_C_CHECK)>>WORD_C_REVEAL)) sr_union->C = 1;

    // Set OVERFLOW flag through the logic performed in ADD, ADDC, SUB, SUBC, CMP, and XOR_

    std::cout << "\t\t\t\tUPDATING SR TO: >>" << regfile[SR] << "<<\n" << std::dec;
}

/*
    Function: put_operand
    Input: asd: Byte or word instruction flag
           type: Instruction type to store value of
    Brief: this function outputs the result of one and two operand instructions
            according to the given AS or AD value. The inputted TYPE (SINGLE or
            DOUBLE) also determines the valid addressing modes.
*/
void put_operand(uint16_t asd, INST_TYPE type) {
    uint16_t regnum = (type == SINGLE) ? single.reg : dbl.dst;
    uint16_t bw = (type == SINGLE) ? single.bw : dbl.bw;

    // mode should be set already (DST was called last in both SINGLE and DOUBLE): TODO: TEST THIS
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
            if (type == SINGLE && single.opcode == PUSH_OPCODE) bus(eff_address, mdr, (bw ? WRITE_B : WRITE_W));
            else emulation_error("(Put Operand) Invalid Immediate dst");
            break;

        default:
            emulation_error("(Put Operand) Invalid Constant Generator dst");
            break;
    }
    std::cout << "\t\t\t\t\t(Put Operand) ASD: " << asd << " | REGNUM: " << regnum << " | MODE: " << mode << " | EFF_ADDRESS: 0x" << std::hex << eff_address << std::endl << std::dec;
}

/*
    Function: bus
    Input:  mar:  Address byte, location in memory to read or write
            &mdr: Data byte reference, either contains data to write or returns data (by reference)
            ctrl: Control flag that differentiates between the types of bus calls
    Brief:  This function simulates an MSP-430 bus by using and MAR, MDR, and CTRL bit. This
            function either writes to a specific memory location by byte or word, or it reads
            a specific memory location by byte or word. It should be noted that memory is
            stored LITTLE-ENDIAN, meaning the LSB goes in the lower memory location. In the case
            that the call tries to access memory locations below 32, the device_bus is called
            to handle the request.
*/
void bus(uint16_t mar, uint16_t &mdr, BUS_CTRL ctrl) {
    // If the user attempts to access memory below 32,
    // initiate the device memory access function
    if (mar <= DEVICE_MEM_MAX) {
        std::cout << "\t\t\t\tDEVICE BUS BEING CALLED: " << mar << "\n";
        device_bus(mar, mdr, ctrl);
    }
    else {
        switch (ctrl) {
            case READ_W:
                mdr = mem_array[mar];
                mdr += (mem_array[mar+1] << BYTE_WIDTH);
                break;
            case READ_B:
                mdr = mem_array[mar];

                break;
            case WRITE_W:
                mem_array[mar] = (mdr & BYTE_MAX);
                mem_array[mar+1] = ((mdr >> BYTE_WIDTH) & BYTE_MAX);

                break;
            case WRITE_B:
                mem_array[mar] = (mdr & BYTE_MAX);

                break;
            default:
                std::cout << "[BUS] INVALID BUS INPUT - ENDING" << std::endl;
                exit(1);
                break;
        }
    }
}

/*
    Function: emulation_error
    Input:  error_msg:  Error message to print
    Brief:  This function simply outputs an error message and exits the program.
            This is used for debugging purposes and should probably be removed.
*/
void emulation_error(std::string error_msg) {
    std::cout << "[EMULATION ERROR] - " << error_msg << std::endl;
    exit(1);
}

