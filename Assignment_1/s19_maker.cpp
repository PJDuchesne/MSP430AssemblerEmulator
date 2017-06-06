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
-> Note: This section in particular is quite similar to
	the example code provided by Dr Hughes
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <iomanip>

#include "Include/s19_maker.h"

#define SREC_MAX_DATA_SIZE 32 // 32 bytes of data, 64 hex characters

extern std::ofstream srec_file;

unsigned short srec_buffer[SREC_MAX_DATA_SIZE];
unsigned short srec_chksum;
unsigned int srec_address;
int srec_index;
int test_cnt = 0;

/*
    Function: init_srec
    Input: Address: The value to initialize the new S1 record with
    Brief: This function is called to start a new S1 record, immediately
		after the srec_buffer is emitted. It resets all global
		variables and adds the address to the checksum.
*/
void init_srec(unsigned int address)
{
	srec_index = 0;
	srec_chksum = 0;
	srec_address = address;
	// Add the checksum first with the LSB and then with the MSB (Order doesn't matter)
	srec_chksum += (srec_address >> 8) & 0xff;
	srec_chksum += srec_address & 0xff;
}

/*
    Function: output_srec_buffer
    Brief: This function is called either by the write_srec_byte() function,
		or by BYTE, WORD, or STRING in the second pass. After being
		called, the contents of the srec_buffer is outputted as a
		complete S1 record with the correct count and address. After
		the data is outputted, the checksum is emitted to finish the
		record as a ones compliment.
*/
void output_srec_buffer()
{
	if(srec_index != 0) // If the buffer is empty, don't print an empty S1. That would be silly.
	{
		unsigned short count = 0;
		count = srec_index + 3; // Plus 3 for the CNT (1) and ADDRESS (2)

		// S1 header, Count, and Address
		srec_file << "S1" << std::right << std::setfill('0') << std::setw(2) << std::hex << count
			<< std::right << std::setfill('0') << std::setw(4) << std::hex << srec_address;

		// DATA
		for(int i = 0; i < srec_index; i++)
		{
			srec_file << std::right << std::setfill('0') << std::setw(2) << std::hex << srec_buffer[i];
		}
		// CHECKSUM
		srec_chksum += count;

		srec_chksum = (~srec_chksum) & 0xff;

		srec_file << std::right << std::setfill('0') << std::setw(2) << std::hex << srec_chksum << std::endl;

		// This may be overwritten if the new Srec is initialized by a directive that moves the LC
		srec_address += srec_index;

		init_srec(srec_address); // This may be overwritten if another emit() is called 
								 // before the first byte is added to the buffer
	}
}

/*
    Function: write_srec_byte
    Input: byte: The byte to add to the srec_buffer
    Brief: Takes an input byte and adds it to the srec_buffer. If the
	buffer is full, the buffer is first emitted before the byte
	is added to the first place in the srec_buffer.
*/
void write_srec_byte(unsigned char byte)
{
	if(srec_index == SREC_MAX_DATA_SIZE)
	{
		output_srec_buffer();
	}
	else if(srec_index > SREC_MAX_DATA_SIZE)
	{
		std::cout << "THIS SHOULD NEVER HAPPEN (Write SREC BYTE, BUFFER OVERFILLED)" << std::endl;
		getchar();
	}

	srec_buffer[srec_index++] = byte & 0xff;
	srec_chksum += byte;
}

/*
    Function: write_srec_word
    Input: byte: The word to add to the srec_buffer
    Brief: Takes an input word and calls write_srec_byte twice. First
	the least significant byte is sent in, and then the most
	significant byte is sent in. This is due to MSB 430 being a
	Little-Endian system.
*/
void write_srec_word(unsigned short word)
{
	write_srec_byte((unsigned char)(word&0xff));			// Send in LSB first
	write_srec_byte((unsigned char)((word >> 8)&0xff));		// Send in MSB second
}

/*
    Function: write_S9
    Brief: This function is called at the end of the file, or by
	the end directive and it serves to add the closing S9 record
	to the srec_file. The srec_file is also closed.
*/
void write_S9(unsigned int s9_srec_address)
{
	// S9 and 03, 03 is the CNT, which is always 3 for the S9 record

	// Emit the previous buffer
	output_srec_buffer();

	// Calculate S9 record checksum
	srec_chksum = 0;
	srec_chksum += (s9_srec_address >> 8) & 0xff;
        srec_chksum += s9_srec_address & 0xff;
        srec_chksum += 0x03; // CNT is always 3 for S9 records
	srec_chksum = (~srec_chksum) & 0xff;

	// Emit the final S9 record and close file
	srec_file << "S903" << std::setfill('0') << std::setw(4) << std::hex << s9_srec_address << std::setw(2) << srec_chksum;
	srec_file.close();
}
