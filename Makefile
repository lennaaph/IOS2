# Makefile - 2. projekt IOS
# VUT FIT 2023
# Phamová Thu Tra, xphamo00
CC=gcc
CFLAGS= -std=gnu99 -Wall -Wextra -Werror -pedantic -lpthread

proj2: proj2.c
	$(CC) $(CFLAGS) proj2.c -o proj2

clean:
	rm -f proj2
	rm -f proj2.out
zip:
	zip proj2.zip proj2.c Makefile