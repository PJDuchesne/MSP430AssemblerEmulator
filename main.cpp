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

std::string current_token;	// Current token string

inst_dir* id_ptr = NULL;			// id_ptr thingy

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

/*
	// Testing INST_DIR Table search (Binary Search)
	id_ptr = get_inst("moVeeee", I);

	if(id_ptr != NULL) std::cout << "LOOKED FOR MOVIE: " << id_ptr->mnemonic << std::endl;
	else std::cout << "NOT FOUND" << std::endl;
*/

	while(!fin.eof())
	{	
		line_num++;
		std::getline(fin, current_record);
		std::cout << "LINE " << line_num << " >>" << current_record << "<<" << std::endl;
		current_token = fft();
		while (current_token != "")
		{
			current_token = fnt();
		}
		current_token = "";
		std::cout << std::endl;
	}

	// Run first pass

	// Run second pass (If no errors or unknowns from first pass)


	// Tidy up to finish (Close file, etc.)


	fin.close();

	return 0;
}

// Find next token
// Deletes comments in line and anything after it

std::string fft()
{
	std::string token;

	int temp = current_record.find_first_of(";") - 1;

	if (temp != -2) current_record.resize(int(current_record.find_first_of(";"))-1);

        char* temp_crecord = new char[current_record.length()];     

        std::strcpy(temp_crecord, current_record.c_str());

        char* temp_ctoken = std::strtok(temp_crecord, " \t\n");
	
	if (temp_ctoken == NULL) return "";

	token.assign(temp_ctoken, strlen(temp_ctoken));

//  	delete[] temp_crecord;  // Do this wayyyy later?

	std::cout << "Token f: >>" << token << "<<" << std::endl;

	return token;
}

std::string fnt()
{
	std::string token;

	char* temp_ctoken = strtok(NULL, " \t\n");

	if (temp_ctoken != NULL) token.assign(temp_ctoken, strlen(temp_ctoken));
	else return "";

	std::cout << "Token x: >>" << token << "<<" << std::endl;

	return token;
}
