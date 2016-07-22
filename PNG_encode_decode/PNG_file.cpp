/* PNG_file
 * author: Grant Curell
 * Performs IO and encoding and decoding on PNG images
 * Feel free to reuse at your leisure. Cite me if you like, but it's no big deal.
 * Thanks to the random dudes I bummed the code for ipow and filesize from on
 * stackoverflow ;-).
 */

#include <stdio.h>
#include <stdlib.h>
#include "PNG_file.h"

#define PNG_SIG_LENGTH 8 //The signature length for PNG
#define BYTE_SIZE 8 //Size of a byte
#define SIZE_WIDTH 32 //The number of bits used for storing the length of a file
					  //Must be a multiple of 8

/* Integer power function
 * The C++ standard pow function uses doubles and I needed an integer version.
 * This is just a standard implementation using modular exponentiation.
 */
int ipow(int base, int exp) {
	int result = 1;
	while (exp)
	{
		if (exp & 1)
			result *= base;
		exp >>= 1;
		base *= base;
	}

	return result;
}


//Dirty function for calculating the size of a file
unsigned int filesize(const char *filename)
{
	FILE *f = fopen(filename,"rb");  /* open the file in read only */

	unsigned int size = 0;
	if (fseek(f,0,SEEK_END)==0) /* seek was successful */
		size = ftell(f);
	fclose(f);
	return size;
}


/* PNG Constructor
 * Constructor for the PNG_file class Simply reads in a PNG file
 */
PNG_file::PNG_file(const char *inputFileName) {

	FILE * inputFile;

	unsigned char header[BYTE_SIZE];

	inputFile = fopen (inputFileName,"rb");

	//Check if the file opened
	if(!inputFile)
		exit(1);

	// START READING HERE

	fread(header, 1, PNG_SIG_LENGTH, inputFile);

	//Check if it is a PNG
	if(png_sig_cmp(header, 0, PNG_SIG_LENGTH))
		exit(1);


	//Set up libPNG data structures and error handling
	read_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!read_ptr)
		exit(1);

	info_ptr = png_create_info_struct(read_ptr);

	if (!info_ptr) {
		png_destroy_read_struct(&read_ptr,
			(png_infopp)NULL, (png_infopp)NULL);
		exit(1);
	}

	png_infop end_info = png_create_info_struct(read_ptr);

	if (!end_info) {
		png_destroy_read_struct(&read_ptr, &info_ptr,
			(png_infopp)NULL);
		exit(1);
	}
	//End data structure/error handling setup

	//Initialize IO for PNG
	png_init_io(read_ptr, inputFile);

	//Alert libPNG that we read PNG_SIG_LENGTH bytes at the beginning
	png_set_sig_bytes(read_ptr, PNG_SIG_LENGTH);

	//Read the entire PNG image into memory
	png_read_png(read_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	row_pointers = png_get_rows(read_ptr, info_ptr);

	//TODO ADD A CHECK SO WE ONLY USE COMPATIBLE PNG IMAGES
	if(read_ptr->bit_depth != BYTE_SIZE)
		exit(1);

	fclose(inputFile);
}

void PNG_file::encode(const char *fileToEncodeName) {
	//BEGIN ENCODING HERE

	FILE * fileToEncode;

	unsigned char buffer = 0;

	fileToEncode = fopen (fileToEncodeName,"rb");

	//Check if the file opened
	if(!fileToEncode)
		exit(1);

	//TODO CONSIDER ADDING CHECK FOR FILES THAT ARE TOO BIG
	unsigned long size = filesize(fileToEncodeName);

	//This section of code encodes the input file into the picture
	//It encodes the input file bit by bit into the least significant
	//bits of the original picture file
	for(int y=0; y < read_ptr->height; y++) {
		int x=0;
		//Write the file size into the file y==0 ensures that it only happens
		//once
		if(y == 0)
			for(x; x < SIZE_WIDTH; x++) {
				if((size & ipow(2,x)))
					*(row_pointers[y]+x) |= 1;
				else
					*(row_pointers[y]+x) &= 0xFE;
			}
		for(x; x < read_ptr->width*3; x++) {
			if(x%BYTE_SIZE == 0) {
				if(!fread(&buffer, 1, 1, fileToEncode)) 
					goto loop_end;
			}
			//png_bytep here = row_pointers[y]+x; for debugging
			if((buffer & ipow(2,x%BYTE_SIZE)))
				*(row_pointers[y]+x) |= 1;
			else
				*(row_pointers[y]+x) &= 0xFE;
		}
		//Make sure that we did not use a file too large that it can't be encoded
		if(y >= read_ptr->height)
			exit(1);
	}

	//goto jumps here to break out of multiple loops
	loop_end:

	fclose(fileToEncode);

}

void PNG_file::decode(const char *outputFileName) {
	//BEGIN DECODING HERE

	FILE * outputFile;

	unsigned char buffer = 0;

	outputFile = fopen (outputFileName,"wb");

	//Check if the file opened
	if(!outputFile)
		exit(1);

	unsigned int size = 0;

	//
	for(int y=0; y < read_ptr->height; y++) {
		int x=0;
		//Write the file size into the file y==0 ensures that it only happens
		//once
		if(y == 0)
			for(x; x < SIZE_WIDTH; x++) {
				size |= ((*(row_pointers[0]+x) & 1 ) << x);
			}
		for(x; x < read_ptr->width*3; x++) {
			if((x > SIZE_WIDTH || y > 0) && x%BYTE_SIZE == 0) {
				fwrite(&buffer, 1, 1, outputFile);
				buffer = 0;
			}
			//png_bytep here = row_pointers[y]+x; for debugging
			if(((read_ptr->width*y)*3+x) == size*BYTE_SIZE+SIZE_WIDTH)
				goto loop_end;
			buffer |= ((*(row_pointers[y]+x) & 1) << x%BYTE_SIZE);
		}
	}

	//goto jumps here to break out of multiple loops
	loop_end:

	fclose(outputFile);

}

void PNG_file::outputPNG(const char *outputFileName) {
	//START WRITING HERE

	FILE * outputFile;
	
	outputFile = fopen (outputFileName,"wb");

	//Check if the file opened
	if(!outputFile)
		exit(1);

	//Initialize the PNG structure for writing
	write_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!write_ptr)
		exit(1);

	png_init_io(write_ptr, outputFile);

	//Set the rows in the PNG structure
	png_set_rows(write_ptr, info_ptr, row_pointers);

	//Write the rows to the file
	png_write_png(write_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	fclose(outputFile);
}