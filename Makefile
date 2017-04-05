#!/usr/bin/make -f
# Phony targets
all: jurassicpark
clean:
	@rm -f jurassicpark

# Binary target
 CC := gcc
 CFLAGS := -pthread $(shell pkg-config --cflags --libs xpm)
jurassicpark:
	$(CC) -o $@ *.c $(CFLAGS)

.PHONY: all clean
# End of file.
