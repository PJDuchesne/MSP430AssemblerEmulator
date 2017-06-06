#!/bin/bash

# Add new .cpp files here

if [ -a main ]
	then
		rm main
fi

clear && g++ -std=c++11 -o main main.cpp loader.cpp

./main Test_files/two_sp.s19 

