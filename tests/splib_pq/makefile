######################################
# makefile for programs and generators
######################################

CCOMP = cc
CFLAGS = -Wall -O4 -fno-stack-protector

all: progs gens

# problem generators
gens: bin/spacyc bin/spgrid bin/sprand

bin/spacyc: src/types_dh.h src/spacyc.c src/random.c
	$(CCOMP) $(CFLAGS) -o bin/spacyc src/spacyc.c

bin/spgrid: src/types_dh.h src/spgrid.c src/random.c
	$(CCOMP) $(CFLAGS) -o bin/spgrid src/spgrid.c

bin/sprand: src/types_dh.h src/sprand.c src/random.c
	$(CCOMP) $(CFLAGS) -o bin/sprand src/sprand.c

# shortest paths programs
progs: bin/acc bin/bf bin/bfp bin/dikb bin/dikba bin/dikbd bin/dikbm bin/dikh\
bin/dikr bin/dikf bin/dikq bin/gor bin/gor1 bin/pape bin/stack bin/thresh bin/two_q

bin/dikh: src/dikh.c src/dikh_run.c src/types_dh.h src/parser_dh.c src/timer.c ../../trace_tools.o
	$(CCOMP) $(CFLAGS) -o bin/dikh src/dikh_run.c ../../trace_tools.o


