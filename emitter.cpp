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

#include "Include/library.h"
#include "Include/symtbl.h"
#include "Include/inst_dir.h"
#include "Include/parser.h"
#include "Include/emitter.h"
#include "Include/s19_maker.h"

#define PC  0	// Program  Counter
#define SR  2	// Status   Register
#define CG1 2	// Constant Generator 1, used for -1, 0, 1, 2
#define CG2 3	// Constant Generator 2, Used for 4 and 8

// As values for setting As and Ad fields (First 4 Ad values equal first 4 Ad values)
// Numbers correspond to enumeration declaration order in library.h
int as_value[] = {0, 1, 1, 1, 2, 3, 3};

// Addressing mode arrays for increasing location counter
int addr_mode_LC_array_src[] = {0, 2, 2, 2, 0, 0, 2};
int addr_mode_LC_array_dst[] = {0, 2, 2, 2, 0, 0, 0};

/*
	Function: emit
	Input: 	inst: string of instruction to carry out
			operand: string of operand(s)
			type: type of instruction
			outfile: file to print out to
			LC: Pointer to LC in the second pass for syncronization
	Brief:The first pass performs error checking on the input .asm file
			and fills the symbol table with all appropriate values. If an error
			is found, it is recorded and will prevent the second pass from starting.
			The first pass works by utilizing a state machine that cycles through
			records individually, one token at a time. Please see the data flow diagram
			in the Diagrams folder for a general overview of the state transitions.

	Note: The reason I used "std::string inst" instead of "inst_dir& inst_ptr" was
			to allow users to input the strings to emit. This is used in the case
			of the byte and word directives and was used heavily for debugging. This
			does however waste clock cycles because get_inst_dir() has to be called
			every time emit is called, but this is somewhat offset by the fact that
			it uses a binary search to find the instruction.
*/
bool emit(std::string inst, std::string operand, INST_TYPE type, int& LC)
{

	inst_dir* id_ptr = get_inst_dir(inst, I);
	symtbl_entry* symtbl_ptr = NULL;
	ADDR_MODE addr_mode0 = WRONG; // Used in general (For One operand, SRC in two operand, and jump operand)
	ADDR_MODE addr_mode1 = WRONG; // Used for DST in double operand

	std::string src_string = "";
	std::string dst_string = "";

	single_overlay single;
	double_overlay dbl;
	jump_overlay jump;

	// Used for JUMP, ONE, and the SRC of TWO operand instructions
	int value0 = -1; // General value
	int value1 = -1; // Used for register in indexed mode

	// Used for DST of two operand instructions
	int value0_dbl = -1; // General value
	int value1_dbl = -1; // Used for register in indexed mode

	// Flag used if the constant generator is used
	bool constant_gen_flag = false;

	// Set up output settings:
	outfile << std::setfill('0') << std::right;

	switch (type)
	{
		case NONE: // Just RETI
			if(id_ptr->mnemonic == "RETI")
			{
				outfile << "\t\t" << std::hex << std::setw(4) << LC << " " << 0x1300 << std::endl;
				write_srec_word(0x1300);

				LC += 2;
			}
			else
			{
				std::cout << "THIS SHOULD NEVER HAPPEN (Default case NONE emit)" << std::endl;
				getchar();
			}
			break;
		case SINGLE:
			single.opcode = id_ptr->opcode/(128);  // Bit shift the opcode to the right 7 times (2^7)
			single.bw = id_ptr->b_w;

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
					// Constant generator (CG) functionality: Tests if the value is on the CG list
					if(value0 == -1||value0 == 0||value0 == 1||value0 == 2||value0 == 4||value0 == 8)
					{
						operand.erase(0,1);
						symtbl_ptr = get_symbol(operand);
						if(symtbl_ptr != NULL) if(symtbl_ptr->line > line_num) break; 

						single.as = (value0 > 4) ? CG1 : CG2; // CG2 deals with -1, 0, 1, and 2
															  // CG1 deals with 4 and 8
						constant_gen_flag = true;
						// Then overwrite As for the specific value
						switch (value0)
						{
							case  0:
								single.as = 0;
								break;
							case  1:
								single.as = 1;
								break;
							case  2:
							case  4:
								single.as = 2;
								break;
							default:  // Note: for "case -1:" and "case 8:", single.as is 
									  // already set to 3 from before the switch statement
								break;	
						}
					}
					break;

				default:
					std::cout << "This is an issue (WRONG addr_mode0 found)" << std::endl;
					getchar();
					break;
			}

			outfile << "\t\t" << std::hex << std::setw(4) << LC << " " 
							  << std::setw(4) << single.us_single << std::endl;;
			write_srec_word(single.us_single);	
			
			// Inrement LC for the instruction
			LC += 2;

			if(addr_mode_LC_array_src[addr_mode0] && !constant_gen_flag) // Emit SRC output if needed
			{
				outfile << "\t\t" << std::hex << std::setw(4) << LC << " " 
											  << std::setw(4) << (unsigned short)value0 << std::endl;
				write_srec_word((unsigned short)value0);	
				LC += 2; // Because the addr_mode_LC_array_src is used to get into this statement,
						 // the LC is always increased if successful
			}

			break;

		case DOUBLE: // 2
			dbl.opcode = id_ptr->opcode/4096; // shift to the right 12 times before inputting (2^12)

			dbl.bw = id_ptr->b_w;

			src_string = operand.substr(0, operand.find_first_of(","));
			dst_string = operand.substr(operand.find_first_of(",")+1);

			addr_mode0 = parse(src_string, value0, value1);
			addr_mode1 = parse(dst_string, value0_dbl, value1_dbl);

			// Sets As and Ad fields for structures
			dbl.as = as_value[addr_mode0];
			dbl.ad = as_value[addr_mode1];	 // As and Ad are identical for first 4 addressing modes

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
					// Constant generator test
					if(value0 == -1||value0 == 0||value0 == 1||value0 == 2||value0 == 4||value0 == 8)
					{
						src_string.erase(0,1);
						symtbl_ptr = get_symbol(src_string);
						if(symtbl_ptr != NULL) 
						{
							{
								if(symtbl_ptr->line > line_num) break;
							}
						}

						dbl.src = (value0 > 4) ? CG1 : CG2; // CG2 deals with -1, 0, 1, and 2
															// CG1 deals with 4 and 8

						constant_gen_flag = true;
						// Then overwrite As for the specific value
						switch (value0)
						{
							case  0:
								dbl.as = 0;
								break;
							case  1:
								dbl.as = 1;
								break;
							case  2:
							case  4:
								dbl.as = 2;
								break;
							default:  // Note: for "case -1:" and "case 8:", dbl.as is already set to 3
								break;	
						}
					}

					break;

				default:
					std::cout << "This should never happen (Double SRC switch case)" << std::endl;
					getchar();
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
					getchar();
					break;
			}

			// Emit INST
			outfile << "\t\t" << std::hex << std::setw(4) << LC << " " << dbl.us_double << std::endl;;
			write_srec_word(dbl.us_double);	

			// Increase LC for INST
			LC += 2;

			// Emit SRC output if needed (Not if constant generator is used
			if(addr_mode_LC_array_src[addr_mode0] && !constant_gen_flag)
			{
				outfile << "\t\t" << std::hex << std::setw(4) << LC << " " 
											  <<  std::setw(4) << (unsigned short)value0 << std::endl;
				write_srec_word((unsigned short)value0);	
				LC += 2; // Because the addr_mode_LC_array_src is used to get into this statement,
						 // the LC is always increased if successful
			}


			if(addr_mode_LC_array_dst[addr_mode1]) // Emit DST output if needed
			{
				outfile << "\t\t" << std::hex << std::setw(4) << LC << " "
								  << std::setw(4) << (unsigned short)value0_dbl << std::endl;
				write_srec_word((unsigned short)value0_dbl);	
				LC += 2; // Because the addr_mode_LC_array_dst is used to get into this statement,
						 // the LC is always increased if successful
			}

			break;

		case JUMP:	// 3
			addr_mode0 = parse(operand, value0, value1);

			jump.opcode = id_ptr->opcode/1024; // Shift to the right 10 times (2^10)
		
			// Increase LC for INST
			LC += 2;

			// Caluclating 10 bit offset for JUMP instruction.
			value0 -= LC;		// Finds address relative to LC -> Must test that it is within -1024 and 1022
			value0 = value0>>1;		// Bitshift to the right once
			value0 = value0 & 0x03FF;	// Only take the least 10 significant bits

			// Add error handling here, this is the only error checking in the second pass
			if(value0 > 1022 || value0 < -1024)
			{
				std::cout << std::dec << "\nJUMP TRIED TO GO TO FAR, OUTSIDE OF BOUNDS on line: >>" << line_num << "<<\n" << std::endl;
				outfile << std::dec << "\nJUMP TRIED TO GO TO FAR, OUTSIDE OF BOUNDS on line: >>" << line_num << "<<\n" << std::endl;
				return false;
			}

			jump.offset = value0;


			// EMIT
			outfile << "\t\t" << std::hex << std::setw(4) << LC << " "
						                  << std::setw(4) << (unsigned short)jump.us_jump << std::endl;;
			write_srec_word(jump.us_jump);	
			break;

		default:
			std::cout << "THIS SHOULD NEVER HAPPEN" << std::endl;
			getchar();
			break;
	}
	std::dec; // Resets output streams to print out decimals, not hex
	return true;
}
