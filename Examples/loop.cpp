#include "../Rapl/crapl/rapl_interface.h"
#include "../Rapl/crapl/measures.h"
#include <stdio.h>
#include <stdlib.h>


void arroz(){
	CRapl rapl = create_rapl(1); 
	rapl_before(rapl);

	rapl_after(1,rapl);
}

int main(int argc, char **argv)  {
 	initMeasure();
 	CRapl rapl = create_rapl(0); 
	rapl_before(rapl);

	int i = atoi(argv[1]);
	int x = 0;
	for(; x < i; x++){
				arroz();	
	}		
		
	rapl_after(0,rapl);
	writeMeasure("loop"); 
	return (0); 
}
