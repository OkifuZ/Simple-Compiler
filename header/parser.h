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

//#define PRINT_ERROR_MESSAGE

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

// programAnalysis highlight
    SynNode* stringP(std::string&);
    SynNode* unsignedIntP(int*);
    SynNode* intP(int*);
    SynNode* charP(char*);
    SynNode* constDefP(int layer, int*); //*
    SynNode* constDecP(int layer); //*
    SynNode* idenP(std::string&, int *);
    SynNode* decHeadP(std::string&, int*, int*);
    SynNode* constP(int*, int*, int*);
    SynNode* varDecP(int layer);
    SynNode* varDefP(int layer, int*);
    SynNode* varDerWithInitP(int layer, int type, int*);
    SynNode* varDerWithoutInitP(int layer, int type, int*);
    SynNode* typeIdenP(int*);
    SynNode* arguListP(int, int*, FuncSymEntry*);
    SynNode* refuncDefineP(int layer);
    SynNode* nonrefuncDefineP(int layer);
    SynNode* termP(int layer, int*, int*);
    SynNode* factorP(int layer, int*, int*);
    SynNode* expressionP(int layer, int*, int*);
    SynNode* callFuncSenP(int layer, int*, int*);
    SynNode* valueArgueListP(int layer, FuncSymEntry* func);
    SynNode* assignSenP(int layer);
    SynNode* sentenceP(int layer, bool inFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* sentenceListP(int layer, bool inFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* ifelseSenP(int layer, bool inFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* conditionP(int, bool*);
    SynNode* loopSenP(int layer, bool isFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* stepLengthP(int*);
    SynNode* switchSenP(int layer, bool isFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* caseListP(int layer, int expType, bool isFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* caseSenP(int layer, int expType, bool isFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* defaultP(int layer, bool isFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* readSenP(int layer);
    SynNode* writeSenP(int layer);
    SynNode* returnSenP(int, int, int*);
    SynNode* compareOpP(int*);
    SynNode* compoundSenP(int layer, bool isFunc, int type = _TYPE_ERROR, int* retNum = nullptr);
    SynNode* mainP(int layer);

// programAnalysis nonhighlight
    SynNode* arrayConstP(int, int*, int*); // {}
    SynNode* doubleArrayConstP(int, int, int*, int*); // {{}}
    SynNode* oneDdeclareP(int* ); // [x]
    SynNode* referenceP(int layer, int*, int*, bool, int);
    void semicnP(NonTerNode*);


// symbol table
    bool checkDuplicate(std::string name, int layer) { return symbolTable.duplicateName(name, layer); }

    FuncSymEntry* getFUNC_CALL(std::string name);

    SymTableEntry* getEntrySymByName(std::string name) { return symbolTable.getSymByName(name); }
    int getEntryTypeByName(std::string name) { return symbolTable.getTypeByName(name); }
    

    void popSym_CurLayer(int layer) {
        symbolTable.popSym(layer);
    }

    void addSymbolEntry(SymTableEntry* sym) { symbolTable.insertSymbolEntry(sym); }

    void printSymTable(std::ostream& os) { symbolTable.printSymTable(os); }

// lexical tools
    bool nextSym();
    void preReadSym(int time = 1);
    void flushPreRead();
    bool cacheContainsSym(TYPE_SYM type);
    int str2int(std::string s);
    char str2char(std::string s);

// error handling
    void printError(std::ostream& out);
    
    void addErrorMessage(int line, char iden, std::string mess="") {
        errorList.push_back(ErrorMessage(line, iden, mess));
    }

private:
    bool flushed = true;
    
    SymbolTable symbolTable;
    std::vector<ErrorMessage> errorList;


};


#endif // PARSER_H_INCLUDED
