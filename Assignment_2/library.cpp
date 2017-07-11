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

-> Name:  library.cpp
-> Brief: Implementation for the library.cpp, a helper library
-> Date: June 6, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include <stdio.h>
#include <inttypes.h>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <iomanip>

#include "Include/library.h"

/*
    Function: load_s19
    Output: <return_value>: This inicates either the starting PC position for emulation,
                taken from the S9 record
    Brief: This program opens the given S19 file and parses the input. The values are stored
                in the proper memory location until either an S9 record is found or the end
                of file is reached.
*/
uint16_t load_s19() {
    std::string current_record = "";
    bool end = false;

    int s1_pos = 0;  // Position of memory location in
    uint8_t cnt = 0;
    uint16_t address = 0;
    uint8_t s1_chksum = 0;
    uint8_t calc_chksum = 0;
    uint8_t temp_byte = 0;

    int i = 0;  // S1 number

    while (1) {
        std::getline(fin, current_record);
        if (current_record.substr(0, 2) == "S9" && current_record.length() == S9_LENGTH) {
            address = std::stoi(current_record.substr(4, 4), nullptr, HEX_BASE);
            return address;
        }
        if (fin.eof()) break;  // This is the only break

        // Get CNT and address, use stoi to convert from hex str to a number
        cnt = std::stoi(current_record.substr(2, 2), nullptr, HEX_BASE);
        address = std::stoi(current_record.substr(4, 4), nullptr, HEX_BASE);
        s1_pos = 0;

        calc_chksum  = cnt;                                 // This also resets calc_chksum
        calc_chksum += address & LOWER_BYTE_MASK;        // Least significant 8 bits
        calc_chksum += (address & UPPER_BYTE_MASK)>>BYTE_WIDTH;   // Most significant 8 bits

        while (s1_pos < cnt-3) {  // Count includes the address and count itself
            // Grab data at this point

            temp_byte = std::stoi(current_record.substr((BYTE_WIDTH+s1_pos*2), 2), nullptr, HEX_BASE);

            // Emit to memory and increment s1_pos
            mem_array[address+s1_pos++] = temp_byte;

            // Update to checksum
            calc_chksum += temp_byte;
        }

        s1_chksum = std::stoi(current_record.substr((BYTE_WIDTH+s1_pos*2), 2), nullptr, HEX_BASE);

        // Check checksum!
        if ((s1_chksum + calc_chksum) != BYTE_MAX) {
            std::cout << "ERROR ON CHECKSUM >>" << std::hex
                    << (uint16_t)calc_chksum << "<< (< CALC) VS (GIVEN >) >>"
                    << (uint16_t)s1_chksum << "<<" << std::endl;
            return 0;
        }  // Else checksum is correct
    }

    fin.close();
}

/*
    Function: dump_mem
    Output: mem.txt (Output File): This file contains all 65536 memory locations and 16 CPU registers
                and their values at the time dump_mem was called
    Brief: This simply opens mem.txt, deleting the previous results, and outputs all the memory
                followed by all the CPU registers. Some space is placed between the two for readability
*/
void dump_mem() {
    // Delete current file
    outfile.open("mem.txt", std::ios::out | std::ios::trunc);

    // Set options for output stream
    outfile << std::right << std::setfill('0') << std::hex;
    // Output to file
    for (int i = 0; i < MAX_MEM_SIZE; i++) outfile << std::setw(4) << i << "\t"
                        << std::setw(2) << (uint16_t)mem_array[i] << std::endl;

    outfile << "\n\n\n";

    for (int i = 0; i < CPU_REG_MAX; i++) outfile << "Regfile [" << std::dec << i << "] " << std::hex << regfile[i] << std::endl;

    outfile.close();
}
