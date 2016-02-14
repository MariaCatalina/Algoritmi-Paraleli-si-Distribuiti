#include "homework1.h"

int num_threads;
int resolution;

void initialize(image *im) {
	int i, j;
	im->type = (char *)malloc(255 * sizeof(char));
	strcpy(im->type,"P5");
	im->width = resolution;
	im->height = resolution;
	im->maxval = 255;

	/* alocare memorie a  stucturii de data a imaginii */
	im->picture = (unsigned char **)malloc(resolution * sizeof(unsigned char *));
	for(i = 0;i < resolution;i ++){
		im->picture[i] = (unsigned char *)malloc(resolution * sizeof(unsigned char));
	} 

	/* initializare pixeli cu alb */
	for(i = 0; i< resolution ;i ++)
		for(j = 0 ;j < resolution ;j ++)
			im->picture[i][j] = 255;
}

void render(image *im) {
	int i, j, sum = 0;
	float dist;

	omp_set_num_threads(num_threads);

	#pragma omp parallel for private(i,j,sum,dist) collapse(2)
	/* parcurgere matrice pixel cu pixel */
	for(i = 0; i < resolution; i++)
		for(j = 0; j < resolution; j++){
			/* calculare distanta de la punct la dreapata */
			sum = (-1 * (j * 100 / resolution)) + (2 * ((resolution - 1 - i) * 100 / resolution));
			dist = abs(sum) / sqrt(5);

			if(dist < 3.0){
				im->picture[i][j] = 0;
			}
		}
}

void writeData(const char * fileName, image *img) {
	FILE * f = fopen(fileName,"wb");
	int i;

	if(f == NULL){
		return;
	} 

	fprintf(f,"%s\n", img->type);
	fprintf(f,"%d %d\n",img->width, img->height);
	fprintf(f,"%d\n",img->maxval);
	
	for(i = 0; i < img->height; i++){
		fwrite(img->picture[i], sizeof(unsigned char), img->width, f);
	}

	fclose(f);
}