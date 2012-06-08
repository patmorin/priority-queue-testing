######################################
# makefile for programs and generators
######################################

CCOMP = gcc
CFLAGS = -O4 -DNDEBUG
CPPCOMP = g++

all: noigen ni

noigen: noigen.c random.c
	$(CCOMP) $(CFLAGS) -o noigen noigen.c

ni: ni.c types_ni.h parser_ni.c heap.c heap.h
	$(CCOMP) $(CFLAGS) -DNO_PR -o ni ni.c ../../trace_tools.o

