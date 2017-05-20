#!/bin/bash

# Add new .cpp files here

g++ -std=c++11 -o main main.cpp symtbl.cpp inst_dir.cpp parser.cpp

./main
