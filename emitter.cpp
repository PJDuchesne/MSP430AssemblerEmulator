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
#include "s19_maker.h"

#define PC 0
#define SR 2

int as_value[] = {0, 1, 1, 1, 2, 3, 3};

struct single_overlay {
	union {
		struct {
			unsigned short reg:4;
			unsigned short as:2;
			unsigned short bw:1;
			unsigned short opcode:9;
		};
		unsigned short us_single;  // Short = 16 bit
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

void emit(std::string inst, std::string operand, INST_TYPE type, std::ostream& outfile, int& LC)
{
	int addr_mode_LC_array_src[] = {0x0, 0x2, 0x2, 0x2, 0x0, 0x0, 0x2};
	int addr_mode_LC_array_dst[] = {0x0, 0x2, 0x2, 0x2, 0x0, 0x0, 0x0};

	std::cout << "\tEMITTING >>" << inst << "<< + >>" << operand << "<< Of type >>" << type << "<<" << std::endl; // For debugging

	inst_dir* id_ptr = get_inst(inst, I);
	symtbl_entry* symtbl_ptr = NULL;
	ADDR_MODE addr_mode0 = WRONG; // Used in general
	ADDR_MODE addr_mode1 = WRONG; // Used for DST in double operand

	single_overlay single;

	std::string src_string = "";
	std::string dst_string = "";

	double_overlay dbl;

	jump_overlay jump;

	int value0 = -1;
	int value1 = -1;

	int value0_dbl = -1;	// Used for the second operand of two operand instructions
	int value1_dbl = -1;

	unsigned short us_value0 = -1;

	switch (type)
	{
		case NONE: // 0  -> Either RETI (NONE INST) or BYTE / WORD (Meaning no instruction)
			std::cout << "\tINST TYPE NONE" << std::endl;  

			addr_mode0 = parse(operand, value0, value1);

			if(inst == "BYTE")
			{
				addr_mode0 = parse(operand, value0, value1);
 				outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << std::setw(4) << (unsigned short)value0 << std::endl;
 			}
 			else if(inst == "WORD")
 			{
 				addr_mode0 = parse(operand, value0, value1);
				outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << std::setw(4) << (unsigned short)value0 << std::endl;
 			}

 			if(inst.length() == 4)
			{
				outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << 0x1300 << std::endl;

				write_srec_word(0x1300);

				LC += 2;
			}
			else
			{
				std::cout << "THIS SHOULD NEVER HAPPEN (Default case NONE emit)" << std::endl;
			}


			break;
		case SINGLE: // 1
		std::cout << "\tINST TYPE SINGLE" << std::endl;

		std::cout << "OPCODE: >>" << id_ptr->opcode << "<<" << std::endl;

			single.opcode = id_ptr->opcode/(128);  // Bit shift the opcode to the right 7 times
			single.bw = id_ptr->b_w;

			std::cout << "OPCODE: >>" << single.opcode << "<<" << std::endl;
			addr_mode0 = parse(operand, value0, value1);

			single.as = as_value[addr_mode0];

			switch(addr_mode0) // DEAL WITH SOURCE
			{
				case REG_DIRECT: // All 3 of these do the same thing in single operand mode
				case INDIRECT:
				case INDIRECT_AI:
					single.reg = value0;
					break;

				case INDEXED:
					single.reg = value1;
					break;

				case RELATIVE:
					single.reg = PC;
					value0 -= LC; // LC of the INSTRUCTION, not value
					break;

				case ABSOLUTE:
					single.reg = SR;
					break;

				case IMMEDIATE:
					single.reg = PC;
					break;

				default:
					std::cout << "This is an issue (WRONG addr_mode0 found)" << std::endl;

					break;

			}

			outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << single.us_single << std::endl;;
			write_srec_word(single.us_single);	

			LC += 2;

			if(addr_mode_LC_array_src[addr_mode0]) // Emit SRC output if needed
			{
				outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << std::right << std::setfill('0') << std::setw(4) << std::hex << (unsigned short)value0 << std::endl;
				write_srec_word((unsigned short)value0);	
				LC += 2;
			}

			break;

		case DOUBLE: // 2
		std::cout << "\tINST TYPE DOUBLE" << std::endl;
			dbl.opcode = id_ptr->opcode/4096; // shift to the right 12 times before inputting

			std::cout << "BW VALUE >>" << id_ptr->b_w << "<<" << std::endl;

			dbl.bw = id_ptr->b_w;

			std::cout << "Operand String >>" << operand << "<<" << std::endl;

			src_string = operand.substr(0, operand.find_first_of(","));
			dst_string = operand.substr(operand.find_first_of(",")+1);

			std::cout << "SRC_String >>" << src_string << "<<" << std::endl;
			std::cout << "DST_String >>" << dst_string << "<<" << std::endl;

			addr_mode0 = parse(src_string, value0, value1);
			addr_mode1 = parse(dst_string, value0_dbl, value1_dbl);

			std::cout << "AS VALUE >>" << as_value[addr_mode0] << "<<" << std::endl;

			dbl.as = as_value[addr_mode0];
			dbl.ad = as_value[addr_mode1];	 // used as_value array again because as and ad values are the same for the first 4 addressing modes

			// These aren't even strictly necessary, could just do if(as_value[addr_mode0]) for the check

			switch(addr_mode0) // DEAL WITH SOURCE
			{
				case REG_DIRECT:
				case INDIRECT:
				case INDIRECT_AI:
					dbl.src = value0;
				break;

				case INDEXED:
					dbl.src = value1;

				break;

				case RELATIVE:
					dbl.src = PC;
					value0 -= LC; // LC of the INSTRUCTION, not value

					break;

				case ABSOLUTE:
					dbl.src = SR;

					break;

				case IMMEDIATE:
					dbl.src = PC;

					break;

				default:
					std::cout << "This should never happen (Double SRC switch case)" << std::endl;
					break;

			}

			switch (addr_mode1)  // FOR DST
			{
				case REG_DIRECT:
					dbl.dst = value0_dbl;
					break;

				case INDEXED:
					dbl.dst = value1_dbl;

					break;

				case RELATIVE:
					dbl.dst = PC;
					value0_dbl -= LC; // LC of the INSTRUCTION, not value

					break;

				case ABSOLUTE:
					dbl.dst = SR;

					break;

				default:
					std::cout << "This should never happen (Double DST switch case)" << std::endl;

					break;
			}

			// Emit INST
			outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << dbl.us_double << std::endl;;
			write_srec_word(dbl.us_double);	

			LC += 2;

			if(addr_mode_LC_array_src[addr_mode0]) // Emit SRC output if needed
			{
				outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << std::right << std::setfill('0') << std::setw(4) << std::hex << (unsigned short)value0 << std::endl;
				write_srec_word((unsigned short)value0);	
				LC += 2;
			}


			if(addr_mode_LC_array_dst[addr_mode1]) // Emit DST output if needed
			{
				outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << std::right << std::setfill('0') << std::setw(4) << std::hex << (unsigned short)value0_dbl << std::endl;
				write_srec_word((unsigned short)value1);	
				LC += 2;
			}

			break;

		case JUMP:	// 3
			addr_mode0 = parse(operand, value0, value1);

			std::cout << "\tINST TYPE JUMP" << std::endl;
			jump.opcode = id_ptr->opcode/1024; // Shift to the right 10 times

			std::cout << "VALUE0 >>" << std::hex << value0 << "<< | LC >>" << std::hex << LC << std::endl;

			value0 -= (unsigned)LC;
			std::cout << "VALUE0 (AFTER LC SUBTRACTION) >>" << std::hex << value0 << std::endl;
			value0 = value0>>1;

			std::cout << "VALUE0 (AFTER BITSHIFT) >>" << std::hex << value0 << std::endl;

			value0 = value0 & 0x03FF;

			std::cout << "VALUE0 (FINAL VALUE) >>" << std::hex << value0 << std::endl << std::dec;

			// std::cout << "JUMP OFFSET: >>" << value0 << "<<" << std::endl;

			jump.offset = value0;

			// EMIT
			outfile << std::right << std::setfill('0') << std::setw(4) << std::hex << LC << " " << (unsigned short)jump.us_jump << std::endl;;
			write_srec_word(jump.us_jump);	

			LC += 2;

			break;

		default:
			std::cout << "THIS SHOULD NEVER HAPPEN" << std::endl;
			break;
	}
	std::dec;
}
