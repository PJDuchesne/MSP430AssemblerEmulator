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

-> Name:  main.cpp
-> Brief: Implementation for the main.cpp the code that runs executive commands
-> Date: June 6, 2017    (Created)
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

#include "Include/main.h"
#include "Include/library.h"
#include "Include/emulate.h"
#include "Include/devices.h"

#define DEVICE_MAX 16
#define SIMULATED_INTERRUPT_MAX 500

// Globals
std::ifstream fin;
std::ifstream dev_fin;
std::ofstream outfile;

uint16_t s9_addr;

uint8_t mem_array[MAX_MEM_SIZE];

device devices[DEVICE_MAX];
interrupt interrupts[SIMULATED_INTERRUPT_MAX];

int main(int argc, char *argv[]) {

    /* TESTING PLACE */



    /* TESTING ENDS HERE */

    bool debug_mode = true;

    bool hex_flag = false;

    std::string input_temp = "";

    uint16_t temp_length = 0;

    uint16_t stoi_temp = 0;

    std::string menuInput = "";

    // PC init position which is updated from places in menu
    uint16_t PC_init = 0;

    while (1) {
        std::cout << "\nMAIN MENU: Please enter command from below\n"
        << "\t(P) Load from previous session\n"
        << "\t(E) Emulate current memory\n"
        << "\t(S) Input PC Start Location\n"
        << "\t(F) Load from file\n"
        << "\t(D) Toggle Debugger Mode (Currently "
                                << (debug_mode ? "On)\n" : "Off)\n")
        << "\t(I) System Info\n"
        << "\t(M) Memory Print Location (For diagnostics)\n"
        << "\t(Q) Quit\n\n"

        << "\tInput: >> ";

        std::getline(std::cin, menuInput);

        std::cout << "MENU INPUT IS >>" << menuInput << "<<" << std::endl << std::endl;

        switch (menuInput[0]) {
            case 'P':   // Load from previous session
            case 'p':
                // Load from mem.txt

                break;

            case 'F':   // Load from file
            case 'f':
                std::cout << "Please enter the filepath of the s19 record to input from" << std::endl;
                //std::getline(std::cin, menuInput);

                //fin.open(menuInput);
                fin.open("../Assignment_1/srec_output.s19");        // TEMPORARILY HARDCODED TO SPEED UP TESTING
                PC_init = load_s19();

                break;

            case 'E':   // Emulate program given current starting location
            case 'e':
                dev_fin.open("devices.txt");

                load_devices();

                if (!emulate(mem_array, debug_mode, PC_init)) {
                    std::cout << "EMULATION ERROR" << std::endl;
                }


                break;

            case 'S':   // Select start location (Maybe put parsing into separate parse function)
            case 's':
                std::cout << "Please enter a 16 bit start location in hex (0xnnnn) or decimal (nnnnn)" << std::endl;
                std::getline(std::cin, input_temp);

                temp_length = input_temp.length();

                if ((temp_length > 3)) {
                    if (input_temp[0] == '0' && input_temp[1] == 'x') {
                        hex_flag = true;
                        input_temp = input_temp.erase(0,2);  // Erase the '0x' from the string
                        temp_length -= 2;
                    }
                }
                else hex_flag = false;

                // Remove preceding 0s
                while (input_temp[0] == '0' && temp_length > 1) {
                    temp_length--;
                    input_temp.erase(0, 1);
                }

                // Ensure the value is within a range of uint16_t
                if (temp_length <= (hex_flag ? 4 : 5)) {
                    if (input_temp.find_first_not_of(hex_flag ? "0123456789abcdefABCDEF" : "0123456789") == std::string::npos) {
                        PC_init = std::stoi(input_temp, nullptr, hex_flag ? 16 : 10);
                        if(hex_flag) std::cout << "PC Init updated to 0x" << std::hex << PC_init << " (Hex)" << std::endl << std::dec;
                                else std::cout << "PC Init updated to " << PC_init << " (Dec)" << std::endl;
                    }
                    else std::cout << "Invalid characters found based on input type, PC init not updated" << std::endl;
                } else std::cout << "Input string is too long for input type, PC init not updated" << std::endl;

                break;

            case 'D':   // Debugger mode
            case 'd':
                debug_mode = !debug_mode;
        std::cout << "\tDebugger mode is now " << (debug_mode ? "ON" : "OFF") << std::endl;
                break;

            case 'I':   // System info (Memory, starting point, etc.)
            case 'i':
                std::cout << "System Info:" << std::endl
                          << "\tPC Init = 0x" << std::hex << PC_init << std::endl; // Add more entries here

                break;

        case 'M':  // Test memory location for number (Used to debug)
        case 'm':  // Simply prints out memory location inputted
        std::cout << "Input Memory location in hex to print out" << std::endl;

        std::getline(std::cin, input_temp);


                temp_length = input_temp.length();

        // Remove preceding 0s
                while (input_temp[0] == '0' && temp_length > 1) {
                    temp_length--;
                    input_temp.erase(0, 1);
                }
                // Ensure the value is within a range of uint16_t
                if (temp_length <= 4) {
                    if (input_temp.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos) {
            stoi_temp = std::stoi(input_temp, nullptr, 16);
            std::cout << "MEM[" << std::hex << stoi_temp << "] contains >>" << static_cast<uint16_t>(mem_array[stoi_temp]) << "<<" << std::dec << std::endl;
                    }
                    else std::cout << "Invalid characters found, could not print memory" << std::endl;
                } else std::cout << "Input string is too long for hex (Max is 4 characters), could not print memory" << std::endl;
        break;

            case 'Q':   // Quit
            case 'q':
                system("aafire");
                exit(0);
                break;
    }
}

}

