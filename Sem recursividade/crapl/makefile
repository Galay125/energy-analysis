all: rapl_inter.o 

rapl_inter.o:  rapl_interface.h rapl.o
	g++ -std=c++11 -c measures.cpp rapl_interface.cpp -lm

rapl.o : rapl.cpp rapl.h measures.o 
	g++ -std=c++11 -c measures.cpp rapl.cpp -lm

measures.o : measures.cpp measures.h
	g++ -std=c++11 -c measures.cpp -lm 

clean:
	rm -rf rapl.o rapl_interface.o measures.o
