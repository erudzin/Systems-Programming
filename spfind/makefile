# Name        : makefile
# Author      : Eric Rudzin and Tudor Rus
# Date        : March 30, 2021
# Description : Sorted Permission Finder Makefile
# Pledge      : I pledge my honor that I have abided by the Stevens Honor System.



CC     = gcc
C_FILE = $(wildcard *.c)
TARGET = $(patsubst %.c,%,$(C_FILE))
CFLAGS = -g -Wall -Werror -pedantic-errors

all:
	$(CC) $(CFLAGS) $(C_FILE) -o $(TARGET)
clean:
	rm -f $(TARGET) $(TARGET).exe
