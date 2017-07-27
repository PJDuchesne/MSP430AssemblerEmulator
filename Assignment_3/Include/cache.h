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

-> Name:  cache.h
-> Brief: header file for cache code
-> Date: July 3, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

// Magic Numbers
#define DIR_MAP_MASK 0x003E
#define CACHE_SIZE 32
#define MAX_DEVICE_MEM 31
#define LOWEST_BIT_MASK 0xFFFE
#define BYTE_SIZE 8

// Algorithms
#define DIRECT_MAP  0
#define ASSOCIATIVE 1

// Writing Policies
#define WRITE_BACK    0
#define WRITE_THROUGH 1

// Used in direct mapping to extract bits 1 through 5 (not 0-4)
// Bitshifts result to have a number from 0-31
#define dir_map(addr) ((addr&DIR_MAP_MASK)>>1)

#ifndef CACHE_H_
#define CACHE_H_

void cache(uint16_t mar, uint16_t &mdr, BUS_CTRL ctrl);

/* 
      BRIEF: Cache lines used for cache array, certain parts
             are hash defined to only exist within modes that
             actually require them
*/
struct cache_line {
    int16_t addr = -1;  // This is signed to allow -1 checks
        union {
            struct {
                uint8_t data_L;  // Low data  (EVEN MEM ADDR)
                uint8_t data_H;  // High Data (ODD MEM ADDR)
            };
            uint16_t data;
        };
    #if ALGORITHM == ASSOCIATIVE
    uint8_t age = 0;
    #endif
    #if POLICY == WRITE_BACK
    bool db;
    #endif
};

// Note: cache_line struct declared in library.h

#endif

