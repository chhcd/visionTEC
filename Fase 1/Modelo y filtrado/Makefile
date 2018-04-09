CC = g++
OPENCVLINKS = `pkg-config --cflags --libs opencv`


all: main

main: etapa1.cpp
	$(CC) -o main etapa1.cpp  $(OPENCVLINKS) 

run: 
	./main

clean:
	rm main
