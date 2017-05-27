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

-> Name:  s19_maker.cpp
-> Brief: Function file for s19_maker.cpp
-> Date: May 26, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <iomanip>

#include "s19_maker.h"

extern std::ofstream srec_file;

#define SREC_MAX_DATA_SIZE 64 // 64 bytes of data, 32 hex characters

unsigned short srec_buffer[SREC_MAX_DATA_SIZE];

int srec_index;
unsigned short srec_chksum;
unsigned int srec_address;

unsigned int first_srec_address = -1;

int test_cnt = 0;

void init_srec(unsigned int address) // Called to start each new srec
{
	srec_index = 0;
	srec_chksum = 0;
	srec_address = address;
	srec_chksum += (srec_address >> 8) & 0xff;
	srec_chksum += srec_address & 0xff;
}

// I believe some of these settings stay until at least the end of file, namely "std::hex".
// Do some testing to see which ones do what (After it's working)
void output_srec_buffer()
{
	if(srec_index != 0) // If the buffer is empty, don't print an empty S1. That would be silly.
	{
		unsigned short byte_pair_cnt = 0;
		byte_pair_cnt = (srec_index/2) +srec_index%2 + 3; // srec is in byte pairs, this ensures the pair count always rounds up for the unlikely event of an odd set of bytes

		// STARTING STUFF
		// 5301 = S1
		srec_file << "5301" << std::right << std::setfill('0') << std::setw(2) << std::hex << byte_pair_cnt 
		<< std::right << std::setfill('0') << std::setw(4) << std::hex << srec_address <<"\t";

		// DATA
		for(int i = 0; i < srec_index; i++)
		{
			srec_file << std::right << std::setfill('0') << std::setw(2) << std::hex << srec_buffer[i];
		}

		// CHECKSUM
		srec_chksum += byte_pair_cnt;

		srec_chksum = (~srec_chksum) & 0xff;

		srec_file << std::right << std::setfill('0') << std::setw(2) << std::hex << srec_chksum << std::endl; // END THE LINE HERE

		if(first_srec_address == -1) 
		{
			first_srec_address = srec_address; // Store first srec_address emitted
			test_cnt++;
		}

		// This may be overwritten if the new Srec is initialized by a directive that moves the LC
		srec_address += srec_index;

		init_srec(srec_address); // This may be overwritten if another emit() is called before the first byte is added to the buffer
	}
}

void write_srec_byte(unsigned char byte)
{
	if(srec_index == SREC_MAX_DATA_SIZE)
	{
		output_srec_buffer();
	}
	else if(srec_index > SREC_MAX_DATA_SIZE)
	{
		std::cout << "THIS SHOULD NEVER HAPPEN (Write SREC BYTE, BUFFER OVERFILLED)" << std::endl;
	}

	srec_buffer[srec_index++] = byte;
	srec_chksum += byte;
}

void write_srec_word(unsigned short word)
{
	write_srec_byte((unsigned char)(word&0xff));			// Send in LSB first (Due to MSB-430 being Little-Endian)
	write_srec_byte((unsigned char)((word >> 8)&0xff));		// Send in MSB second
}

void write_S9()
{
		srec_file << "5309" << std::setfill('0') << std::setw(4) << std::hex << first_srec_address; // Assuming I want to print out first srec address
 		srec_file.close();
}

