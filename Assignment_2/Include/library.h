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

#define MAX_MEM_SIZE  65536

// Library.cpp (ISH)
#define S9_LENGTH 10

// Main.cpp (Ish)
#define MAX_DEVICES 16
#define SIMULATED_INTERRUPT_MAX 500
#define DEC_STOI_RANGE 5
#define HEX_STOI_RANGE 4
#define DEC_BASE 10
#define HEX_BASE 16
#define BYTE 1
#define WORD 2
#define CPU_REG_MAX 16

// Emulate.cpp (Ish)
#define OPCODE_SEPARATION_SHIFT 13
#define UNIQUE_OP_MASK 7
#define DOUBLE_OFFSET 4
#define RETI_OPCODE 0x1300
#define JUMP_CLOCK_INC 2
#define NEG_JUMP_CHECK 0x0400
#define JUMP_SIGN_EXTEND 0xf800
#define Nibble_Mask 0x0f
#define BYTE_MAX 0xff
#define WORD_MAX 0xffff
#define BYTE_N_REVEAL 7
#define WORD_N_REVEAL 15
#define BYTE_C_REVEAL 8
#define WORD_C_REVEAL 16
#define BYTE_C_CHECK 0x1ff
#define WORD_C_CHECK 0x1ffff
#define PUSH_OPCODE 0x120
#define DEVICE_MEM_MAX 31
#define BYTE_WIDTH 8

// single_inst.cpp and double_inst.cpp (Ish)
#define BYTE_SIGN_EXTEND 0xff00
#define UPPER_BYTE_MASK 0xff00
#define LOWER_BYTE_MASK 0x00ff
#define WD_N_CHECK 0x8000
#define BT_N_CHECK 0x0080
#define DADD_NIBBLES 4
#define NIBBLE_WIDTH 4
#define DADD_BT_C_CHECK 0x100
#define DADD_WD_C_CHECK 0x10000

// Devices.cpp (Ish)
#define ASCII_ZERO 48
#define ASCII_ONE 49
#define VECTOR_BASE 0xFFC0
#define INTERRUPT_CLOCK_INC 6
#define PUNCH_CARD_WIDTH 80

// Global fin types for input and output across all files
extern std::ifstream fin;
extern std::ifstream dev_fin;
extern std::ofstream outfile;
extern std::ofstream dev_outfile;
extern uint8_t mem_array[MAX_MEM_SIZE];
extern uint16_t s9_addr;
extern bool debug_mode;

extern uint16_t mdr;
extern uint32_t cpu_clock;
extern uint16_t regfile[CPU_REG_MAX];
extern uint32_t src;
extern uint32_t dst;
extern uint16_t offset;
extern uint32_t result;
extern bool emit_flag;
extern bool temp_GIE_disable;
extern uint16_t mode;
extern uint32_t eff_address;
extern uint16_t next_interrupt;
extern uint16_t interrupt_num;  // Max interrupt

/* 
    BRIEF: These are used with the BUS to differentiate
        between the different commands a bus can perform
*/
enum BUS_CTRL {
    READ_W,
    READ_B,
    WRITE_W,
    WRITE_B
};

/* 
    BRIEF: These are used throughout the code to refer
        to specific registers in the regfile[] array
*/
enum NAMED_REGISTERS {
    PC,
    SP,
    SR,
    CG1 = 2,
    CG2,
};

/* 
    BRIEF: These are used throughout the code to refer
        to the different addressing modes. Note they are
        declared in the order provided in the family guide.
*/
enum ADDR_MODE {
    REG_DIRECT,
    INDEXED,
    RELATIVE,
    ABSOLUTE,
    INDIRECT,
    INDIRECT_AI,
    IMMEDIATE
};

/* 
    BRIEF: These are used as conditionals and switch cases to
        aid readability
*/
enum INST_TYPE {
    SINGLE,
    JUMP,
    DOUBLE
};

/* 
    BRIEF: This is used to overlay single operand instructions
        and quickly access the different sections. It can be loaded
        or cleared all at once using the us_single access.
*/
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

/* 
    BRIEF: This is used to overlay double operand instructions
        and quickly access the different sections. It can be loaded
        or cleared all at once using the us_double access.
*/
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

/* 
    BRIEF: This is used to overlay jump operand instructions
        and quickly access the different sections. It can be loaded
        or cleared all at once using the us_jump access.
*/
struct jump_overlay {
    union {
        struct {
            uint16_t offset:10;
            uint16_t opcode:6;
        };
        uint16_t us_jump;
    };
};

/* 
    BRIEF: This is used to permanently overlay the memory location of
        regfile[SR] in order to quickly access the specific bits within.
        This greatly reduces code usage and aids in readability.
        --> Note: Some of these bitfields are not used in this assignment
*/
struct sr_reg {
    union {
        struct {
            uint16_t C:1;
            uint16_t Z:1;
            uint16_t N:1;
            uint16_t GIE:1;
            uint16_t C_OFF:1;
            uint16_t O_OFF:1;       // Ignored for this assignment
            uint16_t SCG0:1;        // Ignored for this assignment
            uint16_t SCG1:1;        // Ignored for this assignment
            uint16_t V:1;
            uint16_t Reserved:7;    // Ignored for this assignment
        };
        uint16_t us_sr_reg;
    };
};

/* 
    BRIEF: This is used to temporarily overlay the memory location of
        a scr register during the devices.cpp code in order to quickly access
        the sections of the register
*/
struct scr_reg {
    uint8_t IE:1;
    uint8_t IO:1;
    uint8_t DBA:1;
    uint8_t OF:1;
    uint8_t RESERVED:4;
};

/* 
    BRIEF: This is used in the array of interrupts to store all incoming
        interrupts when the code starts.
*/
struct interrupt {
    uint16_t time = 0;
    uint16_t dev = 0;
    uint8_t data = 0;  // Char of input/output data
};

/* 
    BRIEF: This is used in the array of devices to store extra device
        status
*/
struct device {
    uint16_t IO = 0;
    uint16_t IO_data = 0;
    // Following checks are only for OUTPUT devices only
    uint16_t process_time = 0;
    uint16_t end_time = 0;;
    bool output_active = false;
    bool output_interrupt_pending = false;
};

// Global overlay values
extern device devices[MAX_DEVICES];                      // Supports a maximum of 16 devices
extern interrupt interrupts[SIMULATED_INTERRUPT_MAX];    // Supports a maximum of 500 simulated interrupts
extern single_overlay single;
extern jump_overlay jump;
extern double_overlay dbl;
extern sr_reg *sr_union;

uint16_t load_s19();

void dump_mem();

#endif
