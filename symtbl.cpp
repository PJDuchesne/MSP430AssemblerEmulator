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

// Types as string, this order corresponds to the enumeration order in library.h
std::string types[] = {"REGISTER", "KNOWN", "UNKNOWN"};

// Pointer to the start of the symbol table
symtbl_entry* symtbl_master = NULL;

/*
    Function: init_symtbl
    Brief: This function takes initializes the symbol table by adding all the
    		registers and their aliases to the symbol table.
*/
void init_symtbl()
{
	// Added r0-r15, R0-R15, plus aliases (and case values)
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

/*
    Function: add_symbol
    Input: label: input strint to add
    	   value: value associated with the label
    	   type: symbol table type for the symbol (Unknown, Known, or REG)
    Brief: This function adds a symbol to the symbol table. It does no validity
    		testing, that should be done (perhaps with "Valid_Symbol(X)") before
    		calling this function.
*/
void add_symbol(std::string label, int value, SYMTBLTYPE type)
{
	symtbl_entry* new_entry = new symtbl_entry();
	new_entry->label = label;
	new_entry->value = value;
	new_entry->type = type;
	new_entry->next = symtbl_master;
	new_entry->line = line_num;
	symtbl_master = new_entry;
}

/*
    Function: output_symtbl()
    Brief: This function outputs the symbol table when called by iterating
    		through the table until the last result. Some aestetics were
    		added for ease of readibility during debugging, these involve
    		determining the maximum size for each column and setting the
    		cout width to that value.
*/
void output_symtbl()
{
	int temp_cnt = 0;
	int temp_cnt2 = 0;

    symtbl_entry* temp = symtbl_master;

	// PURELY AESTECTIC PORTION (Formatting column width of output print so it looks nice)

    int entry_no_length = 0;
	int line_no_length = 0;

	// To format, I iterate through the symbol table first, obtaining the number
	// of entries, the maximum line number, and the maximum label length
    int max_label_length = 0;	// 
	int max_symbol_line = 0;

	while(temp->next != NULL)
	{
		if(temp->label.length() > max_label_length) max_label_length = temp->label.length();
		if(temp->line > max_symbol_line) max_symbol_line = temp->line;
		temp = temp->next;
		temp_cnt++;
	}

	// Determining width of n in "ENTRY #n" column
	while(temp_cnt >= 1)
	{
		temp_cnt = temp_cnt/10;
		entry_no_length++;
	}

	// Determining width of n in "Line #n" column
	while(max_symbol_line >= 1)
	{
		max_symbol_line /= 10;
		line_no_length++;
	}

	// ACTUAL PRINTING
	std::cout << "SYMBOL TABLE: (Starting With Most Recently Added Entry)" << std::endl << std::endl;
	outfile << "SYMBOL TABLE: (Starting With Most Recently Added Entry)" << std::endl << std::endl;

    // Iterate through points by using the "next" pointer on each value
    temp_cnt = 0;
	temp = symtbl_master;
    while(temp->next != NULL)
    {
	// To terminal
        std::cout << "\tEntry #"  << std::right << std::setfill('0') 
        		  << std::setw(entry_no_length) << std::dec << temp_cnt;
		std::cout << " | Label: " << std::left << std::setfill(' ') 
				  << std::setw(max_label_length) << temp->label;
		// Values of -1 (Unknowns) will appear as ffff (twos compliment output)
		std::cout << " | Value: " << std::right << std::setfill('0')
				  << std::setw(4) << std::hex << (unsigned short)temp->value;
		std::cout << " | Line #"  << std::right << std::setfill('0')
				  << std::setw(line_no_length) << std::dec << temp->line;
		std::cout << " | type: "  << types[temp->type] << std::endl;

	// To diagnostics
        outfile << "\tEntry #"    << std::right << std::setfill('0')
        		<< std::setw(entry_no_length) << std::dec << temp_cnt;
		outfile << " | Label: " << std::left << std::setfill(' ')
				<< std::setw(max_label_length) <<temp->label;
		// Values of -1 (Unknowns) will appear as ffff (twos compliment output)
		outfile << " | Value: " << std::right << std::setfill('0')
				<< std::setw(4) << std::hex << (unsigned short)temp->value;
		outfile << " | Line #"  << std::right << std::setfill('0')
				<< std::setw(line_no_length) << std::dec << temp->line;
		outfile << " | type: "  << types[temp->type] << std::endl;

        temp = temp->next;
        temp_cnt++;
    }
    std::cout << std::endl;
}

/*
    Function: get_symbol
    Input: label: The string symbol to search for.
    Output: symtbl_entry*: Pointer to a symbol table entry, this is NULL
    		if the symbol is not found.
    Brief: This function linearly searches the symbol table starting with
    		the most recently added symbol. When the correct symbol is
    		found, it is returned.
*/
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

/*
    Function: valid_symbol
    Input: token: The string token to check the validity of
    Output: bool: True means the symbol is valid, false means it is not
    Brief: This function checks whether or not the given string is a valid
    		symbol or not. This is used before calling "add_symbol" as an
    		error checker.
*/
bool valid_symbol(std::string token)
{
	inst_dir* id_ptr = get_inst_dir(token, I); 
	if(id_ptr != NULL) return false; // Symbol cannot be an instruction
	id_ptr = get_inst_dir(token, D);
	if(id_ptr != NULL) return false; // Symbol cannot be a directive

	if(token.length() > 31) return false; // Symbol cannot be longer than 31 characters

	// First token must be alphabetic (A(65) to Z(90), a(97) to z(122), or _(95))
	else if(((token[0] >= 65) && (token[0] <= 90))||((token[0] >= 97) && (token[0] <= 122))||(token[0] == 95))
	{
		int temp_cnt = 1;
		
		while(temp_cnt < token.length())
		{
			// Remaining tokens can be alphanumeric ('A'(65) to 'Z'(90), 'a'(97) to 'z'(122), '0'(48) to '9'()57, or '_'(95))
			if(!((token[temp_cnt] >= 65 && token[temp_cnt] <= 90)||(token[temp_cnt] >= 97 && token[temp_cnt] <= 122)||(token[temp_cnt] == 95)||((token[temp_cnt] >= 48)&&(token[temp_cnt] <= 57)))) break;
			temp_cnt++;
		}
		return (temp_cnt == token.length()) ? true : false;
	}
	else return false;
}

/*
    Function: symtbl_unknown_check()
    Brief: This function linearly scans through the symbol table increasining
    		the error count for every UNKNOWN label found. This is called after
    		the first pass is complete in order to ensure everything is in order
    		before starting the second pass.
*/
void symtbl_unknown_check()
{
	symtbl_entry* se_ptr = symtbl_master;
	
	int starting_err_cnt = err_cnt;

	std::cout << std::endl << "\tChecking Symbol Table for Unresolved Unknowns:" << std::endl;

	while(se_ptr->next != NULL)
	{
		if(se_ptr->type == UNKNOWN) err_cnt++;
		se_ptr = se_ptr->next;
	}

	if(err_cnt == starting_err_cnt)
	{
		std::cout << std::endl << "\t\tNo unknowns found in the symbol table" << std::endl;
		outfile << std::endl << "\t\tNo unknowns found in the symbol table" << std::endl;
	}
	else
	{
		std::cout << std::endl << "\t\tTotal unknowns found on the symbol table: >>" << (err_cnt - starting_err_cnt) << "<< (See symbol table below for UNKNOWN line numbers)" << std::endl;
		outfile << std::endl << "\t\tTotal unknowns found on the symbol table: >>" << (err_cnt - starting_err_cnt) << "<< (See symbol table below for UNKNOWN line numbers)" << std::endl;
	}

}
