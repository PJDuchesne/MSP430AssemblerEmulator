#!/bin/bash

# Add new .cpp files here

cd ~/Desktop/school_repositories/Eolian

if [ -a main ]
	then
		rm main
fi

g++ -std=c++11 -o main main.cpp library.cpp symtbl.cpp inst_dir.cpp parser.cpp first_pass.cpp

./main
