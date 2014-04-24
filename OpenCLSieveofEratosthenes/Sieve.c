#include "Header.h"
#define MAXLENGTH 200
#define THRESHOLD 1000000000// Used to break down the work.


//OpenCL objects
cl_program program;// program
cl_context context;// context
cl_device_id device_id;// device ID
cl_kernel kernel; // kernel
cl_command_queue queue;

typedef struct {
	//Previous found primes
	int prime_length;
	int* primes;
} PrimeList;

//Store the OpenCL platform and device info.
typedef struct{
	cl_uint num_platforms;		// the number of platforms
	char* vendor;				// the platform vendor
	cl_uint num_devices;		// the number of devices
	cl_device_id device_id;		// device ID
	int global_size;
} OpenCLPlatform;
OpenCLPlatform platform;


/*
Initialize the necessary OpenCL objects.
*/
int deﬁne_platform(cl_device_type device_type, char* vendor_name){
	cl_int err;
	cl_platform_id cpPlatform[32];    // an array to hold the OpenCL platforms
	cl_uint num_platforms;			  // the number of platforms
	char vendor[MAXLENGTH];				  // the platform vendor
	cl_uint num_devices;			  // the number of devices
	cl_uint i;

	// Return a list of platforms
	err = clGetPlatformIDs(32, cpPlatform, &num_platforms);	

	for(i = 0;i<num_platforms;i++){
		//Get the platform info
		clGetPlatformInfo(cpPlatform[i], CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);
		if(strncmp(vendor_name, vendor, sizeof(vendor)) == 0){			
			// Discover the devices within the platform
			err = clGetDeviceIDs(cpPlatform[i], device_type, 1, &device_id, &num_devices);
			if(err != 0){
				printf("Fail to configure the OpenCL device.\n");
				return NOT_SUCCESS;
			}else{
				platform.num_platforms = num_platforms;
				platform.vendor = vendor_name;
				platform.num_devices = num_devices;
				platform.device_id = device_id;
				break;
			}

		}
	}



	return SUCCESS;
}

/*
Load the computing program from a source file and create the kernel.
*/
int Load_program(char* filename)
{
	FILE* fp;
	size_t sizes;
	char *buffer;
	errno_t fp_err;
	int count;
	cl_int err;
	cl_build_status status;	


	// get size of kernel source
	if ((fp_err = fopen_s(&fp, filename, "r")) != 0){
		printf("Failed to load the file:%s\n", *filename);
		return NOT_SUCCESS;
	}

	//Compute the file size
	count = 0;
	while (!feof(fp)){
		char c;
		fread(&c, 1, 1, fp);
		count++;
	}
	rewind(fp);

	// read kernel source into buffer	
	buffer = (char*)malloc(count + 1);
	buffer[count] = '\0';//End of file.	
	sizes = fread(buffer, 1, count, fp);
	fclose(fp);

	// Create a context for the device.
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	
	// Create the compute program from the source buffer
	program = clCreateProgramWithSource(context, 1, (const char **)& buffer, (const size_t *)&sizes, &err);
	if (program == 0) {
		printf("Failed to create the program with source.");
		return NOT_SUCCESS;
	}

	free(buffer);

	// Build the program executable	
	//err = clBuildProgram(program, 0, NULL, "-cl-nv-maxrregcount=24 -cl-nv-verbose", NULL, NULL);
	//err = clBuildProgram(program, 1, &device_id, "-cl-nv-verbose", NULL, NULL);
	err = clBuildProgram(program, 1, &device_id, "", NULL, NULL);
	// check build error and build status first
	clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, NULL);
	if (status != CL_SUCCESS) {
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &sizes);
		buffer = (char*) calloc (sizes+1, sizeof(char));
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizes, buffer, NULL);
		buffer[sizes]='\0';
		if ((err = fopen_s(&fp, "error.txt", "w")) != 0)
		{
			printf("Failed to open the file:ptxcode.txt\n");
			return NOT_SUCCESS;
		}
		fprintf(fp,"%s", buffer);
		fclose(fp);
		free(buffer);
		//exit(0);
	}


	// Create the compute kernel in the program we wish to run
	kernel = clCreateKernel(program, "sieve", &err);
	if (err != 0){
		printf("Error occurs while creating the kernel.");
		return NOT_SUCCESS;
	}

	return SUCCESS;
}

/*
Release the allocated OpenCL objects.
*/
int free_OpenCL(){

	if(clReleaseKernel(kernel) != 0){
		printf("Fail to release the kernel");
		return NOT_SUCCESS;
	}

	if(clReleaseProgram(program) != 0){
		printf("Fail to release the program");
		return NOT_SUCCESS;
	}

	if(clReleaseCommandQueue(queue) != 0){
		printf("Fail to release the queue");
		return NOT_SUCCESS;
	}

	if(clReleaseContext(context) != 0){
		printf("Fail to release the context");
		return NOT_SUCCESS;
	}
	return SUCCESS;
}


/*
Find all the primes <= sqrt(limit) by sequentially sieving the list. 
*/
PrimeList Sieve_Sqrt_limit(int limit){

	int p, m, n, prime_length;
	char *numbers;
	PrimeList primelist;
	int *primes;
	n = sqrt((double)limit);
	//Allocate a block of memory for an array.
	numbers = (char *)calloc(n + 1,sizeof(char));
	if (numbers == NULL){
		printf("Fail to allocate the array of %d chars.\n", numbers);
		return primelist;
	}
	numbers[0] = 1;
	numbers[1] = 1;
	// standard host allocation
	primes = (int *)malloc(n*sizeof(int));
	//The first prime
	p = 2;
	prime_length = 0;
	while (p <= n){
		//0:true, 1:false
		if (numbers[p] == 0){
			m = p*p;
			while (m <= n){
				numbers[m] = 1;
				m = m + p;
			}
			//Add the prime into the primes list.
			primes[prime_length] = p;
			prime_length++;
		}
		p++;	
	}
	
	//primelist = (PrimeList *)malloc(sizeof(PrimeList));
	primelist.primes = primes;
	primelist.prime_length = prime_length;

	free(numbers);
	return primelist;
}


/*
* Find all of the primes upto the limit by using the OpenCL objects.
* Check if the work requires the decomposition.
If so, then split the work into the small tasks of 1000 million in size.*/
int Sieve_OpenCL(int limit, int arraysize, int workgroupsize){

	int n, i, total;
	int numberOfBlocks, numberOfTasks;
	int start, end, task_id;
	short int *subtotals;
	// Device list buffers	
	cl_mem d_primes;
	cl_mem *d_subtotals;
	cl_int err;
	int global_size;
	PrimeList primelist;
	int* primes;
	int prime_length;


	err = 0;
	n = sqrt((double)limit);
	primelist = Sieve_Sqrt_limit(limit);
	prime_length = primelist.prime_length;
	primes = primelist.primes;

	// Create a command queue to feed the device.
	queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &err);
	//Create the input prime list.
	d_primes = clCreateBuffer(context, CL_MEM_READ_ONLY, n*sizeof(int), NULL, err);
	clEnqueueWriteBuffer(queue, d_primes, CL_TRUE, 0, n*sizeof(int), primes, 0, NULL, NULL);

	start=0;
	numberOfTasks = ceil(limit/(float)THRESHOLD);
	d_subtotals = (cl_mem*)malloc(numberOfTasks * sizeof(cl_mem*));
	//while(start !=limit){
	for(task_id=0;task_id<numberOfTasks;task_id++){
		end = min(start + THRESHOLD, limit);
		// Total number of blocks
		numberOfBlocks = ceil((end- start)/(float)arraysize);
		// Calculate the global size of the task.
		global_size = workgroupsize * ceil((end - start)/(float)(arraysize*workgroupsize));

		// Create the input and output in device memory.		
		d_subtotals[task_id] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, numberOfBlocks*sizeof(short int), NULL, err);

		// Set the arguments to the sieve kernel.		
		err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_primes);
		err |= clSetKernelArg(kernel, 1, sizeof(int), &prime_length);
		err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_subtotals[task_id]);
		err |= clSetKernelArg(kernel, 3, sizeof(int), &arraysize);
		err |= clSetKernelArg(kernel, 4, sizeof(int), &numberOfBlocks);
		err |= clSetKernelArg(kernel, 5, sizeof(int), &start);
		err |= clSetKernelArg(kernel, 6, sizeof(int), &end);

		// Execute the kernel over the data set.
		err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &workgroupsize, 0, NULL, NULL);
		//Set the global size info.
		platform.global_size = global_size;
		if (err != 0){
			printf("Error occurs while enqueue the OpenCL kernels.");
			system("pause");
			return -1;
		}

		start = end;
		
	}

	// Block until all the commands issued to the command queue have been complete before reading back results
	err = clFinish(queue);
	// Read the results from the device
	if(err != 0){
		printf("Error occurs while running the OpenCL kernels.");
		system("pause");
		return -1;
	}

	
	err = clReleaseMemObject(d_primes);
	total=0;
	// Use the host memory to store the subtotals.
	subtotals = (short int*)malloc(numberOfBlocks * sizeof(short int));
	for(task_id=0;task_id<numberOfTasks;task_id++){
		//Block reading until the d_subtotals has been read and copied to subtotals.
		clEnqueueReadBuffer(queue, d_subtotals[task_id], CL_TRUE, 0, numberOfBlocks*sizeof(short int), subtotals, 0, NULL, NULL);		
		for (i = 0; i < numberOfBlocks; i++){
			total += subtotals[i];
		}	
		//Clean up the OpenCL resources.
		err = clReleaseMemObject(d_subtotals[task_id]);		
	}
	//Substract (0 and 1) from the total.
	total -= 2;

	free(subtotals);	
	free(d_subtotals);
	//Free the primes.	
	free(primes);
	
	return total;
}


/*Create a CSV file and write all the benchmarks to this file.*/
void writeBenchmarksToCSV(Benchmark benchmark){

	cl_uint numberOfCores;			  // the number of cores of on a device
	cl_long amountOfMemory;			  // the amount of memory on a device
	cl_uint clockFreq;				  // the clock frequency of a device
	cl_ulong maxAlocatableMem;		  // the maximum allocatable memory
	cl_ulong localMem;				  // the local memory for a device
	cl_bool available;				  // the device is available
	cl_ulong buf_ulong;
	size_t valueSize;
	char value[MAXLENGTH];

	char deviceName[MAXLENGTH];
	char vendor[MAXLENGTH];
	int iter;	
	FILE *file;
	//char *kt_str;
	//Opencl errors
	cl_int err;
	char str[MAXLENGTH];
	char filename[MAXLENGTH] = "benchmark\\openclSieve";
	//Make a directory.
	err = system("mkdir benchmark");	
	
	sprintf_s(str, MAXLENGTH, ".%s.Limit%d.WorkGroupSize%d.ArraySize%d.Global%d", benchmark.kernel_name, benchmark.parameters.limit,
		benchmark.parameters.workgroupsize, benchmark.parameters.arraysize, platform.global_size);
	strcat_s(filename, MAXLENGTH, str);
	strcat_s(filename, MAXLENGTH, ".csv");

	if ((err = fopen_s(&file, filename, "w")) != 0)
	{
		printf("Failed to open the file:%s\n", filename);
		return;
	}

	fprintf(file, "#Number of platforms:\t%u\n", platform.num_platforms);
	fprintf(file, "#Platform Vendor:\t%s\n", platform.vendor);
	fprintf(file, "#Number of devices:\t%u\n", platform.num_devices);

	//Get the device name
	clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
	clGetDeviceInfo(device_id, CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL);
	clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(numberOfCores), &numberOfCores, NULL);
	clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(amountOfMemory), &amountOfMemory, NULL);
	clGetDeviceInfo(device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clockFreq), &clockFreq, NULL);
	clGetDeviceInfo(device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(maxAlocatableMem), &maxAlocatableMem, NULL);
	clGetDeviceInfo(device_id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(localMem), &localMem, NULL);
	clGetDeviceInfo(device_id, CL_DEVICE_AVAILABLE, sizeof(available), &available, NULL);
	clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(buf_ulong), &buf_ulong, NULL);
	clGetDeviceInfo(device_id, CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
	clGetDeviceInfo(device_id, CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);

	fprintf(file, "#Name:\t%s\n", deviceName);
	fprintf(file, "#Vendor:\t%s\n", vendor);
	fprintf(file, "#Available:\t%s\n", available ? "Yes" : "No");
	fprintf(file, "#Compute Units:\t%u\n", numberOfCores);
	fprintf(file, "#Clock Frequency:\t%u mHz\n", clockFreq);
	fprintf(file, "#Global Memory:\t%0.00f mb\n", (double)amountOfMemory / 1048576);
	fprintf(file, "#Max Allocateable Memory:\t%0.00f mb\n", (double)maxAlocatableMem / 1048576);
	fprintf(file, "#Local Memory:\t%u kb\n", (unsigned int)localMem);
	fprintf(file, "#Device Maximal Work-group Size:\t %llu\n", (unsigned long long)buf_ulong);
	fprintf(file, "#OpenCL C version:\t %s\n", value);
	//fprintf(file, "#WorkGroup size=%d, Global Size=%d, Block size=%d\n", workgroupSize, globalSize, block_size);

	//Write the header of the table.
	fprintf(file, "Iteration\tTime(milliseconds)\n");
	iter = 0;
	while (iter < benchmark.repeats){
		fprintf(file, "%d\t%f\n", iter + 1, benchmark.diff[iter]);
		iter++;
	}

	fclose(file);
}