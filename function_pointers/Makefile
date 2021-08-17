CC = g++
CFLAGS = -Wall -O3

.PHONY: run

all: function_pointers

run: function_pointers
	./function_pointers 100 1000000
 
function_pointers: function_pointers.o 
	$(CC) $(CFLAGS) -o $@ $<
  
function_pointers.o: function_pointers.cpp
	$(CC) $(CFLAGS) -c $<
 
clean:
	rm function_pointers.o
	rm function_pointers
