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

#include "second_pass.h"
#include "library.h"
#include "symtbl.h"
#include "inst_dir.h"
#include "parser.h"
#include "emitter.h"

/* Extern Globals Used
   std::string current_record
   std::string current_token
   int err_cnt
 */

void second_pass(std::istream& fin)
{
	std::cout << "Second Pass Starting" << std::endl;

	std::ofstream outfile;
	outfile.open("output.s19");
	// outmyfile << "Writing this to a file.\n";

	line_num = 0;

	STATE next_state;
 	int LC = 0;
	bool end_flag = false;
	bool directive_error_flag = false;

	ADDR_MODE addr_mode = WRONG;

	inst_dir* id_ptr = NULL;

	symtbl_entry* symtbl_ptr = NULL;

	int value0 = -1;
	int value1 = -1;

	std::string last_label = "";

	char temp_dev;

	next_state = CHK_FIRST_TOKEN;
	while(!fin.eof() && !end_flag)
	{
		switch (next_state)
		{
			case CHK_FIRST_TOKEN: // Also iterates to next record
				line_num++;
				current_token = fft(fin);
				std::cout << "\tLC at START OF RECORD >>" << LC << "<<" << std::endl;

				std::cout << "\tCHK_FIRST TOKEN" << std::endl;

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

				symtbl_ptr = get_symbol(current_token); // Check first token for Label, ignore if found

				break;
			case CHK_NEXT_TOKEN: // This happens after a valid label is found
				std::cout << "\tCHK_NEXT_TOKEN" << std::endl;

				// This is either an empty token, and INST, or DIR (OR ERROR)
				current_token = fnt();
				if(current_token == "") 	      // Line only had a label (and maybe a comment)
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

				// If it is not an instruction or directive, this is an error
				next_state = CHK_FIRST_TOKEN;

				break;
			case INST:  // id_ptr should already point to the correct INST
				std::cout << "\tINST" << std::endl;
				LC = LC+ 0x02;

				// Next token should contain either 0, 1, or 2 operands

				// Emit using 'current_token = current_token + " " + fnt();' (This will be the 
	
				next_state = CHK_FIRST_TOKEN;

				switch(id_ptr->type)
				{
					case NONE:
						emit(id_ptr->mnemonic, "", NONE, outfile, LC);
						break;
					case SINGLE:
						emit(id_ptr->mnemonic, fnt(), SINGLE, outfile, LC);
						break;
					case DOUBLE:
						emit(id_ptr->mnemonic, fnt(), DOUBLE, outfile, LC);
						break;
					case JUMP:
						emit(id_ptr->mnemonic, fnt(), JUMP, outfile, LC);
						break;
					default:
						std::cout << "[INST] THIS SHOULD NEVER TRIGGER" << std::endl;
						break;
				}

				break;

				// ================= DIRECTIVES HERE (START) ======================

				// NOTE: Can BYTE or WORD be #UNKNOWN (Where UNKNOWN is an unknown label that will be defined later?)

			case DIRECT: // id_ptr should already point to the correct INST
				std::cout << "\tDIRECT" << std::endl;

				/*	
					> The second letter of all directives are unique with this directive set
					> This avoids having to do another enumeration set (Note: Discussed this with Tom Smith)

					> ALIGN, BSS, BYTE, END, EQU, ORG, STRING, WORD
					>  L      S    Y     N    Q    R    T       O
				 */

				next_state = CHK_FIRST_TOKEN; // Whether or not there's an error, this is always the next state
				directive_error_flag = true;  // Assume error until proven otherwise

				if(id_ptr->mnemonic[1] != 'L' && id_ptr->mnemonic[1] != 'N' && id_ptr->mnemonic[1] != 'T')
				{  // If the type is not ALIGN, END, or STRING, the value needs to be parsed

					current_token = fnt();			// Find next token
					symtbl_ptr = get_symbol(current_token);		// Check symtbl for that token

					// No forward referecing
					if(symtbl_ptr == NULL && valid_symbol(current_token)) error_detected_no_cnt("Directive: Found UNKNOWN label after DIRECTIVE (Value0 parsing, #1)");
					else
					{
						current_token = "#" + current_token;  // make it an indexed value for the parser
						addr_mode = parse(current_token, value0, value1);

						std::cout << "\t\tCurrent addr mode is: >>" << addr_mode << "<<" << std::endl;

						if(addr_mode == IMMEDIATE) directive_error_flag = false;
						else error_detected_no_cnt("Directive: Found Unknown Label after DIRECTIVE (Value0 parsing, #2)");
						// Must be wrong (unless something is wrong with the parser)
					}
				}

				switch (id_ptr->mnemonic[1])  // The second letter of all directives are unique with this directive set
				{
					case 'L':  // Align
						if(LC%2) LC++;

						break;

					case 'S':  // BSS
						if(!directive_error_flag)
						{
							if(value0 >= 0) LC == value0;  // No upper bound on BSS
							else error_detected_no_cnt("Directive: Negative value for BSS");
						}
						break;

					case 'Y':  // BYTE
						if(!directive_error_flag)
						{
							if(value0 >= -128 && value0 < 256) LC += 0x01;
							else error_detected_no_cnt("Directive: Value too large for BYTE directive");
						}

						break;

					case 'N':  // END
						std::cout << "THE END IS COMING" << std::endl;
						end_flag = true;
						break;

					case 'Q':  // EQU
						if(!directive_error_flag)
						{
							// LABEL is required: Check symbtable for label at current LC value
							current_token = fnt();

							std::cout << "Last Addition: >> " << last_label << "<<" << std::endl;

							symtbl_ptr = get_symbol(last_label); 

							if(symtbl_ptr == NULL) error_detected_no_cnt("Directive: No label for EQU directive (Case 1)");
							else if(symtbl_ptr->type == KNOWN && symtbl_ptr->line == line_num)
							{
								// Therefore there is a label preceding EQU
								if(value0 >= 0 && value0 <= 65535)
								{
									symtbl_ptr->value = value0;

									std::cout << "[DIRECTIVE] SUCESSFULLY STORED value0 into label >>" << symtbl_ptr->label << "<<" << std::endl;

								}
								else error_detected_no_cnt("Directive: Value too large for EQU directive");
							}
							else error_detected_no_cnt("Directive: No label for EQU directive (Case 2)");
						}
						break;

					case 'R':  // ORG
						if(!directive_error_flag)
						{
							if(value0 >= 0 && value0 < 65535-LC) 
							{
								LC = value0;
							}
							else error_detected_no_cnt("Directive: Value too large for ORG directive");
						}

						break;

					case 'T':  // STRING ** Special Case **
						current_token = fnt();

						if(current_token.length() <= 130)
						{
							if(current_token.find_first_of("\"")!= 0 || current_token.find_last_of("\"") != current_token.length()-1) error_detected_no_cnt("Directive: Missing Quotes for STRING");
							else
							{
								current_token.erase(0,1); // Removes Opening Quote
								current_token.pop_back(); // Removes Closing Quote

								value0 = current_token.length();

								LC += value0;
							}
						}
						else error_detected_no_cnt("Directive: Value too large for ORG directive");

						break;

					case 'O':  // WORD
						if(!directive_error_flag)
						{
							if(LC%2) LC += 0x01;  // Align LC first
							if(value0 > -65536 && value0 < 65535) LC += 0x02;
							else error_detected_no_cnt("Directive: Value too large for WORD directive");
						}
						break;

					default:
						// This should never happen

						std::cout << "\t\t[Directive] DEFAULT ERROR" << std::endl; // This should literally never happen
						break;
				}

				break;

				// =================== DIRECTIVES HERE (END) =====================================

			default:
				std::cout << "\t\t[First Pass] DEFAULT ERROR" << std::endl; // This should literally never happen
				break;
		}
	}
	std::cout << std::endl << "Second pass completed with LC of: >>" << LC << "<<" << std::endl;

	std::cout << "Second pass ending" << std::endl;

	outfile.close();
}

void error_detected_no_cnt(std::string error_msg)
{
	std::cout << "\t\t[ERROR MSG - FIRST PASS=] " << error_msg << std::endl;
	// char temp123; std::cin >> temp123;
}
