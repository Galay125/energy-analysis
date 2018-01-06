#include "../Rapl/crapl/rapl_interface.h"
#include "../Rapl/crapl/measures.h"
//An example of using templated class to create stack depends on underlying array.
#include <iostream>
#include <cstdlib>
#define default_value 10
using namespace std;
template< class T > class Stack
{
public:
    Stack(int = default_value);//default constructor
    ~Stack()//destructor
    {
     	CRapl rapl = create_rapl(0); 
    	rapl_before(rapl);delete [] values;rapl_after(0,rapl);
    }
    bool push( T );
    T pop();
    bool isEmpty();
    bool isFull();
private:
	int size;
	T *values;
	int index;
};

template< class T > Stack<T>::Stack(int x):
    size(x),//ctor
    values(new T[size]),
    index(-1)
    {}

template< class T > bool Stack<T>::isFull()
    {
     	CRapl rapl = create_rapl(2); 
    	rapl_before(rapl);
    	if((index + 1) == size )
    	{
    			rapl_after(2,rapl);
    		return 1;
    		}
    	else
    	{
    			rapl_after(2,rapl);
    		return 0;
    		}
    }

template< class T > bool Stack<T>::push(T x)
    {
     	CRapl rapl = create_rapl(3); 
    	rapl_before(rapl);
    	bool b = 0;
    	if(!Stack<T>::isFull())
    	{
    		index+=1;
    		values[index] = x;
    		b = 1;
    	}
    	rapl_after(3,rapl);
    	return b;
    }

template< class T > bool Stack<T>::isEmpty()
    {
     	CRapl rapl = create_rapl(4); 
    	rapl_before(rapl);
    if(index==-1)//is empty
    {
    		rapl_after(4,rapl);
    	return 1;
    	}
    else
   {
     rapl_after(4,rapl);
    return 0;
    } //is not empty
}

template< class T > T Stack<T>::pop()
{
 	CRapl rapl = create_rapl(5); 
	rapl_before(rapl);
	T val = -1;
	if(!Stack<T>::isEmpty())
	{
		val = values[index];
		index-=1;
	}
	else
	{
		cerr << "Stack is Empty : ";
	}
	rapl_after(5,rapl);
	return val;
}

int main()
{
 	initMeasure();
 	CRapl rapl = create_rapl(6); 
	rapl_before(rapl);
	Stack <double> stack1;
	Stack <int> stack2(5);
	int y = 1;
	double x = 1.1;
	int i, j; 
	cout << "\n pushed values into stack1: ";

    for(i = 1; i <= 11 ; i++)//start enter 11 elements into stack
    {
    	if(stack1.push(i*x))
    	{
    			cout << endl << i*x;
    		}
    	else
    	{
    			cout << "\n Stack1 is full: ";
    		}
    }

    cout << "\n\n popd values from stack1 : \n";
    for( j = 1 ; j <= 11 ; j++)
    	cout << stack1.pop() << endl;

    cout << "\n pushd values into stack2: ";
    for(i = 1; i <= 6 ; i++) //start enter 6 elements into stack
    {
    	if(stack2.push(i*y))
    	{
    			cout << endl << i*y;
    		}
    	else
    	{
    			cout << "\n Stack2 is full: ";
    		}
    }

    cout << "\n\n popd values from stack2: \n";
    for( j = 1 ; j <= 6 ; j++)
    	cout << stack2.pop() << endl;
    cout << endl << endl;
    rapl_after(6,rapl);
    writeMeasure("templated"); 
    return 0;
}