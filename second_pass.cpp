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

std::ofstream srec_file;

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
void second_pass(std::istream& fin)
{
	std::ofstream outfile;
	outfile.open("output.txt");				// For debugging second pass

	srec_file.open("srec_output.s19");      // For submission

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

	init_srec(0);

	next_state = CHK_FIRST_TOKEN;
	while(!fin.eof() && !end_flag)
	{
		switch (next_state)
		{
			case CHK_FIRST_TOKEN:
				line_num++;
				current_token = fft(fin);

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
				if(current_token == "") 	      // Line only had a label (and maybe a comment)
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
						getchar();
						break;
				}

				break;

			case DIRECT: // id_ptr should already point to the correct DIR
				// No matter the outcome of the directive (error or not), the next state will be CHK_FIRST_TOKEN
				next_state = CHK_FIRST_TOKEN;

				// The directive is assumed to have caused an error until proven otherwise
				directive_error_flag = true;

			 	// If the type is not ALIGN, END, or STRING, the value needs to be parsed
				if(id_ptr->mnemonic[1] != 'L' && id_ptr->mnemonic[1] != 'N' && id_ptr->mnemonic[1] != 'T')
				{
					current_token = fnt();						// Find next token
					symtbl_ptr = get_symbol(current_token);		// Check symtbl for that token

					current_token = "#" + current_token;  // make it an indexed value for the parser
					addr_mode = parse(current_token, value0, value1);

					if(addr_mode == IMMEDIATE) directive_error_flag = false;
					else error_detected_no_cnt("Directive: Found Unknown Label after DIRECTIVE (Value0 parsing, #2)");
				}

				switch (id_ptr->mnemonic[1])
				{
					case 'L':  // Align
						if(LC%2) LC++;
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
							if(value0 >= -128 && value0 < 256) 
							{
								write_srec_byte((unsigned char)value0);

								// For diagnostics and increasing LC of the second_pass
								emit("BYTE", current_token, NONE, outfile, LC);
								LC += 0x01;
							}
							else error_detected_no_cnt("Directive: Value too large for BYTE directive");
						}

						break;

					case 'N':  // END
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
						current_token.pop_back(); // Removes Closing Quote

						for(int i = 0; i < current_token.length(); i++)
						{
							write_srec_byte(current_token[i]);
						}

						value0 = current_token.length();

						LC += value0;

						break;

					case 'O':  // WORD
						if(!directive_error_flag)
						{
							if(LC%2) LC += 0x01;  // Align LC first
							if(value0 > -65536 && value0 < 65535)
							{
								// EMIT WORD
								emit("WORD", current_token, NONE, outfile, LC);
								LC += 0x02;

							}
							else error_detected_no_cnt("Directive: Value too large for WORD directive");
						}
						break;

					default:
						std::cout << "\t\t[Directive] DEFAULT ERROR" << std::endl;
						getchar();  // This should never happen, getchar will stop the runtime and let the user know there is a serious flaw
						break;
				}

				break;
			default:
				std::cout << "\t\t[Second Pass] DEFAULT ERROR" << std::endl; // This should literally never happen
				getchar();  // This should never happen, getchar will stop the runtime and let the user know there is a serious flaw
				break;
		}
	}

	// Output any bytes still in the buffer and then close with an S9
	output_srec_buffer();
	write_S9();

	outfile.close();
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
	std::cout << "\t\t[ERROR MSG - FIRST PASS=] " << error_msg << std::endl;
	getchar();
}
