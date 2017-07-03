#!/bin/bash

# Add new .cpp files here

if [ -a main ]
	then
		rm main
fi

if [ -a srec_output_old.s19 ]
	then
		rm srec_output_old.s19
fi

mv srec_output.s19 srec_output_old.s19

clear && g++ -std=c++11 -o main main.cpp library.cpp symtbl.cpp inst_dir.cpp parser.cpp first_pass.cpp second_pass.cpp emitter.cpp s19_maker.cpp

# ./main Test_Files/two.txt

./main test.txt

