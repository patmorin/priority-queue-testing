CC 		=	gcc
FLAGS 	=	-Wall -g -std=gnu99 -c

all: mem-map trace-tools 

mem-map: memory_management.c memory_management.h
	$(CC) $(FLAGS) memory_management.c -o memory_management.o

trace-tools: trace_tools.c trace_tools.h
	$(CC) $(FLAGS) trace_tools.c -o trace_tools.o

