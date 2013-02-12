CC 		=	gcc
FLAGS 	=	-Wall -g -std=gnu99 -O4

all: lazy eager dumb trace-tools des-converter

lazy: memory_management_lazy.c memory_management_lazy.h
	$(CC) $(FLAGS) -c memory_management_lazy.c -o memory_management_lazy.o

eager: memory_management_eager.c memory_management_eager.h
	$(CC) $(FLAGS) -c memory_management_eager.c -o memory_management_eager.o

dumb: memory_management_dumb.c memory_management_dumb.h
	$(CC) $(FLAGS) -c memory_management_dumb.c -o memory_management_dumb.o

trace-tools: trace_tools.c trace_tools.h
	$(CC) $(FLAGS) -c trace_tools.c -o trace_tools.o

des-converter: des_converter.c trace_tools.o
	$(CC) $(FLAGS) trace_tools.o des_converter.c -o des_converter
