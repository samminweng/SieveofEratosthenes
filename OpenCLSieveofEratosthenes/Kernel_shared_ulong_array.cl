//#pragma cl_nv_compiler_options
#define ARRAYSIZE 64
//#pragma OPENCL EXTENSION cl_intel_printf : enable
/*Count the number of 1 bits in an unsigned long integer.*/
int bitCount(ulong n) {
    int counter = 0;
    while(n) {
        counter += n % 2;
        n >>= 1L;
    }
    return counter;
 }
/*Use the ulong integer to store the nonPrimes list.*/
__kernel void sieve(__global int *primes,
					int numberOfPrimes,
					__global short int *subtotals,
					int block_size,
					int numberOfBlocks,
					int start_limit,
					int end_limit){ 
	int gid, lid, block_start, block_end;
	int m, p, i, subtotal;
	//Declare a shared and local ararys.
	__local ulong nonPrimes[ARRAYSIZE];
	//Get the global thread ID                                 
	gid = get_global_id(0);
	lid = get_local_id(0);
	subtotals[gid]=0;
	if(gid < numberOfBlocks){
		//Get the block size, block start, and block end.
		block_start = (gid * block_size) + start_limit;		
		block_end = min((gid+1)*block_size + start_limit, end_limit+1);
		nonPrimes[lid] = 0L;
		//For each given prime number, mark its multiples in the sub-range.	
		for(i=0; i<numberOfPrimes; i++){
			p = primes[i];
			//Calculate the closest multiple after the block start.
			m = (block_start/p) * p;
			if(m < block_start)
				m = m + p;
			//Force the m to be the multiple of p.
			m = max(m, p*p);
			//Mark the numbers with the prime.
			while(m < block_end){
				nonPrimes[lid] = nonPrimes[lid] | (1L<<(m-block_start));
				//printf("lid:%d\tm:%d\tnonPrimes:%ld\t bitCount:%d\n",lid,m,nonPrimes[lid], bitCount(nonPrimes[lid]));
				m = m + p;
			}
		}
		
		//Count the primes within the range.
		//subtotal = (block_end - block_start) - bitCount(nonPrimes[lid]);
		subtotal = (block_end - block_start) - popcount(nonPrimes[lid]);	
		subtotals[gid]=subtotal;
	}
	
}