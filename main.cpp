#include <iostream>
#include <stdio.h>
#include <string>

struct INST {
	std::string mnemonic;
	int type;
	int opcode;
	int b_w;
};

struct DIR {
	std::string mnemonic;
	int dir_number; // This will trigger a separate DIR switch statement?
};

class symtbl_entry {
	private:

	public:
		symtbl_entry(std::string lbl, int val, int ty, symtbl_entry* prev_p);
		void output(symtbl_entry* top); // Outputs the contents of the symbtol table

		// Why fuck with getters?
		std::string label;
		int value;
		int type;	// -1 = Unknown, 0 = Register, 1 = Known
		symtbl_entry* prev_ptr;
};

void add_symbol(std::string lbl, int val, int ty, symtbl_entry* prev_p);

int main(void)
{
	// Define Symbol Table Pointer (To start)
	symtbl_entry* symtbl_top = NULL;

	// Add registers to symbol table
		// Also add any aliases you'd like (upper/lower case, PC/LC/Etc.)

	// Add instructions to instructions table
		// STDIO input "instructions.txt"
		// Read line by line and:
			// Use "find next token" function to parse, store individual pieces
		// Add to symbol table

	// Add directives to directive table
		// Create directives.txt
		// Input same way as INSTs


	/*
	* Define parser functions
		-> "Find next token"
		-> "Skip to next record/line"
	*/

	// 

}

symtbl_entry::symtbl_entry(std::string lbl, int val, int ty, symtbl_entry* prev_p)
{
	label = lbl;
	value = val;
	type = ty;
	prev_ptr = prev_p;
}

void symtbl_entry::output(symtbl_entry* top)
{
	// Iterate through points by using the "next" pointer on each value
	symtbl_entry* temp = top;
	int temp_cnt = 0;
	while(temp->prev_ptr != NULL)
	{
		temp_cnt++;
		std::cout << "Entry #" << temp_cnt << " | Label: " << temp->label << " | Value: " << temp->value << " | type: " << temp->type << std::endl;
		temp = temp->prev_ptr;
	}
}

void add_symbol(std::string lbl, int val, int ty, symtbl_entry* prev_p)
{
	// Call constructor from symtbl_entry class and add new entry
}