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
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <cstdlib>
#include <algorithm>

#include "Include/inst_dir.h"

/*
    Struct: inst_dir_array
    Brief: Array containing all instructions and directives in
            a tidy table for getting at later.
*/
inst_dir inst_dir_array[] = {
        // Instructions on slots 0,60
        {"ADD",  DOUBLE, 0x5000, WORD}, 
                {"ADD.B",  DOUBLE, 0x5000, BYTE},
                {"ADD.W",  DOUBLE, 0x5000, WORD},
        {"ADDC", DOUBLE, 0x6000, WORD},
                {"ADDC.B", DOUBLE, 0x6000, BYTE},
                {"ADDC.W", DOUBLE, 0x6000, WORD},
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
        {"DADC", DOUBLE, 0xa000, WORD},
                {"DADC.B", DOUBLE, 0xa000, BYTE},
                {"DADC.W", DOUBLE, 0xa000, WORD},
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
                {"XOR.W",  DOUBLE, 0xe000, WORD},
       
        // Directives on slots 61,68
        {"ALIGN",  NONE, 0xffff, WORD},
        {"BSS",    NONE, 0xffff, WORD},
        {"BYTE",   NONE, 0xffff, WORD}, 
        {"END",    NONE, 0xffff, WORD},
        {"EQU",    NONE, 0xffff, WORD},
        {"ORG",    NONE, 0xffff, WORD},
        {"STRING", NONE, 0xffff, WORD},
        {"WORD"  , NONE, 0xffff, WORD},
};

/*
    Function: get_inst_dir
    Input: input: A string to search for; stype: the type to search for (Inst or DIR)
    Output: inst_dir*: A pointer to the matching instruction or directive.
    Brief: Performs a binary search that returns the value searched for or else a NULL
            pointer. All input is transformed to uppercase because instructions and
            directives are case insensitive.
*/
inst_dir* get_inst_dir(std::string input, SEARCHTYPE stype)
{
	std::transform(input.begin(), input.end(), input.begin(), ::toupper);

	int bottom = (stype == I) ? 0  : 61;
	int top    = (stype == I) ? 60 : 68;

	int char_cnt = 0;
	int cnt = 0;

	if(stype != I && stype != D)
	{
		std::cout << "ERROR: INVALID SEARCH TYPE" << std::endl;
		return NULL;
	}

    // Check top/bottom values
    if(inst_dir_array[top].mnemonic == input)
    {
        return &inst_dir_array[top];
    }
    if(inst_dir_array[bottom].mnemonic == input)
    {
        return &inst_dir_array[bottom];
    }

    // Iterate through the list, slowly narrowing down the target search
    // until the value is found or the search window gets small enough.
    while(top-bottom != 1)
    {
        cnt = (top+bottom)/2;
        if (inst_dir_array[cnt].mnemonic == input)
        {
            // std::string temp_str = (cnt <= 60) ? "[INST]" : "[DIR]"; 
            return &inst_dir_array[cnt];
        }

        if(inst_dir_array[cnt].mnemonic > input) top = cnt;
        else bottom = cnt;
    }
	return NULL;
}
