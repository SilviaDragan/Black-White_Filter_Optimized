#include <stdio.h>
#include <stdlib.h>

#define PIXEL_ALB 255
#define NUL 0

typedef struct {
    unsigned char  fileMarker1; /* 'B' */
    unsigned char  fileMarker2; /* 'M' */
    unsigned int   bfSize; /* File's size */
    unsigned short unused1; /* Aplication specific */
    unsigned short unused2; /* Aplication specific */
    unsigned int   imageDataOffset; /* Offset to the start of image data */
} bmp_fileheader;

typedef struct {
    unsigned char R;
    unsigned char G;
    unsigned char B;
} TPixels;

typedef struct {
    unsigned int   biSize; /* Size of the info header - 40 bytes */
    signed int     width; /* Width of the image */
    signed int     height; /* Height of the image */
    unsigned short planes;
    unsigned short bitPix; /* Number of bits per pixel = 3 * 8 (for each channel R, G, B we need 8 bits */
    unsigned int   biCompression; /* Type of compression */
    unsigned int   biSizeImage; /* Size of the image data */
    int            biXPelsPerMeter;
    int            biYPelsPerMeter;
    unsigned int   biClrUsed;
    unsigned int   biClrImportant;
} bmp_infoheader;

/*functie pentru scrierea matricei de pixeli*/
void Write_Pixels(TPixels **image, int height, int width, int pad, FILE *out) {
	int i, j, q;
	for (int i = height - 1; i >= 0; --i)
	{
		for (j=0;j<width;j++) {
			fwrite(&image[i][j].R, sizeof(unsigned char), 1, out);
			fwrite(&image[i][j].G, sizeof(unsigned char), 1, out);
			fwrite(&image[i][j].B, sizeof(unsigned char), 1, out);
		}
		for (q=0;q<pad;q++) {
			fputc(NUL, out);
        }
	}
}

/*functie pentru scrierea a 3 pixeli*/
void write_image(unsigned char pix1, unsigned char pix2, unsigned char pix3, 
	FILE *out) {
	fwrite(&pix1, sizeof(unsigned char), 1, out);
	fwrite(&pix2, sizeof(unsigned char), 1, out);
	fwrite(&pix3, sizeof(unsigned char), 1, out);
}

void Pixel_Alb(FILE *out)
{
	/*scriere in fisier a pixelilor albi*/
	fputc(PIXEL_ALB, out);
	fputc(PIXEL_ALB, out);
	fputc(PIXEL_ALB, out);
}