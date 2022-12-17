#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>


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
    MPI_Datatype new_type;

    int count = 3;
    int blocklens[] = {1, 1, 1};

    MPI_Aint indices[3];
    indices[0] = (MPI_Aint)offsetof(Pixel, R);
    indices[1] = (MPI_Aint)offsetof(Pixel, G);
    indices[2] = (MPI_Aint)offsetof(Pixel, B);


    MPI_Datatype old_types[] = {MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR};

    MPI_Type_create_struct(count, blocklens, indices, old_types, &new_type);
    MPI_Type_commit(&new_type);

    return new_type;
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

    // DE PARALELIZAT
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
    int rank, proc;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc);

	Pixel **pixels = NULL;
	InfoHeader infoHeader;
	FileHeader fileHeader;
    FILE *out;
    int pix=3 * infoHeader.width, pad = 0;
    
    MPI_Datatype pixelDataType = createPixelType();
    
    if (rank == MASTER) {
        pixels = read_bmp("input/input.bmp", &infoHeader, &fileHeader, pixels);
        out = fopen("output/output-mpi.bmp","wb");
        if (out == NULL) {
            printf("error openining file\n");
        }

        write_header(&fileHeader, &infoHeader, "output/output-mpi.bmp");
        fseek(out, fileHeader.imageDataOffset, SEEK_SET);

        
        while (pix % 4 != 0) {
            pix++;
            pad++;
        }
    }

    int nrElements = infoHeader.height * infoHeader.width;
    
    Pixel **bwImage = allocPixelMatrix(infoHeader.height, infoHeader.width);
    Pixel *sendArray = (Pixel *) calloc(sizeof(Pixel), nrElements);
    Pixel *resultArray = (Pixel *) calloc(sizeof(Pixel), nrElements);

    if (rank == MASTER) {
         for (int i = 0; i < infoHeader.height; i++) {
            for (int j = 0; j < infoHeader.width; j++) {
                resultArray[i * proc + j] = pixels[i][j];
            }
        }
    }
    
    int nmin = nrElements / proc;
    int nextra = nrElements % proc;
    int k = 0;
    int *sendcounts = calloc(sizeof(int), proc);
    int *displs = calloc(sizeof(int), proc);


    for (int i = 0; i < proc; i++) {
        if (i < nextra){
            sendcounts[i] = nmin + 1;
        } 
        else {
            sendcounts[i] = nmin;
        }
        displs[i] = k;
        k += sendcounts[i];
    }
    int recvcount = sendcounts[rank];
    Pixel *recv_arr = calloc(sizeof(Pixel), recvcount);

    printf("inainte de scatter proc %d\n", rank);


    MPI_Scatterv(sendArray, sendcounts, displs, pixelDataType, recv_arr, recvcount, pixelDataType, MASTER, MPI_COMM_WORLD);

    for (int i = 0; i < recvcount; i++) {
        int bwPixelValue = (recv_arr[i].R + recv_arr[i].G + recv_arr[i].B) / 3;
        Pixel bwPixel;
        bwPixel.R = (unsigned char) bwPixelValue;
        bwPixel.G = (unsigned char) bwPixelValue;
        bwPixel.B = (unsigned char) bwPixelValue;
        recv_arr[i] = bwPixel;
    }

    printf("inainte de gather proc %d\n", rank);

    MPI_Gatherv(recv_arr, recvcount, pixelDataType, resultArray, sendcounts, displs, pixelDataType, MASTER, MPI_COMM_WORLD);
    printf("dupa gather proc %d\n", rank);

    if (rank == MASTER) {
        printf("master\n");
        // after calculations are done
        // for (int i = 0; i < infoHeader.height; i++) {
        //     for (int j = 0; j < infoHeader.width; j++) {
        //         bwImage[i][j] = resultArray[i * proc + j];
        //     }
        // }


        // for (int i = infoHeader.height - 1; i >= 0; i--) {
        //     for (int j = 0; j < infoHeader.width; j++) {
        //         writeImage(bwImage[i][j].R, bwImage[i][j].G, bwImage[i][j].B, out);
        //     }
        //     for (int k = 0 ;k < pad; k++) {
        //         fputc(0, out);
        //     }
        // }
    }
    
    MPI_Finalize();
	return 0;
}