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
-> Brief: library file for enumeration declarations and other constants
-> Date: June 6, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#ifndef LIBRARY_H
#define LIBRARY_H

// Used for lower and upper bounds during checks (DELETE IF NOT USED)
#define MIN_BYTE -128
#define MAX_BYTE  256
#define MIN_WORD -32768
#define MAX_WORD  65536
#define MAX_MEM_SIZE  65536

// Global fine types for input and output across all files
extern std::ifstream fin;
extern std::ofstream outfile;
extern char mem_array[MAX_MEM_SIZE];
extern uint16_t s9_addr;

enum BUS_CTRL {
    READ_W,
    READ_B,
    WRITE_W,
    WRITE_B
};

enum NAMED_REGISTERS {
    PC,
    SP,
    SR,
    CG1 = 2,
    CG2,
    NEG_ONE = 16,  // Constant Generator Numbers
    ZERO,
    ONE,
    TWO,
    FOUR,
    EIGHT
};

enum ADDR_MODE {
    REG_DIRECT,
    INDEXED,
    RELATIVE,
    ABSOLUTE,
    INDIRECT,
    INDIRECT_AI,
    IMMEDIATE
};

enum INST_TYPE {
    SINGLE,
    JUMP,
    DOUBLE
};

enum ONE_OP_INST {
    RRC,
    SWPB,
    RRA,
    SXT,
    PUSH,
    CALL,
    RETI
};

enum JUMP_INST {
    JNE,
    JEQ,
    JNC,
    JC,
    JN,
    JGE,
    JL,
    JMP
};

struct single_overlay {
    union {
        struct {
            uint16_t reg:4;
            uint16_t as:2;
            uint16_t bw:1;
            uint16_t opcode:9;
        };
        uint16_t us_single;
    };
};

struct double_overlay {
    union {
        struct {
            uint16_t dst:4;
            uint16_t as:2;
            uint16_t bw:1;
            uint16_t ad:1;
            uint16_t src:4;
            uint16_t opcode:4;
        };
        uint16_t us_double;
    };
};

struct jump_overlay {
    union {
        struct {
            uint16_t offset:10;
            uint16_t opcode:6;
        };
        uint16_t us_jump;
    };
};

struct sr_reg {
    union {
        struct {
            uint16_t C:1;
            uint16_t Z:1;
            uint16_t N:1;
            uint16_t GIE:1;
            uint16_t C_OFF:1;
            uint16_t O_OFF:1;
            uint16_t SCG0:1;
            uint16_t SCG1:1;
            uint16_t V:1;
            uint16_t Reserved:7;
        };
        uint16_t us_sr_reg;
    };
};

void load_file();
void load_mem();
void dump_mem();

#endif
