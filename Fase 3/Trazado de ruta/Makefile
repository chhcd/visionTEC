CC = g++
OPENCVLINKS = `pkg-config --cflags --libs opencv`


all: headers main

main: main.cpp rutinas.h
	$(CC) -o main main.cpp  $(OPENCVLINKS) 

headers: rutinas.h
	$(CC) rutinas.h  

run: 
	./main

clean:
	rm main
