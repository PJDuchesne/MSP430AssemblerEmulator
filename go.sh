#!/bin/bash

# Add new .cpp files here

if [ -a main ]
	then
		rm main
fi

clear && g++ -std=c++11 -o main main.cpp library.cpp symtbl.cpp inst_dir.cpp parser.cpp first_pass.cpp second_pass.cpp emitter.cpp s19_maker.cpp

./main Example_Code/Tom_Test_Cases/two.txt
