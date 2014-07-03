CC=gcc
CFLAGS=-pthread -Wno-pointer-sign

build:
	$(CC) -o contest contest.c $(CFLAGS)

clean:
	rm -f contest contest.o
