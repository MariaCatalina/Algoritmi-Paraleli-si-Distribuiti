#include "homework1.h"
#include "omp.h"
#include <stdio.h>

extern int num_threads;
extern int resolution;
int main(int argc, char * argv[]) {
	//argv[1] output
	//argv[2] resolution
	//argv[3] num_threads
	if(argc < 4)
		printf("Incorrect number of arguments\n");
	resolution = atoi(argv[2]);
	num_threads = atoi(argv[3]);

	image im;

	initialize(&im);

	double start = omp_get_wtime();
	render(&im);
	double end = omp_get_wtime();

	printf("%f\n",end - start);

	writeData(argv[1], &im);
	return 0;
}