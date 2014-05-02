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
#include <windows.h>/*Set the environmental variable.*/
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define SUCCESS 1
#define NOT_SUCCESS -1

//Declare nested structures.
typedef struct {	
	//levels
	int limit;
	int workgroupsize;
	int blocksize;
	int result;
	//Global Size
	int global_size;

} Parameters;


typedef struct {
	//Benchmark Configuration
	char* vendor;
	cl_device_type device_type;
	char* kernel_name;
	
	//Time
	double* diff;
	int repeats;
	Parameters parameters;
} Benchmark;


