#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include "lexical.h"
#include "synTree.h"

class Parser {
public:
    Parser(std::istream& file, std::ostream& out) :
        lexicalAnalyzer(LexicalAnalyzer(file)), outStream(out) {};

    SynNode* parse();

// private:
    LexSymbol symbol;
    LexicalAnalyzer lexicalAnalyzer;
    std::ostream& outStream;
    std::vector<LexSymbol> record;
    std::set<std::string> returnFuncList;
    std::set<std::string> nonreturnFuncList;    
    int pos = 0;
    int top = 0;

    void printSym();

    bool nextSym();
    void preReadSym(int time = 1);
    void flushPreRead();
    bool cacheContainsSym(TYPE_SYM type);

    SynNode* stringP();
    SynNode* unsignedIntP();
    SynNode* intP();
    SynNode* charP();
    SynNode* constDefP();
    SynNode* constDecP();
    SynNode* idenP(std::string* S = nullptr);
    SynNode* decHeadP(std::string*);
    SynNode* constP();
    SynNode* varDecP();
    SynNode* varDefP();
    SynNode* varDerWithInitP();
    SynNode* varDerWithoutInitP();
    SynNode* typeIdenP();
    SynNode* arrayConstP(); // {}
    SynNode* doubleArrayConstP(); // {{}}
    SynNode* oneDdeclareP(); // [x]
    SynNode* arguListP();
    SynNode* termP();
    SynNode* factorP();
    SynNode* expressionP();
    SynNode* callFuncSenP();
    SynNode* valueArgueListP();
    SynNode* assignSenP();
    SynNode* sentenceP();
    SynNode* sentenceListP();
    SynNode* ifelseSenP();
    SynNode* conditionP();
    SynNode* loopSenP();
    SynNode* stepLengthP();
    SynNode* switchSenP();
    SynNode* caseListP();
    SynNode* caseSenP();
    SynNode* defaultP();
    SynNode* readSenP();
    SynNode* writeSenP();
    SynNode* returnSenP();
    SynNode* compareOpP();
    SynNode* compoundSenP();
    SynNode* refuncDefineP();
    SynNode* nonrefuncDefineP();
    SynNode* mainP();

private:
    bool flushed = true;

};


#endif // PARSER_H_INCLUDED
