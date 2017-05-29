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

-> Name:  library.cpp
-> Brief: Implementation for the library.cpp, a helper library
-> Date: May 15, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>

#include "Include/library.h"

/*
	Function: fft (find first token)
	Output: std::string: The first token of the new record
	Input: fin: the input .asm input file open to the assembly code
	Brief: Fetches next the record and passes back the first token
			after removing any comments from the line. If there is no
			token on the line, "" is returned.
*/
std::string fft(std::istream& fin)
{
	std::getline(fin, current_record);

	std::string token;

	// Remove comment from line
	current_record = current_record.substr(0, current_record.find_first_of(";"));

	// Used to fix an error an error
	current_record = " " + current_record;

	char* temp_crecord = new char[current_record.length()+1];

	std::strcpy(temp_crecord, current_record.c_str()+1);

	char* temp_ctoken = std::strtok(temp_crecord, " \t\n");

	if (temp_ctoken == NULL) return "";

	token.assign(temp_ctoken, strlen(temp_ctoken));

	return token;
}

/*
	Function: fnt (find next token)
	Output: std::string: The next token of the current record
	Brief: Using the internal storage of 'strtok', the next
			token is returned without having to pass the file
			pointer in. If there is no token on the line, ""
			is returned.
*/
std::string fnt()
{
	std::string token;

	char* temp_ctoken = strtok(NULL, " \t\n");

	if (temp_ctoken != NULL) token.assign(temp_ctoken, strlen(temp_ctoken));
	else return "";	

	return token;
}

