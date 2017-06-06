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
-> Date: May 15, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#ifndef LIBRARY_H
#define LIBRARY_H

// Used for lower and upper bounds during checks
#define MINBYTE -128
#define MAXBYTE  256
#define MINWORD -32768
#define MAXWORD  65536

// Variables used by fft and fnt
extern std::string current_record;
extern std::string current_token;
extern int err_cnt;
extern int line_num;

// Global fine types for input and output across all files
extern std::ifstream fin;
extern std::ofstream outfile;
extern std::ofstream srec_file;

enum SEARCHTYPE {
	I,  // INSTRUCTION
	D   //  DIRECTIVE
};

enum SYMTBLTYPE {
    REG,
    KNOWN,
    UNKNOWN
};

enum INST_TYPE {
    NONE,
    SINGLE,
    DOUBLE,
    JUMP
};

enum BYTE_WORD {
    WORD,
    BYTE,
    OFFSET
};

enum STATE {
    CHK_FIRST_TOKEN,
    CHK_NEXT_TOKEN,
    INST,
    DIRECT,
    CHK_SRC_OP,
    CHK_DST_OP,
    CHK_JMP_OP
};

enum ADDR_MODE {
    REG_DIRECT,
    INDEXED,
    RELATIVE,
    ABSOLUTE,
    INDIRECT,
    INDIRECT_AI,
    IMMEDIATE,
    WRONG
};

std::string fft(std::istream& fin);
std::string fnt();

#endif
