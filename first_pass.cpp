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
-> Brief:
		The first pass performs error checking on the input .asm file
	and fills the symbol table with all appropriate values. If an error
	is found, it is recorded and will prevent the second pass from starting.
	The first pass works by utilizing a state machine that cycles through
	records individually, one token at a time. Please see the data flow diagram
	in the Diagrams folder for a general overview of the state transitions.
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>

#include "Include/first_pass.h"
#include "Include/library.h"
#include "Include/symtbl.h"
#include "Include/inst_dir.h"
#include "Include/parser.h"

struct diagnostics {
	std::string record;
	std::string error_msg;
	int line;
};

diagnostics current_record_diagnostics;

// Array of addressing mode increments for the location counter
int addr_mode_LC_array[] = {0, 2, 2, 2, 0, 0, 2};

// Array of erros for the developer's use. (Arbitrarily capped at 500 errors)
int error_line_array[500] = { };

// A global line number to keep track of which line of the file is being processed
int line_num = 0;

void first_pass(std::istream& fin)
{
	// Enumeration of the state machine states, initialized to the first state
	STATE next_state = CHK_FIRST_TOKEN;

	// Location Counter variable
	int LC = 0;

	// Various flags used within the state machine
	bool end_flag = false;
	bool two_op_flag = false;
	bool directive_error_flag = false;

	// Enumeration of the addressing mode, used for the return value of the parse function
	ADDR_MODE addr_mode = WRONG;

	// General use pointer to an instruction/directive table entry
	inst_dir* id_ptr = NULL;

	// General use pointer to a symbol table entry
	symtbl_entry* symtbl_ptr = NULL;

	// Return values for the Parse function
	int value0 = -1;
	int value1 = -1;

	// Used in the state machine
	std::string src_operand = "";
	std::string dst_operand = "";
	std::string jmp_operand = "";

	std::string last_label = "";

	while(!fin.eof() && !end_flag)
	{
		switch (next_state)
		{
			case CHK_FIRST_TOKEN:
				/*	BRIEF STATE SUMMARY:
					This state gets the first token (using fft()), 
					checks whether it in an instruction, directive, 
					or label respectively, and then sets the next
					state to the appropriate state.
				*/

				line_num++;

				current_token = fft(fin);	// This fetches the next record and it's first token

				// If there is an empty line, move on to next line
				if(current_token == "")
				{
					next_state = CHK_FIRST_TOKEN;
					break;
				}				

				// Check if token is an instruction
				id_ptr = get_inst_dir(current_token, I);
				if(id_ptr != NULL)
				{
					next_state = INST;
					break;
				}

				// Check if token is a directive
				id_ptr = get_inst_dir(current_token, D);
				if(id_ptr != NULL)
				{
					next_state = DIRECT;
					break;
				}

				// Check if token is is a symbol
				symtbl_ptr = get_symbol(current_token);
				if(symtbl_ptr == NULL && valid_symbol(current_token))  	// Therefore this is a new symbol
				{	
					add_symbol(current_token, LC, KNOWN);
					last_label = current_token;  						// Used for the EQU directive
					next_state = CHK_NEXT_TOKEN;
					break;
				}
				else if(symtbl_ptr != NULL)
				{
					if(symtbl_ptr->type == 2) 							// Fill in any forward references
					{
						symtbl_ptr->value = LC;
						symtbl_ptr->type = KNOWN;
						symtbl_ptr->line = line_num; 					// Put in line at which the label is made known
						last_label = symtbl_ptr->label; 				// Used for the EQU directive
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
			case CHK_NEXT_TOKEN:
				/*	BRIEF STATE SUMMARY:
					This state occurs if the first token was a valid label.
					At this point the token is either empty, an instruction,
					or a directive. Otherwise it is an error. This state is
					quite similar to the previous state in functionality and
					thus has less explanatory comments.
				*/

				current_token = fnt();	// This fetches the next token from the current record

				if(current_token == "")
				{
					next_state = CHK_FIRST_TOKEN;
					break;
				}

				id_ptr = get_inst_dir(current_token, I);
				if(id_ptr != NULL)
				{
					next_state = INST;
					break;
				}

                id_ptr = get_inst_dir(current_token, D);
                if(id_ptr != NULL)
                {
                	next_state = DIRECT;
                	break;
                }
                
				// If it is not empty, an instruction, or a directive, it is an error
				error_detected("Chk_Next_Token: Token is not empty, INST, or DIR");
                next_state = CHK_FIRST_TOKEN;
                
                break;
			case INST:
				/*	BRIEF STATE SUMMARY:
					This state parses the current instruction from the provided
					id_ptr (which is set in the previous state). The location counter
					is incremented and the next state is set based on the type of instruction.
					The operand for the instruction is also found and set up for the 
					following state.
				*/
				LC += 2;

				current_token = fnt();	// This fetches the next token from the current record (the operands token)

				// If the next state is not set to check operands (i.e. if there is an error),
				// the next state is set to CHK_FIRST_TOKEN by default
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
							next_state = CHK_DST_OP; // CHK_DST_OP sets the next state to CHK_SRC_OP after completion
						}
						else error_detected("INST: Found Non-Double operand DOUBLE INST");

						break;
					default:
						std::cout << "[INST] THIS SHOULD NEVER TRIGGER" << std::endl;
						getchar();  // This should never happen, getchar will stop the runtime and let the user know there is a serious flaw
						break;
				}
				
				break;
			case DIRECT:
				/*	BRIEF STATE SUMMARY:
					This state parses the current directive from the provided id_ptr
					(which is set in the previous state). The directives are performed
					through a switch case that has different operations for each directive.
					Instead of writing an enumeration for the directives, I simply noticed
					that all directives inthe MSB-430 set have a unique 2nd character and
					performed a switch case off it. (Note: This was discussed with a classmate,
					Tom Smith) See below for the translates.

					> ALIGN, BSS, BYTE, END, EQU, ORG, STRING, WORD
					>  L      S    Y     N    Q    R    T       O
				*/

				// No matter the outcome of the directive (error or not), the next state will be CHK_FIRST_TOKEN
				next_state = CHK_FIRST_TOKEN;

				// The directive is assumed to have caused an error until proven otherwise
				directive_error_flag = true;

			 	// If the type is not ALIGN, END, or STRING, the value needs to be parsed
				if(id_ptr->mnemonic[1] != 'L' && id_ptr->mnemonic[1] != 'N' && id_ptr->mnemonic[1] != 'T')
				{ 
					current_token = fnt();						// Find next token
					symtbl_ptr = get_symbol(current_token);		// Check symtbl for that token

					// "No forward referecing for directives" ~ TA Gary, 2017
					if(symtbl_ptr == NULL && valid_symbol(current_token)) error_detected("Directive: Found UNKNOWN label after DIRECTIVE (Value0 parsing, #1)");
					else
					{
						// The operand parser expects immediates to start with "#", so this is added
						current_token = "#" + current_token;
						addr_mode = parse(current_token, value0, value1);
						if(addr_mode == IMMEDIATE) directive_error_flag = false;
						else error_detected("Directive: Found Unknown Label after DIRECTIVE (Value0 parsing, #2)");

						if(!is_last_token()) error_detected("Directive: Found token after DIRECTIVE");
					}
				}

				switch (id_ptr->mnemonic[1])
				{
					case 'L':  // Align
						if(LC%2) LC++;
						if(!is_last_token()) error_detected("Directive: Found token after ALIGN directive");
						break;

					case 'S':  // BSS
						if(!directive_error_flag)
						{
							if(value0 >= 0 && value0 < 65536) LC += value0;
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
							// EQU requires a label, therefore the 'last_label' added to the symbol table is found
							// If that symbol's line number matches the current line number, EQU can proceed
							current_token = fnt();

							symtbl_ptr = get_symbol(last_label); 

							if(symtbl_ptr == NULL) error_detected("Directive: No label for EQU directive (Case 1)");
							else if(symtbl_ptr->type == KNOWN && symtbl_ptr->line == line_num)
							{
								// Therefore there is a label preceding EQU
								if(value0 >= 0 && value0 <= 65535)
								{
									symtbl_ptr->value = value0;
									symtbl_ptr->line = line_num;

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

					case 'T':  // STRING
						current_token = fnt();
						if(current_token.length() <= 82) // Doing max 80 (Plus two for the quotation marks), vintage punch card width
						{
							// Note: I'm not actually checking the contents, I'm unsure if there are illegal characters for this. 
							if(current_token[0] != '"') error_detected("Directive: Missing OPENING quote for STRING");
							else
							{
								current_token.erase(0,1); // Removes Opening Quote
								if(current_token.find_first_of("\"") != current_token.length()-1) error_detected("Directive: Missing CLOSING quote for STRING");
								else
								{
									current_token.pop_back(); // Removes Closing Quote

									value0 = current_token.length();

									LC += value0;
									if(!is_last_token()) error_detected("Directive: Found Unknown Label after STRING value");
								}
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
						std::cout << "\t\t[Directive] DEFAULT ERROR" << std::endl; // This should literally never happen
						getchar();  // This should never happen, getchar will stop the runtime and let the user know there is a serious flaw
						break;
				}

				break;
			case CHK_SRC_OP:
				/*	BRIEF STATE SUMMARY:
					This state parses the source operand for either one or two
					operand instructions. Most of the work here is done by the
					operand parser. Note: The constant generator check is also
					performed here, it cannot use forward references and takes
					measures to avoid it. If the constant generator is used,
					the location counter is not incremented for the immediate
					addressing mode.
				*/

				next_state = CHK_FIRST_TOKEN;

				addr_mode = parse(src_operand, value0, value1);

				if(addr_mode == WRONG) error_detected("CHK_SRC_OP: Invalid SRC Operand Parsing");
				else
				{
					LC += addr_mode_LC_array[addr_mode];

					// Constant generator check, must also avoid forward references
					if(addr_mode == IMMEDIATE && (value0 == -1 || value0 == 0 || value0 == 1 || value0 == 2 || value0 == 4 || value0 == 8))
					{
						symtbl_ptr = get_symbol(src_operand);
						if(symtbl_ptr != NULL)
						{
							if(symtbl_ptr->type == UNKNOWN) break; // Breaks from case, before undoing the LC increment
							else LC -= 2; // Undo the LC increment from earlier
						}
						else LC -= 2; // Undo the LC increment from earlier
					}
					if(!is_last_token()) error_detected("Directive: Found Unknown Label after SRC operand");
				}

				src_operand = "";
				break;
			case CHK_DST_OP:
				/*	BRIEF STATE SUMMARY:
					This state parses the destination operand for two operand
					instructions. Most of the work here is done by the
					operand parser. Only the first 4 addressing modes are valid,
					so if the parser returns an invalid addressing mode, an error
					is triggered.
				*/

				next_state = CHK_FIRST_TOKEN;

				addr_mode = parse(dst_operand, value0, value1);

				// If the addressing mode is INDIRECT, INDIRECT_AI, IMMEDIATE, or WRONG, there is an error
				if(addr_mode >= 4) error_detected("CHK_DST_OP: Invalid addressing mode or parsing for DST operand");
				else
				{
					LC += addr_mode_LC_array[addr_mode];

					// Must ensure that this is the last token in the record
					dst_operand = fnt();
					if(dst_operand == "") next_state = (two_op_flag) ? CHK_SRC_OP : CHK_NEXT_TOKEN;
					else error_detected("CHK_DST_OP: Invalid extra token after operand token");
				}
				dst_operand = "";
				break;
			case CHK_JMP_OP:
				/*	BRIEF STATE SUMMARY:
					This state parses the jump operand for jump instructions.
					Again, most of the work here is done by the operand
					parser. The only addressing mode allowed by jump instructions
					is relative.
				*/

				next_state = CHK_FIRST_TOKEN;

				addr_mode = parse(jmp_operand, value0, value1);

				// JUMP instructions must have the relative addressing mode1
				if(addr_mode == RELATIVE) 
				{
					LC += addr_mode_LC_array[addr_mode];
					if(!is_last_token()) error_detected("Directive: Found Unknown Label after JMP operand");
				}
				else error_detected("CHK_JMP_OP: Invalid addressing mode or parsing for JMP operand");

				jmp_operand = "";
				break;
			default:
				std::cout << "\t\t[First Pass] DEFAULT ERROR" << std::endl; // This should literally never happen
				getchar();  // This should never happen, getchar will stop the runtime and let the user know there is a serious flaw
				break;
		}
	}

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
}

/*
	Function: error_detected
	Output: bool: True means this is the last token on the line, false means this is not the last token
	Brief: Fetches next token and checks if it is empty. Returns true or false accordingly.
*/
bool is_last_token()
{
	current_token = fnt();
	if(current_token != "") return false;
	else return true;
}

/*
	Function: error_detected
	Input: error_msg: String containing the error message to print out to diagnostics
	Brief: Increments the error counter and performs diagnostics output
*/
void error_detected(std::string error_msg)
{
	std::cout << "\t\t[ERROR MSG - FIRST PASS] " << error_msg << std::endl;
	current_record_diagnostics.line = line_num;
	error_line_array[err_cnt] = line_num;
	err_cnt++;
}
