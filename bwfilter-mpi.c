#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <stddef.h>

#define WHITE 255
#define MASTER 0
// #define HEIGHT_TAG 1
// #define WIDTH_TAG 2
#define IMAGE_TAG 3
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))


typedef struct {
    unsigned char  fileMarker1; /* 'B' */
    unsigned char  fileMarker2; /* 'M' */
    unsigned int   bfSize; /* File's size */
    unsigned short unused1; /* Aplication specific */
    unsigned short unused2; /* Aplication specific */
    unsigned int   imageDataOffset; /* Offset to the start of image data */
} FileHeader;

typedef struct {
    unsigned char R;
    unsigned char G;
    unsigned char B;
} Pixel;

typedef struct {
    unsigned int   biSize; /* Size of the info infoHeader - 40 bytes */
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
} InfoHeader;

MPI_Datatype createPixelType() {
   	const int nitems=3;
    int          blocklengths[3] = {1, 1, 1};
    MPI_Datatype types[3] = {MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR};
    MPI_Datatype mpi_pixel_struct_data;
    MPI_Aint     offsets[3];

    offsets[0] = offsetof(Pixel, R);
    offsets[1] = offsetof(Pixel, G);
	offsets[2] = offsetof(Pixel, B);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_pixel_struct_data);
    MPI_Type_commit(&mpi_pixel_struct_data);

	return mpi_pixel_struct_data;
}

Pixel **allocPixelMatrix(int lines, int columns) {
	Pixel **image = (Pixel **)malloc(sizeof(Pixel *) * lines);
	if (image == NULL){
		printf("alloc error \n");
		return NULL;
	}
	for (int i = 0; i < lines; ++i) {
		image[i]=(Pixel *)malloc(sizeof(Pixel) * columns);
		if (image[i] == NULL) {
			printf("alloc error \n");
			return NULL;
		}
	}

    return image;
}

void freeMatrix(int **mat, int n) {
	for (int i = 0; i < n; i++) {
        free(mat[i]);
    }
	free(mat);
}

void writePixels(Pixel **image, int height, int width, int pad, FILE *out) {
	for (int i = height - 1; i >= 0; --i) {
		for (int j = 0; j < width; j++) {
			fwrite(&image[i][j].R, sizeof(unsigned char), 1, out);
			fwrite(&image[i][j].G, sizeof(unsigned char), 1, out);
			fwrite(&image[i][j].B, sizeof(unsigned char), 1, out);
		}
		for (int k = 0; k < pad; k++) {
			fputc(0, out);
        }
	}
}

void writeImage(unsigned char pix1, unsigned char pix2, unsigned char pix3, FILE *out) {
	fwrite(&pix1, sizeof(unsigned char), 1, out);
	fwrite(&pix2, sizeof(unsigned char), 1, out);
	fwrite(&pix3, sizeof(unsigned char), 1, out);
}

/*scrierea a structurilor FileHeader InfoHeader*/
void write_header(FileHeader *fileHeader, InfoHeader *infoHeader, char *outfile) {
	FILE *out=fopen(outfile, "wb");
	if (out==NULL) {	
		printf("error opening file \n");
		return;
	}

	fwrite(&fileHeader->fileMarker1, sizeof(char), 1, out);
	fwrite(&fileHeader->fileMarker2, sizeof(char), 1, out);
	fwrite(&fileHeader->bfSize, sizeof(int), 1, out);
	fwrite(&fileHeader->unused1, sizeof(short), 1, out);
	fwrite(&fileHeader->unused2, sizeof(short), 1, out);
	fwrite(&fileHeader->imageDataOffset, sizeof(int), 1, out);
	fwrite(&infoHeader->biSize, sizeof(int), 1, out);
	fwrite(&infoHeader->width, sizeof(int), 1, out);
	fwrite(&infoHeader->height, sizeof(int), 1, out);
	fwrite(&infoHeader->planes, sizeof(short), 1, out);
	fwrite(&infoHeader->bitPix, sizeof(short), 1, out);
	fwrite(&infoHeader->biCompression, sizeof(int), 1, out);
	fwrite(&infoHeader->biSizeImage, sizeof(int), 1, out);
	fwrite(&infoHeader->biXPelsPerMeter, sizeof(int), 1, out);
	fwrite(&infoHeader->biYPelsPerMeter, sizeof(int), 1, out);
	fwrite(&infoHeader->biClrUsed, sizeof(int), 1, out);
	fwrite(&infoHeader->biClrImportant, sizeof(int), 1, out);
	return;
}

Pixel **read_bmp(char *fin, InfoHeader *infoHeader, FileHeader *fileHeader, Pixel **image) {
	FILE *in = fopen(fin, "rb");
	if (in == NULL) {
        printf("error readin input file\n");
		return NULL;
    }
		
	fread(&fileHeader->fileMarker1, sizeof(char), 1, in);
	fread(&fileHeader->fileMarker2, sizeof(char), 1, in);
	fread(&fileHeader->bfSize, sizeof(int), 1, in);
	fread(&fileHeader->unused1, sizeof(short), 1, in);
	fread(&fileHeader->unused2, sizeof(short), 1, in);
	fread(&fileHeader->imageDataOffset, sizeof(int), 1, in);
	fread(&infoHeader->biSize, sizeof(int), 1, in);
	fread(&infoHeader->width, sizeof(int), 1, in);
	fread(&infoHeader->height, sizeof(int), 1, in);
	fread(&infoHeader->planes, sizeof(short), 1, in);
	fread(&infoHeader->bitPix, sizeof(short), 1, in);
	fread(&infoHeader->biCompression, sizeof(int), 1, in);
	fread(&infoHeader->biSizeImage, sizeof(int), 1, in);
	fread(&infoHeader->biXPelsPerMeter, sizeof(int), 1, in);
	fread(&infoHeader->biYPelsPerMeter, sizeof(int), 1, in);
	fread(&infoHeader->biClrUsed, sizeof(int), 1, in);
	fread(&infoHeader->biClrImportant, sizeof(int), 1, in);
	fseek(in, fileHeader->imageDataOffset, SEEK_SET);
	
    int pix = 3 * infoHeader->width, pad = 0;

	while (pix % 4 != 0) {
		pad++;
		pix++;
	}

	image = allocPixelMatrix(infoHeader->height, infoHeader->width);

	for (int i = infoHeader->height - 1; i >= 0; i--) {
		for (int j = 0; j < infoHeader->width; j++) {
			fread(&image[i][j].R, sizeof(unsigned char), 1, in);
			fread(&image[i][j].G, sizeof(unsigned char), 1, in);
			fread(&image[i][j].B, sizeof(unsigned char), 1, in);
		}
		fseek(in, pad, SEEK_CUR);
	}
	fclose(in);
	return image;
}

void applyBWFilter(Pixel **image, char *out_black_white, FileHeader *fileHeader, InfoHeader *infoHeader) {
	FILE *out = fopen(out_black_white,"wb");
	if (out == NULL) {
        printf("error openining file\n");
    }

	write_header(fileHeader, infoHeader, out_black_white);
	fseek(out, fileHeader->imageDataOffset, SEEK_SET);

	int pix=3 * infoHeader->width, pad = 0;
	while (pix % 4 != 0) {
		pix++;
		pad++;
	}

    Pixel **bwImage = allocPixelMatrix(infoHeader->height, infoHeader->width);

	for (int i = infoHeader->height - 1; i >= 0; i--) {
		for (int j = 0; j < infoHeader->width; j++) {
			int bwPixelValue = (image[i][j].R + image[i][j].G + image[i][j].B) / 3;
            Pixel bwPixel;
            bwPixel.R = (unsigned char) bwPixelValue;
            bwPixel.G = (unsigned char) bwPixelValue;
            bwPixel.B = (unsigned char) bwPixelValue;
            bwImage[i][j] = bwPixel;
		}
	}

	for (int i = infoHeader->height - 1; i >= 0; i--) {
		for (int j = 0; j < infoHeader->width; j++) {
			writeImage(bwImage[i][j].R, bwImage[i][j].G, bwImage[i][j].B, out);
		}
		for (int k = 0 ;k < pad; k++) {
			fputc(0, out);
        }
	}

	return;
}

int main(int argc, char **argv) {
    int rank, proc, a;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc);

	Pixel **pixels = NULL;
	InfoHeader infoHeader;
	FileHeader fileHeader;
    MPI_Datatype pixelDataType = createPixelType();
    FILE *out;
    int pix, pad;

    if (rank == MASTER) {
        pixels = read_bmp(strdup("input/input.bmp"), &infoHeader, &fileHeader, pixels);
        out = fopen("output/output-mpi.bmp","wb");
        if (out == NULL) {
            printf("error openining file\n");
        }

        write_header(&fileHeader, &infoHeader, strdup("output/output-mpi.bmp"));
        fseek(out, fileHeader.imageDataOffset, SEEK_SET);
        pix=3 * infoHeader.width;
        pad = 0;

        while (pix % 4 != 0) {
            pix++;
            pad++;
        }

        Pixel **bwImage = allocPixelMatrix(infoHeader.height, infoHeader.width);


        for (int p = 1; p < proc; p++) {
            MPI_Send(&infoHeader.height, 1, MPI_INT, p, 0, MPI_COMM_WORLD);
            MPI_Send(&infoHeader.width, 1, MPI_INT, p, 0, MPI_COMM_WORLD);
            for (int i = 0; i < infoHeader.height; i++) {
                MPI_Send(&(pixels[i][0]), infoHeader.width, pixelDataType, p, 0, MPI_COMM_WORLD);
            }
        }

    } else {
        MPI_Status status;
        int height, width;
        MPI_Recv(&height, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&width, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	    Pixel **originalPicture = allocPixelMatrix(height, width);

        for (int i = 0; i < height; i++) {
            MPI_Recv(&originalPicture[i][0], width, pixelDataType, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }

        int start = rank * (double) height / proc;
        int end = MIN((rank + 1) * (double) height / proc, height);

        for (int i = start; i < end; i++) {
            for (int j = 0; j < width; j++) {
                int bwPixelValue = (originalPicture[i][j].R + originalPicture[i][j].G + originalPicture[i][j].B) / 3;
                Pixel bwPixel;
                bwPixel.R = (unsigned char) bwPixelValue;
                bwPixel.G = (unsigned char) bwPixelValue;
                bwPixel.B = (unsigned char) bwPixelValue;
                originalPicture[i][j] = bwPixel;

            }
        }

        for (int i = start; i < end; i++) {
            MPI_Send(&(originalPicture[i][0]), width, pixelDataType, MASTER, 0, MPI_COMM_WORLD);
        }

    }

    if (rank ==  MASTER) {
        Pixel **bwImage = allocPixelMatrix(infoHeader.height, infoHeader.width);

        int end = MIN((rank + 1) * (double) infoHeader.height / proc, infoHeader.height);
        for (int i = 0; i < end; i++) {
            for (int j = 0; j < infoHeader.width; j++) {
                int bwPixelValue = (pixels[i][j].R + pixels[i][j].G + pixels[i][j].B) / 3;
                Pixel bwPixel;
                bwPixel.R = (unsigned char) bwPixelValue;
                bwPixel.G = (unsigned char) bwPixelValue;
                bwPixel.B = (unsigned char) bwPixelValue;
                bwImage[i][j] = bwPixel;
            }
        }


        for (int p = 1; p < proc; p++) {
            int pstart = p * (double) infoHeader.height / proc;
            int pend = MIN((p + 1) * (double) infoHeader.height / proc, infoHeader.height);
            for (int i = pstart; i < pend; i++) {
                MPI_Recv(&(bwImage[i][0]), infoHeader.width, pixelDataType, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        // after calculations are done
        for (int i = infoHeader.height - 1; i >= 0; i--) {
            for (int j = 0; j < infoHeader.width; j++) {
                writeImage(bwImage[i][j].R, bwImage[i][j].G, bwImage[i][j].B, out);
            }
            for (int k = 0 ; k < pad; k++) {
                fputc(0, out);
            }
        }
    }
    
    MPI_Finalize();
	return 0;
}