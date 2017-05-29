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

-> Name:  emitter.h
-> Brief: Implementation file for emitter.ch
-> Date: May 24, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#ifndef EMITTER_H
#define EMITTER_H

struct single_overlay {
	union {
		struct {
			unsigned short reg:4;
			unsigned short as:2;
			unsigned short bw:1;
			unsigned short opcode:9;
		};
		unsigned short us_single;
	};
};

struct double_overlay {
	union {
		struct {
			unsigned short dst:4;
			unsigned short as:2;
			unsigned short bw:1;
			unsigned short ad:1;
			unsigned short src:4;
			unsigned short opcode:4;
		};
		unsigned short us_double;
	};
};

struct jump_overlay {
	union {
		struct {
			unsigned short offset:10;
			unsigned short opcode:6;
		};
		unsigned short us_jump;
	};
};

void emit(std::string inst, std::string operand, INST_TYPE type, int& LC);

#endif
