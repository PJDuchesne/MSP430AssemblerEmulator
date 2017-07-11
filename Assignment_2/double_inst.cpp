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

// Returns TRUE for positive, FALSE for negative
#define get_sign(value) ((value < (dbl.bw ? BT_N_CHECK : WD_N_CHECK)))

/*
    Function: NAME
    Input:  SRC (Globally): Source Data
            DST (Globally): Destination Data
    Output: Results: Results of the computation
            SR_REG : Any Status Register updates that the operation caused
    Brief: DESCRIPTION
*/

/*
    Function: mov
    Input:  SRC (Globally): Source Data
            DST (Globally): Destination Data
    Output: Results: Results of the computation
    Brief: MOV takes the SRC data and places it in the DST address. The DST
            data is lost and the status register is not updated.
*/
void mov() {
    result = src;

    std::cout << "\t\t\t\tEXECUTING MOV (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";
}

/*
    Function: add
    Input:  SRC (Globally): Source Data
            DST (Globally): Destination Data
    Output: Results: Results of the computation
            SR_REG : Any Status Register updates that the operation caused
    Brief: ADD takes the SRC data and adds it to the DST data before storing
            that result in the DST location. The status register is updated normally,
            plus an overflow check is performed.
*/
void add() {
    result = src + dst;
    update_sr(dbl.bw);

    std::cout << "\t\t\t\tEXECUTING ADD (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // If SRC and DST have the same sign, and the result has the opposite sign. Set the overflow bit
    // Note: Overflow bit was reset in update_sr (No need to reset it again)
    if ((get_sign(src)&&get_sign(dst))&&(!get_sign(result))||((!get_sign(src)&&!get_sign(dst))&&(get_sign(result)))) {
        sr_union->V = 1;
    }
}

/*
    Function: addc
    Input:  SRC (Globally): Source Data
            DST (Globally): Destination Data
    Output: Results: Results of the computation
            SR_REG : Any Status Register updates that the operation caused
    Brief: ADDC takes the SRC data and adds it to the DST data and the carry flag before storing
            that result in the DST location. The status register is updated normally,
            plus an overflow check is performed.
*/
void addc() {
    result = src + dst + sr_union->C;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING ADDC (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // If SRC and DST have the same sign, and the result has the opposite sign. Set the overflow bit
    // Note: Overflow bit was reset in update_sr (No need to reset it again)
    if ((get_sign(src)&&get_sign(dst))&&(!get_sign(result))||((!get_sign(src)&&!get_sign(dst))&&(get_sign(result)))) {
        sr_union->V = 1;
    }
}

/*
    Function: subc
    Input:  SRC (Globally): Source Data
            DST (Globally): Destination Data
    Output: Results: Results of the computation
            SR_REG : Any Status Register updates that the operation caused
    Brief: SUBC subtracts the SRC data from the DST and adds the carry bit before
            storing the result in the DST location. The status register is updated
            normally, plus an overflow check is performed.
*/
void subc() {
    result = dst + ~src + 1 + sr_union->C;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING SUBC (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // If SRC and DST have opposite signs, and the result has the same sign as the destination. Set the overflow bit
    // Note: Overflow bit was reset in update_sr (No need to reset it again)
    if ((get_sign(src)&&!get_sign(dst))&&(!get_sign(result))||((!get_sign(src)&&get_sign(dst))&&(get_sign(result)))) {
        sr_union->V = 1;
    }
}

/*
    Function: sub
    Input:  SRC (Globally): Source Data
            DST (Globally): Destination Data
    Output: Results: Results of the computation
            SR_REG : Any Status Register updates that the operation caused
    Brief: SUB subtracts the SRC data from the DST before storing the
            result in the DST location. The status register is updated
            normally, plus an overflow check is performed.
*/
void sub() {
    result = dst + ~src + 1;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING SUB (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // If SRC and DST have opposite signs, and the result has the same sign as the destination. Set the overflow bit
    // Note: Overflow bit was reset in update_sr (No need to reset it again)
    if ((get_sign(src)&&!get_sign(dst))&&(!get_sign(result))||((!get_sign(src)&&get_sign(dst))&&(get_sign(result)))) {
        sr_union->V = 1;
    }
}

/*
    Function: cmp
    Input:  SRC (Globally): Source Data
            DST (Globally): Destination Data
    Output: Results: Results of the computation
            SR_REG : Any Status Register updates that the operation caused
    Brief: CMP subtracts the SRC data from the DST before updating the the
            status register with the result. The status register is updated
            normally, plus an overflow check is performed.
*/
void cmp() {  // NO EMIT
    result = dst + ~src + 1;

    emit_flag = false;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING CMP (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // If SRC and DST have opposite signs, and the result has the same sign as the destination. Set the overflow bit
    // Note: Overflow bit was reset in update_sr (No need to reset it again)
    if ((get_sign(src)&&!get_sign(dst))&&(!get_sign(result))||((!get_sign(src)&&get_sign(dst))&&(get_sign(result)))) {
        sr_union->V = 1;
    }

    std::cout << "\t\t\t\tOPERATION RESULT IS: >>" << std::hex << result << std::dec << "<<\n";
}

/*
    Function: dadd
    Input:  SRC (Globally): Source Data
            DST (Globally): Destination Data
    Output: Results: Results of the computation
            SR_REG : Any Status Register updates that the operation caused
    Brief: DADD performs decimal addition of the nibbles with varry between
            the nibbles. It is assumed that the input hex numbers do not have
            nibbles with values above 9. The result is stored in the DST
            address, and the SR is updated normally except the carry flag is set
            if the value goes over 99 or 9999 (byte or word).
*/
void dadd() {
    uint16_t src_nib, dst_nib;
    uint32_t dec_result;
    bool carry = false;

    result = 0;

    for (int i = 0; i < DADD_NIBBLES; i++) {  // 4 = Nibble count
        dec_result = (src&Nibble_Mask) + (dst&Nibble_Mask) + carry;

        if (dec_result >= DEC_BASE) {
            carry = true;
            dec_result -= DEC_BASE;
        }
        else carry = false;

        result += dec_result<<(i*NIBBLE_WIDTH);

        src = src>>NIBBLE_WIDTH;
        dst = dst>>NIBBLE_WIDTH;
    }

    std::cout << "\t\t\t\tEXECUTING DADD (SRC >>" << std::hex << (result&(dbl.bw ? DADD_BT_C_CHECK : DADD_WD_C_CHECK)) << "<< || DST: >>" << dst << "<< || RESULT: >>" << result << std::dec << "<<)\n";

    update_sr(dbl.bw);

    // // Set carry flag if the result is greater than 9999 (W) or 99 (B)
    sr_union->C = ((result&(dbl.bw ? DADD_BT_C_CHECK : DADD_WD_C_CHECK)) ? true : false);
}

/*
    Function: bit
    Input:  SRC (Globally): Source Data
            DST (Globally): Destination Data
    Output: Results: Results of the computation
            SR_REG : Any Status Register updates that the operation caused
    Brief: BIT performs logical AND on the SRC and DST in order to update
            the status register based on the result.
*/
void bit() {  // NO EMIT
    result = dst & src;

    emit_flag = false;
    update_sr(dbl.bw);
    std::cout << "\t\t\t\tEXECUTING BIT (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";

    // Set carry bit to the opposite of the negative bit
    sr_union->C = (sr_union->N ? 0 : 1);

    std::cout << "\t\t\t\tOPERATION RESULT IS: >>" << std::hex << result << std::dec << "<<\n";
}

/*
    Function: bic
    Input:  SRC (Globally): Source Data
            DST (Globally): Destination Data
    Output: Results: Results of the computation
    Brief: BIC performs logical AND on the ~SRC and DST in order to clear
            the specificied bits. The status register is not affected.
*/
void bic() {
    result = dst & ~src;
    std::cout << "\t\t\t\tEXECUTING BIC (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";
}

/*
    Function: bis
    Input:  SRC (Globally): Source Data
            DST (Globally): Destination Data
    Output: Results: Results of the computation
    Brief: BIS performs logical OR on the SRC and DST in order to set
            the specificied bits. The status register is not affected.
*/
void bis() {
    result = dst | src;
    std::cout << "\t\t\t\tEXECUTING BIS (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";
}

/*
    Function: xor_
    Input:  SRC (Globally): Source Data
            DST (Globally): Destination Data
    Output: Results: Results of the computation
    Brief: XOR_ performs logical XOR on the SRC and DST and stores the
            result in the destination address. This updates the SR normally before
            checking the overflow.
            -> Note: XOR is a keyword in C++, so 'xor_' was used instead
*/
void xor_() {
    result = src ^ dst;
    update_sr(dbl.bw);

    // Set carry bit to the opposite of the negative bit
    sr_union->C = (sr_union->N ? 0 : 1);

    // If SRC and DST are negative, set overflow bit
    sr_union->V = (src >= (dbl.bw ? BT_N_CHECK : WD_N_CHECK) && dst >= (dbl.bw ? BT_N_CHECK : WD_N_CHECK)) ? 1 : 0;


    std::cout << "\t\t\t\tEXECUTING XOR_ (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";
}

/*
    Function: and_
    Input:  SRC (Globally): Source Data
            DST (Globally): Destination Data
    Output: Results: Results of the computation
    Brief: AND_ performs logical AND on the SRC and DST and stores the
            result in the destination address. This updates the SR normally.
            -> Note: AND is a keyword in C++, so 'and_' was used instead
*/
void and_() {
    result = src & dst;
    update_sr(dbl.bw);

    // Set carry bit to the opposite of the negative bit
    sr_union->C = (sr_union->N ? 0 : 1);

    std::cout << "\t\t\t\tEXECUTING AND_ (SRC >>" << std::hex << src << "<< || DST: >>" << dst << std::dec << "<<)\n";
}
