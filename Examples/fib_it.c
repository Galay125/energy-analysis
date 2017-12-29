#include <stdlib.h>
#include <stdio.h>


int fibonacci_iterativo (int n)
{
	int i = 0;
	int j = 1;
	
	
	for (int k = 1; k < n; k++)
	{	

		int t;
		t = i + j;
		i = j;
		j = t;
	}
			

	return j;
	
};


int main(int argc, char **argv) 
{
	int num = atoi(argv[1]);
	int res = fibonacci_iterativo(num);

	printf("Fib: %d \nResultado: %d \n", num, res);
	return 0;
}
