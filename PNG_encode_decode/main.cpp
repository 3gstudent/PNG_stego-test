#include "PNG_file.h"
void main() {
	PNG_file link = PNG_file("big.png");
	link.encode("1.txt");
	link.outputPNG("bigen.png");
//	PNG_file link = PNG_file("bigensimple.png");
//	link.decode("2.txt");
}