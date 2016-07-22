#include <png.h>

/* Class PNG_file
 * Contains the data for a PNG file object
 */
class PNG_file {

public:
	
	//Constructor
	PNG_file(const char *inputFileName);

	//Function for encoding data into the PNG from a file
	void encode(const char *fileToEncodeName);

	//Function for outputing the newly created PNG to a file
	void outputPNG(const char *outputFileName);

	//Function for outputing the decoded PNG to a file
	void decode(const char *outputFileName);

private:
	png_bytep* row_pointers;
	png_infop info_ptr;
	png_structp read_ptr;
	png_structp write_ptr;
};