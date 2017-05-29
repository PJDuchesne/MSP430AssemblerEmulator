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

#define SREC_MAX_DATA_SIZE 32 // 32 bytes of data, 64 hex characters

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
		unsigned short count = 0;
		count = srec_index + 3;

		// STARTING STUFF
		// 5301 = S1

		count & 0xf; // Count should never be above 255 anyway, but just in case

		srec_file << "5301\t" << std::right << std::setfill('0') << std::setw(2) << std::hex << count << "\t"
			<< std::right << std::setfill('0') << std::setw(4) << std::hex << srec_address << "\t";

		// DATA
		for(int i = 0; i < srec_index; i++)
		{
			srec_file << std::right << std::setfill('0') << std::setw(2) << std::hex << srec_buffer[i] << " ";
		}

		// CHECKSUM
		srec_chksum += count;

		srec_chksum = (~srec_chksum) & 0xff;

		srec_file << "\t || CHECKSUM >>" << std::right << std::setfill('0') << std::setw(1) << std::hex << srec_chksum << "<<" << std::endl; // END THE LINE HERE

		if(first_srec_address == -1) 
		{
			first_srec_address = srec_address; // Store first srec_address emitted
			test_cnt++;
		}

		// This may be overwritten if the new Srec is initialized by a directive that moves the LC
		srec_address += srec_index/2 + srec_index%2; // Increases the "LC" of the srec_address by the number of stored bytes divided by two to get the number of words. If this value is even it is aligned and assumed the last value is lost.

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
	srec_file << "5309\t" << std::setfill('0') << std::setw(4) << std::hex << first_srec_address; // Assuming I want to print out first srec address
	srec_file.close();
}

