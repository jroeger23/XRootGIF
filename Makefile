##
# XRootGif
#
# @version 0.1
CC= gcc
CFLAGS = -Wall -lX11 -lgif -lImlib2 -zrelro -znow
ALLCFLAGS = $(CFLAGS) $(shell echo | gcc -xc -E -v - 2>&1 | grep -E '^\s' | sed '1d;s/^\s/ -I/' | tr '\n' ' ') # Explictly include system libraries for cdb
OBJ= xrootgif.o

xrootgif: $(OBJ)
	$(CC) $(ALLCFLAGS) $(OBJ) -o xrootgif

run: xrootgif
	./xrootgif $(ARGS)

debug: clean debug_flag xrootgif

debug_flag:
	$(eval ALLCFLAGS += -g)

clean:
	rm -f $(OBJ) xrootgif

%.o: %.c
	$(CC) $(ALLCFLAGS) -c $< -o $@

.PHONY: xrootgif run clean debug debug_flag

# end
