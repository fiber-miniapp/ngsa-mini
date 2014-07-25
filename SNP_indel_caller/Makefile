#
#	File:	Makefile
#	Copyright(C) 2012-2013 RIKEN, Japan.
#
TARGET=snp

SRCS=snpmain.c indel_SNP.c
HDR=snp.h
OBJS=$(subst .c,.o,$(SRCS))

CC=gcc
CFLAGS=-std=c99 -O2 -g
LDLIBS=-lm

$(TARGET): $(OBJS)
	$(LINK.o) $^ $(CFLAGS) $(LOADLIBS) -o $@ $(LDLIBS)

clean:
	rm -f $(TARGET) *.o
