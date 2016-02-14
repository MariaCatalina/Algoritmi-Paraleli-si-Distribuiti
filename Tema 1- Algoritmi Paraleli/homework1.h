#ifndef HOMEWORK1_H
#define HOMEWORK1_H
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

typedef struct{
	int width, height, maxval;
	char *type;
	unsigned char **picture;

} image;

void initialize(image *im);

void render(image *im);

void writeData(const char * fileName, image *img) ;

#endif