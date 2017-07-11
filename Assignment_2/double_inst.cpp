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

-> Name:  double_inst.cpp
-> Brief: Implementation for the double_inst.cpp code
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
#include "Include/double_inst.h"

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
        sr_union->V = 1;
    }
}

void addc() {
    result = src + dst + sr_union->C;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING ADDC (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // If SRC and DST have the same sign, and the result has the opposite sign. Set the overflow bit
    // Note: Overflow bit was reset in update_sr (No need to reset it again)
    if (((src < 0x8000 && dst < 0x8000)&&(result >= 0x8000))||((src >= 0x8000 && dst >= 0x8000)&&(result < 0x8000))) {
        sr_union->V = 1;
    }
}

void subc() {
    // result = dst + ~src + 1 + sr_union->C; TODO: WHY is that ONE not there?

    std::cout << "SR UNION IS: " << std::hex << sr_union->C << std::dec << "\n";

    result = dst + ~src + 1 + sr_union->C;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING SUBC (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // If SRC and DST have opposite signs, and the result has the same sign as the destination. Set the overflow bit
    // Note: Overflow bit was reset in update_sr (No need to reset it again)
    if (((src < 0x8000 && dst >= 0x8000)&&(result < 0x8000))||((src >= 0x8000 && dst < 0x8000)&&(result >= 0x8000))) {
        sr_union->V = 1;
    }
}

void sub() {
    result = dst + ~src + 1;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING SUB (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // If SRC and DST have opposite signs, and the result has the same sign as the destination. Set the overflow bit
    // Note: Overflow bit was reset in update_sr (No need to reset it again)
    if (((src < 0x8000 && dst >= 0x8000)&&(result < 0x8000))||((src >= 0x8000 && dst < 0x8000)&&(result >= 0x8000))) {
        sr_union->V = 1;
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
        sr_union->V = 1;
    }

    std::cout << "\t\t\t\tOPERATION RESULT IS: >>" << std::hex << result << std::dec << "<<\n";
}

void dadd() {
    // TODO: FINISH

    uint16_t src_nib, dst_nib;
    uint32_t dec_result;
    bool carry = false;

    result = 0;

    for (int i = 0; i < 4; i++) {  // 4 = Nibble count
        dec_result = (src&0x000f) + (dst&0x000f) + carry;

        if (dec_result >= 10) {
            carry = true;
            dec_result -= 10;
        }
        else carry = false;

        result += dec_result<<(i*4);

        src = src>>4;
        dst = dst>>4;
    }

    std::cout << "\t\t\t\tEXECUTING DADD (SRC >>" << std::hex << (result&(dbl.bw ? 0x1000 : 0x10000)) << "<< || DST: >>" << dst << "<< || RESULT: >>" << result << std::dec << "<<)\n";

    update_sr(dbl.bw);

    // // Set carry flag if the result is greater than 9999 (W) or 99 (B)
    sr_union->C = ((result&(dbl.bw ? 0x1000 : 0x10000)) ? true : false);
}

void bit() {  // NO EMIT
    result = dst & src;

    emit_flag = false;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING BIT (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // Set carry bit to the opposite of the negative bit
    sr_union->C = (sr_union->N ? 0 : 1);

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
    sr_union->C = (sr_union->N ? 0 : 1);

    // If SRC and DST are negative, set overflow bit
    sr_union->V = (src >= (dbl.bw ? 0x0080 : 0x8000) && dst >= (dbl.bw ? 0x0080 : 0x8000)) ? 1 : 0;


    std::cout << "\t\t\t\tEXECUTING XOR_ (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";
}

void and_() {
    result = src & dst;
    update_sr(dbl.bw);

    // Set carry bit to the opposite of the negative bit
    sr_union->C = (sr_union->N ? 0 : 1);

    std::cout << "\t\t\t\tEXECUTING AND_ (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";
}
