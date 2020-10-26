#include "../header/lexical.h"
#include "../header/parser.h"
#include "../header/synTree.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

/*CLASS: PARSER*/

using namespace std;

// pos -1 is symbol's index in record
// simple case
// 1 2 3 4 5 6
//     p       t
// empty case
//
// p(t)
// traceback case
// 1 2 3 4 5 6
//   p t
// cache: pos, pos+1, pos+2, ... , top-1; empty when top==pos

bool Parser::nextSym()
{
    if (pos < top)
    {
        symbol = record[pos];
        pos++;
    }
    else
    {
        preReadSym(1);
        symbol = record[pos];
        pos++;
    }
    this->flushed = (top == pos);
    if (symbol.hasError || symbol.type == TYPE_SYM::ERROR)
    {
        addErrorMessage(symbol.line, 'a', "词法错误");
    }
    return !(symbol.type == TYPE_SYM::ENDS || symbol.type == TYPE_SYM::ERROR);
}

void Parser::preReadSym(int time)
{
    this->flushed = false;
    while (time--)
    {
        if (top < record.size())
            top++;
        else
        {
            string token;
            bool hasError = false;
            TYPE_SYM type = this->lexicalAnalyzer.getsym(token, &hasError);
            LexSymbol sym = LexSymbol(type, token, lexicalAnalyzer.getGlobalLine(), hasError);
            record.push_back(sym);
            top++;
        }
    }
}

void Parser::flushPreRead()
{
    top = pos;
    this->flushed = true;
}

bool Parser::cacheContainsSym(TYPE_SYM type)
{
    for (int i = pos; i < top; i++)
    {
        if (record[i].type == type)
            return true;
    }
    return false;
}

void Parser::printError(ostream &out)
{
    int lastLine = -1;
#ifdef PRINT_ERROR_MESSAGE
    for (auto &item : this->errorList)
    {
        if (lastLine != item.line) {
            out << item.line << ' ' << item.iden << ' ' << item.message << '\n';
            lastLine = item.line;
        }
    }
#endif //
#ifndef PRINT_ERROR_MESSAGE
    for (auto &item : this->errorList)
    {
        if (lastLine != item.line) {
            out << item.line << ' ' << item.iden << '\n';
            lastLine = item.line;
        }
    }
#endif // !PRINT_ERROR_MESSAGE
}

int Parser::str2int(std::string s) {
    int ans = 0;
    char c;
    bool neg = false;
    for (int i = 0; i < s.size(); i++) {
        c = s[i];
        if (c > '9' || c < '0') {
            printPos(88754);
            cout << "sefw" << endl;
        }
        else ans = ans * 10 + (c - '0');
    }
    return neg ? -ans : ans;
}

char Parser::str2char(std::string s) {
    if (s.size() > 1) printPos(8069);
    return s[0];
}

FuncSymEntry* Parser::getFUNC_CALL(std::string name) {
    SymTableEntry* sym = getEntrySymByName(name);
    FuncSymEntry* func = dynamic_cast<FuncSymEntry*>(sym);
    return func;
}