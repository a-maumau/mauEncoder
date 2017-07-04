CC = gcc
CFLAGS = -O2
SRC = src/mauEncoder.c
DST = bin/mauEncoder
all :
	$(shell mkdir -p bin)
	$(CC) -o $(DST) $(SRC) $(CFLAGS)