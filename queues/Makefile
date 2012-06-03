CC 		=	gcc
FLAGS 	=	-Wall -g -std=c99 -c

all: queues

queues: 
	$(CC) $(FLAGS) explicit_heap.c -o explicit_heap.o
	$(CC) $(FLAGS) fibonacci_heap.c -o fibonacci_heap.o
	$(CC) $(FLAGS) implicit_heap.c -o implicit_heap.o
	$(CC) $(FLAGS) pairing_heap.c -o pairing_heap.o
	$(CC) $(FLAGS) quake_heap.c -o quake_heap.o
	$(CC) $(FLAGS) rank_pairing_heap.c -o rank_pairing_heap.o
	$(CC) $(FLAGS) violation_heap.c -o violation_heap.o

