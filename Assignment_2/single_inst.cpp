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

void rrc() {
    uint32_t even_dst = ((dst%2) ? dst - 1 : dst);

    std::cout << "RRC SR: >>" << std::hex << regfile[SR] << "<< || shift: >>" << even_dst << " | " << (even_dst >> 1) << "<<" << std::dec << "<<\n";

    result = (even_dst >> 1) + ((regfile[SR]&0x1)<<(single.bw ? 7 : 15));

    update_sr(single.bw);

    // Set carry bit to LSB of dst
    sr_union->C = (dst&0x0001) ? 1 : 0;

    std::cout << "\t\t\t\tEXECUTING RRC (DST >>" << std::hex << dst << std::dec << "<<)\n";
}

void swpb() {
    if (!single.bw) result = (dst >> 8) + (dst << 8);
    else emulation_error("(swpb) Byte attempted on Word only instruction");

    std::cout << "\t\t\t\tEXECUTING SWPB (DST >>" << std::hex << dst << std::dec << "<<)\n";
}

void rra() {
    // RRA sets the MSB to itself after shifting (TODO: FIX)
    // uint32_t even_dst = ((dst%2) ? dst - 1 : dst);
    result = (dst >> 1) + ((dst&(single.bw ? 0x0080 : 0x8000)));

    std::cout << "\t\t\t\tRRA SHITED ONCE >>" << std::hex << (dst >> 1) << std::dec << "<<" << std::endl;
    std::cout << "\t\t\t\tRRA MSB SET TO  >>" << std::hex << ((dst<<(single.bw ? 7 : 15))&0xff) << std::dec << "<<" << std::endl;

    std::cout << "\t\t\t\tEXECUTING RRA (DST >>" << std::hex << dst << std::dec << "<<)\n";

    update_sr(single.bw);

    // Set carry bit to the LSB of the unrotated value

    std::cout << "RRA CARRY SET TO: >>" << std::hex << regfile[SR] << std::dec << "<<\n";

    // Set carry bit to LSB of dst
    sr_union->C = (dst&0x0001) ? 1 : 0;

    std::cout << "RRA SR: >>" << std::hex << regfile[SR] << std::dec << "<<\n";

    std::cout << "RRA RESULT IS >>" << std::hex << result << "<<" << std::dec << std::endl;
}

void sxt() {
    result = (dst & 0xff) + (((dst & 0xff) >> 7) ? 0xff00 : 0);

    std::cout << "\t\t\t\tEXECUTING SXT (DST >>" << std::hex << dst << std::dec << "<<)\n";

    update_sr(single.bw);

    // Set carry bit to the opposite of the negative bit
    sr_union->C = (sr_union->N ? 0 : 1);
}

void push() {  // NO SR
    // If byte instruction, sign extend
    result = dst;
    if (single.bw) result = (dst & 0xff) + (((dst & 0xff) >> 7) ? 0xff00 : 0);

    // Call bus directly due to special case
    mdr = result;

    std::cout << "\t\t\t\tEXECUTING PUSH (MDR >>" << std::hex << mdr << std::dec << "<<)\n";

    regfile[SP] -= 2;

    bus(regfile[SP], mdr, WRITE_W);

    emit_flag = false;
}

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

void reti() {
    // Implement with stuff
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
