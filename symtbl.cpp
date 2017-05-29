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

-> Name:  symtbl.cpp
-> Brief: Implements the symtbl with functions and such
-> Date: May 15, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <cstdlib>
#include <iomanip>

#include "Include/symtbl.h"
#include "Include/inst_dir.h"

std::string types[] = {"REGISTER", "KNOWN", "UNKNOWN"};

// Pointer to the start of the symbol table
symtbl_entry* symtbl_master = NULL;

void init_symtbl()
{
	// Add r0-r15, R0-R15, plus aliases
	add_symbol("R0",  0,  REG);
	add_symbol("R1",  1,  REG);
	add_symbol("R2",  2,  REG);
	add_symbol("R3",  3,  REG);
	add_symbol("R4",  4,  REG);
	add_symbol("R5",  5,  REG);
	add_symbol("R6",  6,  REG);
	add_symbol("R7",  7,  REG);
	add_symbol("R8",  8,  REG);
	add_symbol("R9",  9,  REG);
	add_symbol("R10", 10, REG);
	add_symbol("R11", 11, REG);
	add_symbol("R12", 12, REG);
	add_symbol("R13", 13, REG);
	add_symbol("R14", 14, REG);
	add_symbol("R15", 15, REG);
	add_symbol("r0",  0,  REG);
	add_symbol("r1",  1,  REG);
	add_symbol("r2",  2,  REG);
	add_symbol("r3",  3,  REG);
	add_symbol("r4",  4,  REG);
	add_symbol("r5",  5,  REG);
	add_symbol("r6",  6,  REG);
	add_symbol("r7",  7,  REG);
	add_symbol("r8",  8,  REG);
	add_symbol("r9",  9,  REG);
	add_symbol("r10", 10, REG);
	add_symbol("r11", 11, REG);
	add_symbol("r12", 12, REG);
	add_symbol("r13", 13, REG);
	add_symbol("r14", 14, REG);
	add_symbol("r15", 15, REG);
	add_symbol("PC",  0,  REG);
	add_symbol("Pc",  0,  REG);
	add_symbol("pC",  0,  REG);
	add_symbol("pc",  0,  REG);
	add_symbol("SP",  1,  REG);
	add_symbol("Sp",  1,  REG);
	add_symbol("sP",  1,  REG);
	add_symbol("sp",  1,  REG);
	add_symbol("SR",  2,  REG);
	add_symbol("Sr",  2,  REG);
	add_symbol("sR",  2,  REG);
	add_symbol("sr",  2,  REG);
	add_symbol("CG1", 2,  REG);
	add_symbol("cG1", 2,  REG);
	add_symbol("Cg1", 2,  REG);
	add_symbol("cg1", 2,  REG);
	add_symbol("CG2", 3,  REG);
	add_symbol("Cg2", 3,  REG);
	add_symbol("cG2", 3,  REG);
	add_symbol("cg2", 3,  REG);
}

void add_symbol(std::string label, int value, SYMTBLTYPE type)
{
	symtbl_entry* new_entry = new symtbl_entry();
	new_entry->label = label;
	new_entry->value = value;
	new_entry->type = type;
	new_entry->next = symtbl_master;
	new_entry->line = line_num;
	symtbl_master = new_entry;

	// FOR DEVELOPMENT: Prevents printing of static portion of the symbol table (All the registers)
	// if(new_entry->type != REG) std::cout << "\t\t\t\tAdded >>" << label << "<< to the symbol table with type: >>" << type << "<< and value >>" << new_entry->value << "<<" << std::endl;
}

void output_symtbl()
{
	int temp_cnt = 0;
    symtbl_entry* temp = symtbl_master;

	// PURELY AESTECTIC PORTION (Formatting column width of output print so it looks nice)

    int entry_no_length = 1;	// At least 1 length (Actually 2 because the symtbl is initialized, but this works with the code below)
	int line_no_length = 1;		// At least 1 length
    int max_label_length = 3;	// At least 3 length

	while(temp->next != NULL)
	{
		if(temp->label.length() > max_label_length) max_label_length = temp->label.length();
		temp = temp->next;
		temp_cnt++;
	}

	while(temp_cnt != temp_cnt%10)
	{
		temp_cnt = temp_cnt%10;
		entry_no_length++;
	}

	temp_cnt = line_num;
	while(temp_cnt != temp_cnt%10)
	{
		temp_cnt = temp_cnt%10;
		line_no_length++;
	}

	// ACTUALLY PRINTING

	temp_cnt = 0;
    // Iterate through points by using the "next" pointer on each value
	temp = symtbl_master;
    while(temp->next != NULL)
    {
    	// Don't display registers (FOR DEVELOPMENT)
    	if(temp->type == REG) break;

        std::cout << "Entry #"    << std::right << std::setfill('0') << std::setw(entry_no_length) << std::dec << temp_cnt;
		std::cout << " | Label: " << std::left << std::setfill(' ') << std::setw(max_label_length) << temp->label;
		// Values of -1 (Unknowns) will appear as ffffffff (twos compliment output)
		std::cout << " | Value: " << std::right << std::setfill('0') << std::setw(4) << std::hex << temp->value;
		std::cout << " | Line #"  << std::right << std::setfill('0') << std::setw(line_no_length) << std::dec << temp->line;
		std::cout << " | type: "  << types[temp->type] << std::endl;

        temp = temp->next;
        temp_cnt++;
    }
    std::cout << std::endl;
}

symtbl_entry* get_symbol(std::string label)
{	
	symtbl_entry* temp = symtbl_master;

	while(temp->next != NULL)
	{
		if(temp->label == label) return temp;
		temp=temp->next;
	}
	if(temp->label == label) return temp;
	return NULL;
}

bool valid_symbol(std::string token)
{
	inst_dir* id_ptr = get_inst_dir(token, I); 
	if(id_ptr != NULL) return false; // Symbol cannot be an instruction
	id_ptr = get_inst_dir(token, D);
	if(id_ptr != NULL) return false; // Symbol cannot be a directive

	if(token.length() > 31) return false; // Symbol cannot be longer than 31 characters

	// First token must be alphabetic (A-Z, a-z, or _)
	else if(((token[0] >= 65) && (token[0] <= 90))||((token[0] >= 97) && (token[0] <= 122))||(token[0] == 95))
	{
		int temp_cnt = 1;
		
		while(temp_cnt < token.length())
		{
			// Remaining tokens can be alphanumeric (A-Z, a-z, 0-9, or _)	
			if(!((token[temp_cnt] >= 65 && token[temp_cnt] <= 90)||(token[temp_cnt] >= 97 && token[temp_cnt] <= 122)||(token[temp_cnt] == 95)||((token[temp_cnt] >= 48)&&(token[temp_cnt] <= 57)))) break;
			temp_cnt++;
		}
		return (temp_cnt == token.length()) ? true : false;
	}
	else return false;
}

void symtbl_unknown_check()
{
	symtbl_entry* se_ptr = symtbl_master;
	while(se_ptr->next != NULL)
	{
		if(se_ptr->type == 2) err_cnt++;
		se_ptr = se_ptr->next;
	}
}
