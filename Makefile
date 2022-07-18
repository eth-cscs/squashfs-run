.PHONY: install clean

prefix := /usr/local
exec_prefix := $(prefix)
bindir := $(exec_prefix)/bin

SQUASHFS_RUN_VERSION := $(shell cat VERSION)

CFLAGS ?= -Os -Wall -Wpedantic
LDFLAGS ?= -Wl,--gc-sections,-s
SQUASHFS_RUN_CFLAGS = -std=c99 -DVERSION=\"$(SQUASHFS_RUN_VERSION)\"

RPMBUILD ?= rpmbuild

all: squashfs-run

squashfs-run.o: VERSION

%.o: %.c
	$(CC) $(CFLAGS) $(SQUASHFS_RUN_CFLAGS) -c -o $@ $<

squashfs-run: squashfs-run.o
	$(CC) $< $(LDFLAGS) -o $@

install: squashfs-run
	mkdir -p $(DESTDIR)$(bindir)
	cp -p squashfs-run $(DESTDIR)$(bindir)

rpm: squashfs-run.c VERSION LICENSE Makefile
	./generate-rpm.sh -b $@
	$(RPMBUILD) -bs --define "_topdir $@" $@/SPECS/squashfs-run.spec

clean:
	rm -rf squashfs-run squashfs-run.o rpm
