#ifndef HOMEWORK_H
#define HOMEWORK_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

typedef struct
{
    unsigned char red, green, blue;
} pixel;

typedef struct{
	int width, height, maxval;
	char *type;
	pixel ** pictureColor;
	unsigned char **pictureGw;
} image;

void readInput(const char * fileName, image *img);

void writeData(const char * fileName, image *img);

void resize(image *in, image * out);

void resizeGw(image *in, image * out);

void resizeColor(image *in, image * out);

void resizeGwGK(image *in, image * out);

void resizeColorGK(image *in, image * out);


#endif