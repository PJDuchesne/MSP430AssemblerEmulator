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
	// Transition logic
	// Second pass function
	// Closing

// GLOBAL VARIABLES

std::string current_record = "";
std::string current_token  = "";
int err_cnt = 0;

symtbl_entry* se_ptr = NULL;

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

	std::ifstream fin("Example_Code/dir_tom.txt");

 	first_pass(fin);

	std::cout << std::endl << std::endl << "First pass completed with >>" << err_cnt << "<< Errors" << std::endl << std::endl;

	symtbl_unknown_check();

	std::cout << std::endl << std::endl << "First pass completed with >>" << err_cnt << "<< Errors (Including Unknowns)" << std::endl << std::endl;

 	output_symtbl();


	// Check first pass validity

	// Run second pass (if first pass was valid)

	// Tidy up to finish (Close file, etc.)

	fin.close();

	return 0;
}
