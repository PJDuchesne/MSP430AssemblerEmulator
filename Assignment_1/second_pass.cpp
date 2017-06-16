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

-> Name:  second_pass.cpp
-> Brief: Function file for second_pass.cpp
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

#include "Include/second_pass.h"
#include "Include/library.h"
#include "Include/symtbl.h"
#include "Include/inst_dir.h"
#include "Include/parser.h"
#include "Include/emitter.h"
#include "Include/s19_maker.h"

// Used to signify that the state machine should stop
bool end_flag = false;

/*
	Function: second_pass
	Input: fin: The input file to read records from.
	Brief: The second pass state machine functions similarly to the first
			 pass state machine, but somewhat simpler. There are far fewew
			 error checks, and the check operand states no longer exist.
			 In fact, most of the effort for the second pass is within the
			 emit function that is called from the second pass. Effectively,
			 the second pass parses the input token until it finds an inst, in
			 which case it calls emit and goes to the next line, or until it
			 finds a directive, in which case it performs the directive action.
*/
void second_pass()
{
	// NOTE: This state machine is very similar to the first pass state
	// machine, thus it is far less commented

	// Open s19 record file
	srec_file.open("srec_output.s19");

	STATE next_state = CHK_FIRST_TOKEN;

	// Local LC variable
 	int LC = 0;

	bool directive_error_flag = false;

	ADDR_MODE addr_mode = WRONG;

	inst_dir* id_ptr = NULL;

	symtbl_entry* symtbl_ptr = NULL;

	int string_cnt = 0;
	int string_esc_cnt = 0;	// Used to keep track of number of bytes saved by using ^H instead of \t (or similarly escaped character)

	int value0 = -1;
	int value1 = -1;

	std::string last_label = ""; // Used by EQU

	init_srec(0);

	// Output the diagnostics format to diagnostics.LIS file
	outfile << "\tRecord #n: >>Input Record<<" << std::endl
  	        << "\t\tMemloc INST" << std::endl
	        << "\t\tMemloc [SRC Address] (If it exists)" << std::endl
	        << "\t\tMemloc [DST Address] (If it exists)" << std::endl;
			
	while(!fin.eof() && !end_flag)
	{
		switch (next_state)
		{
			case CHK_FIRST_TOKEN:
				line_num++;
				current_token = fft(fin);
				
				outfile << std::dec << std::endl << "\tRecord #" << line_num 
					    << ": >>" << current_record << "<<" << std::endl;

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

				// Otherwise it is a label, in which case it is skipped
				next_state = CHK_NEXT_TOKEN;

					break;
			case CHK_NEXT_TOKEN:
				// If there is an empty token, move on to next line
				current_token = fnt();
				if(current_token == "") // Line only had a label (and maybe a comment)
				{
					next_state = CHK_FIRST_TOKEN;
					break;
				}

				// Check if token is an instruction
				id_ptr = get_inst_dir(current_token, I);  // Check if it is a valid INST
				if(id_ptr != NULL)
				{
					next_state = INST;
					break;
				}

				// Check if token is a directive
				id_ptr = get_inst_dir(current_token, D);  // Check if it is a valid DIRECTIVE
				if(id_ptr != NULL)
				{
					next_state = DIRECT;
					break;
				}

				// This should never happen, all such errors would be caught in the first pass
				next_state = CHK_FIRST_TOKEN;

				break;
			case INST:
				next_state = CHK_FIRST_TOKEN;

				switch(id_ptr->type)
				{
					case NONE:
						emit(id_ptr->mnemonic, "", NONE, LC);
						break;
					case SINGLE:
						emit(id_ptr->mnemonic, fnt(), SINGLE, LC);
						break;
					case DOUBLE:
						emit(id_ptr->mnemonic, fnt(), DOUBLE, LC);
						break;
					case JUMP:
						if(emit(id_ptr->mnemonic, fnt(), JUMP, LC)) break; // If the jump fails, flows down to default
					default:
						std::cout << "[INST] THIS SHOULD NEVER TRIGGER" << std::endl;
						getchar();
						return;
						break;
				}

				break;

			case DIRECT: // id_ptr should already point to the correct DIR
				// No matter the outcome of the directive (error or not),
				// the next state is CHK_FIRST_TOKEN
				next_state = CHK_FIRST_TOKEN;

				// The directive is assumed to have caused an error until proven otherwise
				directive_error_flag = true;

			 	// If the type is not ALIGN, END, or STRING, the value needs to be parsed
				if(id_ptr->mnemonic[1] != 'L' && id_ptr->mnemonic[1] != 'N' && id_ptr->mnemonic[1] != 'T')
				{
					current_token = fnt();	// Find next token

					current_token = "#" + current_token; // tell parser to process as IMMEDIATE
					addr_mode = parse(current_token, value0, value1);

					if(addr_mode == IMMEDIATE) directive_error_flag = false;
					else error_detected_no_cnt("Directive: Unknown Label after DIRECTIVE (Value0 parsing)");
				}

				switch (id_ptr->mnemonic[1])
				{
					case 'L':  // Align
						if(LC%2) 
						{
							LC++;
							// Output the buffer in order to avoid filling the skipped memory location
							output_srec_buffer();
							init_srec(LC);
						}
						break;

					case 'S':  // BSS
						if(!directive_error_flag)
						{
							if(value0 >= 0)
							{
								LC += value0; // Bounds checked in first pass
								output_srec_buffer();
								init_srec(LC);
							}
							else error_detected_no_cnt("Directive: Negative value for BSS");
						}
						break;

					case 'Y':  // BYTE
						if(!directive_error_flag)
						{
							if(value0 >= MINBYTE && value0 <= MAXBYTE) 
							{
								write_srec_byte((unsigned char)value0);
								LC += 1;
							}
							else error_detected_no_cnt("Directive: Value too large for BYTE directive");
						}

						break;

					case 'N':  // END
						current_token = fnt();	// Find next token
						symtbl_ptr = get_symbol(current_token);	 // Get symbol or NULL
						if(symtbl_ptr != NULL)
						{
							write_S9(symtbl_ptr->value); // Error checking done in first pass
							std::cout << "END VALUE >>" << symtbl_ptr->value <<"<<"<<std::endl;
							std::cout << "END VALUE >>" << symtbl_ptr->label <<"<<"<<std::endl;
						}
						else write_S9(0);
						end_flag = true;
						break;

					case 'Q':  // EQU (Handled in the first pass)
						break;

					case 'R':  // ORG
						if(!directive_error_flag)
						{
							LC = value0;
							output_srec_buffer();
							init_srec(LC);
						}
						break;

					case 'T':  // STRING 
						current_token = fnt();

						// Error detecting for this was done in the first pass
						current_token.erase(0,1); // Removes Opening Quote

						// ITERATE THROUGH LOOKING FOR ESCAPE CHARACTERS
						for(string_cnt = 0; string_cnt < current_token.length(); string_cnt++)
						{
							if(current_token[string_cnt] == '\\')
							{
								string_esc_cnt++;
								string_cnt++;
								// USED IN SECOND PASS TO SWITCH TO ESCAPE SEQUENCES
								switch (current_token[string_cnt])
								{
									case 't': // TAB
										write_srec_byte(0x09); // 0x09 = ^H, which is the escape sequence for \t
										break;
									case '0': // NULL
										write_srec_byte(0x00); // 0x00 = ^@, which is the escape sequence for \0
										break;
									case 'r': // RETURN/ENTER
										write_srec_byte(0x0d); // 0x0d = ^M, which is the escape sequence for \r
										break;
									case 'n': // NEW LINE
										write_srec_byte(0x0a); // 0x0a = ^J, which is the escape sequence for \n
										break;
									// Other sequences could be implemented here
									default: // Therefore the character is something else (Such as double quotes)
										write_srec_byte(current_token[string_cnt]);   // Stores backslash
										break;
								}
							}
							else if(current_token[string_cnt] != '\"') // Looking for end quote
							{
								write_srec_byte(current_token[string_cnt]);
							}
						}

						LC += current_token.length();
						LC -= string_esc_cnt; // For every esc character converted from \t to ^H, a byte is saved
						
						string_esc_cnt = 0; // Reset escape count

						break;

					case 'O':  // WORD
						if(!directive_error_flag)
						{
							if(LC%2) LC += 0x01;  // Align LC first
							if(value0 >= MINWORD && value0 <= MAXWORD)
							{
								write_srec_word(value0);
								LC += 2;
							}
							else error_detected_no_cnt("Directive: Value too large for WORD directive");
						}
						break;

					default:
						std::cout << "\t\t[Directive] DEFAULT ERROR" << std::endl;
						getchar();  // This should never happen, getchar will stop the runtime
									// and let the user know there is a serious assembler flaw
						break;
				}

				break;
			default:
				std::cout << "\t\t[Second Pass] DEFAULT ERROR" << std::endl;
				getchar();  // This should never happen, getchar will stop the runtime
							// and let the user know there is a serious assembler flaw
				break;
		}

	}

	// Output any bytes still in the buffer and then close with an S9
	if(!end_flag) write_S9(0);
}

/*
Function: error_detected_no_cnt
Input: error_msg: Error msg to display if triggered
Brief: Because there should be 0 errors in the second pass, this function
will actually stop the program with a getchar() to prompt the user
that a catastrophic failure has occurred. This is primarily for
testing and debugging
 */
void error_detected_no_cnt(std::string error_msg)
{
	std::cout << "\t\t[ERROR MSG - SECOND PASS] " << error_msg << std::endl;
	outfile << "\t\t[ERROR MSG - SECOND PASS] " << error_msg << std::endl;
	getchar();
	end_flag = true; // Ends run because an error is found on second pass
					 // This is done due to the fact that this is more of an issue
					 // on the second pass given that all errors should be caught on
					 // the first pass
}
