CC 		=	gcc
FLAGS 	=	-Wall -g -std=gnu99 -O4
OBJS	=	../trace_tools.o ../memory_management_lazy.o
HDRS	=	../trace_tools.h ../memory_management_lazy.h

all: drivers trace_stats

drivers: driver_binomial driver_explicit_2 driver_explicit_4 driver_explicit_8 driver_explicit_16 driver_fibonacci driver_implicit_2 driver_implicit_4 driver_implicit_8 driver_implicit_16 driver_implicit_simple_2 driver_implicit_simple_4 driver_implicit_simple_8 driver_implicit_simple_16 driver_pairing driver_quake driver_rank_pairing_t1 driver_rank_pairing_t2 driver_rank_relaxed_weak driver_strict_fibonacci driver_violation driver_dummy

trace_stats: trace_stats.c $(OBJS) $(HDRS)
	$(CC) $(FLAGS) -DDUMMY trace_stats.c $(OBJS) -o trace_stats

driver_binomial: trace_driver.c $(OBJS) $(HDRS) ../queues/binomial_queue.h ../queues/lazy/binomial_queue.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_BINOMIAL trace_driver.c $(OBJS) ../queues/lazy/binomial_queue.o -o lazy/driver_binomial
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_BINOMIAL trace_driver.c $(OBJS) ../queues/lazy/binomial_queue.o -o lazy/driver_cg_binomial
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_BINOMIAL trace_driver.c $(OBJS) ../queues/eager/binomial_queue.o -o eager/driver_binomial
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_BINOMIAL trace_driver.c $(OBJS) ../queues/eager/binomial_queue.o -o eager/driver_cg_binomial
	$(CC) $(FLAGS) -DUSE_BINOMIAL trace_driver.c $(OBJS) ../queues/dumb/binomial_queue.o -o dumb/driver_binomial
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_BINOMIAL trace_driver.c $(OBJS) ../queues/dumb/binomial_queue.o -o dumb/driver_cg_binomial

driver_explicit_2: trace_driver.c $(OBJS) $(HDRS) ../queues/explicit_heap.h ../queues/lazy/explicit_2_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_EXPLICIT_2 trace_driver.c $(OBJS) ../queues/lazy/explicit_2_heap.o -o lazy/driver_explicit_2
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_EXPLICIT_2 trace_driver.c $(OBJS) ../queues/lazy/explicit_2_heap.o -o lazy/driver_cg_explicit_2
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_EXPLICIT_2 trace_driver.c $(OBJS) ../queues/eager/explicit_2_heap.o -o eager/driver_explicit_2
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_EXPLICIT_2 trace_driver.c $(OBJS) ../queues/eager/explicit_2_heap.o -o eager/driver_cg_explicit_2
	$(CC) $(FLAGS) -DUSE_EXPLICIT_2 trace_driver.c $(OBJS) ../queues/dumb/explicit_2_heap.o -o dumb/driver_explicit_2
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_EXPLICIT_2 trace_driver.c $(OBJS) ../queues/dumb/explicit_2_heap.o -o dumb/driver_cg_explicit_2

driver_explicit_4: trace_driver.c $(OBJS) $(HDRS) ../queues/explicit_heap.h ../queues/lazy/explicit_4_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_EXPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/lazy/explicit_4_heap.o -o lazy/driver_explicit_4
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_EXPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/lazy/explicit_4_heap.o -o lazy/driver_cg_explicit_4
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_EXPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/eager/explicit_4_heap.o -o eager/driver_explicit_4
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_EXPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/eager/explicit_4_heap.o -o eager/driver_cg_explicit_4
	$(CC) $(FLAGS) -DUSE_EXPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/dumb/explicit_4_heap.o -o dumb/driver_explicit_4
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_EXPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/dumb/explicit_4_heap.o -o dumb/driver_cg_explicit_4

driver_explicit_8: trace_driver.c $(OBJS) $(HDRS) ../queues/explicit_heap.h ../queues/lazy/explicit_8_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_EXPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/lazy/explicit_8_heap.o -o lazy/driver_explicit_8
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_EXPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/lazy/explicit_8_heap.o -o lazy/driver_cg_explicit_8
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_EXPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/eager/explicit_8_heap.o -o eager/driver_explicit_8
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_EXPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/eager/explicit_8_heap.o -o eager/driver_cg_explicit_8
	$(CC) $(FLAGS) -DUSE_EXPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/dumb/explicit_8_heap.o -o dumb/driver_explicit_8
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_EXPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/dumb/explicit_8_heap.o -o dumb/driver_cg_explicit_8

driver_explicit_16: trace_driver.c $(OBJS) $(HDRS) ../queues/explicit_heap.h ../queues/lazy/explicit_16_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_EXPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/lazy/explicit_16_heap.o -o lazy/driver_explicit_16
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_EXPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/lazy/explicit_16_heap.o -o lazy/driver_cg_explicit_16
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_EXPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/eager/explicit_16_heap.o -o eager/driver_explicit_16
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_EXPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/eager/explicit_16_heap.o -o eager/driver_cg_explicit_16
	$(CC) $(FLAGS) -DUSE_EXPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/dumb/explicit_16_heap.o -o dumb/driver_explicit_16
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_EXPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/dumb/explicit_16_heap.o -o dumb/driver_cg_explicit_16

driver_fibonacci: trace_driver.c $(OBJS) $(HDRS) ../queues/fibonacci_heap.h ../queues/lazy/fibonacci_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_FIBONACCI trace_driver.c $(OBJS) ../queues/lazy/fibonacci_heap.o -o lazy/driver_fibonacci
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_FIBONACCI trace_driver.c $(OBJS) ../queues/lazy/fibonacci_heap.o -o lazy/driver_cg_fibonacci
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_FIBONACCI trace_driver.c $(OBJS) ../queues/eager/fibonacci_heap.o -o eager/driver_fibonacci
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_FIBONACCI trace_driver.c $(OBJS) ../queues/eager/fibonacci_heap.o -o eager/driver_cg_fibonacci
	$(CC) $(FLAGS) -DUSE_FIBONACCI trace_driver.c $(OBJS) ../queues/dumb/fibonacci_heap.o -o dumb/driver_fibonacci
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_FIBONACCI trace_driver.c $(OBJS) ../queues/dumb/fibonacci_heap.o -o dumb/driver_cg_fibonacci

driver_implicit_2: trace_driver.c $(OBJS) $(HDRS) ../queues/implicit_heap.h ../queues/lazy/implicit_2_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/lazy/implicit_2_heap.o -o lazy/driver_implicit_2
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/lazy/implicit_2_heap.o -o lazy/driver_cg_implicit_2
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/eager/implicit_2_heap.o -o eager/driver_implicit_2
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/eager/implicit_2_heap.o -o eager/driver_cg_implicit_2
	$(CC) $(FLAGS) -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/dumb/implicit_2_heap.o -o dumb/driver_implicit_2
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/dumb/implicit_2_heap.o -o dumb/driver_cg_implicit_2

driver_implicit_4: trace_driver.c $(OBJS) $(HDRS) ../queues/implicit_heap.h ../queues/lazy/implicit_4_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_IMPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/lazy/implicit_4_heap.o -o lazy/driver_implicit_4
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_IMPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/lazy/implicit_4_heap.o -o lazy/driver_cg_implicit_4
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_IMPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/eager/implicit_4_heap.o -o eager/driver_implicit_4
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_IMPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/eager/implicit_4_heap.o -o eager/driver_cg_implicit_4
	$(CC) $(FLAGS) -DUSE_IMPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/dumb/implicit_4_heap.o -o dumb/driver_implicit_4
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_IMPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/dumb/implicit_4_heap.o -o dumb/driver_cg_implicit_4

driver_implicit_8: trace_driver.c $(OBJS) $(HDRS) ../queues/implicit_heap.h ../queues/lazy/implicit_8_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_IMPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/lazy/implicit_8_heap.o -o lazy/driver_implicit_8
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_IMPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/lazy/implicit_8_heap.o -o lazy/driver_cg_implicit_8
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_IMPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/eager/implicit_8_heap.o -o eager/driver_implicit_8
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_IMPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/eager/implicit_8_heap.o -o eager/driver_cg_implicit_8
	$(CC) $(FLAGS) -DUSE_IMPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/dumb/implicit_8_heap.o -o dumb/driver_implicit_8
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_IMPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/dumb/implicit_8_heap.o -o dumb/driver_cg_implicit_8

driver_implicit_16: trace_driver.c $(OBJS) $(HDRS) ../queues/implicit_heap.h ../queues/lazy/implicit_16_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_IMPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/lazy/implicit_16_heap.o -o lazy/driver_implicit_16
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_IMPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/lazy/implicit_16_heap.o -o lazy/driver_cg_implicit_16
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_IMPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/eager/implicit_16_heap.o -o eager/driver_implicit_16
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_IMPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/eager/implicit_16_heap.o -o eager/driver_cg_implicit_16
	$(CC) $(FLAGS) -DUSE_IMPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/dumb/implicit_16_heap.o -o dumb/driver_implicit_16
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_IMPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/dumb/implicit_16_heap.o -o dumb/driver_cg_implicit_16

driver_implicit_simple_2: trace_driver.c $(OBJS) $(HDRS) ../queues/implicit_simple_heap.h ../queues/lazy/implicit_simple_2_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/lazy/implicit_simple_2_heap.o -o lazy/driver_implicit_simple_2
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/lazy/implicit_simple_2_heap.o -o lazy/driver_cg_implicit_simple_2
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/eager/implicit_simple_2_heap.o -o eager/driver_implicit_simple_2
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/eager/implicit_simple_2_heap.o -o eager/driver_cg_implicit_simple_2
	$(CC) $(FLAGS) -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/dumb/implicit_simple_2_heap.o -o dumb/driver_implicit_simple_2
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_IMPLICIT_2 trace_driver.c $(OBJS) ../queues/dumb/implicit_simple_2_heap.o -o dumb/driver_cg_implicit_simple_2

driver_implicit_simple_4: trace_driver.c $(OBJS) $(HDRS) ../queues/implicit_simple_heap.h ../queues/lazy/implicit_simple_4_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_IMPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/lazy/implicit_simple_4_heap.o -o lazy/driver_implicit_simple_4
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_IMPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/lazy/implicit_simple_4_heap.o -o lazy/driver_cg_implicit_simple_4
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_IMPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/eager/implicit_simple_4_heap.o -o eager/driver_implicit_simple_4
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_IMPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/eager/implicit_simple_4_heap.o -o eager/driver_cg_implicit_simple_4
	$(CC) $(FLAGS) -DUSE_IMPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/dumb/implicit_simple_4_heap.o -o dumb/driver_implicit_simple_4
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_IMPLICIT_4 -DBRANCH_4 trace_driver.c $(OBJS) ../queues/dumb/implicit_simple_4_heap.o -o dumb/driver_cg_implicit_simple_4

driver_implicit_simple_8: trace_driver.c $(OBJS) $(HDRS) ../queues/implicit_simple_heap.h ../queues/lazy/implicit_simple_8_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_IMPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/lazy/implicit_simple_8_heap.o -o lazy/driver_implicit_simple_8
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_IMPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/lazy/implicit_simple_8_heap.o -o lazy/driver_cg_implicit_simple_8
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_IMPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/eager/implicit_simple_8_heap.o -o eager/driver_implicit_simple_8
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_IMPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/eager/implicit_simple_8_heap.o -o eager/driver_cg_implicit_simple_8
	$(CC) $(FLAGS) -DUSE_IMPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/dumb/implicit_simple_8_heap.o -o dumb/driver_implicit_simple_8
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_IMPLICIT_8 -DBRANCH_8 trace_driver.c $(OBJS) ../queues/dumb/implicit_simple_8_heap.o -o dumb/driver_cg_implicit_simple_8

driver_implicit_simple_16: trace_driver.c $(OBJS) $(HDRS) ../queues/implicit_simple_heap.h ../queues/lazy/implicit_simple_16_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_IMPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/lazy/implicit_simple_16_heap.o -o lazy/driver_implicit_simple_16
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_IMPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/lazy/implicit_simple_16_heap.o -o lazy/driver_cg_implicit_simple_16
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_IMPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/eager/implicit_simple_16_heap.o -o eager/driver_implicit_simple_16
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_IMPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/eager/implicit_simple_16_heap.o -o eager/driver_cg_implicit_simple_16
	$(CC) $(FLAGS) -DUSE_IMPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/dumb/implicit_simple_16_heap.o -o dumb/driver_implicit_simple_16
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_IMPLICIT_16 -DBRANCH_16 trace_driver.c $(OBJS) ../queues/dumb/implicit_simple_16_heap.o -o dumb/driver_cg_implicit_simple_16

driver_pairing: trace_driver.c $(OBJS) $(HDRS) ../queues/pairing_heap.h ../queues/lazy/pairing_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_PAIRING trace_driver.c $(OBJS) ../queues/lazy/pairing_heap.o -o lazy/driver_pairing
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_PAIRING trace_driver.c $(OBJS) ../queues/lazy/pairing_heap.o -o lazy/driver_cg_pairing
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_PAIRING trace_driver.c $(OBJS) ../queues/eager/pairing_heap.o -o eager/driver_pairing
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_PAIRING trace_driver.c $(OBJS) ../queues/eager/pairing_heap.o -o eager/driver_cg_pairing
	$(CC) $(FLAGS) -DUSE_PAIRING trace_driver.c $(OBJS) ../queues/dumb/pairing_heap.o -o dumb/driver_pairing
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_PAIRING trace_driver.c $(OBJS) ../queues/dumb/pairing_heap.o -o dumb/driver_cg_pairing

driver_quake: trace_driver.c $(OBJS) $(HDRS) ../queues/quake_heap.h ../queues/lazy/quake_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_QUAKE trace_driver.c $(OBJS) ../queues/lazy/quake_heap.o -o lazy/driver_quake
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_QUAKE trace_driver.c $(OBJS) ../queues/lazy/quake_heap.o -o lazy/driver_cg_quake
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_QUAKE trace_driver.c $(OBJS) ../queues/eager/quake_heap.o -o eager/driver_quake
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_QUAKE trace_driver.c $(OBJS) ../queues/eager/quake_heap.o -o eager/driver_cg_quake
	$(CC) $(FLAGS) -DUSE_QUAKE trace_driver.c $(OBJS) ../queues/dumb/quake_heap.o -o dumb/driver_quake
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_QUAKE trace_driver.c $(OBJS) ../queues/dumb/quake_heap.o -o dumb/driver_cg_quake

driver_rank_pairing_t1: trace_driver.c $(OBJS) $(HDRS) ../queues/rank_pairing_heap.h ../queues/lazy/rank_pairing_t1_heap.o
	$(CC) $(FLAGS) -DUSE_TYPE_1 -DUSE_LAZY -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/lazy/rank_pairing_t1_heap.o -o lazy/driver_rank_pairing_t1
	$(CC) $(FLAGS) -DUSE_TYPE_1 -DUSE_LAZY -DCACHEGRIND -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/lazy/rank_pairing_t1_heap.o -o lazy/driver_cg_rank_pairing_t1
	$(CC) $(FLAGS) -DUSE_TYPE_1 -DUSE_EAGER -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/eager/rank_pairing_t1_heap.o -o eager/driver_rank_pairing_t1
	$(CC) $(FLAGS) -DUSE_TYPE_1 -DUSE_EAGER -DCACHEGRIND -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/eager/rank_pairing_t1_heap.o -o eager/driver_cg_rank_pairing_t1
	$(CC) $(FLAGS) -DUSE_TYPE_1 -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/dumb/rank_pairing_t1_heap.o -o dumb/driver_rank_pairing_t1
	$(CC) $(FLAGS) -DUSE_TYPE_1 -DCACHEGRIND -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/dumb/rank_pairing_t1_heap.o -o dumb/driver_cg_rank_pairing_t1

driver_rank_pairing_t2: trace_driver.c $(OBJS) $(HDRS) ../queues/rank_pairing_heap.h ../queues/lazy/rank_pairing_t2_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/lazy/rank_pairing_t2_heap.o -o lazy/driver_rank_pairing_t2
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/lazy/rank_pairing_t2_heap.o -o lazy/driver_cg_rank_pairing_t2
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/eager/rank_pairing_t2_heap.o -o eager/driver_rank_pairing_t2
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/eager/rank_pairing_t2_heap.o -o eager/driver_cg_rank_pairing_t2
	$(CC) $(FLAGS) -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/dumb/rank_pairing_t2_heap.o -o dumb/driver_rank_pairing_t2
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_RANK_PAIRING trace_driver.c $(OBJS) ../queues/dumb/rank_pairing_t2_heap.o -o dumb/driver_cg_rank_pairing_t2

driver_rank_relaxed_weak: trace_driver.c $(OBJS) $(HDRS) ../queues/rank_relaxed_weak_queue.h ../queues/lazy/rank_relaxed_weak_queue.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_RANK_RELAXED_WEAK trace_driver.c $(OBJS) ../queues/lazy/rank_relaxed_weak_queue.o -o lazy/driver_rank_relaxed_weak
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_RANK_RELAXED_WEAK trace_driver.c $(OBJS) ../queues/lazy/rank_relaxed_weak_queue.o -o lazy/driver_cg_rank_relaxed_weak
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_RANK_RELAXED_WEAK trace_driver.c $(OBJS) ../queues/eager/rank_relaxed_weak_queue.o -o eager/driver_rank_relaxed_weak
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_RANK_RELAXED_WEAK trace_driver.c $(OBJS) ../queues/eager/rank_relaxed_weak_queue.o -o eager/driver_cg_rank_relaxed_weak
	$(CC) $(FLAGS) -DUSE_RANK_RELAXED_WEAK trace_driver.c $(OBJS) ../queues/dumb/rank_relaxed_weak_queue.o -o dumb/driver_rank_relaxed_weak
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_RANK_RELAXED_WEAK trace_driver.c $(OBJS) ../queues/dumb/rank_relaxed_weak_queue.o -o dumb/driver_cg_rank_relaxed_weak

driver_strict_fibonacci: trace_driver.c $(OBJS) $(HDRS) ../queues/strict_fibonacci_heap.h ../queues/lazy/strict_fibonacci_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_STRICT_FIBONACCI trace_driver.c $(OBJS) ../queues/lazy/strict_fibonacci_heap.o -o lazy/driver_strict_fibonacci
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_STRICT_FIBONACCI trace_driver.c $(OBJS) ../queues/lazy/strict_fibonacci_heap.o -o lazy/driver_cg_strict_fibonacci
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_STRICT_FIBONACCI trace_driver.c $(OBJS) ../queues/eager/strict_fibonacci_heap.o -o eager/driver_strict_fibonacci
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_STRICT_FIBONACCI trace_driver.c $(OBJS) ../queues/eager/strict_fibonacci_heap.o -o eager/driver_cg_strict_fibonacci
	$(CC) $(FLAGS) -DUSE_STRICT_FIBONACCI trace_driver.c $(OBJS) ../queues/dumb/strict_fibonacci_heap.o -o dumb/driver_strict_fibonacci
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_STRICT_FIBONACCI trace_driver.c $(OBJS) ../queues/dumb/strict_fibonacci_heap.o -o dumb/driver_cg_strict_fibonacci

driver_violation: trace_driver.c $(OBJS) $(HDRS) ../queues/violation_heap.h ../queues/lazy/violation_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DUSE_VIOLATION trace_driver.c $(OBJS) ../queues/lazy/violation_heap.o -o lazy/driver_violation
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DUSE_VIOLATION trace_driver.c $(OBJS) ../queues/lazy/violation_heap.o -o lazy/driver_cg_violation
	$(CC) $(FLAGS) -DUSE_EAGER -DUSE_VIOLATION trace_driver.c $(OBJS) ../queues/eager/violation_heap.o -o eager/driver_violation
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DUSE_VIOLATION trace_driver.c $(OBJS) ../queues/eager/violation_heap.o -o eager/driver_cg_violation
	$(CC) $(FLAGS) -DUSE_VIOLATION trace_driver.c $(OBJS) ../queues/dumb/violation_heap.o -o dumb/driver_violation
	$(CC) $(FLAGS) -DCACHEGRIND -DUSE_VIOLATION trace_driver.c $(OBJS) ../queues/dumb/violation_heap.o -o dumb/driver_cg_violation

driver_dummy: trace_driver.c $(OBJS) $(HDRS)
	$(CC) $(FLAGS) -DUSE_LAZY -DDUMMY trace_driver.c $(OBJS) -o lazy/driver_dummy
	$(CC) $(FLAGS) -DUSE_LAZY -DCACHEGRIND -DDUMMY trace_driver.c $(OBJS) -o lazy/driver_cg_dummy
	$(CC) $(FLAGS) -DUSE_EAGER -DDUMMY trace_driver.c $(OBJS) -o eager/driver_dummy
	$(CC) $(FLAGS) -DUSE_EAGER -DCACHEGRIND -DDUMMY trace_driver.c $(OBJS) -o eager/driver_cg_dummy
	$(CC) $(FLAGS) -DDUMMY trace_driver.c $(OBJS) -o dumb/driver_dummy
	$(CC) $(FLAGS) -DCACHEGRIND -DDUMMY trace_driver.c $(OBJS) -o dumb/driver_cg_dummy

