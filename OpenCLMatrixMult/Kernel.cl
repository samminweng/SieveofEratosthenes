                             
__kernel void matrixMult(__global int *a,                     
                         __global int *b,                       
                         __global int *c,                       
                         const unsigned int n, const int len)    
{                                                               
    //Get our global thread ID                                 
    int id = get_global_id(0);                                  
    int i, j, k, sum;                                                            
    //Make sure we do not go out of bounds                      
    if (id < n){                                                 
       i = id/len;										  
       j = id%len;										  
       sum = 0;										 
       for(k=0;k<len;k++){										 
			sum += a[i*len+k]*b[k*len+j];          
       }                                                          
	    c[id] = sum;                                             
    }                                                             
}