#include <stdio.h>
#include <stdlib.h>
#include "../crapl/rapl_interface.h"
#include "../crapl/measures.h"

void it(int nr){
  CRapl rapl = create_rapl(0);
  rapl_before(rapl);
  int aux;

  printf("\n it %d \n", nr);
  for(int i=1; i<10000000; i++){
    aux = nr*i;
  }
  rapl_after(0,rapl);
}

void it2(int nr){
  CRapl rapl = create_rapl(0);
  rapl_before(rapl);
  int aux;

  printf("\n it2 %d \n", nr);
  for(int i=1; i<10000000; i++){
    aux = nr*i;
  }
  rapl_after(1,rapl);
}

int main (int argc, char **argv){

  initMeasure();
  CRapl rapl = create_rapl(0);
  rapl_before(rapl);

  it(2);

  it2(3);

  it(5);

  it(4);

  rapl_after(2,rapl);

  writeMeasure("iteration.c");

  return 0;
}



