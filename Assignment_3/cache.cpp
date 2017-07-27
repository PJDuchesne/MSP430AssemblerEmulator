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

-> Name:  cache.cpp
-> Brief: Implementation for the cache
-> Date: July 12, 2017    (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#include <stdio.h>
#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <cstdlib>

#include "Include/cache.h"
#include "Include/emulate.h"
#include "Include/library.h"

/*
    Note: Used #if <condition> instead of #ifdef <condition> due to
        the ease of eventually converting it to the preferred C++ standard
        using global constants
*/

//  ALGORITHM:
//  0: Direct Mapping
//  1: Associative
#define ALGORITHM   DIRECT_MAP  // Set to DIRECT_MAP or ASSOCIATIVE

//  POLICY:
//  0: WRITE BACK    (WB)
//  1: WRITE THROUGH (WT)
#define POLICY        WRITE_THROUGH  // Set to WRITE_BACK or WRITE_THROUGH

// Counts to track hit ratio
uint16_t miss_cnt = 0;
uint16_t hit_cnt  = 0;

// Global array of cache lines
cache_line cache_array[CACHE_SIZE];

/* 
    PLAN:
        1) Based on algorithm, determine which cache line is related to the addr
            --> DIR MAP: Use function directly
            --> ASSOCIATIVE: 
                    * Search linearly for addr in current cache
                        * IF found: Update cache_Line and previous age variable
                        * ELSE: Search through cache for oldest value, set cache line and previous age to that line
                    * Update age based on the cache_line and previous_age

        // At this point, we have the cache_line that is related the input addr
        // It either does or doesn't have the correct address
        2) Check if the addr is correct or not
            * IF correct: HIT: Set MDR and return, set DB appropriately
            * ELSE (Incorrect) MISS: Call BUS for the value based on the ctrl type

        --> Please see design report for more details            
*/

// MAR contains memory location to be accessed or stored to
// MDR contains data to be stored or retrieved at that location
// CTRL indicated READ_W, READ_B, WRITE_W, or WRITE_B indication
void cache(uint16_t mar, uint16_t &mdr, BUS_CTRL ctrl) {
    std::cout << "CACHE CALL\n";

    // Check if ALGORITHM and POLICY are valid, break if either is invalid
    #if ALGORITHM > 1
    std::cout << "INVALID ALGORITHM\n"
    exit(1)
    #endif
    #if POLICY > 1
    std::cout << "INVALID POLICY\n"
    exit(1)
    #endif

    // Check if device memory is being accessed
    if (mar <= MAX_DEVICE_MEM) {
        bus(mar, mdr, ctrl);
        return;  // Return here is purely to avoid doing an "else" after this if and then indenting EVERYTHING
    }

    // Temp variable for performing other bus calls
    uint16_t data_temp = -1;

    // Set odd_flag to true if mar is odd
    bool odd_flag = (mar%2);

    // Check if this is a valid access: (NO ODD WORD ACCESS)
    if ((ctrl == READ_W || ctrl == WRITE_W)&&(odd_flag)) {
        std::cout << "INVALID MEMORY ACCESS, CANNOT PERFORM WORD OPERATIONS ON AN ODD ADDRESS\n";
        exit(1);
    }

    // Mask off the lowest bit to get the even address (i.e. 0x2001 --> 0x2000)
    uint16_t mar_even = mar&LOWEST_BIT_MASK;  // LOWEST_BIT_MASK = 0xFFFE

    // Cache_addr reset to MAX at the start 
    uint8_t cache_addr = CACHE_SIZE; // Array goes from 0 to CACHE_SIZE-1

    // Obtain cache address depending on mode, store in 'cache_addr'

    #if ALGORITHM == DIRECT_MAP // Obtain cache line based on hash
    cache_addr = dir_map(mar);  // Takes bits 1 through 5 (Not 0 through 4)
    #else  // ALGORITHM == ASSOCIATIVE -> Obtain cache line based on age

    // Max age found used to determine oldest line
    int8_t previous_age = -1;

    // Check if the cache contains the given address
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache_array[i].addr == mar_even) {
            // If found, save the previous age and the location in the cache
            // Also reset the age to be 0 after incrementing
            previous_age = cache_array[i].age;
            cache_array[i].age = -1;  // This will be incremented to 0 with the rest of the ages
            cache_addr = i;
            break;
        }
    }

    // If the addr is not in the cache
    // Determine cache line to be replaced based on age
    if (cache_addr == CACHE_SIZE) {
        for (int i = 0; i < CACHE_SIZE; i++) {
            if (cache_array[i].age > previous_age) {
                previous_age = cache_array[i].age;
                cache_addr = i;
            }
        }
    }

    // Update age based on cache line determined
    // Used '<=' in order to allow the starting cache (With every age set to 0)
    // To properly increment the other ages
    for (int i = 0; i < CACHE_SIZE; i++) if (cache_array[i].age <= previous_age) cache_array[i].age++;

    // Set age to be replaced to 0 (new youngest)
    cache_array[cache_addr].age = 0;
    
    #endif    
    
    // Check if the value is in the cache
    // Note this is a redundant check if the ASSOCIATIVE algorithm is used
    if (mar_even == cache_array[cache_addr].addr) {  // HIT
        hit_cnt++;
        switch (ctrl) {
            case READ_W:
                mdr = cache_array[cache_addr].data;
                break;

            case READ_B:
                // Read the high or low byte of the cache
                mdr = (odd_flag ? cache_array[cache_addr].data_H : cache_array[cache_addr].data_L);
                break;
                
            case WRITE_W:
                #if POLICY == WRITE_BACK
                // Set dirty bit if writing to cache location
                cache_array[cache_addr].db = true; 
                #else  // POLICY == WRITE_BACK
                bus(mar, mdr, WRITE_W);
                #endif
   
                // Update cache_line
                cache_array[cache_addr].addr = mar; 
                cache_array[cache_addr].data = mdr; 
        
                break;

            case WRITE_B:
                #if POLICY == WRITE_BACK
                // Set dirty bit if writing to cache location
                cache_array[cache_addr].db = true; 
                #else  // POLICY == WRITE_BACK
                bus(mar, mdr, WRITE_B);
                #endif
    
                // Update cache_line
                cache_array[cache_addr].addr = mar_even;

                // Set the value of the new byte
                // The other byte of this location is already set to the correct value
                if (odd_flag) cache_array[cache_addr].data_H = mdr;
                else cache_array[cache_addr].data_L = mdr;

                break;

            default:
                std::cout << "CACHE LINE SWITCH CASE ERROR #1\n";
                exit(1);
        }
        
    }
    else {  // MISS
        miss_cnt++;
        #if POLICY == WRITE_BACK
        // If dirty bit is set, perform a writeback
        if (cache_array[cache_addr].db) {
            // Reset then DB
            cache_array[cache_addr].db = false;

            // Store current mdr to a temp (This could be a READ or WRITE)
            data_temp = mdr;

            // Perform Write_Back
            mdr = cache_array[cache_addr].data;
            bus(cache_array[cache_addr].addr, mdr, WRITE_W);

            // Restore previous mdr
            mdr = data_temp;
        }
        #endif

        // If a miss, read the location into the cache
            // For READ_W, used entirely
            // For READ_B, used half of
            // FOR WRITE_W, not used
            // FOR WRITE_B, used to fill the other half in the cache line

        switch (ctrl) {
            case READ_W:
                // Call bus for entire word
                bus(mar_even, mdr, READ_W);

                // Store in cache line
                cache_array[cache_addr].addr = mar_even;
                cache_array[cache_addr].data = mdr;

                // mdr already set up to return the correct value
                break;
            case READ_B:
                // Call bus for entire word
                bus(mar_even, mdr, READ_W);

                // Store in cache line
                cache_array[cache_addr].addr = mar_even;
                cache_array[cache_addr].data = mdr;

                // set up mdr to return the correct value
                // For byte calls, only return the correct byte
                mdr &= (odd_flag ? 0xFF00 : 0x00FF);
                break;

            case WRITE_W:
                // If write_though, actually write byte back to memory
                // If write-back, set the DB
                #if POLICY == WRITE_THROUGH
                bus(mar, mdr, WRITE_W);
                #else  // POLICY == WRITE_BACK
                cache_array[cache_addr].db = true;
                #endif
                cache_array[cache_addr].data = mdr;
                cache_array[cache_addr].addr = mar_even;
                break;
                
            case WRITE_B:
                // If write_though, actually write byte back to memory
                // If write-back, set the DB
                #if POLICY == WRITE_THROUGH
                bus(mar, mdr, WRITE_B);
                #else  // POLICY == WRITE_BACK
                cache_array[cache_addr].db = true;
                #endif

                // Store the original byte
                data_temp = mdr;

                // Call bus for missing byte
                bus((odd_flag ? mar_even : (mar+1)), mdr, READ_B);

                // Store the fetched byte with the provided byte on the cache line
                cache_array[cache_addr].data = (mdr<<(odd_flag ? BYTE_SIZE : 0) + (data_temp<<(odd_flag ? 0 : BYTE_SIZE)));
                cache_array[cache_addr].addr = mar_even;
                break;

            default:
                std::cout << "CACHE LINE SWITCH CASE ERROR #2\n";
                exit(1);
        }
    }

    // Print out Contents of the Cache and Statistics
    // Only done if debug mode is enabled in the main menu
    if (debug_mode) {
        // Print the contents of the cache
        std::cout << "\n\nPRINTING CONTENTS OF THE CACHE\n";
        for (int i = 0; i < CACHE_SIZE; i++) {
            std::cout << "LINE #" << std::dec << i << ": ADDR >>" << std::hex << cache_array[i].addr << "<< || DATA >>" << cache_array[i].data << "<<";
            #if POLICY == WRITE_BACK
            std::cout << " || DB: >>" << cache_array[i].db << "<<";
            #endif
            #if ALGORITHM == ASSOCIATIVE
            std::cout << std::dec << " || AGE: >>" << (int)cache_array[i].age << "<<";
            #endif
            std::cout << "\n";
        }
        // Determine and print the hit ratio
        std::cout << std::dec << std::endl;
        std::cout << "HIT  COUNT: >>" << hit_cnt << "<<\n";
        std::cout << "MISS COUNT: >>" << miss_cnt << "<<\n";
        float ratio = ((float)hit_cnt)/((float)(miss_cnt+hit_cnt));
        std::cout << "RATIO: >>" << ratio << "<<\n";
    }
}
