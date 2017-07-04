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

-> Name:  double_inst.h
-> Brief: header file for double_inst code
-> Date: July 3, 2017   (Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

#ifndef DOUBLE_INST_H_
#define DOUBLE_INST_H_

void mov();
void add();
void addc();
void subc();
void sub();
void cmp();
void dadd();
void bit();
void bic();
void bis();
void xor_();  // 'xor' is a reserved C++ keyword
void and_();  // 'and' is a reserved C++ keyword

#endif
