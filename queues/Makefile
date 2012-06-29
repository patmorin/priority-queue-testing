CC 		=	gcc
FLAGS 	=	-Wall -pedantic -g -std=gnu99 -c

all: queues

queues: binomial_queue.o explicit_2_heap.o fibonacci_heap.o implicit_2_heap.o \
		pairing_heap.o quake_heap.o rank_pairing_heap.o \
		strict_fibonacci_heap.o violation_heap.o

binomial_queue.o: binomial_queue.c binomial_queue.h
	$(CC) $(FLAGS) binomial_queue.c -o binomial_queue.o

explicit_2_heap.o: explicit_heap.c explicit_heap.h
	$(CC) $(FLAGS) explicit_heap.c -o explicit_2_heap.o
	$(CC) $(FLAGS) -DBRANCH_4 explicit_heap.c -o explicit_4_heap.o
	$(CC) $(FLAGS) -DBRANCH_8 explicit_heap.c -o explicit_8_heap.o
	$(CC) $(FLAGS) -DBRANCH_16 explicit_heap.c -o explicit_16_heap.o

fibonacci_heap.o: fibonacci_heap.c fibonacci_heap.h
	$(CC) $(FLAGS) fibonacci_heap.c -o fibonacci_heap.o

implicit_2_heap.o: implicit_heap.c implicit_heap.h
	$(CC) $(FLAGS) implicit_heap.c -o implicit_2_heap.o
	$(CC) $(FLAGS) -DBRANCH_4 implicit_heap.c -o implicit_4_heap.o
	$(CC) $(FLAGS) -DBRANCH_8 implicit_heap.c -o implicit_8_heap.o
	$(CC) $(FLAGS) -DBRANCH_16 implicit_heap.c -o implicit_16_heap.o

pairing_heap.o: pairing_heap.c pairing_heap.h
	$(CC) $(FLAGS) pairing_heap.c -o pairing_heap.o

quake_heap.o: quake_heap.c quake_heap.h
	$(CC) $(FLAGS) quake_heap.c -o quake_heap.o

rank_pairing_heap.o: rank_pairing_heap.c rank_pairing_heap.h
	$(CC) $(FLAGS) rank_pairing_heap.c -o rank_pairing_heap.o

strict_fibonacci_heap.o: strict_fibonacci_heap.c strict_fibonacci_heap.h
	$(CC) $(FLAGS) strict_fibonacci_heap.c -o strict_fibonacci_heap.o

violation_heap.o: violation_heap.c violation_heap.h
	$(CC) $(FLAGS) violation_heap.c -o violation_heap.o

