CC 		=	gcc
FLAGS 	=	-Wall -g -std=gnu99 -O4 -c

all: lazy eager dumb trace-tools

lazy: memory_management_lazy.c memory_management_lazy.h
	$(CC) $(FLAGS) memory_management_lazy.c -o memory_management_lazy.o

eager: memory_management_eager.c memory_management_eager.h
	$(CC) $(FLAGS) memory_management_eager.c -o memory_management_eager.o

dumb: memory_management_dumb.c memory_management_dumb.h
	$(CC) $(FLAGS) memory_management_dumb.c -o memory_management_dumb.o

trace-tools: trace_tools.c trace_tools.h
	$(CC) $(FLAGS) trace_tools.c -o trace_tools.o

