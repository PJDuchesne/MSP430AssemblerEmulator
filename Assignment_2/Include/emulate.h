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

-> Name:  emulate.h
-> Brief: header file for emulate code
-> Date: June 16, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include "library.h"

#ifndef EMULATE_H_
#define EMULATE_H_

static bool HCF = false;

static bool debug_mode;
static bool debug_signal = false;

static uint16_t addr_mode_PC_array[] = {0, 2, 2, 2, 0, 0, 2};

// TO CONVERT TO ENUM EVENTUALLY, BUT IT WORKS LIKE THIS
// Note: 0xC# Denotes a constant
static uint16_t src_dst_matrix[4][16] = {
    {0, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 3, 0xC1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {4, 4, 0xC4, 0xC2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {6, 5, 0xC8, 0xCF, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}
};

// Returns how many clock cycles a single operand INST should take
// Takes: [INST CODE (0 through 6)][MODE]
// From Table 3-15: Page 60 (Family Guide)
static uint16_t single_op_clock[7][7] = {
    {1, 4, 4, 4, 3, 3, 0},
    {1, 4, 4, 4, 3, 3, 0},
    {1, 4, 4, 4, 3, 3, 0},
    {1, 4, 4, 4, 3, 3, 0},
    {3, 5, 5, 5, 4, 5, 4},
    {4, 5, 5, 5, 4, 5, 5},
    {5, 5, 5, 5, 5, 5, 5}
};

// Returns how many clock cycles a double operand INST should take
// Takes: [SRC MODE][DST MODE]
// From Table 3-16: Page 61 (Family Guide)
static uint16_t double_op_clock[7][4] = {
    {1, 4, 4, 4},
    {3, 6, 6, 6},
    {3, 6, 6, 6},
    {3, 6, 6, 6},
    {2, 5, 5, 5},
    {2, 5, 5, 5},
    {2, 5, 5, 5}
};

// ORDER: JUMP TYPE, ZERO, NEGATIVE, CARRY, OVERFLOW
// 8 -> Types of jump, 2 -> Z, 2 -> N, 2 -> C, 2 -> V
// See report for details of how this is mapped
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

// Returns true/false to indicate error or not
bool emulate(uint8_t *mem, bool debug_mode_, uint16_t PC_init);

void decode_execute();

void addressing_mode_fetcher(int type);

// uint16_t matrix_decoder(uint8_t asd, uint8_t regnum, bool bw);
uint32_t matrix_decoder(uint16_t asd, uint16_t regnum, uint16_t bw);

void update_sr(bool bw);

void put_operand(uint16_t asd, INST_TYPE type);

void bus(uint16_t mar, uint16_t &mdr, int ctrl);

void emulation_error(std::string error_msg);

#endif
