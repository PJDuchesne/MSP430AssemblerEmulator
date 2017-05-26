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

int addr_mode_LC_array[] = {0x0, 0x2, 0x2, 0x2, 0x0, 0x0, 0x2};

int error_line_array[300] = { };

int line_num = 0;

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
	bool end_flag = false;
	bool two_op_flag = false;
	bool directive_error_flag = false;

	ADDR_MODE addr_mode = WRONG;

	inst_dir* id_ptr = NULL;

	symtbl_entry* symtbl_ptr = NULL;

	int value0 = -1;
	int value1 = -1;

	std::string src_operand = "";
	std::string dst_operand = "";
	std::string jmp_operand = "";

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
				std::cout << "\tLC at START OF RECORD >>" << std::hex << LC << std::dec << "<<" << std::endl;
				// std::cout << "\tCURRENT_TOKEN: >>" << current_token << "<<" << std::endl;
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

				symtbl_ptr = get_symbol(current_token);
				if(symtbl_ptr == NULL && valid_symbol(current_token))  // New symbol!
				{	
					add_symbol(current_token, LC, KNOWN);
					last_label = current_token;  // For EQU
					next_state = CHK_NEXT_TOKEN;
					break;
				}
				else if(symtbl_ptr != NULL)
				{
					if(symtbl_ptr->type == 2) // Fills in forward references
					{
						symtbl_ptr->value = LC;
						symtbl_ptr->type = KNOWN;
						symtbl_ptr->line = line_num; // UNKNOWN
						last_label = symtbl_ptr->label; // FOR EQU
						next_state = CHK_NEXT_TOKEN;
						break;
					}
					else
					{
						error_detected("Chk_First_Token: Duplicate token");
						next_state = CHK_FIRST_TOKEN;
						break;
					}
				}
				else
				{
					error_detected("Chk_First_Token: Invalid token");
					next_state = CHK_FIRST_TOKEN;
					break;
				} 

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
				
				std::cout << "\tToken: >> " << current_token << "<<" << std::endl;
				

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
				error_detected("Chk_Next_Token: Token is not INST or DIR");
                next_state = CHK_FIRST_TOKEN;
                
                break;
			case INST:  // id_ptr should already point to the correct INST
				std::cout << "\tINST" << std::endl;
				LC = LC+ 0x02;

				// Next token should contain either 0, 1, or 2 operands
				current_token = fnt();

				next_state = CHK_FIRST_TOKEN;

				switch(id_ptr->type)
				{
					case NONE:
						if(current_token != "") error_detected("Found Operand on NONE INST");

						break;
					case SINGLE:
						if(current_token == "") error_detected("INST: Found No Operand on SINGLE INST");
						else
						{
							src_operand = current_token;
							next_state = CHK_SRC_OP;
						}
						break;
					case JUMP:
						if(current_token == "") error_detected("INST: Found No Operand on JMP INST");
						else
						{
							jmp_operand = current_token;
							next_state = CHK_JMP_OP;
						}
						break;
					case DOUBLE:
						two_op_flag = true;

						if(current_token.find_first_of(",") != std::string::npos && current_token.find_first_of(",") != current_token.length()-1)
						{
							src_operand = current_token.substr(0, current_token.find_first_of(","));
							dst_operand = current_token.substr(current_token.find_first_of(",")+1);
							next_state = CHK_DST_OP; // Check DST first, for effectively no reason
						}
						else error_detected("INST: Found Non-Double operand DOUBLE INST");

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

					current_token = fnt();						// Find next token
					symtbl_ptr = get_symbol(current_token);		// Check symtbl for that token

					// No forward referecing
					if(symtbl_ptr == NULL && valid_symbol(current_token)) error_detected("Directive: Found UNKNOWN label after DIRECTIVE (Value0 parsing, #1)");
					else
					{
						current_token = "#" + current_token;  // make it an indexed value for the parser
						addr_mode = parse(current_token, value0, value1);

						std::cout << "\t\tCurrent addr mode is: >>" << addr_mode << "<<" << std::endl;

						if(addr_mode == IMMEDIATE) directive_error_flag = false;
						else error_detected("Directive: Found Unknown Label after DIRECTIVE (Value0 parsing, #2)");
						// Must be wrong (unless something is wrong with the parser)

						if(!is_last_token()) error_detected("Directive: Found token after DIRECTIVE");
					}
				}

				switch (id_ptr->mnemonic[1])  // The second letter of all directives are unique with this directive set
				{
					case 'L':  // Align
						if(LC%2) LC++;

						if(!is_last_token()) error_detected("Directive: Found token after ALIGN directive");
						break;

					case 'S':  // BSS
						if(!directive_error_flag)
						{
							if(value0 >= 0) LC += value0;  // No upper bound on BSS
							else error_detected("Directive: Negative value for BSS");
						}
						break;

					case 'Y':  // BYTE
						if(!directive_error_flag)
						{
							if(value0 >= -128 && value0 < 256) LC += 0x01;
							else error_detected("Directive: Value too large for BYTE directive");
						}

						break;

					case 'N':  // END
						std::cout << "THE END IS COMING" << std::endl;
						end_flag = true;
						if(!is_last_token())
						{
							error_detected("Directive: Found Unknown Label after END");
							end_flag = false;
						}
						break;

					case 'Q':  // EQU
						if(!directive_error_flag)
						{
							// LABEL is required: Check symbtable for label at current LC value
							current_token = fnt();

							std::cout << "Last Addition: >> " << last_label << "<<" << std::endl;

							symtbl_ptr = get_symbol(last_label); 

							if(symtbl_ptr == NULL) error_detected("Directive: No label for EQU directive (Case 1)");
							else if(symtbl_ptr->type == KNOWN && symtbl_ptr->line == line_num)
							{
								// Therefore there is a label preceding EQU
								if(value0 >= 0 && value0 <= 65535)
								{
									symtbl_ptr->value = value0;

									std::cout << "[DIRECTIVE] SUCESSFULLY STORED value0 into label >>" << symtbl_ptr->label << "<<" << std::endl;

									if(!is_last_token()) error_detected("Directive: Found Unknown Label after EQU value");
								}
								else error_detected("Directive: Value too large for EQU directive");
							}
							else error_detected("Directive: No label for EQU directive (Case 2)");
						}
						break;

					case 'R':  // ORG
						if(!directive_error_flag)
						{
							if(value0 >= 0 && value0 < 65535-LC) 
							{
								LC = value0;
								if(!is_last_token()) error_detected("Directive: Found Unknown Label after ORG value");
							}
							else error_detected("Directive: Value too large for ORG directive");
						}

						break;

					case 'T':  // STRING ** Special Case **
						current_token = fnt();

						if(current_token.length() <= 130)
						{
							if(current_token.find_first_of("\"")!= 0 || current_token.find_last_of("\"") != current_token.length()-1) error_detected("Directive: Missing Quotes for STRING");
							else
							{
								current_token.erase(0,1); // Removes Opening Quote
								current_token.pop_back(); // Removes Closing Quote

								value0 = current_token.length();

								LC += value0;
								if(!is_last_token()) error_detected("Directive: Found Unknown Label after STRING value");
							}
						}
						else error_detected("Directive: Value too large for ORG directive");

						break;

					case 'O':  // WORD
						if(!directive_error_flag)
						{
							if(LC%2) LC += 0x01;  // Align LC first
							if(value0 > -65536 && value0 < 65535) LC += 0x02;
							else error_detected("Directive: Value too large for WORD directive");
						}
						break;

					default:
						// This should never happen

						std::cout << "\t\t[Directive] DEFAULT ERROR" << std::endl; // This should literally never happen
						std::cin >> dst_operand[0]; // To stop building and point out the error
						break;
					}

				break;

				// =================== DIRECTIVES HERE (END) =====================================

			case CHK_SRC_OP:
				std::cout << "\tCHK_SRC_OP" << std::endl;

				next_state = CHK_FIRST_TOKEN;
	
				addr_mode = parse(src_operand, value0, value1);
				if(addr_mode == WRONG) error_detected("CHK_SRC_OP: Invalid SRC Operand Parsing");
				else
				{
					LC += addr_mode_LC_array[addr_mode];
					if(!is_last_token()) error_detected("Directive: Found Unknown Label after SRC operand");
				}

				src_operand = "";
				break;
			case CHK_DST_OP:
				std::cout << "\tCHK_DST_OP" << std::endl;

				next_state = CHK_FIRST_TOKEN;

				addr_mode = parse(dst_operand, value0, value1);

				// Corresponds to INDIRECT, INDIRECT_AI, IMMEDIATE, and WRONG
				if(addr_mode >= 4) error_detected("CHK_DST_OP: Invalid addressing mode or parsing for DST operand");
				else
				{
					LC += addr_mode_LC_array[addr_mode];

					// Must ensure that this is the last token in the record
					dst_operand = fnt();
					if(dst_operand == "") next_state = (two_op_flag) ? CHK_SRC_OP : CHK_NEXT_TOKEN; // Therefore this is correctly the last token
					else error_detected("CHK_DST_OP: Invalid extra token after operand token");
				}
				dst_operand = "";
				break;

			case CHK_JMP_OP:
				std::cout << "\tCHK_JMP_OP" << std::endl;

				next_state = CHK_FIRST_TOKEN;

				addr_mode = parse(jmp_operand, value0, value1);
				if(addr_mode == RELATIVE) 
				{  // Jump cannot have registers, therefore must be RELATIVE (ABS and IMM are bad)
					LC += addr_mode_LC_array[addr_mode];
					if(!is_last_token()) error_detected("Directive: Found Unknown Label after JMP operand");
				}
				else error_detected("CHK_JMP_OP: Invalid addressing mode or parsing for JMP operand");

				jmp_operand = "";
				break;

			default:
				std::cout << "\t\t[First Pass] DEFAULT ERROR" << std::endl; // This should literally never happen
				std::cin >> dst_operand[0]; // To stop building and point out the error
				break;
		}
	}
	std::cout << std::endl << "First pass completed with LC of: >>" << LC << "<<" << std::endl;


	// Error lines (For debugging)

	int temp123897 = 0;

	std::cout << "ERRORS ON THE FOLLOWING LINES" << std::endl << std::endl;

	while(temp123897 < err_cnt)
	{
		std::cout << error_line_array[temp123897] << std::endl;
		temp123897++;
	}

	std::cout << std::endl;

	line_num = 0;

	std::cout << "First pass ending" << std::endl;
}

// FALSE = Extra token (This is an error), TRUE = No extra token, this is the last token
bool is_last_token()
{
	current_token = fnt();
	if(current_token != "") return false;
	else return true;
}

// Used to shorten code: could make this inline to be faaaaaancy
void error_detected(std::string error_msg)
{
	std::cout << "\t\t[ERROR MSG - FIRST PASS=] " << error_msg << std::endl;
	error_line_array[err_cnt] = line_num;
	err_cnt++;
}
