CC 		=	gcc
FLAGS 	=	-Wall -g -std=c99

drivers: 
	$(CC) $(FLAGS) -DUSE_BINARY_ARRAY testdriver.c ../queues/binary_array_heap.o -o driver_binary_array
	$(CC) $(FLAGS) -DSTATS -DUSE_BINARY_ARRAY testdriver.c ../queues/binary_array_heap_stats.o ../queues/heap_common.o -o driver_binary_array_stats
	$(CC) $(FLAGS) -DUSE_BINARY_POINTER testdriver.c ../queues/binary_pointer_heap.o -o driver_binary_pointer
	$(CC) $(FLAGS) -DSTATS -DUSE_BINARY_POINTER testdriver.c ../queues/binary_pointer_heap_stats.o ../queues/heap_common.o -o driver_binary_pointer_stats
	$(CC) $(FLAGS) -DUSE_FIBONACCI testdriver.c ../queues/fibonacci_heap.o -o driver_fibonacci
	$(CC) $(FLAGS) -DSTATS -DUSE_FIBONACCI testdriver.c ../queues/fibonacci_heap_stats.o ../queues/heap_common.o -o driver_fibonacci_stats
	$(CC) $(FLAGS) -DUSE_PAIRING testdriver.c ../queues/pairing_heap.o -o driver_pairing
	$(CC) $(FLAGS) -DSTATS -DUSE_PAIRING testdriver.c ../queues/pairing_heap_stats.o ../queues/heap_common.o -o driver_pairing_stats
	$(CC) $(FLAGS) -DUSE_QUAKE testdriver.c ../queues/quake_heap.o -o driver_quake
	$(CC) $(FLAGS) -DSTATS -DUSE_QUAKE testdriver.c ../queues/quake_heap_stats.o ../queues/heap_common.o -o driver_quake_stats
	$(CC) $(FLAGS) -DUSE_RANK_PAIRING testdriver.c ../queues/rank_pairing_heap.o -o driver_rank_pairing
	$(CC) $(FLAGS) -DSTATS -DUSE_RANK_PAIRING testdriver.c ../queues/rank_pairing_heap_stats.o ../queues/heap_common.o -o driver_rank_pairing_stats
	$(CC) $(FLAGS) -DUSE_VIOLATION testdriver.c ../queues/violation_heap.o -o driver_violation
	$(CC) $(FLAGS) -DSTATS -DUSE_VIOLATION testdriver.c ../queues/violation_heap_stats.o ../queues/heap_common.o -o driver_violation_stats
	$(CC) $(FLAGS) -DDUMMY testdriver.c -o driver_dummy

