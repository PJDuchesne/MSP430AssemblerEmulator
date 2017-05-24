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
-> Date: May 17, 2017   (Last Modified)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>

#include "library.h"

// Find first token Implementation
std::string fft(std::istream& fin)
{
	std::getline(fin, current_record);

	std::cout << std::endl << "Record #" << line_num << ": >>" << current_record << "<<" << std::endl;

  std::string token;

  // Remove comment from line
  current_record = current_record.substr(0, current_record.find_first_of(";"));

  current_record = " " + current_record; // Testing

  char* temp_crecord = new char[current_record.length()+1];

  std::strcpy(temp_crecord, current_record.c_str()+1);

  char* temp_ctoken = std::strtok(temp_crecord, " \t\n");

  if (temp_ctoken == NULL) return "";

  token.assign(temp_ctoken, strlen(temp_ctoken));

  return token;
}

// Find next token implementation
std::string fnt()
{
  std::string token;

  char* temp_ctoken = strtok(NULL, " \t\n");

  if (temp_ctoken != NULL) token.assign(temp_ctoken, strlen(temp_ctoken));
  else return "";

//      std::cout << "Token x: >>" << token << "<<" << std::endl;

  return token;
}

