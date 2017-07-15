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

-> Name:  debugger.cpp
-> Brief: Implementation for the debugger.cpp code that performs fetch, decode, and execute
-> Date: July 3, 2017    (Created)
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
#include <csignal>
#include <algorithm>  // Used for ::toupper

#include "Include/debugger.h"
#include "Include/library.h"

/*
    Function: debugger
    Brief: The debugger is used to make changes to the program at runtime. While the
                debugger is active, the program will not continue running. The debugger
                will remain active until the user chooses to end it. In total, the
                debugger has 6 different options, which are explained in detail below
                in the COUT code.
*/
void debugger() {
    std::string input;
    std::string input_temp;
    uint16_t str_len = 0;
    uint16_t hex_loc = 0;
    uint16_t hex_data = 0;
    uint16_t dec_loc = 0;

    std::cout << "\n\tDEBUG MODE ENTERED\n";

    while (1) {
        std::cout << "\n\tDEBUGGER MENU: Please enter command from below\n"
            << "\t\t(Mnnnn)    Read memory location     (Enter location in HEX)\n"
            << "\t\t(Ennnn nn) Emit to memory location  (Enter location and value in HEX)\n"
            << "\t\t(Rnn nnnn) Write CPU Register       (Enter register number in DECIMAL and value in HEX)\n"
            << "\t\t(D)        Dump CPU Registers\n"
            << "\t\t(C)        Continue program by exiting debugging mode\n"
            << "\t\t(Exit)     Exit(2)                  (Normal ctrl-C call, terminates program)\n\n"
            << "\t\t\tNOTE #1: Include all trailing 0s, that just makes the parsing easier\n"
            << "\t\t\tNOTE #2: Memory locations store things LITTLE ENDIAN\n"
            << "\t\t\tNOTE #3: Update PC by reading and changing R00\n\n"
            << "\t\tInput: >> ";

        std::getline(std::cin, input);

        str_len = input.length();

        std::transform(input.begin(), input.end(),input.begin(), ::toupper);

        if (input == "EXIT") { dev_outfile.close(); dump_mem(); exit(2); }

        if (input.find_first_not_of("0123456789abcdefABCDEF MERWC") != std::string::npos) {
            std::cout << "\n\tPlease enter a valid input\n";
            continue;
        }

        if (str_len >= 1) {
            switch (input[0]) {
                case 'M':
                    if (str_len != 5) {
                        std::cout << "\n\tPlease enter a valid input (Wrong length on M)\n";
                        continue;
                    }

                    // Parse location to read
                    input_temp = input.substr(1, 4);
                    hex_loc = std::stoi(input_temp, nullptr, HEX_BASE);

                    std::cout << std::hex << "\n\tMemory Location: >>" << hex_loc << "<< contains >>" << (uint16_t)mem_array[hex_loc] << std::dec << "<<\n";

                    break;
                case 'E':
                    if (str_len != 8) {
                        std::cout << "\n\tPlease enter a valid input (Wrong length on E)\n";
                        continue;
                    }

                    // Parse location to read
                    input_temp = input.substr(1, 4);
                    hex_loc = std::stoi(input_temp, nullptr, HEX_BASE);

                    // Parse the data to store
                    input_temp = input.substr(6, 2);
                    hex_data = std::stoi(input_temp, nullptr, HEX_BASE);

                    mem_array[hex_loc] = hex_data;

                    std::cout << std::hex << "\n\tMemory Location: >>" << hex_loc << "<< updated to >>" << (uint16_t)mem_array[hex_loc] << std::dec << "<<\n";
                    break;
                case 'R':  // Format: 'Rnn nnnn', total of 8 characters
                    // Check if the right number of characters are entered
                    if (str_len != 8) {
                        std::cout << "\n\tPlease enter a valid input (Wrong length on R)\n";
                        continue;
                    }

                    // Parse location to read
                    input_temp = input.substr(1, 2);
                    dec_loc = std::stoi(input_temp, nullptr, DEC_BASE);

                    // Parse the data to store
                    input_temp = input.substr(4, 4);
                    hex_data = std::stoi(input_temp, nullptr, HEX_BASE);

                    // Store data to location
                    regfile[dec_loc] = hex_data;

                    std::cout << "\n\tRegister Number: " << std::dec << dec_loc << " updated to >>" << std::hex << (uint16_t)regfile[dec_loc] << std::dec << "<<\n";

                    break;
                case 'D':
                    std::cout << std::endl;
                    for (int i = 0; i < 16; i++) {
                        std::cout << "Regfile [" << std::dec << i << "]\t" << std::hex << regfile[i] << std::endl;
                    }

                    break;
                case 'C':
                    return;  // Returns from debugger, returning control to the code
                    break;
                default:
                    std::cout << "\n\tPlease enter a valid input\n";
                    break;
            }
        }
        else std::cout << "\n\tPlease enter a valid input\n";
    }
}
