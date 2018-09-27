/*-------------------------------PREPROCESSOR---------------------------------*/
/* -- include libraries -- */
#include <stdlib.h>
#include <stdio.h>

/* -- defined constants -- */
#define SIZE  32                // max cache size [KB]
#define KB    1024              // bytes per kilo-byte
#define LINES 64                // default lines per bank
#define BANKS 8                 // default banks per cache
#define AIO   12                // address index shift offset
#define ATO   18                // address tag shift offset

/*---------------------------STRUCTURE-DEFINITIONS----------------------------*/
// cache specifications data
struct spec
{
    int size;           // max cache size [KB]
    int caches;         // number of caches (levels)
    int banks;          // banks per cache (sets)
    int lines;          // lines per bank
    int bytes;          // bytes per line (line size)
    int offset;         // instruction tag and index bit offset
};

// cache simulation data
struct data
{
    int access;         // access counter == timestamp
    int address;        // 32-bit address value read as input
    int tag;            // tag bits from address
    int index;          // index bits from address == set id
    int hits;           // cache hits counter
    int misses;         // cache misses counter
    int bank;           // current cache bank in use
};

// cache line flag arrays
struct line
{
    int *lastused;      // access count for a given line
    int *valid;         // valid bit for a given line, 0=invalid, 1=valid
    int *tag;           // tag bits for a given line
};

/*---------------------------FUNCTION-PROTYTYPES------------------------------*/
// parser functions
int get_value(int mode, char *argv);

// initialization functions
int *init_array(int cols);
void init_spec(int *values, struct spec *spec);
void read_spec(struct spec *spec, int argc, char *argv[]);
void init_data(struct data *data);
struct line *init_line(int banks, int size);

// search functions
int hit_search(struct spec spec, struct data data, struct line *line);
int rep_search(struct spec spec, struct data data, struct line *line);
int old_search(struct spec spec, struct data data, struct line *line);

// misc math functions
int pow_2(int power);
int log_2(int value);

// printer functions
void print_stats(int hits, int misses);
void print_spec(struct spec spec);
void print_data(struct data data);
void print_usage(void);
void print_error(int mode, char *argv);

/*---------------------------FUNCTION-DEFINITIONS-----------------------------*/

/* -- parser functions ------------------------------------------------------ */

/** get_value()
 *
 * Purpose: returns the integer value of the specified command line argument,
 *          converting KB to bytes if specified by the mode.
 *
 * Inputs:  mode - specifies if the argument is in KB or bytes
 *          argv - a pointer to the command line argument to parse
 *
 * Return:  the integer value of the command line argument.
 *
 */
int get_value(int mode, char *argv)
{
    // get value from command line argument
    const char *arg = (argv[0] == '-') ? &argv[1] : argv;
    int size = atoi(arg);

    // verify cache size
    if (log_2(size) < 0)
        print_error(mode, argv);

    // return cache size
    if (mode == 0)
        return KB*size;

    // return bank or line size
    return size;
}

/* -- initializer functions ------------------------------------------------- */

/** init_array()
 *
 * Purpose: returns a pointer to an allocated memory array with the specified
 *          number of rows and columns.
 *
 * Inputs:  cols - the number of columns to allocate in the array
 * Return:  a pointer to an allocated memory array of size cols
 *
 * Requires:    cols > 0;
 * Ensures:     result[i] = 0, for all 0 <= i < cols; |result| = cols;
 *
 */
int *init_array(int cols)
{
    int *array = malloc(cols*sizeof(int));
    if (array == NULL)
    {
        printf("ERROR! Failed to allocate array of size %d.\n", cols);
        exit(-1);
    }
    int i=0;
    for (i=0; i<cols; i++)
        array[i] = 0;
    return array;
}

/** init_spec()
 *
 * Purpose: initializes the cache spec structure fields to the default values.
 *
 * Inputs:  spec - a pointer to the preallocated cache spec structure
 *
 * Requires:    spec != null;
 * Ensures:     spec.x = DEFAULT, for all x in spec{};
 *
 */
void init_spec(int *values, struct spec *spec)
{
    spec->size = (values[0] > 0) ? values[0] : SIZE;
    spec->caches = (values[1] > 0) ? values[1] : 1;
    spec->banks = (values[2] > 0) ? values[2] : BANKS;
    spec->lines = (values[3] > 0) ? values[3] : LINES;
    spec->bytes = (values[4] > 0) ? values[4] : spec->size/(spec->banks*spec->lines);
    spec->offset = (values[5] > 0) ? values[5] : log_2(spec->bytes);
}

/** read_spec()
 *
 * Purpose: returns a cache spec structure initialized to the values specified
 *          by the command line arguments.
 *
 * Inputs:  spec - a pointer to the preallocated cache spec structure
 *          argc - the number of command line arguments, from main
 *          argv - the command line arguments as an array, from main
 * Return:  a cache spec structure initialized to the values specified by the
 *          command line arguments.
 *
 * Requires:    spec != null; |argv| = argc;
 * Ensures:     the cache spec fields are initialized corresponding to the
 *              command line arguments.
 *
 */
void read_spec(struct spec *spec, int argc, char *argv[])
{
    // initialize cache specs to default
    spec->size = SIZE*KB;
    spec->caches = 1;
    spec->banks = BANKS;
    spec->bytes = LINES;

    // set the cache specs from command line arguments
    int i=0;
    for (i=1; i<argc; i+=2)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
                case 's':
                {
                    spec->size = get_value(0, argv[i+1]);
                    break;
                }
                case 'b':
                {
                    spec->banks = get_value(1, argv[i+1]);
                    break;
                }
                case 'l':
                {
                    spec->bytes = get_value(2, argv[i+1]);
                    break;
                }
                default:
                {
                    if (i == argc-1)
                        spec->bytes = get_value(2, argv[i]);
                    break;
                }
            }
        }
    }

    // determine lines per bank and address offset
    spec->lines = spec->size/(spec->banks*spec->bytes);
    spec->offset = log_2(spec->lines);
}

/** init_data()
 *
 * Purpose: initializes the cache data structure fields to zero.
 *
 * Inputs:  data - a pointer to the preallocated cache data structure
 *
 * Requires:    data != null;
 * Ensures:     data.x = 0, for all x in data{};
 *
 */
void init_data(struct data *data)
{
    data->access = 0;
    data->address = 0;
    data->tag = 0;
    data->index = 0;
    data->hits = 0;
    data->misses = 0;
    data->bank = 0;
}

/** init_line()
 *
 * Purpose: initializes the cache data structure fields to zero.
 *
 * Inputs:  data - a pointer to the preallocated cache data structure
 *
 * Requires:    data != null;
 * Ensures:     data.x = 0, for all x in data{};
 *
 */
struct line *init_line(int banks, int size)
{
    struct line *line = malloc(banks*size*sizeof(struct line));
    if (line == NULL)
    {
        printf("ERROR! Failed to allocate line of size %d.\n", banks*size);
        exit(-1);
    }
    int i;
    for (i=0; i<banks; i++)
    {
        line[i].lastused = init_array(size);
        line[i].valid = init_array(size);
        line[i].tag = init_array(size);
    }
    return line;
}


/* -- line search functions ------------------------------------------------- */

/** hit_search()
 *
 * Purpose: searches the cache for a line with a cache hit.
 *
 * Inputs:  banks - the nubmer of sets in the cache (bank-way set-associative)
 *          index - the index bits from address
 *          tag   - the tag bits from address
 *          valid - the pre-allocated valid bits array for the line
 *          tag   - the pre-allocated tag bits array for the line
 * Return:  the array index for the line with the cache hit.
 *
 * Requires:    banks > 0; lines > 0; |valid| = banks*lines; |tag| = banks*lines;
 * Ensures:     result : valid[result] = 1 and tag[result] = tag;
 *
 */
int hit_search(struct spec spec, struct data data, struct line *line)
{
    int i=0;
    for(i=0; i<spec.banks; i++)
        if(line[i].valid[data.index] && (data.tag == line[i].tag[data.index]))
            return i;
    return -1;
}

/** rep_search()
 *
 * Purpose: searches the cache for the replacement line.
 *
 * Inputs:  banks - the nubmer of sets in the cache (bank-way set-associative)
 *          index - the index bits from address
 *          valid - the pre-allocated valid bits array for the line
 * Return:  the array index for the replacement line.
 *
 * Requires:    banks > 0; lines > 0; |valid| = banks*lines;
 * Ensures:     result : valid[result] = 0 and valid[i] = 1, for all i < result;
 *
 */
int rep_search(struct spec spec, struct data data, struct line *line)
{
    int i=0;
    for(i=0; i<spec.banks; i++)
        if(!line[i].valid[data.index])
            return i;
    return -1;
}

/** old_search()
 *
 * Purpose: searches the cache for the oldest line in the set.
 *
 * Inputs:  banks - the nubmer of sets in the cache (bank-way set-associative)
 *          index - the index bits from address
 *          lastused - the pre-allocated access count array for the line
 * Return:  the array index for the oldest line in the set.
 *
 * Requires:    banks > 0; lines > 0; |lastused| = banks*lines;
 * Ensures:     result : lastused[result] < lastused[i], for all i != result;
 *
 */
int old_search(struct spec spec, struct data data, struct line *line)
{
    int bank = 0;
    int i=0;
    for(i=1; i<spec.banks; i++)
        if(line[i].lastused[data.index] < line[bank].lastused[data.index])
            bank = i;
    return bank;
}


/* -- miscellaneous math -- */

/** pow_2()
 *
 * Purpose: returns the value of 2 raised to the specified power.
 *
 * Inputs:  power - the power exponent to raise 2 to
 * Return:  the value of 2 raised to the power
 *
 * Requires:    power >= 0;
 * Ensures:     result = 2^power;
 *
 */
int pow_2(int power)
{
    if (power < 0)
        return -1;
    int result = 1;
    int i=0;
    for (i=0; i<power; i++)
        result *= 2;
    return result;
}

/** log_2()
 *
 * Purpose: returns the log base 2 of the specified value.
 *
 * Inputs:  value - the value to take the log base 2 of
 * Return:  the log base 2 of the value
 *
 * Requires:    value > 0; value mod 2 = 0 or value = 1;
 * Ensures:     result = log_2(value);
 *
 */
int log_2(int value)
{
    if (value < 1)
        return -1;
    int result = value/2;
    int i=0;
    while (result > 0)
    {
        result /= 2;
        i++;
    }
    return i;
}

/* -- printer functions ----------------------------------------------------- */

/** print_stats()
 *
 * Purpose: print misses, total references, and hit ratio.
 *
 * Inputs:  hits    - the number of cache hits
 *          misses  - the number of cache misses
 *
 * Requires:    hits + misses > 0;
 * Ensures:     the number of cache misses, total cache references,
 *              and hit ratio are printed to stdout.
 *
 */
void print_stats(int hits, int misses)
{
    printf("references:\t%d\n", hits + misses);
    printf("hits:\t\t%d\n", hits);
    printf("misses:\t\t%d\n", misses);
    printf("hit rate:\t%-5.2f%%\n\n", ((float) (hits/(hits + misses)))*100.0);
}

/** print_spec()
 *
 * Purpose: prints the specified cache specs to stdout.
 *
 * Inputs:  spec - the cache specs data structure
 *
 * Requires:    spec != null;
 *
 */
void print_spec(struct spec spec)
{
    printf("total size:\t\b%4d KB\n", spec.size/KB);
    printf("cache levels:\t%3d\n", spec.caches);
    printf("banks (sets):\t%3d\n", spec.banks);
    printf("bank lines:\t%3d\n", spec.lines);
    printf("line size:\t%3d\n", spec.bytes);
    printf("bit offset:\t%3d\n\n", spec.offset);
}

/** print_data()
 *
 * Purpose: prints the specified cache data to stdout.
 *
 * Inputs:  data - the cache data data structure
 *
 * Requires:    data != null;
 *
 */
void print_data(struct data data)
{
    printf("access counter:\t%5d\n", data.access);
    printf("32-bit address:\t    0x%08x\n", data.address);
    printf("address tag:\t    0x%x\n", data.tag);
    printf("address index:\t    0x%x\n", data.index);
    printf("hits counter:\t%5d\n", data.hits);
    printf("miss counter:\t%5d\n", data.misses);
    printf("current bank:\t%5d\n\n", data.bank);
}

/** print_usage()
 *
 * Purpose: prints the usage menu.
 *
 */
void print_usage(void)
{
    printf("Usage:\t./cache-sim [{-OPTION value}] < <filename>\n\n");
    printf("Where -OPTION is one of:\n\n");
    printf("\t-s  - to specify the cache size (in KB)\n");
    printf("\t-b  - to specify the number of blocks\n");
    printf("\t-l  - to specify the line size (in B)\n");
    printf("and <filename> is the name of the input file.\n\n");
    printf("Example:\t./cache-sim -16 < sc10k\n\n");
    printf("\tThis example will simulate a simple 8-way set-associative");
    printf(" 32 KB cache with a 16 byte line size, using the cache addresses");
    printf(" of hits and misses recorded during a benchmark of a program and");
    printf(" stored in the file 'sc10k'.\n\n");
    printf("Example:\t./cache-sim -s 64 -b 4 -l 32 < sc10k\n\n");
    printf("\tThis example will simulate a 4-way set-associative 64 KB cache");
    printf(" with a 32 byte line size, of the benchmark 'sc10k'.\n\n");
}

/** print_error()
 *
 * Purpose: prints the error message for the specified mode and argument.
 *
 */
void print_error(int mode, char *argv)
{
    switch (mode)
    {
        case 0:
        {
            printf("ERROR! Invalid cache size (%s).\n\n", argv);
            break;
        }
        case 1:
        {
            printf("ERROR! Invalid number of banks (%s).\n\n", argv);
            break;
        }
        case 2:
        {
            printf("ERROR! Invalid line size (%s).\n\n", argv);
            break;
        }
        default:
        {
            printf("ERROR! Invalid command line argument (%s).\n\n", argv);
            break;
        }
    }
    print_usage();
    exit(-1);
}

