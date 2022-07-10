# Makefile CPP

CFLAGS=-DPOLL_TIME=500
INC=
CC=g++

# %: %.cpp
# 	$(CC) $(CFLAGS) $(INC) $< -o$@

$(patsubst %.cpp,%,$(wildcard *.cpp)):
	$(CC) $(CFLAGS) $(INC) $@.cpp -o$@

.PHONY: clean

clean:
	rm -rf rot8cpp

