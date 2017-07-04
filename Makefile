CC = gcc
CFLAGS = -O2
SRC = src/mauEncoder.c
DST = bin/mauEncoder

DST : $(SRC)
	$(CC) -o $(DST) $(SRC) $(CFLAGS)