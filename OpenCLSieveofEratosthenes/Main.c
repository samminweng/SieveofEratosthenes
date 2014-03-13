#include "Header.h"
#define MAXITERATION 30	  
#define WORKGROUPSIZE 64

cl_int deﬁne_platform(cl_device_type deviceType, GraphicCard device);
int Load_program(KernelType kerneltype, int isVerbose);
int Sieve_OpenCL(int max, int block_size, int workgroupSize, MemeoryMode memMode);
cl_int free_OpenCL();
void writeBenchmarksToCSV(double diff[], int size, int max, int workgroupSize, int block_size, KernelType kerneltype);

//The number of primes.
int results[] ={0, 4, 25, 168, 1229, 9592, 78498, 664579, 5761455, 50847534, 98222287};
GraphicCard device = NVIDIA;
BenchmarkMode benchmarkMode = Limit;
MemeoryMode mode = TRADITIONAL;
KernelType kerneltype = ULong;
int isVerbose = 0;
int numberOfBenchmarks;
Benchmark *benchmarks;

void generateBenchmark(BenchmarkMode mode){
	int limit;
	int res_index;
	int block_size;	
	int index;

	if (mode == Global){
		limit = 100 * 1000 * 1000;
		/*The maximal block size is 128 because the array size in the kernel is at most 128.*/
		numberOfBenchmarks = 48;
		benchmarks = (Benchmark *)malloc(numberOfBenchmarks*sizeof(Benchmark));
		//Make a list to store all the parameters for benchmarking the global size.
		index = 0;
		//Start from block size of 25 for the limit of 100 million. The maximal block size is 128.
		for (block_size = 30; block_size<=500; block_size+=10){
			benchmarks[index].workgroupSize = WORKGROUPSIZE;
			benchmarks[index].limit = limit;		
			benchmarks[index].block_size = block_size;
			benchmarks[index].numberOfPrimes = results[8];
			index++;
		}		
	}else if(mode == MaxLimit){
		//Make a list to store the parameters for benchmarking the limit.
		block_size = 64;//The optimal block size.
		limit = 1000*1000*1000;
		numberOfBenchmarks = 1;
		benchmarks = (Benchmark *)malloc(numberOfBenchmarks*sizeof(Benchmark));
		res_index = log10((float)limit);	
		benchmarks[0].limit = limit;
		benchmarks[0].workgroupSize = WORKGROUPSIZE;
		benchmarks[0].block_size = block_size;
		benchmarks[0].numberOfPrimes = results[res_index]; 
		
	}
	else{
		//Make a list to store the parameters for benchmarking the limit.
		block_size = 64;//The optimal block size.
		limit = 1000*1000*1000;
		numberOfBenchmarks = log10((float)limit);
		benchmarks = (Benchmark *)malloc(numberOfBenchmarks*sizeof(Benchmark));
		res_index = 1;		
		for (index = 0; index <numberOfBenchmarks; index++){
			limit = pow((float)10, res_index);
			benchmarks[index].limit = limit;
			benchmarks[index].workgroupSize = WORKGROUPSIZE;
			benchmarks[index].block_size = block_size;
			benchmarks[index].numberOfPrimes = results[res_index];
			res_index++;
		}
	}
}


//Defines the entry point for the console application.
int main(int argc, char *args[]){		
	int numberOfprimes;
	int iter;
	cl_int err;
	clock_t start, end;
	double diff[MAXITERATION];
	Benchmark benchmark;
	int index, limit, block_size, workgroupsize;
	

	if (argc < 2){
		printf("Please input the benchmark arugment, for example, 'global', 'limit' or 'maxlimit'.");
		return;
	}else{
		index=1;
		while(index<= argc) {

			if (strncmp(args[index-1], "global", 6) == 0)
				benchmarkMode = Global;
			if(strncmp(args[index-1], "maxlimit", 6) == 0)
				benchmarkMode = MaxLimit;
			if(strncmp(args[index-1], "limit", 6) == 0)
				benchmarkMode = Limit;

			if(strncmp(args[index-1], "traditional", 4) == 0)
				mode = TRADITIONAL;
			if(strncmp(args[index-1], "pinned", 4) == 0)
				mode = PINNED;
			if(strncmp(args[index-1], "verbose", 4) == 0)
				isVerbose =1;

			if(strncmp(args[index-1], "chararray", 7) == 0)
				kerneltype = CharArray;
			if(strncmp(args[index-1], "sharedarray", 7) == 0)
				kerneltype = SharedCharArray;
			if(strncmp(args[index-1], "ulong", 4) == 0)
				kerneltype = ULong;
			if(strncmp(args[index-1], "sharedulongarray", 7) == 0)
				kerneltype = SharedULongArray;

			if (strncmp(args[index-1], "Intel", 5) == 0)
				device = Intel;
			if(strncmp(args[index-1], "NVIDIA", 6) == 0)
				device = NVIDIA;
			if(strncmp(args[index-1], "AMD", 3) == 0)
				device = AMD;



			index++;

		}
	}

	generateBenchmark(benchmarkMode);
	// Benchmark the OpenCL Sieve program.
	for (index = 0; index < numberOfBenchmarks; index++){
		benchmark = benchmarks[index];
		block_size = benchmark.block_size;
		workgroupsize = benchmark.workgroupSize;
		limit = benchmark.limit;
		for (iter = 0; iter < MAXITERATION; iter++){
			/*Get the starting time.*/
			start = clock();
			numberOfprimes = 0;
			/*Create and initialize the OpenCL objects.*/
			//deﬁne_platform(CL_DEVICE_TYPE_CPU, "Intel");
			deﬁne_platform(CL_DEVICE_TYPE_GPU, device);

			/*Load the program and create the kernel.*/
			err = Load_program(kerneltype, isVerbose);

			//Partition one huge work (i.e. 1 billion) into a number of fine-grained tasks in 1-million size.
			numberOfprimes = Sieve_OpenCL(benchmark.limit, benchmark.block_size, benchmark.workgroupSize, mode);

			// release OpenCL resources
			err = free_OpenCL();
			printf("The OpenCL Sieve function is complete.\n");	

			//Check the number of primes is matched with the result.
			if(numberOfprimes != benchmark.numberOfPrimes){
		    		printf("The result (%d) is wrong. The number of primes <= %d is %d", numberOfprimes, benchmark.limit, benchmark.numberOfPrimes);
				system("pause");
			}

			/*Get the ending time.*/
			end = clock();
			/* The total number of primes <= the max is here(http://primes.utm.edu/howmany.shtml) */
			/*Get the difference of starting and ending time.*/
			diff[iter] = (double)(end - start) / (CLOCKS_PER_SEC / 1000);
			printf("\nIn the %dth iteration, the total number of primes upto %d is %d\n", iter, benchmark.limit, numberOfprimes);
			printf("The %dth iteration takes %.2f milliseconds on average.\n", iter, diff[iter]);
		}
		writeBenchmarksToCSV(diff, MAXITERATION, limit, workgroupsize, block_size, kerneltype);
	}
	free(benchmarks);
	//system("pause");
	return 0;
}