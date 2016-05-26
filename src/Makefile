CC=gcc
CFLAGS=-std=c99 -g 
CFLAGS_LIB=$(CFLAGS) -c

all: mstg

mstg: mstg_mp3.o mstg_util.o main.o
	$(CC) mstg_mp3.o mstg_util.o main.o -o mstg

mstg_mp3.o: mstg_mp3.c
	$(CC) $(CFLAGS_LIB) mstg_mp3.c

mstg_util.o: mstg_util.c
	$(CC) $(CFLAGS_LIB) mstg_util.c

main.o: main.c
	$(CC) $(CFLAGS_LIB) main.c

clean:
	rm *.o mstg
