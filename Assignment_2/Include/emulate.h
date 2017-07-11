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

-> Name:  library.h
-> Brief: header file for emulate code
-> Date: June 16, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include "library.h"

#ifndef EMULATE_H_
#define EMULATE_H_

static bool HCF = false;

static bool debug_signal = false;

// This is used to inrement the PC based on the addressing mode
// found in the 'matrix_decoder'
static uint16_t addr_mode_PC_array[] = {0, 2, 2, 2, 0, 0, 2};

/* 
    Name: src_dst_matrix
    Inputs: AS or AD: This is taken from the instruction itself
            Regnum: This is the associated register with the AS or AD command
    Output: The return value is the mode for the given input values
    Brief: This table used in the 'matrix_decoder' to find
            the addressing mode of either the SRC or DST
            data given the current AS or AD and its corresponding
            regnum. The constant generator is also built in
            with the prefix 0xC to separate them from the rest
            --> Note: These could used the enumerations defined for
                ADDR_MODE, but that would bulk up the code. 
*/
static uint16_t src_dst_matrix[4][16] = {
    {0, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 3, 0xC1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {4, 4, 0xC4, 0xC2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {6, 5, 0xC8, 0xCF, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}
};

/* 
    Name: single_operand_clock
    Inputs: Single_Opcode (&0x7): The 3 LSB of the current opcode
            Mode: The addressing mode of that instruction
    Output: The clock timing for that instruction and addressing mode combination
    Brief: This table used to increment the clock cycles
            of one operand instructions by entering the instruction
            (Given by the LSB 3 bits of the opcode, numbers 0 through 6)
            and the addressing mode. The times are taken from Table 3-15
            on page 60 of the MSP-430 Family Guide
*/
static uint16_t single_op_clock[7][7] = {
    {1, 4, 4, 4, 3, 3, 0},
    {1, 4, 4, 4, 3, 3, 0},
    {1, 4, 4, 4, 3, 3, 0},
    {1, 4, 4, 4, 3, 3, 0},
    {3, 5, 5, 5, 4, 5, 4},
    {4, 5, 5, 5, 4, 5, 5},
    {5, 5, 5, 5, 5, 5, 5}
};

/* 
    Name: double_op_clock
    Inputs: SRC Mode: The addressing mode of that instruction's DST data
            DST Mode: The addressing mode of that instruction's DST data
    Output: The clock timing for that double addressing mode combination
    Brief: This table used to increment the clock cycles
            of two operand instructions by entering the SRC
            addressing mode, followed by the DST addressing
            mode. The times are taken from Table 3-16 on page
            61 of the MSP-430 Family Guide
*/
static uint16_t double_op_clock[7][4] = {
    {1, 4, 4, 4},
    {3, 6, 6, 6},
    {3, 6, 6, 6},
    {3, 6, 6, 6},
    {2, 5, 5, 5},
    {2, 5, 5, 5},
    {2, 5, 5, 5}
};

/* 
    Name: jmp_matrix
    Inputs: JMP_Opcode (&0x7): The jump instruction number
            ZERO:       The zero bit from the status register
            NEGATIVE:   The negative bit from the status register
            CARRY:      The carry bit from the status register
            OVERFLOW:   The overflow bit from the status register
    Brief: This table is used to perform all jump commands by inputing
            the specific jump (Based on the 3 LSB of the opcode), followed by
            the status register bits in the oder: ZERO, NEGATIVE, CARRY,
            and OVERFLOW. See the design report for a full truth table.
*/
static bool jmp_matrix[8][2][2][2][2] = {
    // JNE/JNZ -> Jump if Z = 0
    {  // 8 of these (1/5)
        {  // 2 of these (2/5)
            {  // 2 of these (3/5)
                {1, 1}, {1, 1}  // Two 2x2 arrays (4/5 and 5/5)
            },
            {
                {1, 1}, {1, 1}
            }
        },
        {
            {
                {0, 0}, {0, 0}
            },
            {
                {0, 0}, {0, 0}
            }
        }
    },

    // JEQ/JZ -> Jump if Z = 1
    {
        {
            {
                {0, 0}, {0, 0}
            },
            {
                {0, 0}, {0, 0}
            }
        },
        {
            {
                {1, 1}, {1, 1}
            },
            {
                {1, 1}, {1, 1}
            }
        }
    },

    // JNC/JLO --> Jump if C = 0
    {
        {
            {
                {1, 1}, {0, 0}
            },
            {
                {1, 1}, {0, 0}
            }
        },
        {
            {
                {1, 1}, {0, 0}
            },
            {
                {1, 1}, {0, 0}
            }
        }
    },

    // JC/JHS --> Jump if C = 1
    {
        {
            {
                {0, 0}, {1, 1}
            },
            {
                {0, 0}, {1, 1}
            }
        },
        {
            {
                {0, 0}, {1, 1}
            },
            {
                {0, 0}, {1, 1}
            }
        }
    },

    // JN --> Jump if N = 1
    {
        {
            {
                {0, 0}, {0, 0}
            },
            {
                {1, 1}, {1, 1}
            }
        },
        {
            {
                {0, 0}, {0, 0}
            },
            {
                {1, 1}, {1, 1}
            }
        }
    },

    // JGE --> Jump if (N XOR V) = 0 (They are the same, both 0 or 1)
    {
        {
            {
                {1, 0}, {1, 0}
            },
            {
                {0, 1}, {0, 1}
            }
        },
        {
            {
                {1, 0}, {1, 0}
            },
            {
                {0, 1}, {0, 1}
            }
        }
    },

    // JL --> Jump if (N XOR V) = 1 (They are different)
    {
        {  // Z = 0
            {  // N = 0
                {0, 1}, {0, 1}  // C = 0/1, V = 0/1
            },
            {  // N = 1
                {1, 0}, {1, 0}    // C = 0/1, V = 0/1
            }
        },
        {
            {
                {0, 1}, {0, 1}
            },
            {
                {1, 0}, {1, 0}
            }
        }
    },

    // JMP --> Always jump
    {
        {
            {
                {1, 1}, {1, 1}
            },
            {
                {1, 1}, {1, 1}
            }
        },
        {
            {
                {1, 1}, {1, 1}
            },
            {
                {1, 1}, {1, 1}
            }
        }
    }
};

void signalHandler(int signum);

bool emulate(uint16_t PC_init);

void decode_execute();

void addressing_mode_fetcher(INST_TYPE type);

uint32_t matrix_decoder(uint16_t asd, uint16_t regnum, uint16_t bw);

void update_sr(bool bw);

void put_operand(uint16_t asd, INST_TYPE type);

void bus(uint16_t mar, uint16_t &mdr, BUS_CTRL ctrl);

void emulation_error(std::string error_msg);

#endif
