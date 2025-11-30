#!/bin/bash

echo "======================================"
echo "   SORTING ALGORITHM BENCHMARKS"
echo "======================================"
echo ""

SIZES=(10000 100000 500000 1000000 2000000)
NUM_THREADS=4
NUM_PROCESSES=4

for SIZE in "${SIZES[@]}"; do
    echo "* * * * * * * Array Size: $SIZE"
    
    echo -n "with sequential    "
    /usr/bin/time -f "%E" ./bin/sequential_sort $SIZE 2>&1 | grep -v "Sequential"
    
    echo -n "with pthread       "
    /usr/bin/time -f "%E" ./bin/pthread_sort $SIZE $NUM_THREADS 2>&1 | grep -v "Pthread"
    
    echo -n "with openmp        "
    /usr/bin/time -f "%E" ./bin/openmp_sort $SIZE $NUM_THREADS 2>&1 | grep -v "OpenMP"
    
    echo -n "with mpi(4p)       "
    /usr/bin/time -f "%E" mpirun --oversubscribe -np $NUM_PROCESSES ./bin/mpi_sort $SIZE 2>&1 | grep -v "MPI"
    
    echo ""
done
