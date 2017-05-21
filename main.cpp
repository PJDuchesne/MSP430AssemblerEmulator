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

-> Name:  main.cpp
-> Brief: Implementation for the main.cpp the code that runs executive commands
-> Date: May 15, 2017	(Created)
-> Date: May 17, 2017	(Last Modified)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>

#include "main.h"
#include "symtbl.h"
#include "inst_dir.h"
#include "library.h"
#include "parser.h"
#include "first_pass.h"

// TO ADD
	// Input argument options (Drag and drop, input text, etc.)
	// First pass function
	// Transition logic
	// Second pass function
	// Closing

// GLOBAL VARIABLES

std::string current_record = "";
std::string current_token  = "";
int err_cnt = 0;

int main(int argc, char *argv[])
{

/*	
	// Example input argument for eventual adding of input/output parameters. Hardcoded for now
	if(argc < 2)
	{
		std::cout << "ERROR" << std::endl;
		getchar();
		exit(0);
	}
	else
	{
		std::cout << "NO ERROR" << std::endl;
	}
*/
	// Initialize a few things (Open file for one)

 	init_symtbl();

	std::ifstream fin("dev_input.txt");
/*
	while(!fin.eof())
	{	
// 		std::getline(fin, current_record); // Depreciated since fft was moved to library.cpp
		// current_token = fft();
		while (current_token != "")
		{
			// current_token = fnt(); // Depreciated since fnt was moved to library.cpp
		}
		current_token = "";
		std::cout << std::endl;
	}
*/

	// Run first pass

 	first_pass(fin);

	std::cout << std::endl << std::endl << "First pass completed with >>" << err_cnt << "<< Errors" << std::endl << std::endl;

 	output_symtbl();

	// Check first pass validity

	// Run second pass (if first pass was valid)

	// Tidy up to finish (Close file, etc.)


/* TESTING FOR PARSER.CPP

	std::string addr_mode_string[] = {"REG_DIRECT", "INDEXED", "RELATIVE", "ABSOLUTE", "INDIRECT", "INDIRECT_AUTO_INC", "IMMEDIATE", "WRONG"};

	// Parser Testing
	addr_mode am_temp;
	std::string operand;

	add_symbol("Test12", 14, KNOWN);

	operand = "x(R7)";
	am_temp = parse(operand, value0, value1);
	std::cout << "Checked Operand: >>" << operand << "<< and found ADDR MODE of: >>" << addr_mode_string[am_temp] << "<< with value0 of " << value0 << " and value1 of " << value1 << std::endl;

*/

	fin.close();

	return 0;
}
