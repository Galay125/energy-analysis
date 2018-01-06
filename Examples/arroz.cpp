#include <crapl/rapl_interface.h>
#include <crapl/measures.h>
#include <stdio.h>
#include <stdlib.h>

void tantam();

int ab(){
 	CRapl rapl = create_rapl(0); 
	rapl_before(rapl);
	int x;
	rapl_after(0,rapl);
	ab();rapl_before(rapl);

	int a;
	rapl_after(0,rapl);
	return 1;
}

int abc(){
 	CRapl rapl = create_rapl(0); 
	rapl_before(rapl);
	int x = 1;
rapl_after(1,rapl);
		while(x = abc()){
		if(a)
		{
				int a = 5;
			}
		
		
	}rapl_before(rapl);

	x++;
	rapl_after(1,rapl);
	return 2;
}

int dore(){
 	CRapl rapl = create_rapl(0); 
	rapl_before(rapl);
	int x = 1;
	for(int i = 0; i < 10; i++){
		if(i==10)
		{
				rapl_after(2,rapl);
				return dore();
			}
	}
	//int a = dore();
	rapl_after(2,rapl);
	return 512;
}

int main() {
 	initMeasure();
 	CRapl rapl = create_rapl(0); 
	rapl_before(rapl);
	int i = 0;
	for(i = 0; i < 1000; i++){
				dore();	
	}
	printf("Arroz\n");	
	
		
	rapl_after(3,rapl);
	writeMeasure("withoutRapl"); 
	return (0); 
}
