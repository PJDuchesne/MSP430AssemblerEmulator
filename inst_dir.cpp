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

-> Name: inst_dir.cpp
-> Brief: Implementation file for the inst_dir file
-> Date: May 15, 2017   (Created)
-> Date: May 17, 2017   (Last Modified)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

// Get inst/dir (binary search)

#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <cstdlib>

#include <algorithm>

#include "inst_dir.h"

inst_dir inst_dir_array[] = {
        // INSTRUCTIONS
        {"ADD",  DOUBLE, 0x5000, WORD},                 // 0
                {"ADD.B",  DOUBLE, 0x5000, BYTE},
                {"ADD.W",  DOUBLE, 0x5000, WORD},
        {"ADDC", DOUBLE, 0x6000, WORD},
                {"ADDC.B", DOUBLE, 0x5000, BYTE},
                {"ADDC.W", DOUBLE, 0x5000, WORD},
        {"AND",  DOUBLE, 0xf000, WORD},
                {"AND.B",  DOUBLE, 0xf000, BYTE},
                {"AND.W",  DOUBLE, 0xf000, WORD},
        {"BIC",  DOUBLE, 0xc000, WORD},
                {"BIC.B",  DOUBLE, 0xc000, BYTE},
                {"BIC.W",  DOUBLE, 0xc000, WORD},
        {"BIS",  DOUBLE, 0xd000, WORD},
                {"BIS.B",  DOUBLE, 0xd000, BYTE},
                {"BIS.W",  DOUBLE, 0xd000, WORD},
        {"BIT",  DOUBLE, 0xb000, WORD},
                {"BIT.B", DOUBLE, 0xb000, BYTE},
                {"BIT.W", DOUBLE, 0xb000, WORD},
        {"CALL", SINGLE, 0x1280, WORD},
        {"CMP",  DOUBLE, 0x9000, WORD},
                {"CMP.B", DOUBLE, 0x9000, BYTE},
                {"CMP.W", DOUBLE, 0x9000, WORD},
        {"DADD", DOUBLE, 0xa000, WORD},
                {"DADD.B", DOUBLE, 0xa000, BYTE},
                {"DADD.W", DOUBLE, 0xa000, WORD},
        {"JC",   JUMP, 0x2c00, OFFSET},
        {"JEQ",  JUMP, 0x2400, OFFSET},
        {"JGE",  JUMP, 0x3400, OFFSET},
        {"JHS",  JUMP, 0x2c00, OFFSET},
        {"JL",   JUMP, 0x3800, OFFSET},
        {"JLO",  JUMP, 0x2800, OFFSET},
        {"JMP",  JUMP, 0x3c00, OFFSET},
        {"JN",   JUMP, 0x3000, OFFSET},
        {"JNC",  JUMP, 0x2800, OFFSET},
        {"JNE",  JUMP, 0x2000, OFFSET},
        {"JNZ",  JUMP, 0x2000, OFFSET},
        {"JZ",   JUMP, 0x2400, OFFSET},
        {"MOV",  DOUBLE, 0x4000, WORD},
                {"MOV.B",  DOUBLE, 0x4000, BYTE},
                {"MOV.W",  DOUBLE, 0x4000, WORD},
	{"PUSH", SINGLE, 0x1200, WORD},
                {"PUSH.B", SINGLE, 0x1200, BYTE},
                {"PUSH.W", SINGLE, 0x1200, WORD},
        {"RETI", NONE, 0x1300, WORD},
        {"RRA",  SINGLE, 0x1100, WORD},
                {"RRA.B", SINGLE, 0x1100, BYTE},
                {"RRA.W", SINGLE, 0x1100, WORD},
        {"RRC",  SINGLE, 0x1000, WORD},
                {"RRC.B", SINGLE, 0x1000, BYTE},
                {"RRC.W", SINGLE, 0x1000, WORD},
        {"SUB",  DOUBLE, 0x8000, WORD},
                {"SUB.B",  DOUBLE, 0x8000, BYTE},
                {"SUB.W",  DOUBLE, 0x8000, WORD},
        {"SUBC", DOUBLE, 0x7000, WORD},
                {"SUBC.B", DOUBLE, 0x7000, BYTE},
                {"SUBC.W", DOUBLE, 0x7000, WORD},
        {"SWPB", SINGLE, 0x1080, WORD},
        {"SXT",  SINGLE, 0x1180, WORD},
        {"XOR",  DOUBLE, 0xe000, WORD},
                {"XOR.B",  DOUBLE, 0xe000, BYTE},
                {"XOR.W",  DOUBLE, 0xe000, WORD},       // 60

        // DIRECTIVES (To finish)
        {"ALIGN",  NONE, 0xffff, WORD},  // DIR         // 61
        {"BSS",    NONE, 0xffff, WORD},  // DIR
        {"BYTE",   NONE, 0xffff, WORD},  // DIR
        {"END",    NONE, 0xffff, WORD},  // DIR
        {"EQU",    NONE, 0xffff, WORD},  // DIR
        {"ORG",    NONE, 0xffff, WORD},  // DIR
        {"STRING", NONE, 0xffff, WORD}   // DIR         // 67
};

inst_dir* get_inst(std::string input, searchtype stype)
{
	// Transform input string to uppercase
	std::transform(input.begin(), input.end(), input.begin(), ::toupper);

	int bottom;
	int top;

	if (stype = I)
	{
		bottom = 0;
		top    = 60;
	}
	else if (stype = D)
	{
		bottom = 61;
		top    = 68;
	}
	else std::cout << "ERROR: INVALID SEARCH TYPE" << std::endl;

	while(top-bottom != 1)
	{
		std::cout << "Bottom: " << bottom << " | Top: " << top << std::endl;
		int cnt = (top+bottom)/2;						// Update count to middle of new top and bottom
		if (inst_dir_array[cnt].mnemonic == input) return &inst_dir_array[cnt];	// Check array and return if it is the right one
		if(inst_dir_array[cnt].mnemonic[0] > input[0]) top = cnt;		// Check if desired value is higher or lower
		else bottom = cnt;
	}
	return NULL;
}

/*
inst_dir* get_dir(std::string dir);
{
	int cnt = 64	// 

}
*/
