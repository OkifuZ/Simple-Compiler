#include "../header/parser.h"
#include "../header/lexical.h"
#include "../header/synTree.h"
#include <fstream>
#include <iostream>



int main() {
	std::ifstream infile("testfile.txt");
	std::ofstream outfile("output.txt");
	std::ofstream outfile2("tree.txt");
	Parser parser(infile);
	parser.nextSym();
	SynNode* tree = parser.parse();
	tree->print(outfile2);
	parser.printError(outfile);
	return 0;
}