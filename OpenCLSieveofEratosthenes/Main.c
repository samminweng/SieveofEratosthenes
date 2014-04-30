#include "Header.h"
#define MAXITERATION 30
#define VARNAME TEXT("CUDA_CACHE_DISABLE")//Set the environment variable.

enum Vendor {AMD, Intel, NVIDIA};
<<<<<<< HEAD
char* Vendor_Name[]={"Advanced Micro Devices, Inc.", "Intel(R) Corporation", "NVIDIA Corporation"}; 
=======
char* Vendor_Name[]={"Advanced Micro Devices, Inc.", "Intel(R) Corporation", ""}; 
>>>>>>> b1fdb5c5388df077496dd9e0f910653fe024f1d4
char* DS_Files[]={"Kernel_arrayOfchars.cl", "Kernel_sharedArrayOfChars.cl", "Kernel_SharedArrayOfULongs.cl", "Kernel_unsignedLong.cl"};
char* ArraySize_Files[]={"Kernel_arrayOfChars_64.cl", "Kernel_arrayOfChars_128.cl", "Kernel_arrayOfChars_256.cl",
	"Kernel_arrayOfChars_512.cl","Kernel_arrayOfChars_1024.cl","Kernel_arrayOfChars_2048.cl","Kernel_arrayOfChars_4096.cl"};

//The total number of primes associated with the range of upper limit.
int results[] ={0, 4, 25, 168, 1229, 9592, 78498, 664579, 5761455, 50847534, 98222287};

int deﬁne_platform(cl_device_type device_type, char* vendor_name);
int Load_program(char* filename);
int Sieve_OpenCL(int limit, int arraysize, int workgroupsize);
int free_OpenCL();
void writeBenchmarksToCSV(Benchmark benchmark);

//The benchmark list.
Benchmark *benchmarks;

int parseArgument(int argc, char *args[]){
	char *str, *flag, *value;
	int  i, index, isVerbose, limit, arraysize, numberOfBenchmarks, workgroupsize;
	Parameters params;

	if(argc != 2){
		printf("Please specify the arguments: '-b'\n");
		return NOT_SUCCESS;
	}

	i = 1;
	while(i<= argc) {
		str = args[i-1];
		//Split the argument into two words.
		flag = strtok(str, "=");
		value = strtok(NULL,"=");
		if(strncmp(flag, "-b", sizeof("-b"))==0){			
			if(strncmp(value, "datastructure", sizeof("datastructure")) == 0){
				//Benchmark the 4 kinds of data structures.				
				numberOfBenchmarks = 4;
				limit = 1000*1000*1000;
				arraysize = 64;
				workgroupsize = 64;
				benchmarks = (Benchmark *)malloc(numberOfBenchmarks*sizeof(Benchmark));
				//Fixed level
				for (index = 0; index <numberOfBenchmarks; index++){					
					//benchmarks[index].vendor = Vendor_Name[AMD];
<<<<<<< HEAD
					//benchmarks[index].vendor = Vendor_Name[Intel];
					benchmarks[index].vendor = Vendor_Name[NVIDIA];
=======
					benchmarks[index].vendor = Vendor_Name[Intel];
>>>>>>> b1fdb5c5388df077496dd9e0f910653fe024f1d4
					benchmarks[index].device_type = CL_DEVICE_TYPE_GPU;
					benchmarks[index].repeats = MAXITERATION;
					benchmarks[index].kernel_name = DS_Files[index];
					//Parameters
					params.limit = limit;
					params.workgroupsize = workgroupsize;
					params.arraysize = arraysize;//The array size is fixed to 64 for fairly comparing different data structures. 
					params.result = results[(int)log10((float)limit)]; 
					//Set the parameters.
					benchmarks[index].parameters =params;
				}				

			} else if (strncmp(value, "arraysize", sizeof("arraysize")) == 0){
				//Vary the array size for the kernel with array of chars.
				//The array size is 64, 128, ....1024, 2048 and 4096 
				int arraysizelist[]= {64,128,256,512, 1024,2048,4096};
				numberOfBenchmarks = 7;
				limit = 1000*1000*1000;
				workgroupsize = 64;
				benchmarks = (Benchmark *)malloc(numberOfBenchmarks*sizeof(Benchmark));
				//Fixed level
				for (index = 0; index <numberOfBenchmarks; index++){
					arraysize = arraysizelist[index]; 
					//benchmarks[index].vendor = Vendor_Name[AMD];
<<<<<<< HEAD
					//benchmarks[index].vendor = Vendor_Name[Intel];
					benchmarks[index].vendor = Vendor_Name[NVIDIA];
=======
					benchmarks[index].vendor = Vendor_Name[Intel];
>>>>>>> b1fdb5c5388df077496dd9e0f910653fe024f1d4
					benchmarks[index].device_type = CL_DEVICE_TYPE_GPU;
					benchmarks[index].repeats = MAXITERATION;
					benchmarks[index].kernel_name = ArraySize_Files[index];
					//Parameters
					params.limit = limit;
					params.workgroupsize = workgroupsize;
					params.arraysize = arraysize;//The array size is fixed to 64 for fairly comparing different data structures. 
					params.result = results[(int)log10((float)limit)]; 
					//Set the parameters.
					benchmarks[index].parameters =params;
				}

			} else if(strncmp(value, "workgroupsize", sizeof("workgroupsize")) == 0){
				//Vary the workgroupsize for the kernel with the array-of-chars data structure.
<<<<<<< HEAD
				//For AMD, the workgroup size is 64, 128, 256.
				//int wgslist[] = {64,128,256};
				//For Intel, the workgroup size is 64, 128, 256, 512.
				//int wgslist[] = {64,128,256,512};	
				//For NVIDIA the workgroup size is 64, 128, 256, 512, 1024.
				int wgslist[] = {64,128,256,512,1024};	
				numberOfBenchmarks = 5;
				limit = 1000*1000*1000;
				arraysize = 256;
=======
				//The workgroup size is 64, 128, 256.
				int wgslist[] = {64,128,256};				
				numberOfBenchmarks = 3;
				limit = 1000*1000*1000;
				arraysize = 64;
>>>>>>> b1fdb5c5388df077496dd9e0f910653fe024f1d4
				benchmarks = (Benchmark *)malloc(numberOfBenchmarks*sizeof(Benchmark));
				for (index = 0; index <numberOfBenchmarks; index++){
					workgroupsize = wgslist[index];
					//benchmarks[index].vendor = Vendor_Name[AMD];
<<<<<<< HEAD
					//benchmarks[index].vendor = Vendor_Name[Intel];
					benchmarks[index].vendor = Vendor_Name[NVIDIA];
					benchmarks[index].device_type = CL_DEVICE_TYPE_GPU;
					benchmarks[index].repeats = MAXITERATION;
					benchmarks[index].kernel_name = "Kernel_arrayOfChars_256.cl";
					//Parameters					
					params.limit = limit;
					params.workgroupsize = workgroupsize;
					params.arraysize = arraysize;
					params.result = results[(int)log10((float)limit)]; 
					//Set the parameters.
					benchmarks[index].parameters =params;
				}

			} else if(strncmp(value, "limit", sizeof("limit")) == 0){
				//Vary the limit for the kernel with the array-of-chars data structure.
				//and the optimal arraysize and workgroupsize.
				int limitList[] = {10,100,1000, 10*1000, 100*1000, 1000*1000, 10*1000*1000, 100*1000*1000, 1000*1000*1000};				
				numberOfBenchmarks = 9;
				//The optimal setting for AMD
				//workgroupsize = 256;
				//arraysize = 1024;
				//The optimal setting for Intel 
				workgroupsize = 64;
				arraysize = 256;
				//The optimal setting for NVIDIA
				workgroupsize = 64;
				arraysize = 256;
				benchmarks = (Benchmark *)malloc(numberOfBenchmarks*sizeof(Benchmark));
				for (index = 0; index <numberOfBenchmarks; index++){
					limit = limitList[index];
					//benchmarks[index].vendor = Vendor_Name[AMD];
					//benchmarks[index].vendor = Vendor_Name[Intel];
					benchmarks[index].vendor = Vendor_Name[NVIDIA];
					benchmarks[index].device_type = CL_DEVICE_TYPE_GPU;
					benchmarks[index].repeats = MAXITERATION;
					benchmarks[index].kernel_name = "Kernel_arrayOfChars_256.cl";
=======
					benchmarks[index].vendor = Vendor_Name[Intel];
					benchmarks[index].device_type = CL_DEVICE_TYPE_GPU;
					benchmarks[index].repeats = MAXITERATION;
					benchmarks[index].kernel_name = "Kernel_arrayOfChars.cl";
>>>>>>> b1fdb5c5388df077496dd9e0f910653fe024f1d4
					//Parameters					
					params.limit = limit;
					params.workgroupsize = workgroupsize;
					params.arraysize = arraysize;
					params.result = results[(int)log10((float)limit)]; 
					//Set the parameters.
					benchmarks[index].parameters =params;
				}

			}else {
				//Have not assigned the benchmark mode.
				printf("Please specify the benchmark mode: 'arraysize', 'datastructure' or 'limit'.\n");
				printf("For example, '-b=datastructure'.\n");
				return NOT_SUCCESS;
			}			
		}

		i++;

	}

	return numberOfBenchmarks;
}


//Defines the entry point for the console application.
int main(int argc, char *args[]){		
	int numberOfprimes, iter, index, limit, arraysize, workgroupsize, result, numberOfBenchmarks, repeats;	
	clock_t start, end;
	double diff;
	Benchmark benchmark; Parameters param;
	char* vendor; char* kenel_name;
	cl_device_type device;

	if((numberOfBenchmarks=parseArgument(argc,args)) == NOT_SUCCESS){
		printf("Error occurs while parsing the argument");
		return;
	}
	// Benchmark the OpenCL Sieve program.
	for (index = 0; index < numberOfBenchmarks; index++){
		//Get the benchmark parameters.
		benchmark = benchmarks[index];
		vendor = benchmark.vendor;
		device = benchmark.device_type;
		repeats = benchmark.repeats;
		kenel_name = benchmark.kernel_name;

		param = benchmark.parameters;
		arraysize = param.arraysize;
		workgroupsize = param.workgroupsize;
		limit = param.limit;
		result = param.result;
		benchmark.diff = (double *)malloc(repeats*sizeof(double));

		//Repeatedly run the benchmarking experiment.
		for (iter = 0; iter < repeats; iter++){
			/*Get the starting time.*/
			start = clock();
			numberOfprimes = 0;
			/*Create and initialize the OpenCL objects.*/
			if(deﬁne_platform(device, vendor) == NOT_SUCCESS){
				return NOT_SUCCESS;
			}

			/*Load the program and create the kernel.*/
			if(Load_program(kenel_name) == NOT_SUCCESS){
				return NOT_SUCCESS;
			}

			/* For NVIDIA, partition one huge work (i.e. 1 billion) into a number of fine-grained tasks in 1-million size.
			For AMD, no needs to partition the work because the display is not attached to AMD graphic card and thus
			the time limit can be ignored. */
			numberOfprimes = Sieve_OpenCL(limit, arraysize, workgroupsize);

			// release OpenCL resources
			if(free_OpenCL() == NOT_SUCCESS){
				return NOT_SUCCESS;
			}
			//printf("The OpenCL Sieve function is complete.\n");	

			/*Check the number of primes is matched with the result.
			The total number of primes <= the max is here(http://primes.utm.edu/howmany.shtml) */
			if(numberOfprimes != result){
				printf("The result (%d) is wrong. The number of primes <= %d is %d", numberOfprimes, limit, result);
				system("pause");
				break;
			}

			/*Get the ending time.*/
			end = clock();

			/*Get the difference of starting and ending time.*/
			diff = (double)(end - start) / (CLOCKS_PER_SEC / 1000);
			benchmark.diff[iter] = diff;
			printf("Experiment #%d: the total number of primes less than %d is %d.\n", iter, limit, numberOfprimes);
			printf("Experiment #%d: Kernel = %s ArraySize = %d Workgroupsize = %d Vendor = %s\n",  iter, kenel_name, 
				arraysize, workgroupsize, vendor);
			printf("Experiment #%d: %f seconds.\n\n", iter, diff);
		}
		writeBenchmarksToCSV(benchmark);
	}
	free(benchmarks);

	system("pause");
	return 0;
}