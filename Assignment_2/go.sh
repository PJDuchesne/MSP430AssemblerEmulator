#!/bin/bash

# Add new .cpp files here

if [ -a executable ]
	then
		rm executable
fi

clear && g++ -std=c++11 -o executable main.cpp library.cpp

# USED WITH DRAG'n'DROP RAPID TESTING
# ./main Test_files/two_sp.s19

# USED WITH MENU
./executable

