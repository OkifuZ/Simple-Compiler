#include "../header/parser.h"
#include "../header/lexical.h"
#include "../header/synTree.h"
#include "../header/tool.h"
#include "../header/mipsGene.h"
#include <fstream>
#include <iostream>



int main() {
	std::ifstream infile("testfile.txt");
	std::ofstream outfile("error.txt");
	std::ofstream interCodeFile("interCode.txt");
	std::ofstream mipsFile("mips.txt");
	Parser parser(infile);
	parser.nextSym();
	SynNode* tree = parser.parse();
	parser.printError(outfile);
	parser.printInterCode(interCodeFile);
	MipsGenerator mipsGene(&parser);
	mipsGene.GeneMipsCode();
	mipsGene.printMipsCode(mipsFile);
#ifdef PRINT_ERROR_MESSAGE
	
#endif // PRINT_ERROR_MESSAGE
	return 0;
}