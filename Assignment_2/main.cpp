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
-> Date: June 6, 2017	(Created)
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

// Globals
std::ifstream fin;
std::ofstream outfile;

unsigned short s9_addr;

char mem_array[MAX_MEM_SIZE];

int main(int argc, char *argv[])
{
	/*

	// Used for rapid testing of specific test file

	// "Drag and drop" capability, used in command line personnaly
	if(argc < 2)
	{
		std::cout << "ERROR: Missing input file" << std::endl;
		getchar();
		exit(0);
	}

	fin.open(argv[1]); // Open file specified by argument

	if(!fin.is_open())
	{
		std::cout << "ERROR READING FILE" << std::endl;
		getchar();
		exit(0);
	}
	*/

	// LOADER

	char menuInput = 0;

	while(1)
	{
		std::cout << "MAIN MENU: Please enter command from below\n"
				<< "\tLoad from previous session (P)\n"
				<< "\tEmulate current memory (E)\n"
		     		<< "\tLoad from file  (F)\n"
		     		<< "\tDebugger Mode (D)\n"
		     		<< "\tHCF (H)\n"
		     		<< "\tQuit (Q)\n\n"

				<< "\tInput: >> ";

		menuInput = getchar();

		switch(menuInput)
		{
			case 'P':
			case 'p':
				
				
				
				getchar();
				break;

			case 'F':
			case 'f':

				getchar();
				break;

			case 'D':	// Debugger mode, 
			case 'd':	

				getchar();
				break;

			case 'H':	// Halt and catch fire
			case 'h':	
				system("aafire");
				getchar();
				break;
	
			case 'Q':	// Quit
			case 'q':	
				exit(0);
				break;
		}
	}

	load_file();

	outfile.open("mem.txt");

	dump_mem();

	bool end = true;

	while (!end)
	{
		// Fetch
		// Decode
		// Execute
	}

	// For diagnostics

	fin.close();
	outfile.close(); // Note: srec_file is closed in the s9 function 

	return 0;
}

