all: test

test: main.cpp serial.o *.hpp
	g++ -o test main.cpp serial.o

serial.o: serial.cpp serial.h
	g++ -Wall -c serial.cpp

run: test
	./test

clean:
	rm -rf test *.o
