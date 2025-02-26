C = gcc
CFLAGS = -Wall -Wextra -pedantic

all: rush

rush: rush.c
	$(CC) $(CFLAGS) -o rush rush.c

clean:
	rm -f rush

run: rush
	./rush

.PHONY: all clean run
