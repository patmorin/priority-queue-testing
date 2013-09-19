CC 	=	gcc
CCP	=	g++
FLAGS 	=	-Wall -pedantic -g -std=gnu99 -O4 -c
FLAGSCP	=	-Wall -pedantic -g -O4 -c
DEP	=	../memory_management_lazy.o ../memory_management_eager.o ../memory_management_dumb.o

all: queues

queues: binomial_queue.o explicit_2_heap.o fibonacci_heap.o implicit_2_heap.o \
		implicit_simple_2_heap.o pairing_heap.o quake_heap.o \
		rank_pairing_heap.o rank_relaxed_weak_queue.o strict_fibonacci_heap.o \
		violation_heap.o knheap.o

binomial_queue.o: $(DEP) binomial_queue.c binomial_queue.h
	$(CC) $(FLAGS) -DUSE_LAZY binomial_queue.c -o lazy/binomial_queue.o
	$(CC) $(FLAGS) -DUSE_EAGER binomial_queue.c -o eager/binomial_queue.o
	$(CC) $(FLAGS) binomial_queue.c -o dumb/binomial_queue.o

explicit_2_heap.o: $(DEP) explicit_heap.c explicit_heap.h
	$(CC) $(FLAGS) -DUSE_LAZY explicit_heap.c -o lazy/explicit_2_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER explicit_heap.c -o eager/explicit_2_heap.o
	$(CC) $(FLAGS) explicit_heap.c -o dumb/explicit_2_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DBRANCH_4 explicit_heap.c -o lazy/explicit_4_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER -DBRANCH_4 explicit_heap.c -o eager/explicit_4_heap.o
	$(CC) $(FLAGS) -DBRANCH_4 explicit_heap.c -o dumb/explicit_4_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DBRANCH_8 explicit_heap.c -o lazy/explicit_8_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER -DBRANCH_8 explicit_heap.c -o eager/explicit_8_heap.o
	$(CC) $(FLAGS) -DBRANCH_8 explicit_heap.c -o dumb/explicit_8_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DBRANCH_16 explicit_heap.c -o lazy/explicit_16_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER -DBRANCH_16 explicit_heap.c -o eager/explicit_16_heap.o
	$(CC) $(FLAGS) -DBRANCH_16 explicit_heap.c -o dumb/explicit_16_heap.o

fibonacci_heap.o: $(DEP) fibonacci_heap.c fibonacci_heap.h
	$(CC) $(FLAGS) -DUSE_LAZY fibonacci_heap.c -o lazy/fibonacci_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER fibonacci_heap.c -o eager/fibonacci_heap.o
	$(CC) $(FLAGS) fibonacci_heap.c -o dumb/fibonacci_heap.o

implicit_2_heap.o: $(DEP) implicit_heap.c implicit_heap.h
	$(CC) $(FLAGS) -DUSE_LAZY implicit_heap.c -o lazy/implicit_2_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER implicit_heap.c -o eager/implicit_2_heap.o
	$(CC) $(FLAGS) implicit_heap.c -o dumb/implicit_2_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DBRANCH_4 implicit_heap.c -o lazy/implicit_4_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER -DBRANCH_4 implicit_heap.c -o eager/implicit_4_heap.o
	$(CC) $(FLAGS) -DBRANCH_4 implicit_heap.c -o dumb/implicit_4_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DBRANCH_8 implicit_heap.c -o lazy/implicit_8_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER -DBRANCH_8 implicit_heap.c -o eager/implicit_8_heap.o
	$(CC) $(FLAGS) -DBRANCH_8 implicit_heap.c -o dumb/implicit_8_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DBRANCH_16 implicit_heap.c -o lazy/implicit_16_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER -DBRANCH_16 implicit_heap.c -o eager/implicit_16_heap.o
	$(CC) $(FLAGS) -DBRANCH_16 implicit_heap.c -o dumb/implicit_16_heap.o

implicit_simple_2_heap.o: $(DEP) implicit_simple_heap.c implicit_simple_heap.h
	$(CC) $(FLAGS) -DUSE_LAZY implicit_simple_heap.c -o lazy/implicit_simple_2_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER implicit_simple_heap.c -o eager/implicit_simple_2_heap.o
	$(CC) $(FLAGS) implicit_simple_heap.c -o dumb/implicit_simple_2_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DBRANCH_4 implicit_simple_heap.c -o lazy/implicit_simple_4_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER -DBRANCH_4 implicit_simple_heap.c -o eager/implicit_simple_4_heap.o
	$(CC) $(FLAGS) -DBRANCH_4 implicit_simple_heap.c -o dumb/implicit_simple_4_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DBRANCH_8 implicit_simple_heap.c -o lazy/implicit_simple_8_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER -DBRANCH_8 implicit_simple_heap.c -o eager/implicit_simple_8_heap.o
	$(CC) $(FLAGS) -DBRANCH_8 implicit_simple_heap.c -o dumb/implicit_simple_8_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY -DBRANCH_16 implicit_simple_heap.c -o lazy/implicit_simple_16_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER -DBRANCH_16 implicit_simple_heap.c -o eager/implicit_simple_16_heap.o
	$(CC) $(FLAGS) -DBRANCH_16 implicit_simple_heap.c -o dumb/implicit_simple_16_heap.o

pairing_heap.o: $(DEP) pairing_heap.c pairing_heap.h
	$(CC) $(FLAGS) -DUSE_LAZY pairing_heap.c -o lazy/pairing_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER pairing_heap.c -o eager/pairing_heap.o
	$(CC) $(FLAGS) pairing_heap.c -o dumb/pairing_heap.o

quake_heap.o: $(DEP) quake_heap.c quake_heap.h
	$(CC) $(FLAGS) -DUSE_LAZY quake_heap.c -o lazy/quake_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER quake_heap.c -o eager/quake_heap.o
	$(CC) $(FLAGS) quake_heap.c -o dumb/quake_heap.o

rank_pairing_heap.o: $(DEP) rank_pairing_heap.c rank_pairing_heap.h
	$(CC) $(FLAGS) -DUSE_TYPE_1 -DUSE_LAZY rank_pairing_heap.c -o lazy/rank_pairing_t1_heap.o
	$(CC) $(FLAGS) -DUSE_TYPE_1 -DUSE_EAGER rank_pairing_heap.c -o eager/rank_pairing_t1_heap.o
	$(CC) $(FLAGS) -DUSE_TYPE_1 rank_pairing_heap.c -o dumb/rank_pairing_t1_heap.o
	$(CC) $(FLAGS) -DUSE_LAZY rank_pairing_heap.c -o lazy/rank_pairing_t2_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER rank_pairing_heap.c -o eager/rank_pairing_t2_heap.o
	$(CC) $(FLAGS) rank_pairing_heap.c -o dumb/rank_pairing_t2_heap.o

rank_relaxed_weak_queue.o: $(DEP) rank_relaxed_weak_queue.c rank_relaxed_weak_queue.h
	$(CC) $(FLAGS) -DUSE_LAZY rank_relaxed_weak_queue.c -o lazy/rank_relaxed_weak_queue.o
	$(CC) $(FLAGS) -DUSE_EAGER rank_relaxed_weak_queue.c -o eager/rank_relaxed_weak_queue.o
	$(CC) $(FLAGS) rank_relaxed_weak_queue.c -o dumb/rank_relaxed_weak_queue.o

strict_fibonacci_heap.o: $(DEP) strict_fibonacci_heap.c strict_fibonacci_heap.h
	$(CC) $(FLAGS) -DUSE_LAZY strict_fibonacci_heap.c -o lazy/strict_fibonacci_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER strict_fibonacci_heap.c -o eager/strict_fibonacci_heap.o
	$(CC) $(FLAGS) strict_fibonacci_heap.c -o dumb/strict_fibonacci_heap.o

violation_heap.o: $(DEP) violation_heap.c violation_heap.h
	$(CC) $(FLAGS) -DUSE_LAZY violation_heap.c -o lazy/violation_heap.o
	$(CC) $(FLAGS) -DUSE_EAGER violation_heap.c -o eager/violation_heap.o
	$(CC) $(FLAGS) violation_heap.c -o dumb/violation_heap.o

knheap.o: $(DEP) knheap.C knheap.h multiMergeUnrolled.C util.h
	$(CCP) $(FLAGSCP) -DUSE_LAZY knheap.C -o lazy/knheap.o
	$(CCP) $(FLAGSCP) -DUSE_EAGER knheap.C -o eager/knheap.o
	$(CCP) $(FLAGSCP) knheap.C -o dumb/knheap.o

clean: 
	rm dumb/*.o eager/*.o lazy/*.o 