##
# XRootGif
#
CC= gcc
CFLAGS = -Wall -lX11 -lgif -lImlib2 -zrelro -znow
ALLCFLAGS = $(CFLAGS) $(shell echo | gcc -xc -E -v - 2>&1 | grep -E '^\s' | sed '1d;s/^\s/ -I/' | tr '\n' ' ') # Explictly include system libraries for cdb
OBJ= xrootgif.o globals.o output.o sample.o gif.o pixmap_allocate.o

pkgname=xrootgif

xrootgif: $(OBJ)
	$(CC) $(ALLCFLAGS) $(OBJ) -o xrootgif

run: xrootgif
	./xrootgif $(ARGS)

debug: clean debug_flag xrootgif

debug_flag:
	$(eval ALLCFLAGS += -g)

install: xrootgif
	install -D -m755 xrootgif $(prefix)/usr/bin/xrootgif
	install -D -m644 LICENSE.txt $(prefix)/usr/share/licenses/$(pkgname)/LICENSE.txt

clean:
	rm -f $(OBJ) xrootgif

%.o: %.c
	$(CC) $(ALLCFLAGS) -c $< -o $@

.PHONY: xrootgif run clean debug debug_flag install

# end
