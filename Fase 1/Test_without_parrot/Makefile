CC = g++
OPENCVLINKS = `pkg-config --cflags --libs opencv`


all: headers main

main: etapa1.cpp
	$(CC) -o main etapa1.cpp  $(OPENCVLINKS) 

headers: rutinasVision.h
	$(CC)  rutinasVision.h 

run: 
	./main

clean:
	rm main
