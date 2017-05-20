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
-> Date: May 17, 2017   (Last Modified)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#ifndef LIBRARY_H
#define LIBRARY_H

enum searchtype {
	I,  // INSTRUCTION
	D   //  DIRECTIVE
};

enum symtbltype {
        REG,
        KNOWN,
        UNKNOWN
};

enum type_thingy {
        NONE,
        SINGLE,
        DOUBLE,
        JUMP
};

enum byte_word {
        BYTE,
        WORD,
        OFFSET
};

enum state {
        START,
        CHK_FIRST_TOKEN,
        CHK_NEXT_TOKEN,
        INST,
        DIRECT,
        CHK_SRC,
        CHK_DST
};

enum addr_mode {
        REG_DIRECT,
        INDEXED,
        RELATIVE,
        ABSOLUTE,
        INDIRECT,
        INDIRECT_AI,
        IMMEDIATE,
	WRONG
};

#endif

