CC 		=	gcc
FLAGS 	=	-Wall -g -std=c99 -c

heaps: 
	$(CC) $(FLAGS) binary_array_heap.c -o binary_array_heap.o
	$(CC) $(FLAGS) -DSTATS binary_array_heap.c -o binary_array_heap_stats.o
	$(CC) $(FLAGS) binary_pointer_heap.c -o binary_pointer_heap.o
	$(CC) $(FLAGS) -DSTATS binary_pointer_heap.c -o binary_pointer_heap_stats.o
	$(CC) $(FLAGS) fibonacci_heap.c -o fibonacci_heap.o
	$(CC) $(FLAGS) -DSTATS fibonacci_heap.c -o fibonacci_heap_stats.o
	$(CC) $(FLAGS) heap_common.c -o heap_common.o
	$(CC) $(FLAGS) -DSTATS heap_common.c -o heap_common_stats.o
	$(CC) $(FLAGS) pairing_heap.c -o pairing_heap.o
	$(CC) $(FLAGS) -DSTATS pairing_heap.c -o pairing_heap_stats.o
	$(CC) $(FLAGS) quake_heap.c -o quake_heap.o
	$(CC) $(FLAGS) -DSTATS quake_heap.c -o quake_heap_stats.o
	$(CC) $(FLAGS) rank_pairing_heap.c -o rank_pairing_heap.o
	$(CC) $(FLAGS) -DSTATS rank_pairing_heap.c -o rank_pairing_heap_stats.o
	$(CC) $(FLAGS) violation_heap.c -o violation_heap.o
	$(CC) $(FLAGS) -DSTATS violation_heap.c -o violation_heap_stats.o

