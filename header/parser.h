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
#include "errHand.h"

#define PRINT_ERROR_MESSAGE

class Parser {
public:
    Parser(std::istream& file) :
        lexicalAnalyzer(LexicalAnalyzer(file)) {}

    SynNode* parse();

// private:
    LexSymbol symbol;
    LexicalAnalyzer lexicalAnalyzer;

    std::vector<LexSymbol> record;
    std::set<std::string> returnFuncList;
    std::set<std::string> nonreturnFuncList;    
    int pos = 0;
    int top = 0;



    bool nextSym();
    void preReadSym(int time = 1);
    void flushPreRead();
    bool cacheContainsSym(TYPE_SYM type);

    SynNode* stringP();
    SynNode* unsignedIntP();
    SynNode* intP();
    SynNode* charP();
    SynNode* constDefP(int layer); //*
    SynNode* constDecP(int layer); //*
    SynNode* idenP(std::string* S = nullptr, int *line = nullptr);
    SynNode* decHeadP(std::string*, int* line = nullptr);
    SynNode* constP();
    SynNode* varDecP(int layer);
    SynNode* varDefP(int layer);
    SynNode* varDerWithInitP(int layer, int type);
    SynNode* varDerWithoutInitP(int layer, int type);
    SynNode* typeIdenP();
    SynNode* arrayConstP(int size); // {}
    SynNode* doubleArrayConstP(int size1, int size2); // {{}}
    SynNode* oneDdeclareP(int* size); // [x]
    SynNode* arguListP(int layer, int*num=nullptr);
    SynNode* termP(int layer);
    SynNode* factorP(int layer);
    SynNode* expressionP(int layer);
    SynNode* callFuncSenP(int layer);
    SynNode* valueArgueListP(int layer, int argunum);
    SynNode* assignSenP(int layer);
    SynNode* sentenceP(int layer);
    SynNode* sentenceListP(int layer);
    SynNode* ifelseSenP(int layer);
    SynNode* conditionP(int layer);
    SynNode* loopSenP(int layer);
    SynNode* stepLengthP(int layer);
    SynNode* switchSenP(int layer);
    SynNode* caseListP(int layer);
    SynNode* caseSenP(int layer);
    SynNode* defaultP(int layer);
    SynNode* readSenP(int layer);
    SynNode* writeSenP(int layer);
    SynNode* returnSenP(int layer);
    SynNode* compareOpP(int layer);
    SynNode* compoundSenP(int layer);
    SynNode* refuncDefineP(int layer);
    SynNode* nonrefuncDefineP(int layer);
    SynNode* mainP(int layer=0);

    void addErrorMessage(int line, char iden, std::string mess="") {
        errorList.push_back(ErrorMessage(line, iden, mess));
    }

    bool checkDuplicate(std::string name, int layer) {
        return symbolTable.duplicateName(name, layer);
    }

    bool getIDENtype(std::string name, int *type, int layer) {
        int TYPE = symbolTable.idenTYPE(name, layer);
        *type = TYPE;
        return TYPE != TYPE_ERROR;
    }

    void printError(std::ostream& out);

    int str2int(std::string s) {
        int ans = 0;
        char c;
        for (int i = 0; i < s.size(); i++) {
            c = s[i];
            if (c >= '9' || c <= '0') printPos(88754);
            ans = ans * 10 + (c - '0');
        }
        return ans;
    }

    char str2char(std::string s) {
        if (s.size() > 1) printPos(8069);
        return s[0];
    }

private:
    bool flushed = true;
    
    SymbolTable symbolTable;
    std::vector<ErrorMessage> errorList;


};


#endif // PARSER_H_INCLUDED
