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

-> Name:  README.md
-> Brief: Useful commands and such for the project
-> Date: May 15, 2017	(Created)
-> Author: Paul Duchesne (B00332119)
-> Contact: pl332718@dal.ca
*/

// Build Command: See inside go.sh for what this does

	./go.sh

// Valgrind (used for random memory issues)

valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./main

// General Notes:
-> There are no error messages in parse.cpp, the error count is done in first_pass.cpp


// TO DO:
--> DATA FLOW DIAGRAMS
* First pass
* Second pass
* Parser
* Emitter
* S19 Generator

--> FIRST PASS TESTS
* One operand
* Two operand
* Jump operand
* Directives
* LABELS

