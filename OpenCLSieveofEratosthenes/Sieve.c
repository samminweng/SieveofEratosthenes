#include "Header.h"
#define MAXLENGTH 200

/*Global variables*/
//OpenCL objects
cl_platform_id cpPlatform[32];    // an array to hold the OpenCL platforms
cl_device_id device_id;           // device ID
cl_context context;               // context
cl_command_queue queue;           // command queue
cl_program program;               // program
cl_kernel kernel;                 // kernel
cl_int err;
/*Global variables*/
int global_size;
//For printing out the device info.
cl_uint num_platforms;			  // the number of platforms
cl_uint num_devices;			  // the number of devices
char vendor[1024];				  // the platform vendor
char deviceName[1024];			  // the devices name



/*
Initialize the necessary OpenCL objects.
*/
cl_int deﬁne_platform(cl_device_type device, GraphicCard card){
	cl_int err;

	// Get the platform
	err = clGetPlatformIDs(32, &cpPlatform, &num_platforms);	

	if (device == CL_DEVICE_TYPE_CPU){
		//Get the platform info
		clGetPlatformInfo(cpPlatform[0], CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);
		// Discover the devices within the platform
		err = clGetDeviceIDs(cpPlatform[0], device, 1, &device_id, &num_devices);
	}
	else if (device == CL_DEVICE_TYPE_GPU ){

		if(card == AMD || card == NVIDIA){
			//Get the platform info
			clGetPlatformInfo(cpPlatform[1], CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);
			// Discover the devices within the platform
			err = clGetDeviceIDs(cpPlatform[1], device, 1, &device_id, &num_devices);
		}else{
			clGetPlatformInfo(cpPlatform[0], CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);
			err = clGetDeviceIDs(cpPlatform[0], device, 1, &device_id, &num_devices);
		}


	}


	// Create a context for the device.
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	// Create a command queue to feed the device.
	queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &err);

	return err;
}

/*
Load the computing program from a source file and create the kernel.
*/
int Load_program(const char* filename, int isVerbose)
{
	FILE* fp;
	FILE *ptx;
	size_t fileSize;
	char *buffer;

	errno_t fp_err;
	int count;
	cl_int err;
	cl_build_status status;
	size_t sizes;
	int i;
	char* log;

	// get size of kernel source
	if ((fp_err = fopen_s(&fp, filename, "r")) != 0){
		printf("Failed to load the file:%s\n", *filename);
		return fp_err;
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
	fileSize = fread(buffer, 1, count, fp);
	fclose(fp);

	// Create the compute program from the source buffer

	program = clCreateProgramWithSource(context, 1, (const char **)& buffer, (const size_t *)&fileSize, &err);
	//program = clCreateProgramWithSource(context, 1, (const char **) & kernelSourceCode, NULL, &err);
	if (program == 0) {
		return err;
	}

	free(buffer);

	// Build the program executable	
	//err = clBuildProgram(program, 0, NULL, "-cl-nv-maxrregcount=24 -cl-nv-verbose", NULL, NULL);
	err = clBuildProgram(program, 1, &device_id, "-Werror -cl-std=CL1.1 -cl-nv-verbose", NULL, NULL);
	// check build error and build status first
	clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, NULL);
	if (isVerbose==1) {
		//Get the list of binary sizes
		for (i=0;i<(int)num_devices;++i) {
			//sizes = (size_t*)malloc(num_devices*sizeof(size_t));
			clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &sizes);

			//Get the binaries
			//binaries = new unsigned char*[num_devices];
			log = (char*) calloc (sizes+1, sizeof(char));
			clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizes, log, NULL);
			log[sizes]='\0';
			if ((err = fopen_s(&ptx, "ptxcode.txt", "w")) != 0)
			{
				printf("Failed to open the file:ptxcode.txt\n");
				return;
			}
			fprintf(ptx,"%s", log);
			fclose(ptx);
			free(log);
			//free(sizes);
		}
	}



	// Create the compute kernel in the program we wish to run
	kernel = clCreateKernel(program, "sieve", &err);
	if (err != 0){
		printf("Error occurs while creating the kernel.");
		return -1;
	}

	return err;
}

/*
Release the allocated OpenCL objects.
*/
cl_int free_OpenCL(){
	cl_int err;
	err = clReleaseKernel(kernel);
	err = clReleaseProgram(program);
	err = clReleaseCommandQueue(queue);
	err = clReleaseContext(context);
	return err;
}

/*
Allocate a block of size integer of memory to store the prime flags and assign their initial values (0:true, 1:false).
*/
char* makeNonPrimes(int max){
	int i;
	char *nonPrimes;
	/*Allocate a block of memory for an array, and initialize all its bits to zero.
	*/
	nonPrimes = (char *)calloc(max + 1,sizeof(char));
	if (nonPrimes == NULL){
		printf("Fail to allocate the array of %d chars.\n", max);
		return NULL;
	}
	nonPrimes[0] = 1;
	nonPrimes[1] = 1;

	return nonPrimes;
}
/*
* Find all of the primes upto the limit by using the OpenCL objects.
* Check if the work requires the decomposition.
If so, then split the work into the small tasks of 100 million in size.*/
int Sieve_OpenCL(int max, int block_size, int workgroupSize, MemeoryMode memMode){

	int n, p, i, m, total;
	int numberOfBlocks, numberOfTasks;
	int start, end, task_id, threshold;
	short int *subtotals;
	// Device list buffers	
	cl_mem d_primes;
	cl_mem *d_subtotals;
	cl_mem cmPinnedDataIn = NULL;
	cl_mem cmPinnedDataOut = NULL;
	void* dm_idata = NULL;
	int *primes;
	int prime_length;
	char *nonPrimes;

	threshold = pow((float)10, 8);
	numberOfTasks = ceil(max/(float)threshold);
	err=0;
	nonPrimes = NULL;
	prime_length = 1;
	n = sqrt((double)max);
	d_primes = NULL;
	d_primes = clCreateBuffer(context, CL_MEM_READ_ONLY, n*sizeof(int), NULL, err);
	if(memMode == PINNED){
		//Create the previous found prime list on the host by using the pinned memory whose bandwidth is highest.
		cmPinnedDataIn = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, n*sizeof(int), NULL, err);	
		//map the d_primes pointer (on the device) to the host primes pointer.
		primes = (int *)clEnqueueMapBuffer(queue, cmPinnedDataIn, CL_FALSE, CL_MAP_READ, 0, n*sizeof(int), 0, NULL, NULL, NULL);
		//clEnqueueUnmapMemObject(queue, cmPinnedData, (void*)primes, 0, NULL, NULL);
	}else{
		// standard host allocation
		primes = (int *)malloc(n*sizeof(int));

	}

	nonPrimes = makeNonPrimes(n);
	/*The maximal length of prime list*/

	primes[prime_length-1] = 2;
	p = 2;
	while (p <= n){
		//0:true, 1:false
		if (nonPrimes[p] == 0){
			m = p*p;
			while (m <= n){
				nonPrimes[m] = 1;
				m = m + p;
			}
			//Add the prime into the primes list.
			prime_length++;
			primes[prime_length-1] = p;

		}
		p++;	
	}
	if(memMode == PINNED){
		//Write out the prime to the device memory 
		clEnqueueWriteBuffer(queue, d_primes, CL_FALSE, 0, n*sizeof(int), primes, 0, NULL, NULL);
	}else{
		clEnqueueWriteBuffer(queue, d_primes, CL_TRUE, 0, n*sizeof(int), primes, 0, NULL, NULL);
	}


	free(nonPrimes);
	task_id=0;
	start = 0;

	d_subtotals = (cl_mem*)malloc(numberOfTasks * sizeof(cl_mem*));
	while(start != max){
		end = min(start + threshold, max);
		// Total number of blocks
		numberOfBlocks = ceil((end- start)/(float)block_size);
		// Calculate the global size of the task.
		global_size = workgroupSize * ceil((end - start)/(float)(block_size*workgroupSize));

		// Create the input and output in device memory.		
		d_subtotals[task_id] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, numberOfBlocks*sizeof(short int), NULL, err);

		// Set the arguments to the sieve kernel.		
		err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_primes);
		err |= clSetKernelArg(kernel, 1, sizeof(int), &prime_length);
		err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_subtotals[task_id]);
		err |= clSetKernelArg(kernel, 3, sizeof(int), &block_size);
		err |= clSetKernelArg(kernel, 4, sizeof(int), &numberOfBlocks);
		err |= clSetKernelArg(kernel, 5, sizeof(int), &start);
		err |= clSetKernelArg(kernel, 6, sizeof(int), &end);
		// Execute the kernel over the data set.
		err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &workgroupSize, 0, NULL, NULL);
		if (err != 0){
			printf("Error occurs while enqueue the OpenCL kernels.");
			system("pause");
			return -1;
		}

		start = end;
		task_id++;
	}

	// Block until all the commands issued to the command queue have been complete before reading back results
	err = clFinish(queue);
	// Read the results from the device
	if(err != 0){
		printf("Error occurs while running the OpenCL kernels.");
		system("pause");
		return -1;
	}

	//UnMap the d_primes to primes.	
	if(primes){
		if(memMode == PINNED){
			err = clEnqueueUnmapMemObject(queue, cmPinnedDataIn, (void*)primes, 0, NULL,NULL);
			err = clReleaseMemObject(cmPinnedDataIn);
		}else{
			free(primes);
		}
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
	free(subtotals);	
	free(d_subtotals);


	return total;
}


/*Create a CSV file and write all the benchmarks to this file.*/
void writeBenchmarksToCSV(double diff[], int size, long long max, int workgroupSize, int block_size){

	cl_uint numberOfCores;			  // the number of cores of on a device
	cl_long amountOfMemory;			  // the amount of memory on a device
	cl_uint clockFreq;				  // the clock frequency of a device
	cl_ulong maxAlocatableMem;		  // the maximum allocatable memory
	cl_ulong localMem;				  // the local memory for a device
	cl_bool available;				  // the device is available
	cl_ulong buf_ulong;
	size_t valueSize;
	char value[MAXLENGTH];
	int iter;	
	FILE *file;
	/*Opencl errors*/
	cl_int err;
	char str[MAXLENGTH];
	char filename[MAXLENGTH] = "benchmark\\openclSieve";
	//Make a directory.
	err = system("mkdir benchmark");
	sprintf_s(str, MAXLENGTH, ".Limit%d.Local%d.Block%d.Global%d", max, workgroupSize, block_size, global_size);
	strcat_s(filename, MAXLENGTH, str);
	strcat_s(filename, MAXLENGTH, ".csv");

	if ((err = fopen_s(&file, filename, "w")) != 0)
	{
		printf("Failed to open the file:%s\n", filename);
		return;
	}

	fprintf(file, "#Number of platforms:\t%u\n", num_platforms);
	fprintf(file, "#Platform Vendor:\t%s\n", vendor);
	fprintf(file, "#Number of devices:\t%u\n", num_devices);

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
	while (iter < size){
		fprintf(file, "%d\t%f\n", iter + 1, diff[iter]);
		iter++;
	}

	fclose(file);
}
