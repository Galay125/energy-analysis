#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cstdio>
#include <cmath>
#include "measures.h"


float **results;
int nr_funcs;
int nr_calls;
//int size;
//std::vector<std::vector<float>> matrix;
//std::vector<char const * > fns;

int number_of_lines(){
	int ch, lines = 0;
	FILE* myfile = fopen("CRapl_Gen/index.txt", "r");

	do 
	{
		ch = fgetc(myfile);
		if(ch == '\n')
			lines++;
	} while (ch != EOF);

	fclose(myfile);

	return lines;
}

/*int last_funct(){
	int ch, lines = 0;
	FILE* myfile = fopen("../Generated/index.txt", "r");
	fseek(myfile, -4, SEEK_END);
	do 
	{
		ch = fgetc(myfile);
		if(ch == '\n')
			lines++;
	} while (ch != EOF);
	
	ch = fgetc(myfile);

	printf("%d \n", ch);

	fclose(myfile);

	return lines;
}*/


	void initMeasure(){
	//size = 0;
		printf("INIT\n");
		nr_funcs = number_of_lines();
		results = (float**) malloc(nr_funcs * sizeof(float *));
		for(int x = 0; x < nr_funcs; x++){
			results[x] = (float *)malloc(5+1);
			for(int i=0; i<6; i++){
				results[x][i]=0.0;
			}
		}
	}

/*void insert(char const * fn, float pack, float pp0, float pp1, float dram, float cl){
	std::vector<float> aux;
	fns.push_back(fn);

	aux.push_back(pack);
	aux.push_back(pp0);
	aux.push_back(pp1);
	aux.push_back(dram);
	aux.push_back(cl);

	matrix.push_back(aux);
}*/

	void incrementCall(int fn){
		if(nr_funcs>0){
			results[fn][5] += 1;
		}
		nr_calls++;
	}

	void insertC(int fn, float pack, float pp0, float pp1, float dram, float cl){
	//printf("Total: %.10f \n", cl);
	//printf("fn: %d \n", fn);
		if(nr_funcs>0){
	//printf("%d \n", nr_funcs);

	//printf("pack: %.9f - pp0: %.9f - pp1: %.9f - dram: %.9f - cl: %.9f \n", pack, pp0, pp1, dram, cl);
			/*results[fn][0] += (pack - (nr_calls*0.0004));
			results[fn][1] += (pp0  - (nr_calls*0.0003));
			results[fn][2] += (pp1  - (nr_calls*0.000015));
			results[fn][3] += (dram  - (nr_calls*0.00004));
			results[fn][4] += cl;
			/*
			
			*/
		results[fn][0] += pack;
			results[fn][1] += pp0;
			results[fn][2] += pp1;
			results[fn][3] += dram;
			results[fn][4] += cl;
	}
	//size++;
	//printf("Sumatorio: %.10f\n", results[fn][4]);
}


void writeMeasure(char const * name_fl){

	printf("Program: %s \n", name_fl);
	
	int nr_used = 0;
	FILE * fp;
	char name_out[50];
	strcpy(name_out,"CRapl_Gen/");
	strcat(name_out, name_fl);
	strcat(name_out, ".txt");

	fp = fopen (name_out, "w+");
	fprintf(fp, "events: package pp0 pp1 dram time\n");
	fprintf(fp, "\n");

	for(int x=0; x < nr_funcs; x++){
		if((int)results[x][5]){
			nr_used++;
			fprintf(fp, "fn=(%d) \n", x);
			fprintf(fp, "calls=%d \n", (int)results[x][5]);
			for(int i=0; i<5; i++){
				//fprintf(fp, "%.6f \t", results[x][i]);
				fprintf(fp, "%d ", (int)(results[x][i]*1000));
			}
			fprintf(fp, "\n");
		}
	}

	printf("Used Functions: %d \n", nr_used);

	fclose(fp);
}
