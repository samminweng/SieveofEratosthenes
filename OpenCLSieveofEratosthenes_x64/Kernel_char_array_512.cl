//#pragma cl_nv_compiler_options
#define ARRAYSIZE 512
/*Use the constant array to store the nonPrimes list.*/
__kernel void sieve(__global int *primes,
					int numberOfPrimes,
					__global short int *subtotals,
					int block_size,
					int numberOfBlocks,
					int start_limit,
					int end_limit){ 
	int gid, lid, block_start, block_end;
	int m, p, i, subtotal, offset;
	//Declare a fixed-length array.
	char nonPrimes[ARRAYSIZE];
	//Get the global thread ID                                 
	gid  = get_global_id(0);
	lid = get_local_id(0);
	if(gid < numberOfBlocks){
		//Get the block size, block start, and block end.
		block_start = (gid * block_size) + start_limit;		
		block_end = min((gid+1)*block_size + start_limit, end_limit+1);
		
		//Initialize the nonPrimes array.
		for(i=0;i<ARRAYSIZE;i++)
			nonPrimes[i]=0;

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
				nonPrimes[m-block_start] = 1;
				m = m + p;
			}
		}
		subtotal=0;
		//Count the primes within the range.	
		for(i=block_start; i < block_end; i++){
			//if ((i >= 2) && (nonPrimes[i-block_start] == 0))
			if(nonPrimes[i-block_start] == 0){
				subtotal++;
			}
		}
		subtotals[gid]=subtotal;
	}
}