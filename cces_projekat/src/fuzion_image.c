/*****************************************************************************
 * fuzion_image.c
 *****************************************************************************/


#include <sys/platform.h>

#include <def21489.h>
#include <sru21489.h>
#include <SYSREG.h>
#include <cycle_count.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>



#include "adi_initialize.h"
#include "fuzion_image.h"

  /*OBRADA JPG SLIKA*/

/*
#include "ulazna_slikaA_jpg.h"
#include "ulazna_slikaB_jpg.h"
*/

  /*OBRADA BMP SLIKA*/


#include "ulazna_slikaA_bmp.h"
#include "ulazna_slikaB_bmp.h"


#define SIZE ( WIDTH * HEIGHT )
#define SIZE_EXTREMA 1000
#define EPSILON 500


#define FIXED_POINT_SCALE (256)


typedef struct {
    int idx_value;
    unsigned int  value;
} Extremum;


#pragma section("seg_sdram1")
unsigned char pixelsA[SIZE];

#pragma section("seg_sdram1")
unsigned  char pixelsB[SIZE];

#pragma section("seg_sdram1")
Extremum maxPoints[SIZE_EXTREMA];

#pragma section("seg_sdram1")
Extremum minPoints[SIZE_EXTREMA];

#pragma section("seg_sdram1")
unsigned char up_envelope[SIZE];

#pragma section("seg_sdram1")
unsigned char down_envelope[SIZE];

#pragma section("seg_sdram1")
unsigned char mean_envelope[SIZE];

#pragma section("seg_sdram1")
unsigned char imfA[SIZE];

#pragma section("seg_sdram1")
unsigned char imfB[SIZE];

#pragma section("seg_sdram1")
unsigned char pixels_imfA[HEIGHT][WIDTH];

#pragma section("seg_sdram1")
unsigned char pixels_imfB[HEIGHT][WIDTH];

#pragma section("seg_sdram1")
unsigned char mask[HEIGHT][WIDTH];

#pragma section("seg_sdram1")
unsigned char pixelsOutput[SIZE];


cycle_t count_start;
cycle_t count_stop;


void convert_rgb_to_grayscale(const unsigned char* pixels ,int slika);
void save_image_to_header(const char *filename, unsigned char* pixels);


void interpolate_envelope(unsigned char *signal, Extremum *extrema_idx, int num_extrema, unsigned char *envelope);
void emd(unsigned char* signal, unsigned char* imf);
int compute_local_variance(unsigned char pixels [HEIGHT][WIDTH], int x, int y);
void generate_decision_mask(void );
void generate_focused_image(unsigned char * pixA  , unsigned char * pixB);


int main(int argc, char *argv[])
{
	 printf("Pocetak izvrsavanja programa  .\n");



	 /*OBRADA JPG SLIKE*/

     convert_rgb_to_grayscale(ulazna_slikaA_bmp, 1);
	 convert_rgb_to_grayscale(ulazna_slikaB_bmp, 2);

	 /*OBRADA BMP SLIKE*/
/*
	 START_CYCLE_COUNT(count_start);
	 convert_rgb_to_grayscale(ulazna_slikaA_jpg, 1);
	 convert_rgb_to_grayscale(ulazna_slikaB_jpg, 2);
	 STOP_CYCLE_COUNT(count_stop, count_start);
	 PRINT_CYCLES("Konvertovanje slika u gray scale: ", count_stop);
*/


	 START_CYCLE_COUNT(count_start);
	 printf("Pocinjem EMD ....\n");
     emd(pixelsA, imfA);

     emd(pixelsB, imfB);
     printf("Zavrsen EMD ....\n");
     STOP_CYCLE_COUNT(count_stop, count_start);
   	 PRINT_CYCLES("Racunanje emd ulaznih slika: ", count_stop);


     START_CYCLE_COUNT(count_start);
     for (int y = 0; y < HEIGHT; y++) {
         memcpy(pixels_imfA[y], &imfA[y * WIDTH], WIDTH * sizeof(unsigned char));
     }

     for (int y = 0; y < HEIGHT; y++) {
             memcpy(pixels_imfB[y], &imfB[y * WIDTH], WIDTH * sizeof(unsigned char));
         }
     STOP_CYCLE_COUNT(count_stop, count_start);
   	 PRINT_CYCLES("Konverzija 1D vektora u 2D: ", count_stop);

     START_CYCLE_COUNT(count_start);
     generate_decision_mask();
     STOP_CYCLE_COUNT(count_stop, count_start);
     PRINT_CYCLES("Generisanje slike maske: ", count_stop);


     START_CYCLE_COUNT(count_start);
     generate_focused_image(pixelsA, pixelsB);
     STOP_CYCLE_COUNT(count_stop, count_start);
   	 PRINT_CYCLES("Formatiranje slike u punom fokusu: ", count_stop);

   	 START_CYCLE_COUNT(count_start);
	 save_image_to_header("izlazna_slika.h",  pixelsOutput);
  	 STOP_CYCLE_COUNT(count_stop, count_start);
  	 PRINT_CYCLES("Pisanje izlazne slike u header fajl: ", count_stop);



	    printf("Gotovo!\n");
	    return 0;
}


void convert_rgb_to_grayscale(const unsigned char* pixels, int slika) {
	unsigned char r,g,b;
	if(1 == slika){
		 for (int i = 0, j = 0; i < WIDTH * HEIGHT * 3; i += 3, j++) {
		         r = pixels[i];
		         g = pixels[i + 1];
		         b = pixels[i + 2];
		        pixelsA[j] = (unsigned char)((77 * r + 150 * g + 29 * b) >> 8);
		    }
		printf("Zavrseno konvertovanje slike A u gray scale.\n");
	}else{
		for (int i = 0, j = 0; i < WIDTH * HEIGHT * 3; i += 3, j++) {
			  r = pixels[i];
			  g = pixels[i + 1];
		      b = pixels[i + 2];
		      pixelsB[j] = (unsigned char)((77 * r + 150 * g + 29 * b) >> 8);
			   }
		printf("Zavrseno konvertovanje slike B u gray scale.\n");
	}

}



void save_image_to_header(const char *filename, unsigned char* pixels) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Greska pri otvaranju fajla!\n");
        return;
    }

    fprintf(file, "#ifndef GRAYSCALE_IMAGE_H\n#define GRAYSCALE_IMAGE_H\n\n");
    fprintf(file, "#define WIDTH %d\n#define HEIGHT %d\n\n", WIDTH, HEIGHT);
    fprintf(file, "const unsigned char izlazna_slika[%d] = {\n", SIZE);
    for (int i = 0; i < SIZE; i++) {
        fprintf(file, "%u, \n", pixels[i]);
    }

    fprintf(file, "\n};\n\n#endif // GRAYSCALE_IMAGE_H\n");
    fclose(file);

    printf("Zavrsen upis  izlazne slike .\n");
}



void interpolate_envelope(unsigned char *signal, Extremum *extrema_idx, int num_extrema, unsigned char *envelope) {
    if (num_extrema < 2) {
        printf("Nema dovoljno ekstrennih tacaka.  \n");
        return;
    }

    int first_extreme = extrema_idx[0].idx_value;
    int last_extreme = extrema_idx[num_extrema - 1].idx_value;

    /* Popunjavanje pocetne i krajnje vrednosti pre interpolacije*/
    for (int i = 0; i < first_extreme; i++) {
        envelope[i] = signal[first_extreme];
    }
    for (int i = last_extreme; i < SIZE; i++) {
        envelope[i] = signal[last_extreme];
    }

    /* Interpolacija izmeðu ekstremnih taèaka*/
    int start, end, j;
    unsigned char start_value, end_value, t;
    for (int i = 0; i < num_extrema - 1; i++) {
        start = extrema_idx[i].idx_value;
        end = extrema_idx[i + 1].idx_value;

        start_value = signal[start];
        end_value = signal[end];

        for (j = start; j <= end; j++) {
            t = (j - start) / (unsigned char)(end - start);
            envelope[j] = (unsigned char)((1 - t) * start_value + t * end_value);
        }
    }
}



void emd(unsigned char* signal, unsigned char* imf) {
    int max_count = 0, min_count = 0;

    for (int i = 1; i < SIZE - 1; i++) {
        if (signal[i] > signal[i - 1] && signal[i] > signal[i + 1]) {
        	maxPoints[max_count].idx_value = i;
            maxPoints[max_count].value = signal[i];
             max_count++;

        }
    }
    for (int i = 1; i < SIZE - 1; i++) {
        if (signal[i] < signal[i - 1] && signal[i] < signal[i + 1]) {
        	minPoints[min_count].idx_value = i;
            minPoints[min_count].value = signal[i];
             min_count++;
        }
    }

    interpolate_envelope(signal, maxPoints, max_count, up_envelope);
    interpolate_envelope(signal, minPoints, min_count, down_envelope);

    for (int i = 0; i < SIZE; i++) {
        mean_envelope[i] = (up_envelope[i] + down_envelope[i]) >> 1;
    }

    for (int i = 0; i < SIZE; i++) {
        imf[i] = signal[i] - mean_envelope[i];
    }
}


int compute_local_variance(unsigned char pixels [HEIGHT][WIDTH], int x, int y) {
    int mean = 0, variance = 0;
    int count = 0;
    int nx, ny;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
             nx = x + i;
             ny = y + j;
            if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT) {
                mean += pixels[nx][ny];
                count++;
            }
        }
    }
    mean /= count;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
             nx = x + i;
             ny = y + j;
           if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT) {
                variance += (pixels[nx][ny] - mean) * (pixels[nx][ny] - mean);
            }
        }
    }

    return variance / count;
}

void generate_decision_mask( ){

    int varA, varB;
    int diff;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
             varA = compute_local_variance(pixels_imfA, x, y);
             varB = compute_local_variance(pixels_imfB, x, y);
             diff = varA - varB;
    // printf("%d \n",diff);
             if (diff < -EPSILON) {
                  mask[y][x] = 0;
             } else if (diff < EPSILON) {
                 mask[y][x] = FIXED_POINT_SCALE / 2;
             } else {
                  mask[y][x] = FIXED_POINT_SCALE;
             }
        }
    }
    printf("Genrisana maska odlucivanja.. \n");
}

void generate_focused_image(unsigned char * pixA  , unsigned char * pixB){
	int idx;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
             idx = y * WIDTH + x;

            if (mask[y][x] == 0) {
                pixelsOutput[idx] = pixA[idx];
            }
            else if (mask[y][x] == FIXED_POINT_SCALE) {
                pixelsOutput[idx] = pixB[idx];
            }
            else {
                pixelsOutput[idx] = (pixA[idx] + pixB[idx]) >> 1 ;
            }
        }
    }
    printf("Generisanje slike izlaza zavrseno. \n");
}




