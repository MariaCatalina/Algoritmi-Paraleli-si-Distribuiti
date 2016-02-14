#include "homework.h"

int num_threads;
int resize_factor;
int GaussianKernel[3][3] = { {1, 2, 1},
							 { 2, 4, 2},
							 { 1, 2, 1}
							};

void readInput(const char * fileName, image *img) {
	FILE * f = fopen(fileName,"rb");
	char * type = (char *)malloc(255 * sizeof(char));
	int width, height, maxval, i;

	if(f == NULL){
		return;
	}

	fscanf(f,"%s\n%d %d\n%d\n", type, &width, &height, &maxval);
	
	img->type = (char *)malloc(255 * sizeof(char));
	strcpy(img->type, type);
	strcat(img-> type,"\0");

	img->width = width;
	img->height = height;
	img->maxval = maxval;

	/* in functie de tipul imaginii se alege forma de retinere a pixelilor */
	if(strcmp(type,"P6") == 0){
		/* matrice ce contine pixeleli pentru fiecare culoare */
		img->pictureColor = (pixel **)malloc(height * sizeof(pixel *));
		
		/* aloacare si citire din fisier a pixelilor */
		for(i = 0; i < height; i++){
			img->pictureColor[i] = (pixel *)malloc(width * sizeof(pixel));
			fread(img->pictureColor[i], sizeof(pixel),width,f);
		}
	}
	else
		if(strcmp(type, "P5") == 0){
			/* matrice ce contine doar un pixel */
			img->pictureGw = (unsigned char **)malloc(height * sizeof(unsigned char *));
		
			/* aloacare si citire din fisier a pixelilor */
			for(i = 0; i < height; i++){
				img->pictureGw[i] = (unsigned char *)malloc(width * sizeof(unsigned char));
				fread(img->pictureGw[i], sizeof(unsigned char), width, f);
			}
		}	

	fclose(f);
	free(type);
}

void writeData(const char * fileName, image *img) {
	FILE * f = fopen(fileName,"wb");
	int i;

	if(f == NULL){
		return;
	} 

	fprintf(f,"%s\n", img->type);
	fprintf(f,"%d %d\n", img->width, img->height);
	fprintf(f,"%d\n", img->maxval);
	
	if(strcmp(img->type,"P6") == 0){
		for(i = 0; i < img->height; i++){
			fwrite(img->pictureColor[i], sizeof(pixel), img->width, f);
		}
	}
	else
		if(strcmp(img->type,"P5") == 0){
			for(i = 0; i < img->height; i++){
				fwrite(img->pictureGw[i], sizeof(unsigned char), img->width, f);
		}
	}

	fclose(f);
}

/* functie de resize imagini gri cu resize_factor multiplu de 2*/
void resizeGw(image *in, image * out){
	int i, j, k, p, pixel;

	/* alocare memorie structura output */
	out->pictureGw = (unsigned char **)malloc(in->height * sizeof(unsigned char *));
	for(i = 0;i < in->height; i++){
		out->pictureGw[i] = (unsigned char *)malloc(in->width * sizeof(unsigned char));
	}
	out->type = (char *)malloc(255 * sizeof(char));

	out->width = in->width / resize_factor;
	out->height = in->height / resize_factor;
	out->maxval = in->maxval;
	strcpy(out->type, in->type);

	omp_set_num_threads(num_threads);
	
	#pragma omp parallel for private(i,j,p,k,pixel) collapse(2)
	/* parcurgere matrice din resize_factor in resize_factor pana
	la valoare intreaga a dimensiunii matricei in functie de resize_factor */
	for(i = 0; i < out->height * resize_factor; i += resize_factor){		
		for(j = 0; j < out->width * resize_factor; j += resize_factor){
			pixel = 0;
			/* calculare suma pixeli din blocurile de matrici interioare */
			for(k = i; k < resize_factor + i ; k++){
				for(p = j; p < resize_factor + j; p++){
					pixel += in->pictureGw[k][p];
				}
			}
			pixel /= resize_factor * resize_factor;
			out->pictureGw[i/ resize_factor][j / resize_factor] = pixel;
		}

	}
}

/*functie de resize imagini color cu resize_factor multiplu de 2*/
void resizeColor(image *in, image * out) { 
	int i, j, k, p, r, g, b;
	
	/* alocare memorie structura output */
	out->pictureColor = (pixel **)malloc(in->height * sizeof(pixel*));
	for(i = 0;i < in->height; i++){
		out->pictureColor[i] = (pixel *)malloc(in->width * sizeof(pixel));
	}

	out->type = (char *)malloc(255 * sizeof(char));
	out->width = in->width / resize_factor;
	out->height = in->height / resize_factor;
	out->maxval = in->maxval;
	strcpy(out->type, in->type);

	omp_set_num_threads(num_threads);
	
	#pragma omp parallel for private(i,j,p,k,r,g,b) collapse(2)
	/* parcurgere matrice din resize_factor in resize_factor 
	pana la valoare intreaga a matricei in functie de resize_factor */
	for(i = 0;i < out->height * resize_factor; i += resize_factor){
		for(j = 0; j < out->width * resize_factor; j += resize_factor){
			r = 0, g = 0, b = 0;
			/* calculare suma pixeli din blocurile de matrici interioare */
			for(k = i; k < resize_factor + i; k++){
				for(p = j; p < resize_factor + j; p++){
					r += in->pictureColor[k][p].red;
					g += in->pictureColor[k][p].green;
					b += in->pictureColor[k][p].blue;
				}
			}
			r /= resize_factor * resize_factor;
			g /= resize_factor * resize_factor;
			b /= resize_factor * resize_factor;

			out->pictureColor[i / resize_factor][j / resize_factor].red = r;
			out->pictureColor[i / resize_factor][j / resize_factor].green = g;
			out->pictureColor[i / resize_factor][j / resize_factor].blue = b;
		}
	}
}

/* functie de resize imagini color cu resize_factor 3 */
void resizeColorGK(image *in, image * out) { 
	int i, j, k, p;
	int r, g, b, gI = 0, gJ = 0;

	/* alocare memorie structura output */
	out->pictureColor = (pixel **)malloc(in->height * sizeof(pixel*));
	for(i = 0;i < in->height; i++){
		out->pictureColor[i] = (pixel *)malloc(in->width * sizeof(pixel));
	}

	out->type = (char *)malloc(255 * sizeof(char));
	out->width = in->width / resize_factor;
	out->height = in->height / resize_factor;
	out->maxval = in->maxval;
	strcpy(out->type, in->type);

	omp_set_num_threads(num_threads);
	
	#pragma omp parallel for private(i,j,p,k,r,g,b) firstprivate(gI,gJ) collapse(2)
	/* parcurgere matrice din resize_factor in resize_factor
	 pana la valoare intreaga a dimensiunii imaginii in functie de resize_factor */
	for(i = 0; i < out->height * resize_factor; i += resize_factor){
		for(j = 0; j < out->width * resize_factor; j += resize_factor){
			r = 0, g = 0, b = 0;
			gI = gJ = 0;
			/* parcurgere matrici interioare si calculare suma pixeli imnultita cu
			procentul situat pe accesi pozitie in kernelul gaussian */
			for(k = i; k < resize_factor + i; k++){
				for(p = j; p < resize_factor + j; p++){
					r += in->pictureColor[k][p].red * GaussianKernel[gI][gJ];
					g += in->pictureColor[k][p].green * GaussianKernel[gI][gJ];
					b += in->pictureColor[k][p].blue* GaussianKernel[gI][gJ];
					gJ ++;
				}
				gI ++;
				gJ = 0;
			}
			r /= 16;
			g /= 16;
			b /= 16;

			out->pictureColor[i / resize_factor][j / resize_factor].red = r;
			out->pictureColor[i / resize_factor][j / resize_factor].green = g;
			out->pictureColor[i / resize_factor][j / resize_factor].blue = b;
		}
	}
}

/* functie de resize imagini gri cu resize_factor 3 */
void resizeGwGK(image *in, image * out){
	int i, j, k, p;
	int pixel, iI = 0, jJ = 0;

	/* alocare memorie structura output */
	out->pictureGw = (unsigned char **)malloc(in->height * sizeof(unsigned char *));
	for(i = 0; i < in->height; i++){
		out->pictureGw[i] = (unsigned char *)malloc(in->width * sizeof(unsigned char));
	}

	out->type = (char *)malloc(255 * sizeof(char));
	out->width = in->width / resize_factor;
	out->height = in->height / resize_factor;
	out->maxval = in->maxval;
	strcpy(out->type, in->type);

	omp_set_num_threads(num_threads);

	#pragma omp parallel for private(i,j,p,k,pixel) firstprivate(iI,jJ) collapse(2)
	/* parcurgere matrice din resize_factor in resize_factor 
	 pana la valoare intreaga a dimensiunii imaginii in functie de resize_factor */
	for(i = 0; i < out->height * resize_factor; i += resize_factor){
		for(j = 0; j < out->width * resize_factor; j += resize_factor){
			iI = jJ = 0;
			pixel = 0;
		/* parcurgere matrici interioare si calculare suma pixeli imnultita cu
			procentul situat pe accesi pozitie in kernelul gaussian */
			for(k = i; k < resize_factor + i; k++){
				for(p = j; p < resize_factor + j; p++){
					pixel += in->pictureGw[k][p] * GaussianKernel[iI][jJ];
					jJ ++;
				}
				iI ++;
				jJ = 0;
			}
			pixel /= 16;
			out->pictureGw[i / resize_factor][j / resize_factor] = pixel;
		}
	}
}

/* functie resize care apeleaza fuctiile corespunzatorare in functie de resize_factor */
void resize(image *in, image *out){
	if(resize_factor % 2 == 0){
		if(strcmp(in->type,"P5") == 0)
			resizeGw(in, out);
		else
			resizeColor(in, out);
	}else if(resize_factor == 3){
		  	if(strcmp(in->type,"P5") == 0)
				resizeGwGK(in, out);
			else
				resizeColorGK(in, out);
	}
}