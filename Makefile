.PHONY: install clean

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin

CFLAGS ?= -Os -Wall -Wpedantic
LDFLAGS ?= -Wl,--gc-sections,-s
SQUASHFS_RUN = -std=c99

all: squashfs-run

%.o: %.c
	$(CC) $(CFLAGS) $(SQUASHFS_RUN) -c -o $@ $<

squashfs-run: squashfs-run.o
	$(CC) $< $(LDFLAGS) -o $@

install: squashfs-run
	mkdir -p $(DESTDIR)$(bindir)
	cp -p squashfs-run $(DESTDIR)$(bindir)

clean:
	rm -f squashfs-run squashfs-run.o
