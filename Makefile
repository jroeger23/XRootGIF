##
# XRootImg
#
# @file
# @version 0.1
CC= gcc
CFLAGS = -Wall -lX11 -lgif -lImlib2
ALLCFLAGS = $(CFLAGS) $(shell echo | gcc -xc -E -v - 2>&1 | grep -E '^\s' | sed '1d;s/^\s/ -I/' | tr '\n' ' ') # Explictly include system libraries for cdb
OBJ= xrootimg.o

xrootimg: $(OBJ)
	$(CC) $(ALLCFLAGS) $(OBJ) -o xrootimg

run: xrootimg
	./xrootimg $(ARGS)

debug: clean debug_flag xrootimg

debug_flag:
	$(eval ALLCFLAGS += -g)

clean:
	rm -f $(OBJ) xrootimg

%.o: %.c
	$(CC) $(ALLCFLAGS) -c $< -o $@

.PHONY: xrootimg run clean

# end
