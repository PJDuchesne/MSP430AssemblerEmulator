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
#include <fstream>
#include <cstdlib>

#include "main.h"
#include "symtbl.h"
#include "inst_dir.h"
#include "library.h"

// TO ADD
	// Input argument options (Drag and drop, input text, etc.)
	// Operand Parser
	// First pass function
	// Transition logic
	// Second pass function
	// Closing

// GLOBAL VARIABLES

symtbl_entry* symtbl_ptr = NULL;  		// symtbl_ptr

int LC = 0;  					// Define Location Counter

state next_state = START;  			// Define state variable

int line_num = 0;  				// Current Line Number

std::string current_record; 			// Current record string

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
	// Initialize a few things (LC = 0, init_symtbl, open file
	LC = 0;

 	init_symtbl();

	std::ifstream fin("dev_input.txt");

	// Run first pass


	// Run second pass (If no errors or unknowns from first pass)


	// Tidy up to finish (Close file, etc.)


	fin.close();

	return 0;
}

// Find next token
std::string fnt(std::string, int record_pos)
{
	// Returns NULL of there is no next token (hits EOL)
	std::string token = NULL;
}
