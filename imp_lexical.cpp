#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "lexical.h"

using namespace std;


/*CLASS: LexSymbol*/
ostream & operator<<(ostream &out, LexSymbol& sy)
{
    // out << sy.token << "->" << LexicalAnalyzer::getTypeString(sy.type);
    out << LexicalAnalyzer::getTypeString(sy.type) << " " << sy.token;
    return out;
}

/*END CLASS: LexSymbol*/

/*CLASS: LexicalAnalyzer*/

vector<string> LexicalAnalyzer::typeStrings{"IDENFR", "INTCON", "CHARCON", "STRCON",
                                            "CONSTTK", "INTTK", "CHARTK", "VOIDTK",
                                            "MAINTK","IFTK", "ELSETK", "SWITCHTK",
                                            "CASETK", "DEFAULTTK", "WHILETK","FORTK",
                                            "SCANFTK", "PRINTFTK", "RETURNTK", "PLUS",
                                            "MINU", "MULT", "DIV", "LSS",
                                            "LEQ", "GRE", "GEQ","EQL",
                                            "NEQ", "COLON", "ASSIGN", "SEMICN",
                                            "COMMA","LPARENT", "RPARENT", "LBRACK",
                                            "RBRACK", "LBRACE", "RBRACE"};

vector<string> LexicalAnalyzer::reservedTable{"CONST", "INT", "CHAR", "VOID",
                                              "MAIN", "IF", "ELSE", "SWITCH",
                                              "CASE", "DEFAULT", "WHILE",
                                              "FOR", "SCANF", "PRINTF", "RETURN"};


TYPE_SYM LexicalAnalyzer::getsym(string& token) {
    token.clear();
    char c = '\0';
    while (isEmpty(c)) c = this->file.get();
    if (c == EOF) return TYPE_SYM::ENDS;
    if (isLetter(c)) {
        while (isLetter(c) || isNum(c)) {
            token += c;
            c = this->file.get();
        }
        this->file.putback(c);
        return LexicalAnalyzer::searchRWT(token);
    }
    else if (isNum(c)) {
        while (isNum(c)) {
            token += c;
            c = this->file.get();
        }
        this->file.putback(c);
        return TYPE_SYM::INTCON;
    }
    else if (c == '\'') {
        c = this->file.get();
        if (c == '+' || c == '-' || c == '*' || c == '/' || isNum(c) || isLetter(c)) {
            token += c;
            c = this->file.get();
            if (c == '\'') return TYPE_SYM::CHARCON;
        }
        error(1);
    }
    else if (c == '"') {
        c = this->file.get();
        while (isASCII(c)) {
            token += c;
            c = this->file.get();
        }
        if (c == '"') {
            return TYPE_SYM::STRCON;
        }
        error(2);
    }
    else if (c == '-') {
        token += c;
        return TYPE_SYM::MINU;
    }
    else if (c == '+') {
        token += c;
        return TYPE_SYM::PLUS;
    }
    else if (c == '*') {
        token += c;
        return TYPE_SYM::MULT;
    }
    else if (c == '/') {
        token += c;
        return TYPE_SYM::DIV;
    }
    else if (c == '<') {
        token += c;
        c = this->file.get();
        if (c == '=') {
            token += c;
            return TYPE_SYM::LEQ;
        } else {
            this->file.putback(c);
            return TYPE_SYM::LSS;
        }
    }
    else if (c == '>') {
        token += c;
        c = this->file.get();
        if (c == '=') {
            token += c;
            return TYPE_SYM::GEQ;
        } else {
            this->file.putback(c);
            return TYPE_SYM::GRE;
        }
    }
    else if (c == '=') {
        token += c;
        c = this->file.get();
        if (c == '=') {
            token += c;
            return TYPE_SYM::EQL;
        } else {
            this->file.putback(c);
            return TYPE_SYM::ASSIGN;
        }
    }
    else if (c == '!') {
        token += c;
        c = this->file.get();
        if (c == '=') {
            token += c;
            return TYPE_SYM::NEQ;
        } else error(3);
    }
    else if (c == ':') {
        token += c;
        return TYPE_SYM::COLON;
    }
    else if (c == ';') {
        token += c;
        return TYPE_SYM::SEMICN;
    }
    else if (c == ',') {
        token += c;
        return TYPE_SYM::COMMA;
    }
    else if (c == '(') {
        token += c;
        return TYPE_SYM::LPARENT;
    }
    else if (c == ')') {
        token += c;
        return TYPE_SYM::RPARENT;
    }
    else if (c == '[') {
        token += c;
        return TYPE_SYM::LBRACK;
    }
    else if (c == ']') {
        token += c;
        return TYPE_SYM::RBRACK;
    }
    else if (c == '{') {
        token += c;
        return TYPE_SYM::LBRACE;
    }
    else if (c == '}') {
        token += c;
        return TYPE_SYM::RBRACE;
    }
    return TYPE_SYM::ENDS;
}

/*END CLASS: LexicalAnalyzer*/


/*SOME TOOLS*/

string upperCase(const string& s) {
    string ans;
    char c;
    for (auto it = s.begin(); it != s.end(); it++) {
        c = *it;
        if (c <= 'z' && c >= 'a') c += 'A' - 'a';
        ans += c;
    }
    return ans;
}

bool isEmpty(const char& c) {
    return (c == ' ' || c == '\0' || c == '\n' || c == '\t' || c == '\r');
}

bool isLetter(const char& c) {
    return ((c <='Z' && c >='A') || (c <='z' && c >= 'a') || c == '_');
}

bool isNum(const char& s) {
    return s <= '9' && s >= '0';
}

bool isASCII(const char& c) {
    return (c <= 126 && c >= 35) || c == 32 || c == 33;
}

void error(int i){
    cout << "error!!!" << " " << i << endl;
}

/*END SOME TOOLS*/

