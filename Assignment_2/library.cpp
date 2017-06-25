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

// Returns true if a S9 is found with a start address, false if no S9 is found

// Loads the file currently stored in fin (a global variable)
uint16_t load_file() {
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
        if (current_record.substr(0, 2) == "S9" && current_record.length() == 10) {
            address = std::stoi(current_record.substr(4, 4), nullptr, 16);
            return address;
        }
        if (fin.eof()) break;  // This is the only break

        // Get CNT and address, use stoi to convert from hex str to a number
        cnt = std::stoi(current_record.substr(2, 2), nullptr, 16);
        address = std::stoi(current_record.substr(4, 4), nullptr, 16);
        s1_pos = 0;

        calc_chksum  = cnt;                     // This also resets calc_chksum
        calc_chksum += address & 0x00ff;        // Least significant 8 bits
        calc_chksum += (address & 0xff00)>>8;   // Most significant 8 bits

        while (s1_pos < cnt-3) {  // Count includes the address and count itself
            // Grab data at this point

            temp_byte = std::stoi(current_record.substr((8+s1_pos*2), 2), nullptr, 16);

            // Emit to memory and increment s1_pos
            mem_array[address+s1_pos++] = temp_byte;

            // Update to checksum

            calc_chksum += temp_byte;
        }

        s1_chksum = std::stoi(current_record.substr((8+s1_pos*2), 2), nullptr, 16);

        // Check checksum!
        if ((s1_chksum + calc_chksum) != 0xff) {
            std::cout << "ERROR ON CHECKSUM >>" << std::hex
                    << (uint16_t)calc_chksum << "<< (< CALC) VS (GIVEN >) >>"
                    << (uint16_t)s1_chksum << "<<" << std::endl;
            getchar();
            return 0;
        }  // Else checksum is correct
    }
}

// Dumps contents of memory into the output memory for diagnostics
void dump_mem() {
    // Set options for output stream
    outfile << std::right << std::setfill('0') << std::hex;
    // Output to file
    for (int i = 0; i < MAX_MEM_SIZE; i++) outfile << std::setw(4) << i << "\t"
                        << std::setw(2) << (uint16_t)mem_array[i] << std::endl;
}


