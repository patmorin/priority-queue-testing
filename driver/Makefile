CC 		=	gcc
FLAGS 	=	-Wall -g -std=c99

all: drivers

drivers: 
	$(CC) $(FLAGS) -DUSE_EXPLICIT_2 trace_driver.c ../trace_tools.o ../queues/explicit_2_heap.o -o driver_explicit_2
	$(CC) $(FLAGS) -DUSE_EXPLICIT_4 trace_driver.c ../trace_tools.o ../queues/explicit_4_heap.o -o driver_explicit_4
	$(CC) $(FLAGS) -DUSE_EXPLICIT_8 trace_driver.c ../trace_tools.o ../queues/explicit_8_heap.o -o driver_explicit_8
	$(CC) $(FLAGS) -DUSE_EXPLICIT_16 trace_driver.c ../trace_tools.o ../queues/explicit_16_heap.o -o driver_explicit_16
	$(CC) $(FLAGS) -DUSE_FIBONACCI trace_driver.c ../trace_tools.o ../queues/fibonacci_heap.o -o driver_fibonacci
	$(CC) $(FLAGS) -DUSE_IMPLICIT_2 trace_driver.c ../trace_tools.o ../queues/implicit_2_heap.o -o driver_implicit_2
	$(CC) $(FLAGS) -DUSE_IMPLICIT_4 trace_driver.c ../trace_tools.o ../queues/implicit_4_heap.o -o driver_implicit_4
	$(CC) $(FLAGS) -DUSE_IMPLICIT_8 trace_driver.c ../trace_tools.o ../queues/implicit_8_heap.o -o driver_implicit_8
	$(CC) $(FLAGS) -DUSE_IMPLICIT_16 trace_driver.c ../trace_tools.o ../queues/implicit_16_heap.o -o driver_implicit_16
	$(CC) $(FLAGS) -DUSE_PAIRING trace_driver.c ../trace_tools.o ../queues/pairing_heap.o -o driver_pairing
	$(CC) $(FLAGS) -DUSE_QUAKE trace_driver.c ../trace_tools.o ../queues/quake_heap.o -o driver_quake
	$(CC) $(FLAGS) -DUSE_RANK_PAIRING trace_driver.c ../trace_tools.o ../queues/rank_pairing_heap.o -o driver_rank_pairing
	$(CC) $(FLAGS) -DUSE_VIOLATION trace_driver.c ../trace_tools.o ../queues/violation_heap.o -o driver_violation
	$(CC) $(FLAGS) -DDUMMY trace_driver.c ../trace_tools.o -o driver_dummy

