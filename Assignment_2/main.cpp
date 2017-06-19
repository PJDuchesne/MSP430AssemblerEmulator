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

    char menuInput;

    // PC init position which is updated from places in menu
    uint16_t PC_init = 0;


    while (1) {
        std::cout << "\nMAIN MENU: Please enter command from below\n"
        << "\t(P) Load from previous session\n"
        << "\t(E) Emulate current memory\n"
        << "\t(F) Load from file\n"
        << "\t(D) Toggle Debugger Mode (Currently "
                                << (debug_mode ? "On)\n" : "Off)\n")
        << "\t(I) System Info\n\n"
        << "\t(Q) Quit\n\n"

        << "\tInput: >> ";

        menuInput = getchar();

        switch (menuInput) {
            case 'P':   // Load from previous session
            case 'p':


                break;

            case 'F':   // Load from file
            case 'f':

                break;

            case 'E':   // Load from file
            case 'e':
                if (!emulate(mem_array, debug_mode, PC_init)) {
                    std::cout << "EMULATION ERROR" << std::endl;
                }


                break;

            case 'D':   // Debugger mode
            case 'd':
                debug_mode = !debug_mode;
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

