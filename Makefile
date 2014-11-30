CC=gcc
CFLAGS=

build:
	$(CC) -o contest contest.c $(CFLAGS)

clean:
	rm -f contest contest.o
