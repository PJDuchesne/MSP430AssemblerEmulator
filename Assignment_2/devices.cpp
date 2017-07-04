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

-> Name:  emulate.cpp
-> Brief: Implementation for the emulate.cpp code that performs fetch, decode, and execute
-> Date: June 16, 2017    (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

// TO DO: Move instructions to their own separate files

#include <stdio.h>
#include <inttypes.h>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>

#include "Include/devices.h"
#include "Include/library.h"
#include "Include/emulate.h"

#define ASCII_ZERO 48
#define ASCII_ONE 49
#define MAX_DEVICES 16

#define VECTOR_BASE 0xFFC0

bool load_devices()
{
    std::cout << "LOADING DEVICES (START)\n";

    std::string current_record;
    std::string token;

    uint16_t line_num = -1;
    uint16_t device_num = -1;
    uint16_t interrupt_num = -1;

    uint16_t interrupt_check = 0;

    uint16_t temp_decimal = 0;

    bool end_flag = false;

    bool dev = true;  // TRUE = Devices section, FALSE = Interrupts section

    char* temp_crecord = new char[80];  // Max input length of 80 characters for a line, punch card width

    while(!dev_fin.eof() && !end_flag) {
        std::getline(dev_fin, current_record);
        line_num++;

        if (current_record == "") continue;  // If empty line, skip to next record

        std::cout << "\tRECORD #" << line_num << " >>" << current_record << "<<\n";

        std::strcpy(temp_crecord, current_record.c_str());

        char* temp_ctoken = std::strtok(temp_crecord, " \t\n");

        token.assign(temp_ctoken, strlen(temp_ctoken));

        if (temp_ctoken == NULL) continue;  // If no valid token, skip to next record

        std::cout << "\t\tFIRST  TOKEN: >>" << token << "<<\n";

        if (dev) {  // Reading devices first
            if (token == "INTERRUPTS") dev = false;
            else {
                device_num++;

                if (device_num >= MAX_DEVICES) {
                    std::cout << "\t\tINVALID DEVICE FILE (Line #" << line_num << " || Too many devices)\n";
                    return false;
                }

                std::cout << "\t\tTOKEN[0]: " << token[0] << "\n";

                // Ensure the input is either 0 or 1 (48 or 49 in ASCII)
                if (!(token[0] == ASCII_ZERO || token[0] == ASCII_ONE)) {
                    std::cout << "\t\tINVALID DEVICE FILE (Line #" << line_num << " || Invalid I/O token)\n";
                    return false;
                }

                devices[device_num].IO = (token[0] - ASCII_ZERO);  // Convert token from ASCII to decimal (Subtract 48)

                temp_ctoken = strtok(NULL, " \t\n");

                if (temp_ctoken != NULL) token.assign(temp_ctoken, strlen(temp_ctoken));
                else {
                    std::cout << "\t\tINVALID DEVICE FILE (Line #" << line_num << ")\n";
                    return false;
                }

                std::cout << "\t\tSECOND TOKEN: >>" << token << "<<\n";

                if (token.find_first_not_of("0123456789") != std::string::npos) {
                    std::cout << "\t\tINVALID DEVICE FILE (Line #" << line_num << " || Invalid processing time token)\n";
                    return false;
                }

                devices[device_num].process_time = std::stoi(token, nullptr, 10);
            }
        }
        else {  // Reading interrupts second
            if (token == "END") end_flag = true;
            else {
                interrupt_num++;

                if (device_num >= 500) {
                    std::cout << "\t\tINVALID DEVICE FILE (Line #" << line_num << " || Too many interrupts)\n";
                    return false;
                }

                if (token.find_first_not_of("0123456789") != std::string::npos) {
                    std::cout << "\t\tINVALID DEVICE FILE (Line #" << line_num << " || Invalid interrupt time token)\n";
                    return false;
                }

                interrupts[interrupt_num].time = std::stoi(token, nullptr, 10);

                if (interrupt_check > interrupts[interrupt_num].time) {
                    std::cout << "\t\tINVALID DEVICE FILE (Line #" << line_num << " || Invalid interrupt time token, must be in ascending order)\n";
                    return false;
                }

                interrupt_check = interrupts[interrupt_num].time;

                temp_ctoken = strtok(NULL, " \t\n");

                if (temp_ctoken != NULL) token.assign(temp_ctoken, strlen(temp_ctoken));
                else {
                    std::cout << "\t\tINVALID DEVICE FILE (Line #" << line_num << ")\n";
                    return false;
                }

                std::cout << "\t\tSECOND TOKEN: >>" << token << "<<\n";

                if (token.find_first_not_of("0123456789") != std::string::npos) {
                    std::cout << "\t\tINVALID DEVICE FILE (Line #" << line_num << " || Invalid processing time token)\n";
                    return false;
                }

                interrupts[interrupt_num].dev = std::stoi(token, nullptr, 10);

                if (interrupts[interrupt_num].dev > 15) {
                    std::cout << "\t\tINVALID DEVICE FILE (Line #" << line_num << " || Interrupt device number too high)\n";
                    return false;
                }

                temp_ctoken = strtok(NULL, " \t\n");

                if (temp_ctoken != NULL) token.assign(temp_ctoken, strlen(temp_ctoken));
                else {
                    std::cout << "\t\tINVALID DEVICE FILE (Line #" << line_num << ")\n";
                    return false;
                }

                std::cout << "\t\tTHIRD  TOKEN: >>" << token << "<<\n";

                if (token.length() != 1) {
                    std::cout << "\t\tINVALID DEVICE FILE (Line #" << line_num << " || Invalid interrupt data)\n";
                    return false;
                }
                interrupts[interrupt_num].data = token[0];
            }
        }
    }

    delete[] temp_crecord;
    std::cout << "LOADING DEVICES (END)\n";

    // STORE THE DEVICES IN MEMORY

    for (int i = 0; i < MAX_DEVICES; i++) {
        mem_array[i*2] = (devices[i].IO)<<1;
    }

    // SHOULD END HERE: REST IS DIAGNOSTICS

    // SET IE for ALL DEVICES

    for (int i = 0; i < MAX_DEVICES; i++) mem_array[i*2] += 1;

    std::cout << std::dec << "TROUBLESHOOTING PRINTOUT\n\n";

    for (int i = 0; i < MAX_DEVICES; i++) {
        std::cout << "DEVICE #" << i << "\n"
                << "\tStatus Reg: >>" << devices[i].IO << "<<\n"
                << "\tProc Time:  >>" << devices[i].process_time << "<<\n";
    }

    std::cout << std::endl;

    for (int i = 0; i <= interrupt_num; i++)
    {
        std::cout << "Interrupt #" << i << "\n"
                << "\tTime: >>" << interrupts[i].time << "<<\n"
                << "\tDev:  >>" << interrupts[i].dev << "<<\n"
                << "\tData: >>" << std::hex << interrupts[i].data << std::dec << "<<\n";
    }

    dump_mem();
}

void trigger_interrupt(uint16_t dev_num) {
    // Push SR to stack
    regfile[SP] -= 2;
    mdr = regfile[SR];
    bus(regfile[SP], mdr, WRITE_W);

    // Push PC to stack
    regfile[SP] -= 2;
    mdr = regfile[PC];
    bus(regfile[SP], mdr, WRITE_W);

    // Clear SR to disable interrupts
    regfile[SR] = 0;

    // Put PC to location pointed to by vector table
    regfile[PC] = mem_array[VECTOR_BASE + dev_num*2];
}

void check_interrupts() {
    // Check if the next interrupt should have triggered

    // Finish any pending interrupts

    // 
}
