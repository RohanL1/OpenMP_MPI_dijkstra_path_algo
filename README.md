# OpenMP_MPI_dijkstra_path_algo
Author : Rohan Lagare

two programs, dijkstra_mpi and dijkstra_omp/dijkstra_acc, 
that will take as input a single file (an adjacency matrix) and a number of random 
input source vertex IDs to test and output to a file with the weights of a shortest 
path from the source vertex 0 to every other vertex. As the names suggest, the first
program will use MPI and the second OpenMP or OpenACC for parallelizing the 
algorithm. The input file will consist of n+1 lines, with the first line containing only 
the number n, and each other line containing n values (i.e., a dense matrix), with 
each value representing the weight of the edge. You should assume that the values 
in the input file are represented as quad-precision floats. An edge that does not 
exist in the graph is represented by the value inf. Your output file should also 
follow this convention.


The program takes the following parameters:
<PROGRAM> <graph> <num_sources> <nthreads> [<output_file>]

graph: input graph
num_sources: no of random source searches
output_file: B output file for result of program
num_threads : no. of threads for parallel block

Parallel Algorithm

In parallel approach, we are parallelizing the loop where we calculate local minimum, to 
divide the work between each thread as shown in fig 1 below. We are dividing rows of L
between different threads, so each thread will get bunch of elements of L, and it will calculate 
local minimum for it. After calculating all the local minimums for each thread, we loop through 
all the thread minimums to get local minimum.

With just that we have parallelized our serial program, but if we use the one min variable as it 
is, there might be data race between different threads, and we might not get correct min values 
due to random execution of loop. And the result L will be incorrect. We can use the ordered
OMP clause, but it will significantly impact our runtime as it will synchronize the threads, 
increasing overhead.

To tackle this, we are replacing the above min variable with array of float_int type, so that for 
each thread, we would have separate element, as shown in fig 3 below.

So, at the end of 1st for parallel block, we will calculate local minimum from different thread 
minimums.

After this Loop through (2nd parallel for) all the nodes and update distances for all by checking if 
current minimum plus distance from current node to minimum is less than that of l.
After looping, free all the unnecessary objects and return result array l.


Execution Time and speedup

We have executed serial version of program for few data sets. We are comparing above serial 
time parallel version calculating the speedup.
We can see 2-7X speedup compared to serial program depending on problem size.
