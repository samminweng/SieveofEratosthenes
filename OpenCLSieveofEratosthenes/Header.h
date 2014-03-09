#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdlib.h> /*Use the malloc.*/
#include <string.h> /*String.h manipulate C strings and array.*/
#include <errno.h>/*errno.h contains the definitions of the errno values.*/
#include <ctype.h>/*ctype contains 'isalpha' and 'isdigit' methods. */
//#include <LIMITS.H>/*The limit for integer types.*/
#include <math.h>/*Calculate the power of a number.*/
#include <locale.h>/*Set the locale.*/
#include <time.h>/*Get the current time.*/
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define SUCCESS 1
#define NOT_SUCCESS -1

typedef enum memoryMode { TRADITIONAL, PINNED } MemeoryMode; // the memory type
typedef enum benchmarkMode { Global, MaxLimit, Limit } BenchmarkMode; // the memory type
typedef enum graphicCard { Intel, NVIDIA, AMD} GraphicCard; // the memory type

typedef struct {
	int limit;
	int workgroupSize;
	int block_size;
	int numberOfPrimes;
} Benchmark;