CC 		=	gcc
FLAGS 	=	-Wall -g -std=gnu99 -c

all: queues

queues: 
	$(CC) $(FLAGS) explicit_heap.c -o explicit_2_heap.o
	$(CC) $(FLAGS) -DBRANCH_4 explicit_heap.c -o explicit_4_heap.o
	$(CC) $(FLAGS) -DBRANCH_8 explicit_heap.c -o explicit_8_heap.o
	$(CC) $(FLAGS) -DBRANCH_16 explicit_heap.c -o explicit_16_heap.o
	$(CC) $(FLAGS) fibonacci_heap.c -o fibonacci_heap.o
	$(CC) $(FLAGS) implicit_heap.c -o implicit_2_heap.o
	$(CC) $(FLAGS) -DBRANCH_4 implicit_heap.c -o implicit_4_heap.o
	$(CC) $(FLAGS) -DBRANCH_8 implicit_heap.c -o implicit_8_heap.o
	$(CC) $(FLAGS) -DBRANCH_16 implicit_heap.c -o implicit_16_heap.o
	$(CC) $(FLAGS) pairing_heap.c -o pairing_heap.o
	$(CC) $(FLAGS) quake_heap.c -o quake_heap.o
	$(CC) $(FLAGS) rank_pairing_heap.c -o rank_pairing_heap.o
	$(CC) $(FLAGS) violation_heap.c -o violation_heap.o

