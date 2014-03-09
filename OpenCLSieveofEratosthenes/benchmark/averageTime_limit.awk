#!/bin/awk -f
BEGIN { 
	filename="";
	limit=1;
	local=64;
	block=1;
	global=1;
	limits[""]=0;
	globals[""]=0;
	blocks[""]=0;	
	num=1;	
	iter=1;
	times[""]=0;
	hareware="";
}
{
	if( filename != FILENAME){
		filename = FILENAME;
		split(filename,array,".");
		limit = substr(array[2],6,length(array[2])+1)+0;
		local = substr(array[3],6,length(array[3])+1)+0;
		block = substr(array[4],6,length(array[4])+1) + 0;
		global = substr(array[5],7,length(array[5])+1) + 0;
		limits[num]=limit;
		num++;
		hareware="";
		#printf("limit:%s\n",limit);
	}

	if( $0 !~ "#" && $0 !~ "Iteration"){
		split($0, array, " ");
		iter=array[1];
		key=limit"_"local"_"iter;		
		times[key]=array[2]+0;
		globals[limit]=global;
		blocks[limit]=block;
		#printf("%s\t%s\n",key, times[key]);
	}else if ($0 ~ "#" && $0 !~ "#Local size"){
		hareware = hareware""$0"\n";
	}	

}
END {
	printf("%s", hareware);
	printf("Limit");
	printf("\tGlobal_Size");
	printf("\tLocal_Size");
	printf("\tBlock_Size");
	for (iter = 1; iter <= 30; iter++){
		printf("\t%sth_run",iter);
	}
	printf("\tAverage_Time\n");
	#populate the array data
	
	num = asort(limits);    # index values are now sorted
	for (i = 1; i <= num; i++){
		if(limits[i] != 0){
			limit=limits[i];
			printf("%s",limit);
			global=globals[limit];
			printf("\t%s",global);
			printf("\t%s",local);
			block = blocks[limit];
			printf("\t%d",block);
			for(iter=1;iter<=30;iter++){
				key=limit"_"local"_"iter;
				if(times[key] != 0)
					printf("\t%.2f",times[key]);
			}
			printf("\n");
		}
	}

}
