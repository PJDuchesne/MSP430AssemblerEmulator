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

-> Name:  symtbl.h
-> Brief: Header file for the symtbl table
-> Date: May 15, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#ifndef SYMTBL_H
#define SYMTBL_H

#include "library.h"

struct symtbl_entry {
        std::string label;  // The name given by the user
        int value;          // Value stored in the symbol
        SYMTBLTYPE type;    // REG, KNOWN, or UNKNOWN
        int line;           // Line number label found on (Updated if an UNKNOWN is filled in)
        symtbl_entry *next; // Pointer to next entry on list
};


void init_symtbl();

void add_symbol(std::string label, int value, SYMTBLTYPE type);

symtbl_entry* get_symbol(std::string lbl);

void output_symtbl();

bool valid_symbol(std::string token);

void symtbl_unknown_check();

#endif
