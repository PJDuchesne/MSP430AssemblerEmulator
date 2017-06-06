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

-> Name:  parser.h
-> Brief: Header file for parser.h.cpp
-> Date: May 18, 2017
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>

#include "Include/parser.h"
#include "Include/symtbl.h"
#include "Include/library.h"

/*
    Function: parse
    Input: op: String containing the operand, 
    	   value0: Pointer to the general return value for most addressing modes
    	   value1: Pointer to the typically unused value storage of the register in indexed mode
    Output: ADDR_MODE: An enumeration of the addressing mode associated with the given input
    Brief: This function takes in an operand string and parses it into one of the 7 addressing
    		modes, or into an error (Denoted by the "WRONG" addressing mode). This is done
    		through a switch statement and many many checks.
*/
ADDR_MODE parse(std::string op, int& value0, int& value1)
{
	std::string operand = op;   // Make a copy of the operand in case the original is needed
	std::string temp_indexed;	// Used to store the register in indexed mode

	bool hex_flag = false;

	int temp_cnt = 0;

	// Setting the return values to a default -1
	value0 = -1;  // The general return value
	value1 = -1;  // The return value of the REGISTER in INDEXED mode

	// General use pointer to a symtbl entry
	symtbl_entry* se_ptr = NULL;

	// Flag for auto increment mode
	bool auto_flag = false;

	switch (operand[0])	// Checking first character of operand
	{
		case '&': // Absolute Mode (Or Error)
			/*	BRIEF STATE SUMMARY:
				This state handles the absolute addressing mode by checking
				the validity of the value. To be valid, the operand must either
				already be on the symbol table (backwards reference) or be a
				valid symbol and then added to the symbol table (forward reference)
			*/
			// Remove '&' from operand for future parsing
			operand.erase(0, 1);

			// The input '&' is an error
			if(operand == "") return WRONG;
			
			se_ptr = get_symbol(operand);

		 	// If symbol not in table, and the operand is a valid label, add to symbol table
			if(se_ptr == NULL && valid_symbol(operand))
			{
				add_symbol(operand, -1, UNKNOWN);  // This creates the forward reference
				if(value0 < MINWORD || value0 > MAXWORD) return WRONG; // Value0 cannot be larger than a word
				else return ABSOLUTE; 
			}
			else if(se_ptr != NULL)
			{
				// The se_ptr came back with something, ensure it is not a REG
				if(se_ptr->type != REG)
				{
					value0 = se_ptr->value;
					return ABSOLUTE;
				}
				else return WRONG; // Registers cannot be used in absolute mode
			}
			else return WRONG; // Therefore not in symbol table and is also not valid symbol
			break;
		case '@':	// (64) Indirect or Indirect auto-increment (OR BUST)
			/*	BRIEF STATE SUMMARY:
				This state handles the indirect and indirect auto-increment
				addressing mode of registers. This is done by searching the
				symbol table for the input operand (minus the beginning @), and
				then checking if the last character is a "+". If a plus sign is
				found in the correct spot, the auto-increment flag is set and
				the plus is removed. The remaining string is searched for in the
				symbol table and the result is checked to be a register or not.
			*/
			// Remove '@' from operand for future parsing
			operand.erase(0, 1);

			// "@" is an invalid operand
			if(operand == "") return WRONG;

			if(operand.find_last_of("+") == operand.length()-1)
			{
				operand.erase(operand.length()-1, operand.length());
				// "@+" is in an invalid operand
				if(operand == "") return WRONG;
				auto_flag = true;
			}

			// Check symbol table for the remaining operand, it must be a REG
			se_ptr = get_symbol(operand);
			if (se_ptr == NULL) return WRONG; 			 // Invalid symbol from indirect or indirect+
			else if (se_ptr->type !=REG) return WRONG;   // Invalid symbol type from indirect or indirect+
			else value0 = se_ptr->value;				 // Register value, will be between 0 and 15
			return (auto_flag ? INDIRECT_AI : INDIRECT);	
			break;

		case '#':
			/*	BRIEF STATE SUMMARY:
				This state handles the immediate addressing mode, including
				using labels as immediates. First the # symbol is erased, then
				the symbol table is checked for the current operand. If it is
				in the symbol table, it must not be a register. If it is not in
				the symbol table, yet is a valid symbol, it is added as a forward
				reference. Otherwise, it is parsed as a Decimal or Hex number
				depending on the first character being $ or not. 
			*/

			operand.erase(0, 1);

			// "#" is in invalid operand
			if(operand == "") return WRONG; // This means the input was "#" alone
			
			se_ptr = get_symbol(operand);

			if(se_ptr == NULL && valid_symbol(operand))
			{
				// Forward reference of label within immediate
				add_symbol(operand, -1, UNKNOWN);
				value0 = -1;
				return IMMEDIATE;
			}
			if(se_ptr != NULL && se_ptr->type != REG)
			{ 
				// Therefore, constant is the value from the label
				value0 = se_ptr->value;
				return IMMEDIATE;
			}
			else
			{
				// Therefore, value is a HEX number or DECIMAL number
	 			if(operand[0] == '$')
				{
					operand.erase(0, 1); 
					// "#$" is an invalid operand
					if(operand == "") return WRONG;
					hex_flag = true;
				}
				else if(operand[0] == '-' && operand.length() == 1) return WRONG; // "#-" is invalid
				// Delete preceding 0s
				while(operand[0] == '0' && operand.length() > 1) operand.erase(0, 1);

				if(operand.length() > 8 && hex_flag) return WRONG; 		// TOO LONG FOR STOL (Hex)
				if(operand.length() > 10 && !hex_flag) return WRONG; 	// TOO LONG FOR STOL (Decimal)

				// Check that all remaining characters are numeric
				if(hex_flag && operand.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) return WRONG;
				else if(!hex_flag && operand.find_first_not_of("-0123456789") != std::string::npos) return WRONG;

				// THEREFORE: "Operand" is numeric and contains a number
				value0 = std::stol(operand, nullptr, hex_flag ? 16 : 10);

				// Value0 cannot be larger than a word
				if(value0 < MINWORD || value0 > MAXWORD) return WRONG;
				else return IMMEDIATE;
			}
			break;
		default:	// Reg, Indexed, Relative	
			/*	BRIEF STATE SUMMARY:
				This state handles the remaining addressing modes, register
				direct, indexed, and relative. First indexed is checked for
				by searching for "(" and ")" in the token. Next the symbol
				table is searched to see if it is register direct or relative.
				Both are handled quite similarly after checking the type of
				the returned symbol table erntry pointer.
			*/

			 // If either is not found, 'find_first_of' returns n_pos, which is equal to -1
			if(operand.find_first_of("(") != -1 && operand.find_first_of(")") != -1)
			{
				// Therefore the operand is indexed mode
				// (Below, if wrong) Invalid INDEXED OPERAND 
				// (Closing bracket just after opening bracket)
				if(operand.find_first_of("(") + 1 == operand.find_first_of(")")) return WRONG;
		
				// (Below, if wrong) Invalid INDEXED OPERAND
				// (Closing bracket appears before opening bracket)
				if(operand.find_first_of("(") > operand.find_first_of(")")) return WRONG; 

				// Obtains x from x(Rn)
				while(operand[0] != '(')
				{
					temp_indexed += operand[0]; // temp_indexed is the x in x(Rn)
					operand.erase(0,1);
				}

				operand.erase(0,1); // Erases the "("

				// (Below, if wrong) Invalid closing bracket position (Not last character)
				if(operand.find_first_of(")") != operand.length()-1) return WRONG;
				operand.pop_back(); // Removes last character of the string, ")"

				// temp_indexed is the x in x(Rn)
				// operand is now the Rn in x(Rn)
				// Now check validity of both

				// Check validity of X in X(Rn)
				se_ptr = get_symbol(temp_indexed);
				// If the symbol is not in the symbol table, and is a valid symbol
				// add it as a forward reference	
				if(se_ptr == NULL)
				{
					if(valid_symbol(temp_indexed))
					{
						add_symbol(temp_indexed, -1, UNKNOWN);
						value0 = -1;
					}
					else return WRONG; // Symbol is not in symbol table, and is also invalid
				}
				else if(se_ptr->type == REG) return WRONG; // X in x(Rn) cannot be a register
				else // X is KNOWN or UNKNOWN, which is valid
				{
					value0 = se_ptr->value;
				}
				// Else KNOWN or UNKNOWN, therefore set value0 to se_ptr->value

				// Check validity of Rn in x(Rn)
				se_ptr = get_symbol(operand);
				if(se_ptr != NULL)
				{
					if(se_ptr->type == REG) value1 = se_ptr->value;
					else return WRONG; // Rn in x(Rn) must be REG type
				}
				else return WRONG; // Therefore it must be REG type

				return INDEXED;		
			}
			else if(get_symbol(operand) == NULL && valid_symbol(operand))
			{
				add_symbol(operand, -1, UNKNOWN);
				value0 = -1;
				// Value0 cannot be larger than a word
				if(value0 < MINWORD || value0 > MAXWORD) return WRONG;
				else return RELATIVE;
			}
			else if(get_symbol(operand) != NULL)
			{ 
				// Therefore the return is either register direct or relative
				se_ptr = get_symbol(operand);
				value0 = se_ptr->value;
				return (se_ptr->type == REG) ? REG_DIRECT : RELATIVE;
			}
			break;
	}		
	// If the operand was none of the modes above, it is clearly an error.
	return WRONG;
}

