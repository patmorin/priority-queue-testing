CC 		=	gcc
FLAGS 	=	-Wall -g -std=gnu99 -O0
OBJS	=	../trace_tools.o ../memory_management.o
HDRS	=	../trace_tools.h ../memory_management.h

all: drivers

drivers: driver_binomial driver_explicit_2 driver_explicit_4 driver_explicit_8 driver_explicit_16 driver_fibonacci driver_implicit_2 driver_implicit_4 driver_implicit_8 driver_implicit_16 driver_pairing driver_quake driver_rank_pairing driver_rank_relaxed_weak driver_strict_fibonacci driver_violation driver_dummy

driver_binomial: trace_driver.c $(OBJS) $(HDRS) ../queues/binomial_queue.h ../queues/binomial_queue.o
	$(CC) $(FLAGS) -DUSE_BINOMIAL trace_driver.c $(OBJS) ../queues/binomial_queue.o -o driver_binomial

driver_explicit_2: trace_driver.c $(OBJS) $(HDRS) ../queues/explicit_heap.h ../queues/explicit_2_heap.o
	$(CC) $(FLAGS) -DUSE_EXPLICIT_2 trace_driver.c $(OBJS) ../queues/explicit_2_heap.o -o driver_explicit_2

driver_explicit_4: trace_driver.c $(OBJS) $(HDRS) ../queues/explicit_heap.h ../queues/explicit_4_heap.o
	$(CC) $(FLAGS) -DUSE_EXPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/explicit_4_heap.o -o driver_explicit_4

driver_explicit_8: trace_driver.c $(OBJS) $(HDRS) ../queues/explicit_heap.h ../queues/explicit_8_heap.o
	$(CC) $(FLAGS) -DUSE_EXPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/explicit_8_heap.o -o driver_explicit_8

driver_explicit_16: trace_driver.c $(OBJS) $(HDRS) ../queues/explicit_heap.h ../queues/explicit_16_heap.o
	$(CC) $(FLAGS) -DUSE_EXPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/explicit_16_heap.o -o driver_explicit_16

driver_fibonacci: trace_driver.c $(OBJS) $(HDRS) ../queues/fibonacci_heap.h ../queues/fibonacci_heap.o
	$(CC) $(FLAGS) -DUSE_FIBONACCI trace_driver.c $(OBJS) ../queues/fibonacci_heap.o -o driver_fibonacci

driver_implicit_2: trace_driver.c $(OBJS) $(HDRS) ../queues/implicit_heap.h ../queues/implicit_2_heap.o
	$(CC) $(FLAGS) -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/implicit_2_heap.o -o driver_implicit_2

driver_implicit_4: trace_driver.c $(OBJS) $(HDRS) ../queues/implicit_heap.h ../queues/implicit_4_heap.o
	$(CC) $(FLAGS) -DUSE_IMPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/implicit_4_heap.o -o driver_implicit_4

driver_implicit_8: trace_driver.c $(OBJS) $(HDRS) ../queues/implicit_heap.h ../queues/implicit_8_heap.o
	$(CC) $(FLAGS) -DUSE_IMPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/implicit_8_heap.o -o driver_implicit_8

driver_implicit_16: trace_driver.c $(OBJS) $(HDRS) ../queues/implicit_heap.h ../queues/implicit_16_heap.o
	$(CC) $(FLAGS) -DUSE_IMPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/implicit_16_heap.o -o driver_implicit_16

driver_pairing: trace_driver.c $(OBJS) $(HDRS) ../queues/pairing_heap.h ../queues/pairing_heap.o
	$(CC) $(FLAGS) -DUSE_PAIRING trace_driver.c $(OBJS) ../queues/pairing_heap.o -o driver_pairing

driver_quake: trace_driver.c $(OBJS) $(HDRS) ../queues/quake_heap.h ../queues/quake_heap.o
	$(CC) $(FLAGS) -DUSE_QUAKE trace_driver.c $(OBJS) ../queues/quake_heap.o -o driver_quake

driver_rank_pairing: trace_driver.c $(OBJS) $(HDRS) ../queues/rank_pairing_heap.h ../queues/rank_pairing_heap.o
	$(CC) $(FLAGS) -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/rank_pairing_heap.o -o driver_rank_pairing

driver_rank_relaxed_weak: trace_driver.c $(OBJS) $(HDRS) ../queues/rank_relaxed_weak_queue.h ../queues/rank_relaxed_weak_queue.o
	$(CC) $(FLAGS) -DUSE_RANK_RELAXED_WEAK trace_driver.c $(OBJS) ../queues/rank_relaxed_weak_queue.o -o driver_rank_relaxed_weak

driver_strict_fibonacci: trace_driver.c $(OBJS) $(HDRS) ../queues/strict_fibonacci_heap.h ../queues/strict_fibonacci_heap.o
	$(CC) $(FLAGS) -DUSE_STRICT_FIBONACCI trace_driver.c $(OBJS) ../queues/strict_fibonacci_heap.o -o driver_strict_fibonacci

driver_violation: trace_driver.c $(OBJS) $(HDRS) ../queues/violation_heap.h ../queues/violation_heap.o
	$(CC) $(FLAGS) -DUSE_VIOLATION trace_driver.c $(OBJS) ../queues/violation_heap.o -o driver_violation

driver_dummy: trace_driver.c $(OBJS) $(HDRS)
	$(CC) $(FLAGS) -DDUMMY trace_driver.c $(OBJS) -o driver_dummy

