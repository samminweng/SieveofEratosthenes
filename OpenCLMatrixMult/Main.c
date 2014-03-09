// Main.c : Defines the entry point for the console application.
//

#include "Header.h"
#define MAX_BENCHMARKS 100



/*
Initial the necessary OpenCL objects.
*/
cl_int initialize_OpenCL(cl_device_type device){
	cl_int err;
	// Bind to platform
	err = clGetPlatformIDs(1, &cpPlatform, NULL);

	// Get ID for the device
	err = clGetDeviceIDs(cpPlatform, device, 1, &device_id, NULL);

	// Create a context 
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);

	// Create a command queue
	queue = clCreateCommandQueue(context, device_id, 0, &err);

	return err;
}

/*
Load the computing program from a source file and create the kernel.
*/
int create_Kernel(const char* filename, const char* method)
{
	FILE* fp;
	size_t fileSize;
	char *buffer;
	errno_t fp_err;
	int count;
	cl_int err;

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
	if ((err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL)) != CL_SUCCESS) {
		return err;
	}

	// Create the compute kernel in the program we wish to run
	kernel = clCreateKernel(program, method, &err);

	return err;
}
/*
Execute the kernel and wait for the execution result.
*/
cl_int execute_kernel(size_t globalSize, size_t localSize){
	cl_int err;
	// Execute the kernel over the entire range of the data set 
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);

	// Wait for the command queue to get serviced before reading back results
	err = clFinish(queue);

	return err;
}
/*
Release the allocated OpenCL objects.
*/
cl_int free_OpenCL(){
	cl_int err;
	err = clReleaseProgram(program);
	err = clReleaseKernel(kernel);
	err = clReleaseCommandQueue(queue);
	err = clReleaseContext(context);
	return err;
}


int main(int argc, char *args[]){
	//Time the execution time of CPU.
	time_t start, end;
	double diff;
	int benchmark;
	cl_int err;
	int len, array_length;
	//The input arrays
	int *a, *b;
	//The output array
	int *c;
	// Device input buffers
	cl_mem d_a, d_b;
	// Device output buffer
	cl_mem d_c;
	size_t globalSize, localSize;
	size_t array_size;
	int i, j;

	/*Get the starting time.*/
	time(&start);

	for (benchmark = 1; benchmark <= MAX_BENCHMARKS; benchmark++){
		if (argc < 2){
			printf("No parameter provided! Please specify the length of the matrix.");
			return 0;
		}
		else{
			
			len = strtol(args[1], NULL, 0); // Length of the matrix
			
			// Number of work items in each local work group
			localSize = 64;
			// Number of total work items - localSize must be devisor
			globalSize = ceil((len*len) / (float)localSize)*localSize;

			

			// Create and initialize the array
			//Array Size
			array_length = len * len;
			array_size = sizeof(int *)* (len * len);
			a = (int *)malloc(array_size);
			b = (int *)malloc(array_size);
			c = (int *)malloc(array_size);
			
			for (i = 0; i < len; i++){
				for (j = 0; j < len; j++){
					a[i*len + j] = 1;
					b[i*len + j] = 2;
				}
			}

			err = initialize_OpenCL(CL_DEVICE_TYPE_GPU);
			if (err != 0){
				printf("Error occurs while initializing the OpenCL objects.");
				system("pause");
				return 1;
			}

			err = create_Kernel("Kernel.cl", "matrixMult");
			if (err != 0){
				printf("Error occurs while creating the OpenCL kernels.");
				system("pause");
				return 1;
			}
			
			// Create the input and output arrays in device memory for our calculation.
			d_a = clCreateBuffer(context, CL_MEM_READ_ONLY, array_size, NULL, NULL);
			d_b = clCreateBuffer(context, CL_MEM_READ_ONLY, array_size, NULL, NULL);
			d_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, array_size, NULL, NULL);

			// Write our data set into the input array in the device's memory.
			err = clEnqueueWriteBuffer(queue, d_a, CL_TRUE, 0, array_size, a, 0, NULL, NULL);
			err |= clEnqueueWriteBuffer(queue, d_b, CL_TRUE, 0, array_size, b, 0, NULL, NULL);

			// Set the arguments to our computing kernel.
			err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a);
			err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_b);
			err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_c);
			err |= clSetKernelArg(kernel, 3, sizeof(int), &array_length);
			err |= clSetKernelArg(kernel, 4, sizeof(int), &len);

			err = execute_kernel(globalSize, localSize);
			if (err != 0){
				printf("Error occurs while executing the OpenCL kernels.");
				system("pause");
				return 1;
			}

			// Read the results from the device
			clEnqueueReadBuffer(queue, d_c, CL_TRUE, 0, array_size, c, 0, NULL, NULL);

			//Print out the resulting matrix
			for (i = 0; i < len; i++){
				for (j = 0; j < len; j++){
					printf("%d ", c[i*len + j]);
				}
				printf("\n");
			}

			// release OpenCL resources
			clReleaseMemObject(d_a);
			clReleaseMemObject(d_b);
			clReleaseMemObject(d_c);
			free_OpenCL();


			//Free memory
			free(a);
			free(b);
			free(c);
			printf("Finish the %dth iteration.\n", benchmark);

		}

	}

	/*Get the ending time.*/
	time(&end);
	/*Get the difference of starting and ending time.*/
	diff = difftime(end, start) / (float)benchmark;
	printf("The calculation takes %f seconds on average.\n", diff);
	system("pause");
	return 0;
}

