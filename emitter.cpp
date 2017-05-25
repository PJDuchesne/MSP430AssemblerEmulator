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

-> Name:  emitter.cpp
-> Brief: Function file for emitter.cpp
-> Date: May 24, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <iomanip>

#include "library.h"
#include "symtbl.h"
#include "inst_dir.h"
#include "parser.h"
#include "emitter.h"

#define PC 0
#define SR 2

int as_value[] = {0, 1, 1, 1, 2, 3, 3};
int ad_value[] = {0, 1, 1, 1};

struct single_overlay {
	union {
		struct {
			unsigned int reg:4;
			unsigned int as:2;
			unsigned int bw:1;
			unsigned int opcode:9;
		};
		unsigned int us_single;
	};
};

struct double_overlay {
	union {
		struct {
			unsigned int dst:4;
			unsigned int as:2;
			unsigned int bw:1;
			unsigned int ad:1;
			unsigned int src:4;
			unsigned int opcode:4;
		};
	};
};

struct jump_overlay {
	union {
		struct {
			unsigned int offset:10;
			unsigned int opcode:6;
		};
	};
};
/*
union single_overlay
{
	emit_single field;
	unsigned int us_single;
};
*/
void emit(std::string inst, std::string operand, INST_TYPE type, std::ostream& outfile, int& LC)
{
	int addr_mode_LC_array[] = {0x0, 0x2, 0x2, 0x2, 0x0, 0x0, 0x2};

	std::cout << "\tEMITTING >>" << inst << "<< + >>" << operand << "<< Of type >>" << type << "<<" << std::endl; // For debugging

	inst_dir* id_ptr = get_inst(inst, I);
	symtbl_entry* symtbl_ptr = NULL;
	ADDR_MODE addr_mode = WRONG;

	single_overlay single;

	double_overlay dbl;

	jump_overlay jump;

	int value0 = -1;
	int value1 = -1;

	unsigned int value0_us = -1;
	unsigned int value1_us = -1;

	switch (type)
	{
		case NONE: // 0  // Literally only RETI
			std::cout << "\tINST TYPE NONE" << std::endl;  
			outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << 0x1300 << std::endl;

			// Also output to cout for debugging
			std::cout << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << 0x1300 << std::endl;

			break;
		case SINGLE: // 1
			std::cout << "\tINST TYPE SINGLE" << std::endl;

			std::cout << "OPCODE: >>" << id_ptr->opcode << "<<" << std::endl;

			single.opcode = id_ptr->opcode/(128);  // Bit shift the opcode to the right 7 times
			single.bw = id_ptr->b_w;

			std::cout << "OPCODE: >>" << single.opcode << "<<" << std::endl;
			addr_mode = parse(operand, value0, value1);

			single.as = as_value[addr_mode];

			switch (addr_mode)
			{
				case ABSOLUTE:
					single.reg = SR;
					outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << single.us_single << std::endl;
					LC += 2;

					value0 -= LC;
					outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << value0 << std::endl;

					break;

				case RELATIVE:
					single.reg = PC;
					outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << single.us_single << std::endl;
					LC += 2;

					outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << value0 << std::endl;

					break;

				case REG_DIRECT:	// All 3 of these do the same thing in single operand mode
				case INDIRECT:
				case INDIRECT_AI:
					single.reg = value0;
					outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << single.us_single << std::endl;
					LC += 2;

					break;

				case IMMEDIATE:
					std::cout << "IMMEDIATE" << std::endl;
					single.reg = PC;

					std::cout << std::hex << "OPCODE >>" << single.opcode << "<< | BW >>" << single.bw << "<< | As >>" << single.as << "<< | REG >>" << single.reg << "<<" << std::endl;

					outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << std::setw(4) << single.us_single << std::endl;
					LC += 2;
					outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << std::setw(4) << value0 << std::endl;

					break;

				case INDEXED:
					single.reg = value1;
					outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << single.us_single << std::endl;

					LC +=2;

					outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << value0 << std::endl;

					break;

				case WRONG:

					break;

				default: // REG_DIRECT, INDIRECT, INDIRECT_AI
					if(addr_mode == WRONG) std::cout << "This is an issue (WRONG addr_mode found)" << std::endl;

					single.reg = value0;




					break;

			}

			LC += addr_mode_LC_array[addr_mode];

			break;

		case DOUBLE: // 2
			std::cout << "\tINST TYPE DOUBLE" << std::endl;
			dbl.opcode = id_ptr->opcode/4096; // shift to the right 12 times
			dbl.bw = id_ptr->b_w;


			break;

		case JUMP:	// 3
			std::cout << "\tINST TYPE JUMP" << std::endl;
			jump.opcode = id_ptr->opcode/1024; // Shift to the right 10 times

			// Find magical 10 digit offset
				// Value0 will be the starting offset, double it? Sign extend it? Etc??


			break;

		default:
			std::cout << "THIS SHOULD NEVER HAPPE" << std::endl;
			break;
	}
}
