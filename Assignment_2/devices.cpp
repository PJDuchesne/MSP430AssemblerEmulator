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

uint16_t interrupt_num = -1;

bool load_devices()
{
    std::cout << "LOADING DEVICES (START)\n";

    std::string current_record;
    std::string token;

    uint16_t line_num = -1;
    uint16_t device_num = -1;

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
    scr_reg *scr;
    for (int i = 0; i < MAX_DEVICES; i++) {
        scr = (scr_reg *)&mem_array[i*2];    
        scr->IO = devices[i].IO;
        scr->DBA = (scr->IO ? 0 : 1);  // Initialize DBA to opposite of IO bit
    }

    // SHOULD END HERE: REST IS DIAGNOSTICS

    // SET IE for ALL DEVICES BY DEFAULT (ASM SHOULD DO THIS)
    for (int i = 0; i < MAX_DEVICES; i++) mem_array[i*2] += 1;

    for (int i = 0; i < MAX_DEVICES; i++) {
        std::cout << "DEVICE #" << i << "\n"
                << "\tStatus Reg: >>" << devices[i].IO << "<<\n"
                << "\tProc Time:  >>" << devices[i].process_time << "<<\n";
    }

    std::cout << std::endl;

    for (int i = 0; i <= interrupt_num; i++) {
        std::cout << "Interrupt #" << i << "\n"
                << "\tTime: >>" << interrupts[i].time << "<<\n"
                << "\tDev:  >>" << interrupts[i].dev << "<<\n"
                << "\tData: >>" << std::hex << interrupts[i].data << std::dec << "<<\n";
    }

    return true;
}

void trigger_interrupt(uint16_t dev_num) {
    // DEBUG
    std::cout << "TRIGGER_INTERRUPT #" << dev_num << std::endl;

    // Push PC to stack
    std::cout << "PUSHING PC AS: " << std::hex << regfile[PC] << std::dec << std::endl;
    regfile[SP] -= 2;
    mdr = regfile[PC];
    bus(regfile[SP], mdr, WRITE_W);

    // Push SR to stack
    std::cout << "PUSHING SR AS: " << std::hex << regfile[SR] << std::dec << std::endl;
    regfile[SP] -= 2;
    mdr = regfile[SR];
    bus(regfile[SP], mdr, WRITE_W);

    // Clear SR to disable interrupts
    regfile[SR] = 0;

    // Put PC to location pointed to by vector table

    bus((VECTOR_BASE + dev_num*2), mdr, READ_W);

    regfile[PC] = mdr;
    std::cout << "PC Updated to ISR at: " << std::hex << mdr << std::dec << std::endl;

    cpu_clock += 6;
}

void update_device_statuses() {
    // Check if the next interrupt should have triggered

    scr_reg *scr;
    uint16_t device_num;

    // DEAL WITH PENDING INPUT INTERRUPTS
    while (interrupts[next_interrupt].time <= cpu_clock) {
        if (next_interrupt > interrupt_num) break;

        // Get the status register of that device
        scr = (scr_reg *)&mem_array[(interrupts[next_interrupt].dev)*2];
    
        // If DBA is already high, an overflow has occurred
        if (scr->DBA) scr->OF = 1;
        else scr->OF = 0;

        // Set the DBA high to signify there is data waiting
        scr->DBA = 1;

        // Set the data register with the interrupt data
        mem_array[(interrupts[next_interrupt].dev)*2 + 1] = interrupts[next_interrupt].data;

        // Iterate to the next interrupt
        next_interrupt++;
    }
    
    // DEAL WITH WITH PENDING WRITES
    for (device_num = 0; device_num < 16; device_num++) {
        // std::cout << "CHECKING PENDING WRITES\n";
        // Output_active is only ever set on output devices, so no need to check if this device is input or output
        if (devices[device_num].output_active && cpu_clock >= devices[device_num].end_time) {
            // Get the status register of that device
            scr = (scr_reg *)&mem_array[device_num*2];

            if (scr->IE) devices[device_num].output_interrupt_pending = true;

            std::cout << "\tCHECKING PENDING WRITES: GOT ONE\n";
            
            // Write to output file:

            dev_outfile << "Device: " << std::dec << device_num << "\n"
                        << "Write Start:  " << (devices[device_num].end_time - devices[device_num].process_time) << "\n"
                        << "Write End:    " << devices[device_num].end_time << "\n"
                        << "Overflow:    " << (uint16_t)scr->OF << "\n"
                        << "Data Written: " << (char)devices[device_num].IO_data << "\n\n";

            // Set DBA to high to signify that the device has finished writing
            scr->DBA = 1;
            devices[device_num].output_active = false;
            devices[device_num].end_time = 0;
        }
    }
}

// Check for interrupts in a priority order

// This is only done if GIE is set (MAYBE? TODO:)
void check_for_interrupts() {
    scr_reg *scr;
    
    if (!temp_GIE_disable) {
        if (sr_union->GIE) {
            for (int i = 0; i < MAX_DEVICES; i++) {
                scr = (scr_reg *)&mem_array[i*2];
                // First bracket:  If an input  device with DBA and IE set
                // Second bracket: If an output device with an output interrupt pending
                if (((scr->DBA && scr->IE && devices[i].IO))||(devices[i].output_interrupt_pending && scr->IE)) {
                    trigger_interrupt(i);
                    // If this was an outPUT interrupt, set the pending status to false
                    devices[i].output_interrupt_pending = false;
                    break;  // Execute interrupt, stop checking the rest
                }  
            }
        }
    }
    else temp_GIE_disable = false;
}

void device_bus(uint16_t mar, uint16_t &mdr, int ctrl) {

    uint16_t device_num = mar/2;  // Will round down on odd bits and still find the correct device
    uint16_t scr_addr = device_num*2;
    scr_reg *scr = (scr_reg *)&mem_array[scr_addr];

    std::cout << "DEVICE BUS: dev: " << device_num << " || mar: " << mar << " || scr_addr: " << scr_addr << "\n";

    if (scr_addr == mar) {  // Therefore is the scr register
        std::cout << "\t\t\t\tTHIS IS A SRC REG ACCESS\n";
        switch (ctrl) {
            case READ_W:
                std::cout << "\t\t\t\tTHIS IS A READ_W\n";
                mdr = mem_array[scr_addr];  // Grab contents of SCR
                break;
            case WRITE_W:
                std::cout << "\t\t\t\tTHIS IS A WRITE_W\n";
                // Write to SCR, but only IE bit is RW enabled. So only update that
                scr->IE = (mdr&0x01);
                break;
            default:  // READ_B and WRITE_B should not be called on devices
                std::cout << "ERROR: READ_B and WRITE_B should not be called on device memory\n";
                break;
        }
    }
    else {  // scr_addr != mar (therefore is the data register)
        std::cout << "\t\t\t\tTHIS IS A DATA REGISTER ACCESS\n";
        if (scr->IO) {  // Device is an INPUT
            std::cout << "\t\t\t\tTHIS IS AN INPUT DEVICE\n";
            if (ctrl == READ_W || ctrl == READ_B) {  // If the user is trying to read
                std::cout << "\t\t\t\tTHIS IS A READ_W/B (scr_addr: " << scr_addr << ", INTERRUPT ACKNOWLEDGED)\n";
                scr->DBA = 0;  // Set DBA low, signifying device is ready to receieve more data
                // Overflow is cleared where overflow is set
                mdr = mem_array[mar];
            }
            // Else: User cannot write to a INPUT device
        }
        else {  // Device is an OUTPUT
            std::cout << "\t\t\t\tTHIS IS AN OUTPUT DEVICE\n";
            if (ctrl == WRITE_W || ctrl == WRITE_B) {
                std::cout << "\t\t\t\tTHIS IS A WRITE\n";
                if (scr->DBA == 0) {
                    scr->OF = 1; 
                }
                else scr->OF = 0;
                
                scr->DBA = 0;  // LOW indicates the device is actively outputting         CHECKED

                mem_array[mar] = mdr;
                
                devices[device_num].output_active = true;
                devices[device_num].IO_data = mdr;
                devices[device_num].end_time = cpu_clock + devices[device_num].process_time;
            }
            else std::cout << "\t\t\t\tTHIS IS A READ (WHICH IS IGNORED)\n";
            // Else: User cannot read an OUTPUT device
        }
    }
}

