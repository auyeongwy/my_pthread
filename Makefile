CC = gcc
OPT = -O2 -Wall
LIB = -lpthread

all: my_pthread

my_pthread: my_pthread.c worker.o
	$(CC) $(OPT) my_pthread.c -o my_pthread $(LIB) *.o
	
worker.o: worker.h worker.c
	$(CC) $(OPT) worker.c -c
	
clean:
	rm -f *.o
	rm -f my_pthread