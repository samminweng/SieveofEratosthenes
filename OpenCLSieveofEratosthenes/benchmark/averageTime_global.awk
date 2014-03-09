#!/bin/awk -f
BEGIN { 
	filename="";
	limit=1;
	global=1;
	local=1;
	block=1;
	globals[""]=0;
	blocks[""]=0;
	sorted[""]=0;	
	num=1;	
	iter=1;
	times[""]=0;
	hareware="";
	workgroup=0;
}
{
	if( filename != FILENAME){
		filename = FILENAME;
		split(filename,array,".");
		limit = substr(array[2],6,length(array[2])+1)+0;
		local = substr(array[3],6,length(array[3])+1)+0;
		block = substr(array[4],6,length(array[4])+1) + 0;
		global = substr(array[5],7,length(array[5])+1) + 0;
		globals[num]=global;
		blocks[global]=block;
		num++;
		hareware="";
		#printf("global:%s\n",global);	
	}

	if( $0 !~ "#" && $0 !~ "Iteration"){
		split($0, array, " ");
		run=array[1];
		key=limit","local","global","run;		
		times[key]=array[2];		
		#printf("%s %s",key, time[key]);
	}else if ($0 ~ "#" && $0 !~ "#Local size"){
		hareware = hareware""$0"\n";
	}	

}
END {
	printf("%s", hareware);
	printf("#Limit:\t%s\n",limit); 	
	printf("Global_Size");
	printf("\tWork_Group");
	printf("\tBlock_Size");
	for (run = 1; run <= 30; run++){
		printf("\t%sth_run",run);
	}
	printf("\tAverage_Time\n");
	#populate the array data
	
	num = asort(globals);    # index values are now sorted
	for (i = 1; i <= num; i++){
		if(globals[i] != 0){
			global=globals[i];
			workgroup=global/local;
			block=blocks[global];
			printf("%s",global);
			printf("\t%s",workgroup);
			printf("\t%s",block);
			for(run=1;run<=30;run++){
				key=limit","local","global","run;
				if(times[key] != 0)
					printf("\t%.2f",times[key]);			
				
			}
			printf("\n");
		}
	}

}
