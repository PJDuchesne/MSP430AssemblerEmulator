# Eolian

// Build Command: 

	./go.sh

// Neat things to do:
-> Remove code repetition in first_pass.cpp by adding an error function that takes a string to output
-> Add error messages in parse.cpp (Also through a function to avoid expanding brackets)

// Valgrind

valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./main
