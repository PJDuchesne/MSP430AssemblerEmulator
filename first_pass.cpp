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

int error_line_array[1000] = { };

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
	std::string dst_operand = "";

	char temp_dev;

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
				else if(symtbl_ptr != NULL)
				{
					if(symtbl_ptr->type == 2) // Fills in forward references
					{
						symtbl_ptr->value = LC;
						symtbl_ptr->type = KNOWN;
						next_state = CHK_NEXT_TOKEN;
						break;
					}
					else
					{
						next_state = CHK_FIRST_TOKEN;
						std::cout << "\t\t\t\t[ERROR MSG - FIRST PASS] Chk_First_Token: Duplicate token" << std::endl;
		   				error_line_array[err_cnt] = line_num;
						err_cnt++;
						break;
					}
				}
				else
				{ // Either token is already in symtbl, or it is not a valid token
					next_state = CHK_FIRST_TOKEN;
					std::cout << "\t\t\t\t[ERROR MSG - FIRST PASS] Chk_First_Token: Invalid token" << std::endl;
	   				error_line_array[err_cnt] = line_num;
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
   				error_line_array[err_cnt] = line_num;
                err_cnt++;
				std::cout << "\t\t\t\t[ERROR MSG - FIRST PASS] Chk_Next_Token: Token is not INST or DIR" << std::endl;
                
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
						if(current_token != "")
						{
							error_line_array[err_cnt] = line_num;
							err_cnt++; // ERROR: Found operand where there should not be one
							std::cout << "\t\t\t\t[ERROR MSG - FIRST PASS] INST: Found Operand on NONE INST" << std::endl;
						}
						break;
					case SINGLE: // Only DST addr_modes are allowed
						if(current_token == "")
						{
							error_line_array[err_cnt] = line_num;
							err_cnt++;
							std::cout << "\t\t\t\t[ERROR MSG - FIRST PASS] INST: Found No Operand on SINGLE INST" << std::endl;
							next_state = CHK_FIRST_TOKEN;
						}
						else
						{
							src_operand = current_token;
							next_state = CHK_SRC_OP;
						}
						break;
					case JUMP:
						if(current_token == "")
						{
							error_line_array[err_cnt] = line_num;
							err_cnt++;
							std::cout << "\t\t\t\t[ERROR MSG - FIRST PASS] INST: Found No Operand on JMP INST" << std::endl;
							next_state = CHK_FIRST_TOKEN;
						}
						else
						{
							src_operand = current_token;
							next_state = CHK_SRC_OP;
						}
						break;
					case DOUBLE:
						two_op_flag = true;

						// Need to turn "current_token" into DST_OPERAND and fill in "src_operand"

						if(current_token.find_first_of(",") != std::string::npos && current_token.find_first_of(",") != current_token.length()-1)
						{
							src_operand = current_token.substr(0, current_token.find_first_of(","));
							dst_operand = current_token.substr(current_token.find_first_of(",")+1);
							next_state = CHK_DST_OP; // Check DST first, for effectively no reason
						}
						else
						{
							next_state = CHK_FIRST_TOKEN;
							error_line_array[err_cnt] = line_num;
							err_cnt++; // ERROR, MISSING DST OPERAND FOR TWO OPERAND COMMAND
							std::cout << "\t\t\t\t[ERROR MSG - FIRST PASS] INST: Found Non-Double operand DOUBLE INST" << std::endl;
						}

						break;
					default:
						next_state = CHK_FIRST_TOKEN;
						std::cout << "[INST] THIS SHOULD NEVER TRIGGER" << std::endl;
						break;
				}
				
				break;

			// ================= DIRECTIVES HERE (START) ======================

			case DIRECT: // id_ptr should already point to the correct INST
				std::cout << "\t\t\t DIRECT" << std::endl;
				


















				next_state = CHK_FIRST_TOKEN;
				break;

			// =================== DIRECTIVES HERE (END) =====================================

			case CHK_SRC_OP:
				std::cout << "\t\t\t CHK_SRC_OP" << std::endl;

				std::cout << "\t\t\t CHECKING: >>" << src_operand << "<<" << std::endl;

				addr_mode = parse(src_operand, value0, value1);
				if(addr_mode == WRONG)
				{
					error_line_array[err_cnt] = line_num;
					err_cnt++;	// ERROR: INVALID SRC OPERAND PARSING
					std::cout << "\t\t\t\t[ERROR MSG - FIRST PASS] CHK_SRC_OP: Invalid SRC Operand Parsing" << std::endl;
					next_state = CHK_FIRST_TOKEN;
				}
				else
				{
					LC = LC + addr_mode_LC_array[addr_mode];

					// Must ensure that this is the last token in the record
					src_operand = fnt();
					if(src_operand == "")	next_state = (two_op_flag) ? CHK_SRC_OP : CHK_NEXT_TOKEN; // Therefore this is correctly the last token
					else
					{
						error_line_array[err_cnt] = line_num;
						err_cnt++;
						std::cout << "\t\t\t\t[ERROR MSG - FIRST PASS] CHK_SRC_OP: Invalid extra token after operand token" << std::endl;
						next_state = CHK_FIRST_TOKEN;
					}

					next_state = CHK_NEXT_TOKEN;
				}

				src_operand = "";  // Reset variable
				break;
			case CHK_DST_OP:
				std::cout << "\t\t\t CHK_DST_OP" << std::endl;

				std::cout << "\t\t\t CHECKING: >>" << dst_operand << "<<" << std::endl;
				
				addr_mode = parse(dst_operand, value0, value1);
				if(addr_mode >= 4) // Corresponds to INDIRECT, INDIRECT_AI, IMMEDIATE, and WRONG
				{
					error_line_array[err_cnt] = line_num;
					err_cnt++;
					std::cout << "\t\t\t\t[ERROR MSG - FIRST PASS] CHK_DST_OP: Invalid addressing mode or parsing for DST operand" << std::endl;
					next_state = CHK_FIRST_TOKEN;
				}
				else
				{
					LC = LC + addr_mode_LC_array[addr_mode];

					// Must ensure that this is the last token in the record
					dst_operand = fnt();
					if(dst_operand == "")	next_state = (two_op_flag) ? CHK_SRC_OP : CHK_NEXT_TOKEN; // Therefore this is correctly the last token
					else
					{
						error_line_array[err_cnt] = line_num;
						err_cnt++;
						std::cout << "\t\t\t\t[ERROR MSG - FIRST PASS] CHK_DST_OP: Invalid extra token after operand token" << std::endl;
						next_state = CHK_FIRST_TOKEN;
					}
				}
			
				break;
			default:
				std::cout << "\t\t\t\t[First Pass] DEFAULT ERROR" << std::endl; // This should literally never happen
				std::cin >> dst_operand[0]; // To stop building and point out the error
				break;
		}
	}

 	std::cout << std::endl << "First pass completed with LC of: >>" << LC << "<<" << std::endl;

 	// Error lines

/*
 	int temp123897 = 0;

 	std::cout << "ERRORS ON THE FOLLOWING LINES" << std::endl << std::endl;

 	while(temp123897 < err_cnt)
 	{
 		std::cout << error_line_array[temp123897] << std::endl;
 		temp123897++;
 	}

 	std::cout << std::endl;
*/
	std::cout << "First pass ending" << std::endl;
}
