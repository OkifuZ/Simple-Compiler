#ifndef LEXICAL_H_INCLUDED
#define LEXICAL_H_INCLUDED

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


void printPos(int i);


enum class TYPE_SYM{OTHERS = -3, ERROR = -2, ENDS = -1, IDENFR = 0, INTCON, CHARCON, STRCON, CONSTTK,
          INTTK, CHARTK, VOIDTK, MAINTK, IFTK, ELSETK,
          SWITCHTK, CASETK, DEFAULTTK, WHILETK, FORTK,
          SCANFTK, PRINTFTK, RETURNTK, PLUS, MINU, MULT,
          DIV, LSS, LEQ, GRE, GEQ, EQL, NEQ, COLON,
          ASSIGN, SEMICN, COMMA, LPARENT, RPARENT,
          LBRACK, RBRACK, LBRACE, RBRACE};

class LexSymbol {
    friend std::ostream & operator<<(std::ostream &out, LexSymbol &sy);

public:
    std::string token;
    TYPE_SYM type;
    int line;
    bool hasError;

    LexSymbol(TYPE_SYM typ, const std::string& tok, int line_, bool hasError_) : 
        token(tok), type(typ), line(line_), hasError(hasError_) {}
    LexSymbol() : token(""), type(TYPE_SYM::ERROR), line(-1), hasError(false) {}
};

std::ostream & operator<<(std::ostream &out, LexSymbol &sy);


class LexicalAnalyzer {
private:
    std::istream& file;
    static std::vector<std::string> typeStrings;
    static std::vector<std::string> reservedTable;
    int global_Line = 1;

public:
    LexicalAnalyzer(std::istream& fs) : file(fs) {}
    TYPE_SYM getsym(std::string& token, bool* hasError);

    static std::string& getTypeString(TYPE_SYM type) {
        int index = static_cast<int>(type);
        return LexicalAnalyzer::typeStrings[index];
    }

    static TYPE_SYM searchRWT(const std::string& word) {
        int index;
        for (auto it = LexicalAnalyzer::reservedTable.begin(); it != LexicalAnalyzer::reservedTable.end(); it++) {
            if (upperCase(*it) == upperCase(word)) {
                index = it - LexicalAnalyzer::reservedTable.begin();
                return static_cast<TYPE_SYM>(static_cast<int>(TYPE_SYM::CONSTTK) + index);
            }
        }
        return TYPE_SYM::IDENFR;
    }

    bool isEmpty(const char& c);
    int getGlobalLine() { return global_Line; }
    
    static std::string upperCase(const std::string& s);

    static bool isLetter(const char& c);

    static bool isNum(const char& s);

    static bool isASCII(const char& c);


};


#endif // LEXICAL_H_INCLUDED
