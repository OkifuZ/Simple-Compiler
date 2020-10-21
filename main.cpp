#include "parser.h"
#include "lexical.h"
#include "synTree.h"
#include <fstream>
#include <iostream>



int main() {
	std::ifstream infile("testfile.txt");
	std::ofstream outfile("output.txt");
	Parser parser(infile);
	parser.nextSym();
	SynNode* tree = parser.parse();
	//tree->print(outfile);
	parser.printError(outfile);
	return 0;
}