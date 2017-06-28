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
static uint16_t mar  = 0;
static uint16_t mdr  = 0;

static uint32_t cpu_clock = 0;

// 0-15 are visible registers, 16-X are invisible
static uint16_t regfile[22] = {};  // All initialized to 0

static uint16_t src = 0;  // Not used in the case of single operand
static uint16_t dst = 0;  // Used in the case of single operand
static uint16_t offset = 0; // Used for jump commands
static uint32_t result = 0;

static int16_t signed_offset = 0;

// Used to write back to a place
static uint8_t mode = 0;
static uint32_t eff_address = 0;

static bool emit_flag = true;

static uint8_t addr_mode_PC_array[] = {0, 2, 2, 2, 0, 0, 2};

// TO CONVERT TO ENUM EVENTUALLY, BUT IT WORKS LIKE THIS
// Note: 0xC# Denotes a constant
static uint8_t src_dst_matrix[4][16] = {
    {0, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 3, 0xC1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {4, 4, 0xC4, 0xC2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {6, 5, 0xC8, 0xCF, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}
};

// Returns how many clock cycles the inst should take
// static int8_t clock_timing[][] = { }

// ORDER: JUMP TYPE, ZERO, NEGATIVE, CARRY, OVERFLOW
// 8 -> Types of jump, 2 -> Z, 2 -> N, 2 -> C, 2 -> V
static uint8_t jmp_matrix[8][2][2][2][2] = {

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

    // JGE --> Jump if (N XOR V) = 0
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

    // JL --> Jump if (N XOR V) = 1
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

// Returns true/false to indicate error or not
bool emulate(uint8_t *mem, bool debug_mode_, uint16_t PC_init);

void init_regfile();

void decode_execute();

void addressing_mode_fetcher(int type);

uint16_t matrix_decoder(uint8_t asd, uint8_t regnum, bool bw);

void update_sr(bool bw, INST_TYPE type);

void put_operand(uint8_t asd, INST_TYPE type);

void bus(uint16_t mar, uint16_t &mdr, int ctrl);

void emulation_error(std::string error_msg);

// INST: One Operand
void rrc();
void swpb();
void rra();
void sxt();
void push();
void call();
void reti();

// INST: Two Operand
void mov();
void add();
void addc();
void subc();
void sub();
void cmp();
void dadc();
void bit();
void bic();
void bis();
void xor_();  // 'xor' is a reserved C++ keyword
void and_();  // 'and' is a reserved C++ keyword

#endif
