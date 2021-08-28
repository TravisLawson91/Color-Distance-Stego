#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "set.h"

/* compile as gcc -m32 -g -o bmp bitmap.c setLinkedListImp.c -lm*/

/********************************************************************************
 * 			    bitmap.c
 * 	Written by: Travis Lawson, Rakerd Calhoun, and Joshua Romero
 *
 * Purpose:
 * 	The purpose of this program is to hide a payload in a bitmap
 * 	image, and extract the payload at a later time.
 * 	
 * 	We based our project on the paper written by Dr. Jiri Fridrich,
 * 	    A New Steganographic Method for Palette-Based Images
 *
 * 	The program will take a 256 color bitmap and hide a payload 
 * 	based on the closest color of the current pixel color. When
 * 	we find the closest color, a we calculate the parity of the 
 * 	RBG values from the palette entry referenced by the pixel.
 * 	If the parity matches, we update the pixel index to reference
 * 	the new palette entry. If the parity of the current closest color * 	does not match our message bit, we check to see if the next 
 * 	closest color will. 
 *
 *  	Best result is the image is a error-difused bitmap
 *
 *	Commands:
 *		To hide:
 *			./bmp -hide [cover image] [payload]
 *		To extract:
 *			./bmp -extract outfile.bmp
 *
 *	Compile as 
 *		gcc -m32 -g -o bmp bitmap.c setLinkedListImp.c -lm
 *
 *	Notes:
 *		'outfile.bmp' is the name of the stego-image produced when hiding.
 *		'recovered' is the file name of the recovered payload.
 ***********************************************************************************/


#pragma pack(push, 1)
typedef struct BITMAPFILEHEADER{
	unsigned char bfType[2];
	unsigned int bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int bfOffbits;
} BITMAPFILEDHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct BITMAPINFOHEADER{
	unsigned int biSize; // DWORD
	long biWidth;
	long biHeight;
	unsigned short biPlanes; // WORD
	unsigned short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct RGBQUAD{
	//unsigned char rgb[4];
	unsigned char BLU;
	unsigned char GRN;
	unsigned char RED;
	unsigned char RES;
} RGBQUAD[];
#pragma pack(pop)

/* Prototypes */
unsigned char *loadBitMap( char *filename,
		struct RGBQUAD c[256], 
		struct BITMAPFILEHEADER *bmpFileHeader, 
		struct BITMAPINFOHEADER *bmpInfoHeader);

void printData( struct RGBQUAD c[256], 
		struct BITMAPFILEHEADER bmpFileHeader, 
		struct BITMAPINFOHEADER bmpInfoHeader);

void writeFile( struct RGBQUAD c[256],
		struct BITMAPFILEHEADER bmpFileHeader,
		struct BITMAPINFOHEADER bmpInfoheader,
		unsigned char *bmpData);

unsigned char *convertToBinary(char *filename, 
			       int *size);

unsigned char *hideMessage(struct RGBQUAD p[256], 
			   unsigned char *cvrImg, 
			   unsigned char *msgDataBin, 
			   int msgSize, 
			   unsigned int cvrSize);

void extractPayload(unsigned char *bmpData, 
		    struct RGBQUAD p[256], 
		    int cvrSize);



/*************** main ***************
 * Purpose: It's main, it's needed to run
 * 	    the program.
 *
 ************************************/
int main(int argc, char *argv[]){
	
	printf("Group 5 - Travis Lawson, Joshua Romero, Rakerd Calhoun\n");

	/* parts that make up a paletted bitmap image */
	struct BITMAPFILEHEADER bmpFileHeader;
        struct BITMAPINFOHEADER bmpInfoHeader;
        struct RGBQUAD palette[256];
        unsigned char *bmpData;

       	/* variable used for our hidden message */
       	unsigned char *msgDataBinary;
        int msgSizeBinary;
	
	if( argc == 1){
		fprintf(stderr, "Usage ./bmp -hide [cover image].bmp [payload].bmp\n" \
				"      ./bmp -extract outfile.bmp\n");
		exit(-1);
	} else if( (strcmp(argv[1], "-hide") == 0) && argc == 4){
		/* used to calculate if the cover can hide the payload */
		struct stat statCover, statPayload;
		stat(argv[2], &statCover);	// setting up our stats for the cover image
		stat(argv[3], &statPayload);	// setting up our stats for the payload

		/* calcuate the number of bits for the payload
		 * if the bits exceed the number of bytes
		 * the cover image cannot conceal the payload
		 */
		if(statPayload.st_size * 8 > statCover.st_size){
			fprintf(stderr, "The payload size in bits, %ld, will not fit in the cover image of %ld bytes\n",
					statPayload.st_size * 8, statCover.st_size);
			exit(-1);
		}

		// load our cover image into memory
        	bmpData = loadBitMap(argv[2], palette, &bmpFileHeader, &bmpInfoHeader);
		// covert our payload to binary
        	msgDataBinary = convertToBinary(argv[3], &msgSizeBinary);

        	unsigned char *newBmpData;
		// hide the payload in the cover, returns a pointer to the altered bitmap data
		// altered bitmap data has updated pixel indexes that reference new palette colors
        	newBmpData = hideMessage(palette, bmpData, msgDataBinary, msgSizeBinary, bmpInfoHeader.biSizeImage);

		//write the stego image to the file.
       		writeFile(palette, bmpFileHeader, bmpInfoHeader, bmpData);
	
		free(bmpData);
		free(msgDataBinary);

		printf("Payload %s has been hidden in %s as outfile.bmp\n", argv[3], argv[2]);

	} else if( strcmp(argv[1], "-extract") == 0 && strcmp(argv[2], "outfile.bmp") == 0 && argc == 3){
		
		// load our stego image into memory
		bmpData = loadBitMap(argv[2], palette, &bmpFileHeader, &bmpInfoHeader);
		// extract the payload and reassemble
		extractPayload(bmpData, palette, bmpInfoHeader.biSizeImage);

		free(bmpData);

		printf("Payload has been extracted from %s as 'recovered'\n", argv[2]);
	} else {
		fprintf(stderr, "Usage ./bmp -hide [cover image].bmp [payload].bmp\n" \
				"      ./bmp -extract output.bmp\n");
		exit(-1);
	}
			
	return 0;
}

unsigned char *hideMessage(struct RGBQUAD p[256], unsigned char *cvrImg, unsigned char *msgDataBin, int msgSize, unsigned int cvrSize){
	
	srand((unsigned int) 76);
	unsigned char r1, g1, b1, r2, g2, b2;
	double close, tmp;
	int palIndex, binIndex, pixel;
	int i; 
	///////////////////////////////
	int cacheOfIndex[256][2]; //Stores a cache of closest color indexs of both 0 parity and 1 parity.
	memset(cacheOfIndex, -1, sizeof(cacheOfIndex));//Initialized to -1 to act as a null case.
	///////////////////////////////

	
	setADT colorSet; // init a linked list
	binIndex = 0;    // used to keep track of where we are in the binary stream

	/*
	 * This will begin hidng our payload. We will increase
	 * the index each time we hid a bit. 
	 *
	 * To hide bits, this is done psuedo randomly, based on
	 * a see selected above, currently hardcoded, or added security
	 * this use be user selected.
	 *
	 * once we have are seed selected we can  now choose a random pixel
	 * based on the cover image size. We then will calculate the color
	 * distance using 
	 * sqrt( pow(r1 - r2, 2) + pow(g1 - g2, 2) + pow(b1 - b2, 2) );
	 * we will keep track of the closest value to our current pixel color
	 * RBG1. Once the closed color is found, we check the parity of the 
	 * found color, R+G+B mod 2, and if this value matches of payload bit
	 * we change the pixel index to reference a new entry in the palette.
	 *
	 */
	// close = 0.0;
	int pIndex[256] = {-1}; // used to set out pixel index to reference a new palette entry
	int rgb; // used to calculate the parity bit used to hide our message

	pixel = 0;
	while( binIndex != msgSize) {
		colorSet = setNew();
		//pixel = rand() % cvrSize;
		///////////////////////////////////////
		if (cacheOfIndex[cvrImg[pixel]][msgDataBin[binIndex]] != -1){ //Check if value has been solved already
			cvrImg[pixel] = cacheOfIndex[cvrImg[pixel]][msgDataBin[binIndex]]; //If in cache use cached answer
			binIndex++;		//Then move on to the next bit
			pixel++;
			continue; //Return to the top of the While Loop
		}
		///////////////////////////////////////
		//for(i = cvrImg[pixel]; i < 255; i++){ // i will need to equal the index of the pixel we're on
		for(i = 0; i < 256; i++){ // It should iterate through every palette color, 
					  // since it's just looking for the closest palette color palette-wise and 
					  // that isn't limited to the palette numbers after it but EVERY palette option 
					  // Just because we're initially looking at palette index 200 doesn't mean that 
					  // we can only use 200-254, we should use 0-255 for every index.
			// palette entry of the currect pixel
			r1 = p[cvrImg[pixel]].RED;
			g1 = p[cvrImg[pixel]].GRN;
			b1 = p[cvrImg[pixel]].BLU;

			// ith palette entry
			r2 = p[i].RED;
			g2 = p[i].GRN;
			b2 = p[i].BLU;

			// calculate the color distance
			close = sqrt( pow(r1 - r2, 2) + pow(g1 - g2, 2) + pow(b1 - b2, 2) );
			
			/*
			 * instert the color distance and the accompanying index
			 * into an linked list. The link list will be sorted each time
			 * a new node is inserted.
			 */
			setInsertElementSorted(colorSet, close, i);
		}

		//setPrint(colorSet);
		setColorDistance(colorSet, pIndex);
		for(i = 0; i < 256; i++){ // This should potentially iterate through every value 
					  // inlcuding the closest which would be itself because if 
					  // it matches the parity already we don't have to change it. 
					  // That way the message bit still matches and it's even less detectable since we change nothing.
			rgb = p[ pIndex[i] ].RED + p[ pIndex[i] ].GRN + p[ pIndex[i] ].BLU;
			if( rgb % 2  == msgDataBin[binIndex] ){
				///////////////////////////////////////////
				//Once the closest color for that parity is discovered it is 
				//saved to the cache for faster reference in the future
				cacheOfIndex[cvrImg[pixel]][msgDataBin[binIndex]] = pIndex[i]; 
				///////////////////////////////////////////
				
				//The pixel's value is then assigned to the new index
				cvrImg[pixel] = pIndex[i];
				binIndex++;
				pixel++;
				break;
			}
		}

		// free our linked list
		clearSet(colorSet);
	}
		
	return cvrImg; 
	
}
/******************** convertToBinary ********************
 * Purpose:
 * 	Traverse a given file byte by byte and 
 * 	convert the ascii representation to binary.
 *
 * 	Function will return an array of 1s and 0s
 * 	off the file so that it can be hidden within
 * 	the cover image.
 *********************************************************/
unsigned char *convertToBinary (char *fileName, int *size){
	FILE *fptr;
	struct stat statBuff;
	fptr = fopen(fileName, "rb");
	if(fptr == NULL){
		fprintf(stderr, "Unable to open file %s\n", fileName);
		exit(-1);
	}
	
	stat(fileName, &statBuff);

	char *bin;
	bin = (char *) malloc( (statBuff.st_size * 8) + 32);
	// the + 32 will be the bits required to hide the size of the payload
	// so at the start of the extraction processes, the payload size will
	// not be required.
	if(bin == NULL){
		fprintf(stderr, "unable to allocate memory in convertToBinary\n");
		exit(-1);
	}
	
	int i,j, index;
	int msgSize = statBuff.st_size;
	index = 0; 
	
	/*
	 * conver the file size to binary
	 * and store the corresponding 0s and 1s
	 * in the bin array.
	 */
	while( index != 32){
		for(i = 0; i < 32; i++){
			if(msgSize % 2 == 1)
				bin[index] = 1;
			else
				bin[index] = 0;
		}
		msgSize = msgSize / 2;
		index++;
	}

	unsigned char *input;
	input = (char *) malloc( statBuff.st_size );
	if( input == NULL ){
		fprintf(stderr, "Malloc failed in converToBinary\n");
		exit(-1);
	}
	/* may still have an issue here */
	//fgets(input, statBuff.st_size, fptr);
	size_t result;
	result = fread(input, sizeof(unsigned char), statBuff.st_size, fptr);

	/* loop through the length of the message and
	 * will shift bits to the LSB and will mask
	 * with 1 to isoloate the targetd bit. Once we
	 * have the bit it will be placed a array for
	 * easy use when we begin hiding.
	 */
	for( i = 0; i < statBuff.st_size; i++){
		for( j = 7; j >= 0; j--){
			bin[index] = input[i] >> j & 1;
			index++;
		}
	}
	fclose(fptr);

	/* provides the element count of the binary arry
	 * so it can be used in other functions.
	 */
	*size = statBuff.st_size * 8;
	return bin;
}

/***************** writeFile ****************
 * Purpose: Currently takes the information of
 * 	    a provided bitmap image and writes to
 * 	    a new file.
 *
 * 	    Needs to be adjusted to write to file
 * 	    the bitmap that carries the payload.
 * 
 ********************************************/
void writeFile( struct RGBQUAD c[256],
		struct BITMAPFILEHEADER bmpFileHeader,
		struct BITMAPINFOHEADER bmpInfoHeader,
		unsigned char *bmpData){

	FILE *out;
	out = fopen("outfile.bmp", "wb");
	if(out == NULL){
		fprintf(stderr, "Failed to Open output file\n");
		exit(-1);
	}

	/* steps to write a bit map to file */
	fwrite(&bmpFileHeader, sizeof(struct BITMAPFILEHEADER), 1, out);
	fwrite(&bmpInfoHeader, sizeof(struct BITMAPINFOHEADER), 1, out);

	int i;
	for(i = 0; i < 256; i++)
		fwrite(&c[i], sizeof(struct RGBQUAD), 1, out);

	fwrite(bmpData, bmpInfoHeader.biSizeImage, 1, out);
	fclose(out);
}

/************************ loadBitMap *****************************
 * Purpose: Opens provided bitmap cover image, and extracts
 * 	    the file header, bitmap information header, color
 * 	    palette, and the image data.
 *
 * 	    The image data will be required to help calculation the 
 * 	    color distance for hiding the payload.
 *******************************************************************/
unsigned char *loadBitMap(char *filename, struct RGBQUAD c[256], 
		struct BITMAPFILEHEADER *bmpFileHeader, 
		struct BITMAPINFOHEADER *bmpInfoHeader){

	FILE *fPtr;
	fPtr = fopen(filename, "rb");
	if(fPtr == NULL){
		fprintf(stderr, "Unable to open file %s\n", filename);
		exit(-1);
	}

	// read bitmap file header
	fread(bmpFileHeader, sizeof(struct BITMAPFILEHEADER),1, fPtr);
	/* check to make sure provided file is a bitmap */
	if(bmpFileHeader->bfType[0] != 'B' && bmpFileHeader->bfType[1] != 'M'){
		fprintf(stderr, "%s is not a bmp image\n", filename);
		exit(-1);
	} 
	
	// read bitmap information header
	fread(bmpInfoHeader, sizeof(struct BITMAPINFOHEADER), 1, fPtr);
	/* check to see if the bitmap is an 8-bit bitmap */ 
	if( bmpInfoHeader->biBitCount != 8){
		fprintf(stderr, "%s is not a 8-bit bmp image\n", filename);
		exit(-1);
	}

	// read in the palette 
	int i;
	for(i = 0; i < 256; i++)
		fread(&c[i], sizeof(struct RGBQUAD), 1, fPtr);

	// read in image
	unsigned char *bmpImg; // store image data
	bmpImg = (unsigned char *) malloc(bmpInfoHeader->biSizeImage);
	if(bmpImg == NULL){
		fprintf(stderr, "Unable to malloc in loadBitMap\n");
		exit(-1);
	}
	fread(bmpImg, bmpInfoHeader->biSizeImage, 1, fPtr);
	
	fclose(fPtr);
	return bmpImg;
}

/**************************** extractPayload **********************************
 *
 * Purpose:
 * 	To extract the payload for our stego-imgae 'output.bmp'.
 * 	This function will loop throught the stego-image and calculate
 * 	the parity bit for each pixel that contains the updated index. 
 * 	Once the parity bits have been recovered, the function will begin to
 * 	set the bits back in the correct order for each byte of data.
 *
 * 	When was the data has been successfully reordered, it will write the
 * 	data to a file called 'recovered'
 *
 * ***************************************************************************/
void extractPayload(unsigned char *bmpData, struct RGBQUAD p[256], int cvrSize){
	srand((unsigned int) 76);
        int i, pixel, j;
        int binSize[32] = {0}; // zero set array
        int index = 0;
        j = 0;
        pixel = 0;

	/*
	 * Reading the first 32 bits of the payload
	 * this will give us the size of our message and
	 * allow us to calulate how many bits will need to be
	 * read.
	 */
        for(i = 0; i < 32; i++){
                binSize[i] =  (p[ bmpData[pixel] ].RED + p[ bmpData[pixel] ].GRN + p[ bmpData[pixel] ].BLU )% 2;
                pixel++;
        }

        int size = 0; 		// size of the payload
        int binIndex = 0;	// keeping track of the index for our binArray, 
        for(i = 0; i < 32; i++){
		// setting the correct bits to recover the size of the image, in decimal
                size |= (binSize[binIndex] == 1) << (i);
                binIndex++;
        }

	/*
	 * Setting up a chunk of memory to get our binary stream
	 * so that we can begin to rebuild the payload.
	 *
	 * we multiply the size by 8, this gives us the number of
	 * bits we need to read from the stego image.
	 */
        unsigned char  *result; 
	result = (unsigned char *)  malloc((size * 8));
        if(result == NULL){
                fprintf(stderr, "malloc failed in bin2txt\n");
                exit(-1);
        }

	/*
	 * Allocating a space in memory so we can set the bits
	 * for each byte of the payload. This will hold our
	 * recovered payload from the stego image.
	 */
        unsigned char *recover; // how many bytes
        recover = (unsigned char *) malloc(size);
        if(recover == NULL){
                fprintf(stderr, "malloc for t failed in bin2txt\n");
                exit(-1);
        }
        memset(recover, 0, size); // setting memory to 0, makes setting bits easier later

	/*
	 * each pixel references a location in the palette.
	 * we use the RGB values in the palette entry, to get
	 * our hidden bit.
	 *
	 * to get the parity bit,  R+G+B mod 2
	 *
	 * store the results
	 */
        for(i = 0; i < size * 8; i++){
                result[i] = (p[ bmpData[pixel] ].RED + p[ bmpData[pixel] ].GRN + p[ bmpData[pixel] ].BLU )% 2;
                pixel++;
        }

        // attempting to place the bits in the correction positions
        // within a byte.
	/*
	 * We loop through the total number of bits that are hidden
	 * and restore the into a byte. 
	 * 
	 * for every 8th bit, a nested for-loop is exectued
	 * this will loop 8 times and set each bit from the
	 * result[] array, 
	 */
        int rIndex = 0; // used to keep track result[] position
	for(i = 0; i < size * 8; i++){

                // for every 8 bits we want to put the bits
                // into a proper byte.
                if(i % 8 == 0 && i != 0){
                        for(j = 0; j < 8; ++j){
                                // bitwise or operation that I looked up
                                // based on how to read from a string.
                                recover[index] |= (result[rIndex] == 1) << (7 - j);
                                rIndex++;
                        }
			index++; // keeping track of where we are in our array recover
                                 // recover, is the array being used to put our bits
                                 // back together. It will certainly be renamed.
                }
        }

	/*
	 * Begin writing to file
	 */
        FILE *out;
        out = fopen("recovered", "wb");
        if( out == NULL){
                fprintf(stderr, "Failed to open output file\n");
                exit(-1);
        }
        fwrite(recover, sizeof(unsigned char), size, out);
        fclose(out);
        free(recover);
        free(result);

}
