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
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>

#include "Include/main.h"
#include "Include/library.h"
#include "Include/symtbl.h"
#include "Include/inst_dir.h"
#include "Include/parser.h"
#include "Include/first_pass.h"
#include "Include/second_pass.h"
#include "Include/emitter.h"

std::string current_record = "";
std::string current_token  = "";
int err_cnt = 0;

symtbl_entry* se_ptr = NULL;

int main(int argc, char *argv[])
{
	// "Drag and drop" capability, used in command line personnaly
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

	init_symtbl();

	std::ifstream fin(argv[1]);

 	first_pass(fin);

	std::cout << std::endl << std::endl << "First pass completed with >>" << err_cnt << "<< Errors" << std::endl << std::endl;

	symtbl_unknown_check();

	std::cout << std::endl << "First pass completed with >>" << err_cnt << "<< Errors (Including Unknowns)" << std::endl << std::endl;

 	output_symtbl();

 	if(err_cnt == 0)
 	{
		// Rewind file to beginning
		fin.clear();
		fin.seekg(0);

  		second_pass(fin);
 	}
 	else
 	{
 		// Print symtbl to LOG file
 	}

	fin.close();

	return 0;
}
