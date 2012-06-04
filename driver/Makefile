CC 		=	gcc
FLAGS 	=	-Wall -g -std=c99
OBJS	=	../trace_tools.o ../memory_management.o

all: drivers

drivers: 
	$(CC) $(FLAGS) -DUSE_EXPLICIT_2 trace_driver.c $(OBJS) ../queues/explicit_2_heap.o -o driver_explicit_2
	$(CC) $(FLAGS) -DUSE_EXPLICIT_4 trace_driver.c $(OBJS) ../queues/explicit_4_heap.o -o driver_explicit_4
	$(CC) $(FLAGS) -DUSE_EXPLICIT_8 trace_driver.c $(OBJS) ../queues/explicit_8_heap.o -o driver_explicit_8
	$(CC) $(FLAGS) -DUSE_EXPLICIT_16 trace_driver.c $(OBJS) ../queues/explicit_16_heap.o -o driver_explicit_16
	$(CC) $(FLAGS) -DUSE_FIBONACCI trace_driver.c $(OBJS) ../queues/fibonacci_heap.o -o driver_fibonacci
	$(CC) $(FLAGS) -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/implicit_2_heap.o -o driver_implicit_2
	$(CC) $(FLAGS) -DUSE_IMPLICIT_4 trace_driver.c $(OBJS) ../queues/implicit_4_heap.o -o driver_implicit_4
	$(CC) $(FLAGS) -DUSE_IMPLICIT_8 trace_driver.c $(OBJS) ../queues/implicit_8_heap.o -o driver_implicit_8
	$(CC) $(FLAGS) -DUSE_IMPLICIT_16 trace_driver.c $(OBJS) ../queues/implicit_16_heap.o -o driver_implicit_16
	$(CC) $(FLAGS) -DUSE_PAIRING trace_driver.c $(OBJS) ../queues/pairing_heap.o -o driver_pairing
	$(CC) $(FLAGS) -DUSE_QUAKE trace_driver.c $(OBJS) ../queues/quake_heap.o -o driver_quake
	$(CC) $(FLAGS) -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/rank_pairing_heap.o -o driver_rank_pairing
	$(CC) $(FLAGS) -DUSE_VIOLATION trace_driver.c $(OBJS) ../queues/violation_heap.o -o driver_violation
	$(CC) $(FLAGS) -DDUMMY trace_driver.c $(OBJS) -o driver_dummy

