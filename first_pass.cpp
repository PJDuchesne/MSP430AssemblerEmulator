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

-> Name:  first_pass.cpp
-> Brief: Function file for first_pass.cpp
-> Date: May 21, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>

#include "first_pass.h"
#include "library.h"
#include "symtbl.h"
#include "inst_dir.h"
#include "parser.h"

int addr_mode_LC_array[] = {0, 2, 2, 2, 0, 0, 2};

/* Extern Globals Used
	std::string current_record
	std::string current_token
	int err_cnt
*/

void first_pass(std::istream& fin)
{
	std::cout << "First Pass Starting" << std::endl;

	STATE next_state;
	int LC = 0;
	int line_num = 1;
	bool end_flag = false;
	bool two_op_flag = false;

	ADDR_MODE addr_mode = WRONG;

	inst_dir* id_ptr = NULL;

	symtbl_entry* symtbl_ptr = NULL;

	int value0 = -1;
	int value1 = -1;

	std::string src_operand = "";

	char temp_dev;

	// Do "Start" things here	

	next_state = CHK_FIRST_TOKEN;
	while(!fin.eof())
	{
		switch (next_state)
		{
			case CHK_FIRST_TOKEN: // Also iterates to next record
				//	std::cin >> temp_dev;  // FOR DEVELOPMENT: Ask for input between lines
				current_token = fft(fin);
				std::cout << "\t#" << line_num << ": >>" << current_record << "<<" << std::endl;
				std::cout << "\t\t\t CHK_FIRST TOKEN" << std::endl;
				line_num++;
				if(current_token == "")  // Empty line, no token on line
				{
					next_state = CHK_FIRST_TOKEN;
					break;
				}				

				id_ptr = get_inst(current_token, I);  // Check if it is a valid INST
				if(id_ptr != NULL)
				{
					next_state = INST;
					break;
				}

				id_ptr = get_inst(current_token, D);  // Check if it is a valid DIRECTIVE
				if(id_ptr != NULL)
				{
					next_state = DIRECT;
					break;
				}

				symtbl_ptr = get_symbol(current_token);
				if(symtbl_ptr == NULL && valid_symbol(current_token))  // New symbol!
				{	
					add_symbol(current_token, LC, KNOWN);
					next_state = CHK_NEXT_TOKEN;
					break;
				}
				else if(symtbl_ptr->type == 2) // Fills in forward references
				{
					symtbl_ptr->value = LC;
					next_state = CHK_NEXT_TOKEN;
					break;
				}
				else
				{ // Either token is already in symtbl, or it is not a valid token
					next_state = CHK_FIRST_TOKEN;
					std::cout << "\t\t\t\tERROR FOUND (Check first token)" << std::endl;
					err_cnt++;
					break;
				}

				break;
			case CHK_NEXT_TOKEN: // This happens after a valid label is found
				std::cout << "\t\t\t CHK_NEXT_TOKEN" << std::endl;

				// This is either an empty token, and INST, or DIR (OR ERROR)
				current_token = fnt();

				if(current_token == "") 	      // Line only had a label (and maybe a comment)
				{
					next_state = CHK_FIRST_TOKEN;
					break;
				}
				
				std::cout << "\t\t\t Token: >> " << current_token << "<<" << std::endl;
				

				id_ptr = get_inst(current_token, I);  // Check if it is a valid INST
				if(id_ptr != NULL)
				{
					next_state = INST;
					break;
				}

                id_ptr = get_inst(current_token, D);  // Check if it is a valid DIRECTIVE
                if(id_ptr != NULL)
                {
                	next_state = DIRECT;
                	break;
                }
                
				// If it is not an instruction or directive, this is an error	
                next_state = CHK_FIRST_TOKEN;
                std::cout << "\t\t\t\tERROR FOUND (Check next token)" << std::endl;
                err_cnt++;
                
                break;
			case INST:  // id_ptr should already point to the correct INST
				std::cout << "\t\t\t INST" << std::endl;
				LC = LC+2;

				// Next token should contain either 0, 1, or 2 operands
				current_token = fnt();

				switch(id_ptr->type)
				{
					case NONE:
						next_state = CHK_FIRST_TOKEN;
						if(current_token != "") err_cnt++; // ERROR: Found operand where there should not be one
						break;
					case SINGLE: // Only DST addr_modes are allowed
					case JUMP:
						next_state = CHK_DST_OP;
						break;
					case DOUBLE:
						two_op_flag = true;

						// Need to turn "current_token" into DST_OPERAND and fill in "src_operand"
						src_operand = current_token.substr(current_token.find_first_of(","), current_token.size());
						current_token = current_token.substr(0, current_token.find_first_of(","));
						next_state = CHK_SRC_OP;
						break;
					default:
						next_state = CHK_FIRST_TOKEN;
						std::cout << "[INST] THIS SHOULD NEVER TRIGGER" << std::endl;
						break;
				}
				
				break;
			case DIRECT: // id_ptr should already point to the correct INST
				std::cout << "\t\t\t DIRECT" << std::endl;
			
				// SKIP FOR NOW, WILL ADD IN LATER	









				next_state = CHK_FIRST_TOKEN;
				break;
			case CHK_SRC_OP:
				std::cout << "\t\t\t CHK_SRC_OP" << std::endl;
				if(current_token == "")
				{
					err_cnt++;  // ERROR: MISSING OPERAND F OPERAND INST
					next_state = CHK_FIRST_TOKEN;
				}
				else if(valid_symbol(current_token))
				{
					addr_mode = parse(current_token, value0, value1);
					if(addr_mode == WRONG)
					{
						err_cnt++;	// ERROR: INVALID DST OPERAND PARSING
						next_state == CHK_FIRST_TOKEN;
					}
					else
					{
						LC = LC + addr_mode_LC_array[addr_mode];

						// No need to check if this is last token, 
						// that has been done in CHK_DST_OP, which always precedes this

						next_state = CHK_NEXT_TOKEN;
					}
				}
				else
				{
					err_cnt++;  // ERROR, Invalid symbol or double operand detected
					next_state = CHK_FIRST_TOKEN;
				}
				break;
			case CHK_DST_OP:
				std::cout << "\t\t\t CHK_DST_OP" << std::endl;

				if(current_token == "")
				{
					std::cout << "ERROR1" << std::endl;
					next_state = CHK_FIRST_TOKEN;
				}
				else if(valid_symbol(current_token)) // valid symbol can't have "," in it, meaning it can't be a double
				{
					addr_mode = parse(current_token, value0, value1);
					if(addr_mode >= 4) // Corresponds to INDIRECT, INDIRECT_AI, IMMEDIATE, and WRONG
					{
						std::cout << "ERROR2" << std::endl;
						next_state == CHK_FIRST_TOKEN;
					}
					else
					{
						LC = LC + addr_mode_LC_array[addr_mode];

						// Must ensure that this is the last token in the record
						current_token = fnt();
						if(current_token == "")
						{
							// Then this is correctly the last token
							// Check SRC if this is a two operand INST
							next_state = (two_op_flag) ? CHK_SRC_OP : CHK_NEXT_TOKEN;
						}
						else
						{
							std::cout << "ERROR3" << std::endl;
							next_state = CHK_FIRST_TOKEN;
						}
					}
				}
				else
				{
					std::cout << "ERROR4" << std::endl;
					next_state = CHK_FIRST_TOKEN;
				}
				break;
			default:
				std::cout << "\t\t\t\t[First Pass] DEFAULT ERROR" << std::endl;
				break;
		}
	}

	std::cout << "First pass ending" << std::endl;
}
