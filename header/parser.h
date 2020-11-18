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
#include "tool.h"
#include "globalRec.h"
#include "InterCode.h"




class Parser {
public:
    Parser(std::istream& file) :
        lexicalAnalyzer(LexicalAnalyzer(file)) {
        this->intermediate = new Intermediate(&envTable);
    }

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
    SynNode* constDefP(int*); //*
    SynNode* constDecP(); //*
    SynNode* idenP(std::string&, int *);
    SynNode* decHeadP(std::string&, int*, int*);
    SynNode* constP(int*, int*, int*);
    SynNode* varDecP();
    SynNode* varDefP(int*);
    SynNode* varDerWithInitP(int type, int*);
    SynNode* varDerWithoutInitP(int type, int*);
    SynNode* typeIdenP(int*);
    SynNode* arguListP(int*, FuncSymEntry*);
    SynNode* refuncDefineP();
    SynNode* nonrefuncDefineP();
    SynNode* termP(int*, bool*, std::string&);
    SynNode* factorP(int*, bool*, std::string&);
    SynNode* expressionP(int*, bool*, std::string&);
    SynNode* callFuncSenP(int*);
    SynNode* valueArgueListP(FuncSymEntry* func);
    SynNode* assignSenP();
    SynNode* sentenceP(bool inFunc, int type = _TYPE_ERROR, int* retNum=0, bool inMain = false);
    SynNode* sentenceListP(bool inFunc, int type = _TYPE_ERROR, int* retNum=0, bool inMain = false);
    SynNode* ifelseSenP(bool inFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* conditionP();
    SynNode* loopSenP(bool isFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* stepLengthP(int*);
    SynNode* switchSenP(bool isFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* caseListP(int expType, bool isFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* caseSenP(int expType, bool isFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* defaultP(bool isFunc, int type = _TYPE_ERROR, int* retNum=0);
    SynNode* readSenP();
    SynNode* writeSenP();
    SynNode* returnSenP(int, int*, bool);
    SynNode* compareOpP(int*);
    SynNode* compoundSenP(bool isFunc, int type = _TYPE_ERROR, int* retNum = nullptr, bool inMain = false);
    SynNode* mainP();

// programAnalysis nonhighlight
    SynNode* arrayConstP(int, int*, int*, std::vector<int> *); // {}
    SynNode* doubleArrayConstP(int, int, int*, int*, std::vector<int> *); // {{}}
    SynNode* oneDdeclareP(int* ); // [x]
    SynNode* referenceP(int*, bool, std::string&, bool*, std::string&, std::string&);
    void semicnP(NonTerNode*);


// symbol table

    EnvTable envTable;
    
    bool checkDuplicate(std::string name) { return envTable.checkDuplicate(name); }

    FuncSymEntry* getFUNC_CALL(std::string name);

    SymTableEntry* getEntrySymByName(std::string name) { return envTable.getSymByName(name); }

    void popCurTable() {
        envTable.popTable();
    }

    void pushNewTable(std::string funcName) {
        envTable.addTable(funcName);
    }

    int offset = 0;

    void addSymbolEntry(SymTableEntry* sym) { 
        envTable.top->insertSymbolEntry(sym);
    }


// lexical tools
    bool nextSym();
    void preReadSym(int time = 1);
    void flushPreRead();
    bool cacheContainsSym(TYPE_SYM type);
    int getPreLine();

// error handling
    void printError(std::ostream& out);
    
    void addErrorMessage(int line, char iden, std::string mess="") {
        errorList.push_back(ErrorMessage(line, iden, mess));
    }


    

// code gene
    void addString2Global(std::string s) {
        globalStringList.push_back(s);
    }

    void printInterCode(std::ostream& os) {
        intermediate->printInterCode(os);
    }

    std::vector<std::string> globalStringList;
    Intermediate* intermediate;
    

    

private:
    bool flushed = true;
    
    std::vector<ErrorMessage> errorList;

};


#endif // PARSER_H_INCLUDED
