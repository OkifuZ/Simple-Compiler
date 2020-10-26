#include "../header/parser.h"
#include "../header/lexical.h"
#include "../header/synTree.h"
#include <fstream>
#include <iostream>



int main() {
	std::ifstream infile("testfile.txt");
	std::ofstream outfile("error.txt");
	Parser parser(infile);
	parser.nextSym();
	SynNode* tree = parser.parse();
	parser.printError(outfile);
#ifdef PRINT_ERROR_MESSAGE
	std::ofstream outfile2("tree.txt");
	tree->print(outfile2);
#endif // PRINT_ERROR_MESSAGE
	return 0;
}