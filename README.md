#Simple cache simulator program.

Files:
        
    cache-sim.c - the source file of the main cache simulator program
    csim.h      - the library file of cache constants and finctions
    bsim.c      - the original source file of the program [M. Smotherman]
    project.txt - the original project assignment details [M. Smotherman]
    sc10k.txt   - a test snippet of a benchmark file of cache references
    README.md   - this readme file for the cache simulator project

#Project Description:

Program:    cache-sim.c
Title:      Simple Cache Simulation
Author:     Phillip Yommer
Course:     CPSC 3300
Problem:    Programming Assignment 2
Date:       04/23/2018

Purpose:

- Simulates the cache operations for a specified benchmark output
file and determines the cache hits, misses, and corresponding hit
ratio.

Compile:

     gcc -Wall cache-sim.c -o cache-sim

Run:

     ./cache-sim [-<line-size>] [{-OPTION <value>}] < <filename>

Options:

     -s  - specify the cache size (in KB), default 32 KB
     -b  - specify the number of cache banks, default 8-way
     -l  - specify the line size (in bytes), default 64 bytes

Benchmark File:

- The filename used in the command line is used to read a sequence
of cache references logged from a benchmark on a program. This
file contains the cache addresses of the benchmark, and this
program simulates the references for the desired cache
specifications to determine the cache hit ratio of the benchmark
on the specified cache.

- The file "sc10k.txt" contains a snippet of the originak "sc10k"
file which contained a 10k sequence of cache references. This can
be used for testing.

Cache Initialization:

- The cache size, number of banks, and line size can be specified
via the option flags ('-s', '-b', '-l') in the command line
arguments, followed by the integer value corresponding to the
specified option.

                         default       specified
         ----------------------------------------
         cache size      32 [KB]         s
         # of banks      8               b
         line size       64 [bytes]      l

- The bank size, number of lines per bank, and the total number of
lines in the cache are calculated from the specified or default
cache specification values.

         ----------------------------------------
         # of lines  = cache size / line size
         bank size   = cache size / # of banks
         lines/bank  = bank size / line size

- The cache specifications are used to initialize a simulated cache
of s size, b-way set associative, with l size lines, and s/(b*l)
lines per bank (set), with all values initialized to zero.

Cache Simulation:

- The program reads each line of the benchmark file until an EOF and
simulates each reference on the cache to by updating the cache
data and control lines. For each reference, the program determines
if it would be a cache hit or cache miss, and increments the
appropriate counters.

- Once the EOF is read, the final cache contents are displayed and
the number of references, number of hits, and the hit ratio are
displayed.

Notes:

- The line size can alternatively be specified by preceding the
desired line size integer with a '-' in the command line
arguments (i.e.: ./cache-sim -16).

- All specfied cache specification values are required to be some
power of two, such that log_2(value) is an integer, otherwise
the default value is used.

- The default cache is a 32 KB 8-way set-associative cache with
64 lines per bank (set) and a line size of 64 bytes.

#Program Modification:

    This software is a modification of Mark Smotherman's "bsim.c"
    demonstration source code for the project described in "project.txt",
    released as follows:

    simple cache simulation

    CPSC 3300 / Clemson University / Fall 2014

    32 KiB eight-way set-associative cache

      512 total lines, 64 lines/bank, 64 bytes/line

    true LRU replacement using access count as a timestamp

    input: 32-bit addresses (read as hex values)
    output: cache stats

#Project Assignment:

    CPSC 3300 - Spring 2018 - Project 3 Assignment

      Due date:         before midnight on the night of Friday, April 27
      Submission:       use handin.cs.clemson.edu to turn in a source file
      Late penalty:     10% off per day late, up to five days
      Tools needed:     gcc (or other language)
      Concepts needed:  set associative caches

      This is an individual assignment.


    A simple cache simulator is given (bsim.c) that reads in byte addresses
    from a file and prints the cache miss count and hit rate. The simulator
    uses a 64 byte line size for an 8-way set-associative cache with total
    size of 32 KiB.

    The input will be redirected on the command line from a test file. The
    test file will consist of 8-digit hex values, one per line, that are
    32-bit memory addresses taken from the memory access trace for a
    benchmark program. For example, the first five lines of the test file
    sc10k are:

        00002020
        00002024
        ffffe860
        00002028
        0000202c

    You task is to revise the simulator to accept line size values of 4, 8,
    16, 32, and 64 as a command line parameter. The value will be specified
    as -4, etc., so that the simulator can be run with command lines of

	    ./a.out -4 < sc10k
	    ./a.out -8 < sc10k
	    ./a.out -16 < sc10k

    etc., where < is input file redirection and sc10k is the name of the
    input file. (The simulator must conform to this usage pattern to receive
    full credit. E.g., you must not change the program to read the test file
    name as a command line parameter nor to prompt the user to enter the file
    name.)

    Line sizes other than 4, 8, 16, 32, and 64 are invalid and will not be
    tested.

    Regardless of the line size value chosen, the simulated cache must remain
    as 32 KiB in total size. (That is, the choice of a 32 byte line size must
    not somehow result in simulating only a 16 KiB cache.) This allows the
    line size to be studied as an independent design choice.

    For the test files gcc10k, sc10k, and swm10k, the miss counts are:

	    line	----- trace file -----
	    size	gcc10k	sc10k	swm10k
	    4	1803	775	1951
	    8	1017	431	1052
	    16	613	257	600
	    32	364	161	354
	    64	214	102	214


