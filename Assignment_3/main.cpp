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

// Globals
std::ifstream fin;
std::ifstream dev_fin;
std::ofstream outfile;
std::ofstream dev_outfile;

uint8_t mem_array[MAX_MEM_SIZE];

// Variable used to keep track of debug mode
bool debug_mode = true;

device devices[MAX_DEVICES];
interrupt interrupts[SIMULATED_INTERRUPT_MAX];

// Local variable
// PC init position, updated within the menu
static uint16_t PC_init = 0;

/*
    Function: Main
    Input:  argc and *argc[]: Command line input number and associated array
    Brief: Contains initilization setup for the emulator, as well as the
            menu for the start the emulator with
*/
int main(int argc, char *argv[]) {
    
    dev_outfile.open("dev_out.txt");
    // dev_outfile.open("dev_out.txt", std::ios::out | std::ios::trunc);

    // Variables used for input parsing
    bool hex_flag = false;
    std::string menuInput = "";

    while (1) {
        std::cout << "\nMAIN MENU: Please enter command from below\n"
        << "\t(E) Emulate current memory\n"
        << "\t(S) Input PC Start Location\n"
        << "\t(F) Load from file\n"
        << "\t(D) Toggle Debugger Mode (Currently "
                                << (debug_mode ? "On)\n" : "Off)\n")
        << "\t(Q) Quit\n\n"

        << "\tInput: >> ";

        std::getline(std::cin, menuInput);

        std::cout << "MENU INPUT IS >>" << menuInput << "<<" << std::endl << std::endl;

        switch (menuInput[0]) {
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

                if (!load_devices()) break;

                if (!emulate(PC_init)) {
                    std::cout << "EMULATION ERROR" << std::endl;
                }
                break;

            case 'S':   // Select start location 
            case 's':
                std::cout << "Please enter a 16 bit start location in hex (0xnnnn) or decimal (nnnnn)" << std::endl;

                update_PC();

                break;

            case 'D':   // Debugger mode
            case 'd':
                debug_mode = !debug_mode;
                std::cout << "\tDebugger mode is now " << (debug_mode ? "ON" : "OFF") << std::endl;
                break;

            case 'Q':   // Quit
            case 'q':
                exit(0);
                break;
        }
    }
}

/*
    Function: update_PC 
    Input: input_temp: The user's inputted value to be processed
    Output: PC_init (Globally): The inputted value converted from characters to hex
    Brief: Converts the given input string using cstring, stoi, and some error checking.
            The output is stored to the global PC_init value for later use
*/
void update_PC() {
    // Temporary variables used to parse the input
    std::string input_temp = "";
    bool hex_flag = false;
    uint16_t temp_length = 0;

    // Get the line with the new PC
    std::getline(std::cin, input_temp);

    // Use str.length() just once, then increment the value
    temp_length = input_temp.length();

    // Ensure the minimum length is met, then remove preceding 0x
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

    // Ensure the value is within a range of uint16_t, then update PC_init to the value from stoi
    if (temp_length <= (hex_flag ? HEX_STOI_RANGE : DEC_STOI_RANGE)) {
        if (input_temp.find_first_not_of(hex_flag ? "0123456789abcdefABCDEF" : "0123456789") == std::string::npos) {
            PC_init = std::stoi(input_temp, nullptr, hex_flag ? HEX_BASE : DEC_BASE);
            if(hex_flag) std::cout << "PC Init updated to 0x" << std::hex << PC_init << " (Hex)" << std::endl << std::dec;
                    else std::cout << "PC Init updated to " << PC_init << " (Dec)" << std::endl;
        }
        else std::cout << "Invalid characters found based on input type, PC init not updated" << std::endl;
    } else std::cout << "Input string is too long for input type, PC init not updated" << std::endl;
}

