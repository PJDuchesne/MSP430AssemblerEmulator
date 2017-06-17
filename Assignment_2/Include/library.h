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

-> Name:  library.h
-> Brief: library file for enumeration declarations and other constants
-> Date: June 6, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#ifndef LIBRARY_H
#define LIBRARY_H

// Used for lower and upper bounds during checks (DELETE IF NOT USED)
#define MIN_BYTE -128
#define MAX_BYTE  256
#define MIN_WORD -32768
#define MAX_WORD  65536
#define MAX_MEM_SIZE  65536

// Global fine types for input and output across all files
extern std::ifstream fin;
extern std::ofstream outfile;
extern char mem_array[MAX_MEM_SIZE];
extern unsigned short s9_addr;

void load_file();
void load_mem();
void dump_mem();

void bus();

enum BUS_CTRL {
	READ_W,
	READ_B,
	WRITE_W,
	WRITE_B
};

#endif
