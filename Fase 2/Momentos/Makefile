CC = g++
OPENCVLINKS = `pkg-config --cflags --libs opencv`


all: sources headers main 

main: main.cpp rutinas.h rutinas.cpp
	$(CC) -o main main.cpp  $(OPENCVLINKS) 

headers: rutinas.h

sources: rutinas.cpp

run: 
	./main

clean:
	rm main
