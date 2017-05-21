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
#include "symtbl.h"		// For access to symbol table
#include "inst_dir.h"		// 
#include "parser.h"

void first_pass(std::istream& fin)
{
	std::cout << "First Pass Starting" << std::endl;

        state next_state;
	int LC = 0;
	int line_num = 1;
	bool end_flag = false;
	bool two_op_flag = false;

	/* Extern Globals Used
		std::string current_record
		std::string current_token
		int err_cnt
	*/

	inst_dir* id_ptr = NULL;                        // id_ptr thingy

	symtbl_entry* symtbl_ptr = NULL;                // symtbl_ptr

	int value0 = -1;
	int value1 = -1;

	char temp_dev;

	// Do "Start" things here	

	next_state = CHK_FIRST_TOKEN;
	while(!fin.eof())
	{
		switch (next_state)
		{
			case CHK_FIRST_TOKEN: // Also iterates to next record
				// FOR DEVELOPMENT: Ask for input between lines
				//	std::cin >> temp_dev;

				std::cout << "\t\t\t CHK_FIRST TOKEN" << std::endl;
				current_token = fft(fin);
				std::cout << "\t#" << line_num << ": >>" << current_record << "<<" << std::endl;
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
				if(id_ptr == NULL && valid_symbol(current_token))  // New symbol!
				{	
					std::cout << "Added >>" << current_token << "<< to the symbol table" << std::endl;
					add_symbol(current_token, LC, KNOWN);
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
				current_token = fnt(fin);

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
				
				next_state = CHK_FIRST_TOKEN;
				break;
			case DIRECT: // id_ptr should already point to the correct INST
				std::cout << "\t\t\t DIRECT" << std::endl;
			
				// SKIP FOR NOW, WILL ADD IN LATER	

				next_state = CHK_FIRST_TOKEN;
				break;
			case CHK_SRC_OP:
				std::cout << "\t\t\t CHK_SRC_OP" << std::endl;

				break;
			case CHK_DST_OP:
				std::cout << "\t\t\t CHK_DST_OP" << std::endl;

				break;
			default:
				std::cout << "\t\t\t\t DEFAULT - ERROR1" << std::endl;
				std::cout << "\t\t\t\t DEFAULT - ERROR2" << std::endl;
				std::cout << "\t\t\t\t DEFAULT - ERROR3" << std::endl;
				std::cout << "\t\t\t\t DEFAULT - ERROR4" << std::endl;
				std::cout << "\t\t\t\t DEFAULT - ERROR5" << std::endl;

				break;

		}
	}

	std::cout << "First pass ending" << std::endl;
}
