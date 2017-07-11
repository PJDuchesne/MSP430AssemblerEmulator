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

-> Name:  single_inst.cpp
-> Brief: Implementation for the single_inst.cpp code
-> Date: July 3, 2017    (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include <stdio.h>
#include <inttypes.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

#include "Include/emulate.h"
#include "Include/library.h"
#include "Include/single_inst.h"

/*
    Function: rrc
    Input: DST (Globally): Source and Destination Data
    Output: Results: Results of the computation
            SR_REG : Any Status Register updates that the operation caused
    Brief: RRC rotates the input data 1 place to the right through carry. The
            status bits are updated normally
*/
void rrc() {
    uint32_t even_dst = ((dst%2) ? dst - 1 : dst);

    std::cout << "RRC SR: >>" << std::hex << regfile[SR] << "<< || shift: >>" << even_dst << " | " << (even_dst >> 1) << "<<" << std::dec << "<<\n";

    result = (even_dst >> 1) + ((regfile[SR]&1)<<(single.bw ? BYTE_N_REVEAL : WORD_N_REVEAL));

    update_sr(single.bw);

    // Set carry bit to LSB of dst
    sr_union->C = (dst&1) ? 1 : 0;

    std::cout << "\t\t\t\tEXECUTING RRC (DST >>" << std::hex << dst << std::dec << "<<)\n";
}

/*
    Function: swpb
    Input: DST (Globally): Source and Destination Data
    Output: Results: Results of the computation
    Brief: SWPB swaps the bytes of the input, the status bits are not affected
*/
void swpb() {
    if (!single.bw) result = (dst >> BYTE_C_REVEAL) + (dst << BYTE_C_REVEAL);
    else emulation_error("(swpb) Byte attempted on Word only instruction");

    std::cout << "\t\t\t\tEXECUTING SWPB (DST >>" << std::hex << dst << std::dec << "<<)\n";
}

/*
    Function: rra
    Input: DST (Globally): Source and Destination Data
    Output: Results: Results of the computation
            SR_REG : Any Status Register updates that the operation caused
    Brief: RRA rotates the input data 1 place to the right through carry. The
            sign bit is also extended into itself instead of being rotated right.
            The status bits are updated normally.
*/
void rra() {
    result = (dst >> 1) + ((dst&(single.bw ? BT_N_CHECK : WD_N_CHECK)));

    std::cout << "\t\t\t\tRRA SHITED ONCE >>" << std::hex << (dst >> 1) << std::dec << "<<" << std::endl;
    std::cout << "\t\t\t\tRRA MSB SET TO  >>" << std::hex << ((dst<<(single.bw ? BYTE_N_REVEAL : WORD_N_REVEAL))&BYTE_MAX) << std::dec << "<<" << std::endl;

    std::cout << "\t\t\t\tEXECUTING RRA (DST >>" << std::hex << dst << std::dec << "<<)\n";

    update_sr(single.bw);

    std::cout << "RRA CARRY SET TO: >>" << std::hex << regfile[SR] << std::dec << "<<\n";

    // Set carry bit to the LSB of the unrotated value
    sr_union->C = (dst&1) ? 1 : 0;

    std::cout << "RRA SR: >>" << std::hex << regfile[SR] << std::dec << "<<\n";

    std::cout << "RRA RESULT IS >>" << std::hex << result << "<<" << std::dec << std::endl;
}

/*
    Function: sxt
    Input: DST (Globally): Source and Destination Data
    Output: Results: Results of the computation
            SR_REG : Any Status Register updates that the operation caused
    Brief: SXT extends the sign of the input data by taking the 7th bit and storing
            it in the 8-15th bit locations. The status bits are updated normally.
*/
void sxt() {
    result = (dst & BYTE_MAX) + (((dst & BYTE_MAX) >> BYTE_N_REVEAL) ? BYTE_SIGN_EXTEND : 0);

    std::cout << "\t\t\t\tEXECUTING SXT (DST >>" << std::hex << dst << std::dec << "<<)\n";

    update_sr(single.bw);

    // Set carry bit to the opposite of the negative bit
    sr_union->C = (sr_union->N ? 0 : 1);
}

/*
    Function: push
    Input: DST (Globally): Source and Destination Data
    Output: Results: Results of the computation
    Brief: PUSH takes the data value and pushes it to the stack. If it is a byte
            instruction, it first sign extends it because the stack is 1 word wide.
            The status registers are not affected.
*/
void push() {  // NO SR
    // If byte instruction, sign extend
    result = dst;
    if (single.bw) result = (dst & BYTE_MAX) + (((dst & BYTE_MAX) >> BYTE_N_REVEAL) ? BYTE_SIGN_EXTEND : 0);

    // Call bus directly due to special case
    mdr = result;

    std::cout << "\t\t\t\tEXECUTING PUSH (MDR >>" << std::hex << mdr << std::dec << "<<)\n";

    regfile[SP] -= WORD;

    bus(regfile[SP], mdr, WRITE_W);

    emit_flag = false;
}

/*
    Function: call
    Input: DST (Globally): Source and Destination Data
    Output: Results: Results of the computation
    Brief: CALL takes the data value and calls a function at that location.
            The status registers are not affected.
*/
void call() {
    // Call bus directly due to special case

    std::cout << "\t\t\t\tEXECUTING CALL (DST >>" << std::hex << dst << std::dec << "<<)\n";

    mdr = regfile[PC];

    // Push PC to stack
    regfile[SP] -= 2;
    bus(regfile[SP], mdr, WRITE_W);
    // Store Source to Program counter
    regfile[PC] = dst;

    emit_flag = false;
}

/*
    Function: reti
    Input: DST (Globally): Source and Destination Data
    Output: Results: Results of the computation
    Brief: RETI returns from interrupt by popping the SR from the stack
            and then popping the PC from the stack. It also temporarily
            disables GIE for 1 fetch-decode-execute cycle in order to force
            control of the code back to the machine for at least 1 command.
            This is a design choice that prevents interrupt storms from being
            a complete disaster.
*/
void reti() {
    std::cout << "\t\t\t\tEXECUTING RETI (NO DST)\n";

    // Pop SR
    bus(regfile[SP], mdr, READ_W);
    regfile[SP] += 2;   // Move SP to point to next position on stack
    regfile[SR] = mdr;
    std::cout << "\t\t\t\tPOPPED SR AS: " << std::hex <<  regfile[SR] << std::dec << std::endl;

    // Pop PC
    bus(regfile[SP], mdr, READ_W);
    regfile[SP] += 2;   // Move SP to point to next position on stack
    regfile[PC] = mdr;
    std::cout << "\t\t\t\tPOPPED PC AS: " << std::hex <<  regfile[PC] << std::dec << std::endl;

    temp_GIE_disable = true;
    emit_flag = false;
}
