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

    SynNode* stringP(std::string*);
    SynNode* unsignedIntP(int*);
    SynNode* intP(int*);
    SynNode* charP(char*);
    SynNode* constDefP(int layer); //*
    SynNode* constDecP(int layer); //*
    SynNode* idenP(std::string*, int *);
    SynNode* decHeadP(std::string*, int*, int*);
    SynNode* constP(int*, int*, int*);
    SynNode* varDecP(int layer);
    SynNode* varDefP(int layer);
    SynNode* varDerWithInitP(int layer, int type);
    SynNode* varDerWithoutInitP(int layer, int type);
    SynNode* typeIdenP(int*);
    SynNode* arrayConstP(int, int*, int*); // {}
    SynNode* doubleArrayConstP(int, int, int*, int*); // {{}}
    SynNode* oneDdeclareP(int* size); // [x]
    SynNode* arguListP(int layer, int*num, FuncSymEntry* func);
    SynNode* termP(int layer);
    SynNode* factorP(int layer);
    SynNode* expressionP(int layer);
    SynNode* callFuncSenP(int layer);
    SynNode* valueArgueListP(int layer, FuncSymEntry* func);
    SynNode* assignSenP(int layer);
    SynNode* sentenceP(int layer, int type = TYPE_VOID);
    SynNode* sentenceListP(int layer, int type = TYPE_VOID);
    SynNode* ifelseSenP(int layer);
    SynNode* conditionP(int layer);
    SynNode* loopSenP(int layer);
    SynNode* stepLengthP();
    SynNode* switchSenP(int layer);
    SynNode* caseListP(int layer, int type);
    SynNode* caseSenP(int layer, int type);
    SynNode* defaultP(int layer);
    SynNode* readSenP();
    SynNode* writeSenP(int layer);
    SynNode* returnSenP(int layer, int type = TYPE_VOID);
    SynNode* compareOpP();
    SynNode* compoundSenP(int layer, int type = TYPE_VOID);
    SynNode* refuncDefineP(int layer);
    SynNode* nonrefuncDefineP(int layer);
    SynNode* mainP(int layer=0);

    void addErrorMessage(int line, char iden, std::string mess="") {
        errorList.push_back(ErrorMessage(line, iden, mess));
    }

    bool checkDuplicate(std::string name, int layer) {
        return symbolTable.duplicateName(name, layer);
    }

    FuncSymEntry* getFUNC_CALL(std::string name) {
        SymTableEntry* sym = symbolTable.getSymByName(name);
        FuncSymEntry* func = dynamic_cast<FuncSymEntry*>(sym);
        return func;
    }

    SymTableEntry* getEntryByName(std::string name) {
        SymTableEntry* sym = symbolTable.getSymByName(name);
        return sym;
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
