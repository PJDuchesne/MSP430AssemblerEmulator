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
#include "Include/loader.h"

// Globals
std::ofstream fin;
std::ofstream outfile;

int main(int argc, char *argv[])
{
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

	// For diagnostics
	outfile.open("diagnostics.LIS");

	fin.close();
	outfile.close(); // Note: srec_file is closed in the s9 function 

	return 0;
}

