#include "Header.h"
#define MAXITERATION 30
#define MAXLENGTH 200

SYSTEM_INFO siSysInfo;
char *nonPrime;

/*Allocate a block of size bool of memory to store the prime flags and assign their initial values. (0:true, 1:false)*/
int makeIntList(int max){
	int i;
	free(nonPrime);
	nonPrime = (char *)calloc(max + 1, sizeof(char));

	if (nonPrime == NULL){
		printf("Fail to allocate the array of %d integers.\n", max);
		return NOT_SUCCESS;
	}

	nonPrime[0] = 1;
	nonPrime[1] = 1;

	return SUCCESS;
}

/*
Find all of the primes upto the limit.  (0:true, 1:false)
*/
int Sieve(int max){
	int n, p, numberOfPrime, i, m;

	n = sqrt((double)max);
	p = 3;
	while (p <= n){
		if (nonPrime[p] == 0){
			m = p*p;
			while (m <= max){
				nonPrime[m] = 1;
				m = m + p;
			}
			//printf("primes:%d.\n", p);
		}
		p+=2;
	}

	/*The number 2 is the first prime number.*/
	numberOfPrime = 1;
	/*Start from 3 to max and count the odd primes.*/
	for (i = 3; i <= max; i+=2)
	{
		if (nonPrime[i] == 0)
			numberOfPrime++;
	}
	return numberOfPrime;
}
/*Obtain the hardware information.*/
void getWinHardware(){
	// Copy the hardware information to the SYSTEM_INFO structure. 
	GetSystemInfo(&siSysInfo);
	// Display the contents of the SYSTEM_INFO structure. 
	printf("Hardware information: \n");
	printf("  OEM ID: %u\n", siSysInfo.dwOemId);
	printf("  Number of processors: %u\n", siSysInfo.dwNumberOfProcessors);
	printf("  Page size: %u\n", siSysInfo.dwPageSize);
	printf("  Processor type: %u\n", siSysInfo.dwProcessorType);
	printf("  Minimum application address: %lx\n", siSysInfo.lpMinimumApplicationAddress);
	printf("  Maximum application address: %lx\n", siSysInfo.lpMaximumApplicationAddress);
	printf("  Active processor mask: %u\n", siSysInfo.dwActiveProcessorMask);

}

/*Create a CSV file and write all the benchmarks to this file.*/
void writeBenchmarksToCSV(int limit, double diff[], int length){
	
	FILE *file;
	double *ptr;
	int iter;
	errno_t err;
	char str[MAXLENGTH];
	char filename[MAXLENGTH] = "benchmark\\CSieve";
	//Make a directory.
	err = system("mkdir benchmark");
	sprintf_s(str, MAXLENGTH, ".limit%d", limit);
	strcat_s(filename, MAXLENGTH, str);
	strcat_s(filename, MAXLENGTH, ".csv");
	if ((err = fopen_s(&file, filename, "w")) != 0)
	{
		printf("Failed to open the file:%s\n", filename);
		return;
	}

	/*Write out the hardware information.*/
	fprintf(file, "#Hardware information: \n");
	fprintf(file, "#\tOEM ID: %u\n", siSysInfo.dwOemId);
	fprintf(file, "#\tNumber of processors: %u\n", siSysInfo.dwNumberOfProcessors);
	fprintf(file, "#\tPage size: %u\n", siSysInfo.dwPageSize);
	fprintf(file, "#\tProcessor type: %u\n", siSysInfo.dwProcessorType);
	fprintf(file, "#\tMinimum application address: %lx\n", siSysInfo.lpMinimumApplicationAddress);
	fprintf(file, "#\tMaximum application address: %lx\n", siSysInfo.lpMaximumApplicationAddress);
	fprintf(file, "#\tActive processor mask: %u\n", siSysInfo.dwActiveProcessorMask);

	//Write the header of the table.
	fprintf(file, "Iteration\tTime(milliseconds)\n");
	iter = 0;
	ptr = diff;
	while (iter != length){
		fprintf(file, "%d\t%f\n", iter + 1, ptr[iter]);
		iter++;
	}
	fclose(file);
}


/*Main entry point.*/
int main(int argc, char *args[]){
	int limit;
	int numberOfprimes;
	int iter, num;
	clock_t start, end;
	double diff[MAXITERATION], avg_time;

	///*Convert an array of chars into an integer.*/
	//limit = strtol(args[1], NULL, 0);
	getWinHardware();
	/*The range of limit is from 10 to 100 million.*/
	for (limit = 10; limit <= pow(10,9); limit*=10){
		avg_time = 0;
		for (iter = 0; iter < MAXITERATION; iter++){
			/*Get the starting time.*/
			start = clock();
			/* The total number of primes <= the limit is:http://primes.utm.edu/howmany.shtml */
			if (makeIntList(limit) == NOT_SUCCESS){
				break;
			}
			numberOfprimes = Sieve(limit);
			/*Get the ending time.*/
			end = clock();
			/*Get the difference of starting and ending time.*/
			diff[iter] = (double)(end - start) / (CLOCKS_PER_SEC / 1000);
			printf("\nIn the %dth iteration, the total number of primes upto %d is %d\n", iter, limit, numberOfprimes);
			printf("The %dth iteration takes %.2f milliseconds on average.\n", iter, diff[iter]);
			avg_time += diff[iter] / MAXITERATION;
		}
		printf("The average time is %.2f milliseconds.\n", avg_time);
		/*Write the benchmark results to a CSV file (benchmark.csv).*/
		writeBenchmarksToCSV(limit, diff, MAXITERATION);
	}
	system("pause");
	return 0;
}
