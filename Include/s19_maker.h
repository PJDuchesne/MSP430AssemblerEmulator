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

-> Name:  s19_maker.h
-> Brief: Header file for s19_maker.cpp
-> Date: May 26, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#ifndef S19_MAKER_H
#define S19_MAKER_H

void init_srec(unsigned int address); // Called to start each new srec

void output_srec_buffer(); // Emits the current buffer of srecords: Includes emitting S1 hex code (0x5331)

void write_srec_byte(unsigned char byte);

void write_srec_word(unsigned short word);

void write_S9();

#endif
