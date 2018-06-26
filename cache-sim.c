// cache-sim.c - simple cache simulator
/**
 *
 * Program: cache-sim.c
 * Title:   Simple Cache Simulation
 * Author:  Phillip Yommer
 * Course:  CPSC 3300
 * Problem: Programming Assignment 2
 * Date:    04/23/2018
 *
 *
 * Purpose:
 *
 *      Simulates the cache operations for a specified benchmark output
 *      file and determines the cache hits, misses, and corresponding hit
 *      ratio.
 *
 * Compile:
 *
 *      gcc -Wall cache-sim.c -o cache-sim
 *
 * Run:
 *
 *      ./cache-sim [-<line-size>] [{-OPTION <value>}] < <filename>
 *
 * Options:
 *
 *      -s  - specify the cache size (in KB), default 32 KB
 *      -b  - specify the number of cache banks, default 8-way
 *      -l  - specify the line size (in bytes), default 64 bytes
 *
 * Benchmark File:
 *
 *      - The filename used in the command line is used to read a sequence
 *        of cache references logged from a benchmark on a program. This
 *        file contains the cache addresses of the benchmark, and this
 *        program simulates the references for the desired cache
 *        specifications to determine the cache hit ratio of the benchmark
 *        on the specified cache.
 *
 *      - The file "sc10k.txt" contains a snippet of the originak "sc10k"
 *        file which contained a 10k sequence of cache references. This can
 *        be used for testing.
 *
 * Cache Initialization:
 *
 *      - The cache size, number of banks, and line size can be specified
 *        via the option flags ('-s', '-b', '-l') in the command line
 *        arguments, followed by the integer value corresponding to the
 *        specified option.
 *
 *                              default       specified
 *              ----------------------------------------
 *              cache size      32 [KB]         s
 *              # of banks      8               b
 *              line size       64 [bytes]      l
 *
 *      - The bank size, number of lines per bank, and the total number of
 *        lines in the cache are calculated from the specified or default
 *        cache specification values.
 *
 *              ----------------------------------------
 *              # of lines  = cache size / line size
 *              bank size   = cache size / # of banks
 *              lines/bank  = bank size / line size
 *
 *      - The cache specifications are used to initialize a simulated cache
 *        of s size, b-way set associative, with l size lines, and s/(b*l)
 *        lines per bank (set), with all values initialized to zero.
 *
 * Cache Simulation:
 *
 *      - The program reads each line of the benchmark file until an EOF and
 *        simulates each reference on the cache to by updating the cache
 *        data and control lines. For each reference, the program determines
 *        if it would be a cache hit or cache miss, and increments the
 *        appropriate counters.
 *
 *      - Once the EOF is read, the final cache contents are displayed and
 *        the number of references, number of hits, and the hit ratio are
 *        displayed.
 *
 * Notes:
 *
 *      - The line size can alternatively be specified by preceding the
 *        desired line size integer with a '-' in the command line
 *        arguments (i.e.: ./cache-sim -16).
 *
 *      - All specfied cache specification values are required to be some
 *        power of two, such that log_2(value) is an integer, otherwise
 *        the default value is used.
 *
 *      - The default cache is a 32 KB 8-way set-associative cache with
 *        64 lines per bank (set) and a line size of 64 bytes.
 *
 ******************************************************************************
 *
 * This software is a modification of Mark Smotherman's "bsim.c"
 * demonstration source code for the project described in "project.txt",
 * released as follows:
 *
 * simple cache simulation
 *
 * CPSC 3300 / Clemson University / Fall 2014
 *
 * 32 KiB eight-way set-associative cache
 *
 *   512 total lines, 64 lines/bank, 64 bytes/line
 *
 * true LRU replacement using access count as a timestamp
 *
 * input: 32-bit addresses (read as hex values)
 * output: cache stats
 *
 ******************************************************************************
 *
 */
/*-------------------------------PREPROCESSOR---------------------------------*/
/* -- include libraries -- */
#include <stdlib.h>
#include <stdio.h>
#include "csim.h"

/*-------------------------------MAIN-FUNCTION--------------------------------*/
int main(int argc, char *argv[])
{
    printf("cache-sim.c - simple cache simulation\n\n");

    // initialize cache specifications
    struct spec spec;
    read_spec(&spec, argc, argv);
    printf("cache specs:\n\n");
    print_spec(spec);

    // allocate and initialize cache line arrays
    struct line *line = init_line(spec.banks, spec.lines);

    // initialize cache simulation data
    struct data data;
    init_data(&data);
    printf("initial cache data:\n\n");
    print_data(data);

    // read input file lines as cache memory addresses
    int address = 0;                    // temp address
    while(scanf("%x", &address) != EOF)
    {
        data.address = address;
        data.access++;
        data.index = (data.address >> (AIO - spec.offset)) & (spec.lines-1);
        data.tag = data.address >> (ATO - spec.offset);

        // search for hit
        data.bank = hit_search(spec, data, line);
        if(data.bank != -1)
        {
            data.hits++;
            line[data.bank].lastused[data.index] = data.access;
        }
        else
        {
            data.misses++;

            // search for replacement
            data.bank = rep_search(spec, data, line);

            if(data.bank == -1)
                data.bank = old_search(spec, data, line);

            // use previously invalid line or oldest
            line[data.bank].valid[data.index] = 1;
            line[data.bank].tag[data.index] = data.tag;
            line[data.bank].lastused[data.index] = data.access;
        }
    }

    // display stats
    printf("final cache data:\n\n");
    print_data(data);
    printf("cache hit rate:\n\n");
    print_stats(data.hits, data.misses);

    // free allocated memory
    int i;
    for (i=0; i<spec.banks; i++)
    {
        free(line[i].lastused);
        free(line[i].valid);
        free(line[i].tag);
    }
    free(line);

    return 0;
}

