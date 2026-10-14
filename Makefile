CC     = gcc
CFLAGS = -Wall -Wextra -g -Ilist

all: serv cli

serv: serv.c list/list.c list/list.h
	$(CC) $(CFLAGS) -o server serv.c list/list.c

cli: cli.c
	$(CC) $(CFLAGS) -o client cli.c

clean:
	rm -f server client
