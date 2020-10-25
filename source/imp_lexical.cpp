#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "../header/lexical.h"
#include "../header/parser.h"

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
        printPos(17761);
        return TYPE_SYM::ERROR;
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
        printPos(20098);
        return TYPE_SYM::ERROR;
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
        }
        else {
            printPos(3);
            return TYPE_SYM::ERROR;
        }
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
    return TYPE_SYM::ERROR;
}

/*END CLASS: LexicalAnalyzer*/


/*SOME TOOLS*/

string LexicalAnalyzer::upperCase(const string& s) {
    string ans;
    char c;
    for (auto it = s.begin(); it != s.end(); it++) {
        c = *it;
        if (c <= 'z' && c >= 'a') c += 'A' - 'a';
        ans += c;
    }
    return ans;
}

bool LexicalAnalyzer::isEmpty(const char& c) {
    if (c == '\n') {
        global_Line++;
    }
    return (c == ' ' || c == '\0' || c == '\n' || c == '\t' || c == '\r');
}

bool LexicalAnalyzer::isLetter(const char& c) {
    return ((c <='Z' && c >='A') || (c <='z' && c >= 'a') || c == '_');
}

bool LexicalAnalyzer::isNum(const char& s) {
    return s <= '9' && s >= '0';
}

bool LexicalAnalyzer::isASCII(const char& c) {
    return (c <= 126 && c >= 35) || c == 32 || c == 33;
}

void printPos(int i){
#ifdef PRINT_ERROR_MESSAGE
    cout << "error!!!" << " " << i << endl;
#endif // PRINT_ERROR_MESSAGE
}

/*END SOME TOOLS*/

