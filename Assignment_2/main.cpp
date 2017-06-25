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

#include "Include/main.h"
#include "Include/library.h"
#include "Include/emulate.h"

// Globals
std::ifstream fin;
std::ofstream outfile;

uint16_t s9_addr;

char mem_array[MAX_MEM_SIZE];

int main(int argc, char *argv[]) {
    bool debug_mode = false;

    bool hex_flag = false;

    std::string input_temp = "";

    uint16_t temp_length = 0;

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
        << "\t(Q) Quit\n\n"

        << "\tInput: >> ";

	std::getline(std::cin, menuInput);

	std::cout << "MENU INPUT IS >>" << menuInput << "<<" << std::endl << std::endl;

        switch (menuInput[0]) {
            case 'P':   // Load from previous session
            case 'p':


                break;

            case 'F':   // Load from file
            case 'f':

                break;

            case 'E':   // Emulate program given current starting location
            case 'e':
                if (!emulate(mem_array, debug_mode, PC_init)) {
                    std::cout << "EMULATION ERROR" << std::endl;
                }


                break;

	    case 'S':	// Select start location (Maybe put parsing into separate parse function)
	    case 's':
		std::cout << "Please enter a 16 bit start location in hex (0xnnnn) or decimal (nnnnn)" << std::endl;
		std::getline(std::cin, input_temp);

		temp_length = input_temp.length();

		std::cout << "Input of >>" << input_temp << "<< with length of >>" << temp_length << "<<" << std::endl;

		if ((temp_length > 3)) {
			if (input_temp[0] == '0' && input_temp[1] == 'x') {
				hex_flag = true;
				input_temp = input_temp.erase(0,2);  // Erase the '0x' from the string
				temp_length -= 2;
				std::cout << "(Removed 0x) Input of >>" << input_temp << "<<" << std::endl;
			}
		}
		else hex_flag = false;
	
		// Remove preceding 0s	
		while (input_temp[0] == '0' && temp_length > 1) {
			temp_length--;
			input_temp.erase(0,1);
			std::cout << "(Removed preceding 0) Input of >>" << input_temp << "<<" << std::endl;
		}

		// Ensure the value is within a range of uint16_t

		std::cout << "TEMP LENGTH >>" << temp_length << "<<" << std::endl;

		if (temp_length <= (hex_flag ? 4 : 5)) {
			std::cout << "Number is the correct size" << std::endl;
			if (input_temp.find_first_not_of(hex_flag ? "0123456789abcdefABCDEF" : "0123456789") == std::string::npos) {
				std::cout << "Number is the correct characters" << std::endl;
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

                break;

            case 'Q':   // Quit
            case 'q':
                system("aafire");
                exit(0);
                break;
    }
}

load_file();

outfile.open("mem.txt");

dump_mem();

bool end = true;

while (!end) {
    // Fetch
    // Decode
    // Execute
}

    // For diagnostics

fin.close();
    outfile.close();  // Note: srec_file is closed in the s9 function

    return 0;
}

