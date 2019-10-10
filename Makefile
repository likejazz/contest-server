CC=gcc
CFLAGS=-I.
DEPS = contest.h
OBJ = contest.o client.o error.o

%.o: %.c $(DEPS)
	$(CC) -c -pthread -o $@ $< $(CFLAGS)

contest: $(OBJ)
	$(CC) -pthread -o $@ $^ $(CFLAGS)

clean:
	rm -f contest contest.o client.o error.o
