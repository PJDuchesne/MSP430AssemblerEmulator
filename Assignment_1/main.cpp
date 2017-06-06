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

// Globals
std::string current_record = "";
std::string current_token  = "";
int err_cnt = 0;

std::ifstream fin;
std::ofstream outfile;
std::ofstream srec_file;

int main(int argc, char *argv[])
{
	// "Drag and drop" capability, used in command line personnaly
	if(argc < 2)
	{
		std::cout << "ERROR: Missing input file" << std::endl;
		getchar();
		exit(0);
	}

	fin.open(argv[1]);

	if(!fin.is_open())
	{
		std::cout << "ERROR READING FILE" << std::endl;
		getchar();
		exit(0);
	}
	
	init_symtbl();

	// For diagnostics
	outfile.open("diagnostics.LIS");

	outfile << "FIRST PASS DIAGNOSTICS (Emitted Records are in ERROR)" << std::endl << std::endl;

	// Runs the first pass
 	first_pass();

	std::cout << std::endl << "\tFirst Pass Completed with >>" << err_cnt 
	          << "<< Errors (Not including unknowns)" << std::endl;
	outfile << std::endl << "\tFirst Pass Completed with >>" << err_cnt
		    << "<< Errors (Not including unknowns)" << std::endl;

	// Check the symbol table for unresolved unknowns
	symtbl_unknown_check();

	std::cout << std::endl << "\tFirst Pass Completed with >>" << err_cnt
			  << "<< Errors (Including unknowns)" << std::endl;
	outfile << std::endl << "\tFirst Pass Completed with >>" << err_cnt
		    << "<< Errors (Including unknowns)" << std::endl;

	// If there are no errors, rewind file and run second pass
 	if(err_cnt == 0)
 	{
		// Rewind file to beginning
		fin.clear();
		fin.seekg(0);

		// Run second pass
		outfile << std::endl << "SECOND PASS DIAGNOSTICS (All Records emitted with format shown below)";
		outfile << std::endl << std::endl;
  		second_pass();
 	}

	std::cout << std::endl;
	outfile << std::endl;
	
	output_symtbl();

	outfile << std::endl << "END OF DIAGNOSTICS OUTFILE" << std::endl;

	fin.close();
	outfile.close(); // Note: srec_file is closed in the s9 function 

	return 0;
}
