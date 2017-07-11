#!/bin/bash

# Add new .cpp files here

if [ -a executable ]
	then
		rm executable
fi

clear && g++ -std=c++11 -o executable main.cpp library.cpp emulate.cpp devices.cpp single_inst.cpp double_inst.cpp debugger.cpp

# USED WITH MENU
./executable

# scan-build -o ./output g++ -std=c++11 -o main main.cpp library.cpp emulate.cpp devices.cpp single_inst.cpp double_inst.cpp debugger.cpp
