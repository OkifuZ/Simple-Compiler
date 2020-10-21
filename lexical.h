#ifndef LEXICAL_H_INCLUDED
#define LEXICAL_H_INCLUDED

#include <iostream>
#include <fstream>
#include <string>
#include <vector>



std::string upperCase(const std::string& s);

void error(int i);

bool isEmpty(const char& c);

bool isLetter(const char& c);

bool isNum(const char& s);

bool isASCII(const char& c);




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

    LexSymbol(TYPE_SYM typ, const std::string& tok) : token(tok), type(typ), line(-1) {}
    LexSymbol() : token(""), type(TYPE_SYM::ERROR), line(-1) {}
};

std::ostream & operator<<(std::ostream &out, LexSymbol &sy);


class LexicalAnalyzer {
private:
    std::istream& file;
    static std::vector<std::string> typeStrings;
    static std::vector<std::string> reservedTable;

public:
    LexicalAnalyzer(std::istream& fs) : file(fs) {}
    TYPE_SYM getsym(std::string& token);

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
};


#endif // LEXICAL_H_INCLUDED
