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



bool Parser::nextSym() {
    if (pos < top) {
        symbol = record[pos];
        pos++;
    } else {
        preReadSym(1);
        symbol = record[pos];
        pos++;
    }
    this->flushed = (top == pos);
    if (symbol.type == TYPE_SYM::ERROR) {
        addErrorMessage(symbol.line, 'a');
    }
    return !(symbol.type == TYPE_SYM::ENDS || symbol.type == TYPE_SYM::ERROR);
}

void Parser::preReadSym(int time) {
    this->flushed = false;
    while (time--) {
        if (top < record.size()) top++;
        else {
            string token;
            TYPE_SYM type = this->lexicalAnalyzer.getsym(token);
            LexSymbol sym = LexSymbol(type, token);
            record.push_back(sym);
            top++;
        }
    }
}

void Parser::flushPreRead() { 
    top = pos;
    this->flushed = true;
}

bool Parser::cacheContainsSym(TYPE_SYM type) {
    for (int i = pos; i < top; i++) {
        if (record[i].type == type) return true;
    }
    return false;
}

void Parser::printError(ostream& out) {
#ifdef PRINT_ERROR_MESSAGE
    for (auto &item : this->errorList) {
        out << item.line << ' ' << item.iden << ' ' << item.message << '\n';
    }
#endif // 
#ifndef PRINT_ERROR_MESSAGE
    for (auto &item : this->errorList) {
        out << item.line << ' ' << item.iden << '\n';
}
#endif // !PRINT_ERROR_MESSAGE
}


inline SynNode* Parser::stringP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::STRING, true);
    if (symbol.type == TYPE_SYM::STRCON) {
        node->addChild(new TerNode(symbol));
    } else printPos(881779);
    nextSym();
    return node;
}

inline SynNode* Parser::unsignedIntP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::UNSIGNED_INT, true);
    if (symbol.type == TYPE_SYM::INTCON) {
        node->addChild(new TerNode(symbol));
    }
    else printPos(231313);
    node->INT_value = str2int(symbol.token);
    nextSym();
    return node;
}

inline SynNode* Parser::intP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::INT, true);
    bool neg = (this->symbol.type == TYPE_SYM::MINU);
    if (this->symbol.type == TYPE_SYM::PLUS || 
        this->symbol.type == TYPE_SYM::MINU) {
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    node->addChild(unsignedIntP());
    node->INT_value = node->getLastChild()->INT_value;
    return node;
}

inline SynNode* Parser::charP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::CHAR, false);
    if (symbol.type == TYPE_SYM::CHARCON) {
        node->addChild(new TerNode(symbol));
        node->CHAR_value = str2char(symbol.token);
    }
    else printPos(313);
    nextSym();
    return node;
}

inline SynNode* Parser::constDefP(int layer) {
    int CATE = CAT_CONST, TYPE = -1, LAYER = layer, line = 0;
    string NAME;
    NonTerNode* node = new NonTerNode(TYPE_NTS::CONST_DEF, true);
    if (symbol.type == TYPE_SYM::INTTK) {
        node->addChild(new TerNode(symbol));
        TYPE = TYPE_INT;
        nextSym();
        node->addChild(idenP(&NAME, &line));
        if (checkDuplicate(NAME, LAYER)) {
            addErrorMessage(line, 'b', "常量定义时名字重定义");
        }
        if (!(symbol.type == TYPE_SYM::ASSIGN)) {
            printPos(414123);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(intP());
        symbolTable.insertSymbolEntry(new ScalerSymEntry(NAME, CATE, TYPE, LAYER));
        while (symbol.type == TYPE_SYM::COMMA) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP(&NAME, &line));
            if (checkDuplicate(NAME, LAYER)) {
                addErrorMessage(line, 'b', "常量定义时名字重定义");
            }
            if (!(symbol.type == TYPE_SYM::ASSIGN)) printPos(97135);
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(intP());
            symbolTable.insertSymbolEntry(new ScalerSymEntry(NAME, CATE, TYPE, LAYER));
        }
    }
    else if (this->symbol.type == TYPE_SYM::CHARTK) {
        node->addChild(new TerNode(symbol));
        TYPE = TYPE_CHAR;
        nextSym();
        node->addChild(idenP(&NAME, &line));
        if (checkDuplicate(NAME, LAYER)) {
            addErrorMessage(line, 'b', "常量定义时名字重定义");
        }
        if (!(this->symbol.type == TYPE_SYM::ASSIGN)) {
            printPos(3);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(charP());
        symbolTable.insertSymbolEntry(new ScalerSymEntry(NAME, CATE, TYPE, LAYER));
        while (this->symbol.type == TYPE_SYM::COMMA) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP(&NAME, &line));
            if (checkDuplicate(NAME, LAYER)) {
                addErrorMessage(line, 'b', "常量定义时名字重定义");
            }
            if (!(symbol.type == TYPE_SYM::ASSIGN)) printPos(97135);
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(charP());
            symbolTable.insertSymbolEntry(new ScalerSymEntry(NAME, CATE, TYPE, LAYER));
        }
    }
    else {
        printPos(4123123);
    }  
    return node;
}

inline SynNode* Parser::constDecP(int layer) {
    int LAYER = layer;
    NonTerNode* node = new NonTerNode(TYPE_NTS::CONST_DEC, true);
    if (symbol.type == TYPE_SYM::CONSTTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constDefP(LAYER));
        if (!(symbol.type == TYPE_SYM::SEMICN)) { 
            addErrorMessage(symbol.line, 'k', "常量定义中缺少分号");
            printPos(76262); 
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    } else {
        printPos(6381);
    }
    while (this->symbol.type == TYPE_SYM::CONSTTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constDefP(LAYER));
        if (!(this->symbol.type == TYPE_SYM::SEMICN)) {
            addErrorMessage(symbol.line, 'k', "常量定义中缺少分号");
            printPos(51242);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    return node;
}

inline SynNode* Parser::idenP(string* s, int* line) { // both default = nullptr
    NonTerNode* node = new NonTerNode(TYPE_NTS::IDEN, false);
    if (!(symbol.type == TYPE_SYM::IDENFR)) printPos(22);
    node->addChild(new TerNode(symbol));
    if (s) *s = symbol.token;
    if (line) *line = symbol.line;
    nextSym();
    return node;
}

inline SynNode* Parser::decHeadP(string* s, int* line) {
    NonTerNode* node = new NonTerNode(TYPE_NTS::DEC_HEAD, true);
    if (this->symbol.type == TYPE_SYM::INTTK || 
        this->symbol.type == TYPE_SYM::CHARTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        *line = symbol.line;
        node->addChild(idenP(s));
        node->IDEN_TYPE = symbol.type == TYPE_SYM::INTTK ? TYPE_INT : TYPE_CHAR; 
    } else {
        printPos(3313);
    }
    return node;
}

inline SynNode* Parser::constP(int* line) {
    NonTerNode* node = new NonTerNode(TYPE_NTS::CONSTANT, true);
    if (symbol.type == TYPE_SYM::CHARCON) {
        node->addChild(new TerNode(symbol));
        node->setCHARexp(true);
        if (line != nullptr) { *line = symbol.line; }
        nextSym();
    }
    else {
        if (line != nullptr) { *line = symbol.line; }
        node->addChild(intP());
        node->setCHARexp(false);
    }
    return node;
}

inline SynNode* Parser::arrayConstP(int size, int* type) {
    int SIZE = size, i = 0, TYPE = TYPE_VOID, TYPE_TEM;
    NonTerNode* node = new NonTerNode(TYPE_NTS::ARRAY_CONST, false);
    if (symbol.type == TYPE_SYM::LBRACE) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constP());
        TYPE = node->getLastChild()->isCHARconst() ? TYPE_CHAR : TYPE_INT;
        i++;
        while (symbol.type == TYPE_SYM::COMMA) {
            node->addChild(new TerNode(symbol));
            i++;
            if (i > SIZE) {
                printPos(99965);
                addErrorMessage(symbol.line, 'n', "一维数组初始化个数不匹配");
            }
            nextSym();
            node->addChild(constP());
            TYPE_TEM = node->getLastChild()->isCHARconst() ? TYPE_CHAR : TYPE_INT;
            if(TYPE != TYPE_TEM && TYPE != TYPE_ERROR) { TYPE = TYPE_ERROR; }
        }
        if (!(symbol.type == TYPE_SYM::RBRACE)) {
            printPos(99046);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    } 
    else {
        printPos(25261);
    }
    if (TYPE == TYPE_VOID) { TYPE = TYPE_ERROR; }
    *type = TYPE;
    return node;
}

inline SynNode* Parser::doubleArrayConstP(int size1, int size2, int* type) {
    int SIZE1 = size1, SIZE2 = size2, i = 0, TYPE = TYPE_VOID, TYPE_TEM;
    NonTerNode* node = new NonTerNode(TYPE_NTS::DBARRAY_CONST, false);
    if (symbol.type == TYPE_SYM::LBRACE) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(arrayConstP(SIZE1, &TYPE));
        i++;
        while (symbol.type == TYPE_SYM::COMMA) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(arrayConstP(SIZE1, &TYPE_TEM));
            if (TYPE != TYPE_TEM && TYPE != TYPE_ERROR) {
                TYPE = TYPE_ERROR;
            }
            i++;
            if (i > SIZE2) {
                printPos(91651);
                addErrorMessage(symbol.line, 'n', "二维数组初始化维数不匹配");
            }
        }
        if (!(symbol.type == TYPE_SYM::RBRACE)) printPos(84762);
        node->addChild(new TerNode(symbol));
        nextSym();
    } else {
        printPos(151515);
    }
    *type = TYPE;
    return node;
}

inline SynNode* Parser::oneDdeclareP(int* size) {
    NonTerNode* node = new NonTerNode(TYPE_NTS::ONED_DEC, false);
    if (symbol.type == TYPE_SYM::LBRACK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(unsignedIntP()); // TODO CHECK OR NOT?
        *size = node->getLastChild()->INT_value;
        if (!(symbol.type == TYPE_SYM::RBRACK)) {
            printPos(36536);
            addErrorMessage(symbol.line, 'm', "一维数组定义无右括号");
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    } else {
        printPos(42424);
    }
    return node;
}

inline SynNode* Parser::varDerWithInitP(int layer, int type) {
    int CATE = CAT_VAR, TYPE = type, LAYER = layer, SIZE1, SIZE2, line = 0, TYPE_CON;
    string NAME;
    bool hasASSIGN = false;
    NonTerNode* node = new NonTerNode(TYPE_NTS::VARDEF_WITH_INIT, true);
    node->addChild(idenP(&NAME, &line));
    if (checkDuplicate(NAME, LAYER)) {
        addErrorMessage(line, 'b', "有初始化的变量定义时名字重定义");
    }
    if (symbol.type == TYPE_SYM::ASSIGN) {
        hasASSIGN = true;
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constP(&line));
        TYPE_CON = node->getLastChild()->isCHARconst() ? TYPE_CHAR : TYPE_INT;
        if (TYPE != TYPE_CON) {
            addErrorMessage(line, 'o', "变量定义初始化标量常量类型不一致");
        }
        symbolTable.insertSymbolEntry(new ScalerSymEntry(NAME, CATE, TYPE, LAYER));
    } 
    else if (symbol.type == TYPE_SYM::LBRACK) {
        node->addChild(oneDdeclareP(&SIZE1));
        if (symbol.type == TYPE_SYM::ASSIGN) {
            hasASSIGN = true;
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(arrayConstP(SIZE1, &TYPE_CON));
            if (TYPE != TYPE_CON) {
                addErrorMessage(line, 'o', "变量定义初始化一维数组常量类型不一致");
            }
            symbolTable.insertSymbolEntry(new ArraySymEntry(NAME, CATE, TYPE, LAYER, SIZE1));
        }
        else if (symbol.type == TYPE_SYM::LBRACK) {
            node->addChild(oneDdeclareP(&SIZE2));
            if (symbol.type == TYPE_SYM::ASSIGN) {
                hasASSIGN = true;
                node->addChild(new TerNode(symbol));
                nextSym();
                node->addChild(doubleArrayConstP(SIZE1, SIZE2, &TYPE_CON));
                if (TYPE != TYPE_CON) {
                    addErrorMessage(line, 'o', "变量定义初始化二维数组常量类型不一致");
                }
                symbolTable.insertSymbolEntry(new ArraySymEntry(NAME, CATE, TYPE, LAYER, SIZE1, SIZE2));
            } else {
                printPos(55255);
            }
        } else {
            printPos(62673);
        }
    }
    else printPos(16161);
    if (!hasASSIGN) {
        addErrorMessage(symbol.line, 'o', "变量定义及初始化没有赋值");
    }
    return node;
}

inline SynNode* Parser::varDerWithoutInitP(int layer, int type) {
    int CATE = CAT_VAR, TYPE = type, SIZE1, SIZE2, LAYER = layer, line = 0;
    string NAME;
    NonTerNode* node = new NonTerNode(TYPE_NTS::VAR_DEFWIOU_INIT, true);
    node->addChild(idenP(&NAME, &line));
    if (checkDuplicate(NAME, LAYER)) {
        addErrorMessage(line, 'b', "无初始化变量定义时名字重定义");
    }
    int dim = 0;
    if (symbol.type == TYPE_SYM::LBRACK) {
        dim++;
        node->addChild(oneDdeclareP(&SIZE1));
        if (symbol.type == TYPE_SYM::LBRACK) {
            dim++;
            node->addChild(oneDdeclareP(&SIZE2));
        }
    }
    if (dim == 0) {
        symbolTable.insertSymbolEntry(new ScalerSymEntry(NAME, CATE, TYPE, LAYER));
    }
    else if (dim == 1) {
        symbolTable.insertSymbolEntry(new ArraySymEntry(NAME, CATE, TYPE, LAYER, SIZE1));
    }
    else if (dim == 2) {
        symbolTable.insertSymbolEntry(new ArraySymEntry(NAME, CATE, TYPE, LAYER, SIZE1, SIZE2));
    }
    while (symbol.type == TYPE_SYM::COMMA) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(idenP(&NAME));
        if (checkDuplicate(NAME, LAYER)) {
            addErrorMessage(line, 'b', "无初始化变量定义时名字重定义");
        }
        int dim = 0;
        if (symbol.type == TYPE_SYM::LBRACK) {
            dim++;
            node->addChild(oneDdeclareP(&SIZE1));
            if (symbol.type == TYPE_SYM::LBRACK) {
                dim++;
                node->addChild(oneDdeclareP(&SIZE2));
            }
        }
        if (dim == 0) {
            symbolTable.insertSymbolEntry(new ScalerSymEntry(NAME, CATE, TYPE, LAYER));
        }
        else if (dim == 1) {
            symbolTable.insertSymbolEntry(new ArraySymEntry(NAME, CATE, TYPE, LAYER, SIZE1));
        }
        else if (dim == 2) {
            symbolTable.insertSymbolEntry(new ArraySymEntry(NAME, CATE, TYPE, LAYER, SIZE1, SIZE2));
        }
    }
    return node;
}

SynNode* Parser::varDecP(int layer) {
    int LAYER = layer;
    NonTerNode* node = new NonTerNode(TYPE_NTS::VAR_DEC, true);
    node->addChild(varDefP(LAYER));
    if (!(symbol.type == TYPE_SYM::SEMICN)) printPos(9146);
    node->addChild(new TerNode(symbol));
    nextSym();
    while (true) 
    {
        preReadSym(2);
        if (!cacheContainsSym(TYPE_SYM::LPARENT) &&
            (symbol.type == TYPE_SYM::CHARTK ||
                symbol.type == TYPE_SYM::INTTK)) 
        {
            flushPreRead();
            node->addChild(varDefP(LAYER));
            if (!(symbol.type == TYPE_SYM::SEMICN)) printPos(22431);
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else 
        {
            flushPreRead();
            break;
        }
    }
    return node;
}

inline SynNode* Parser::varDefP(int layer) {
    int TYPE, LAYER = layer;
    NonTerNode* node = new NonTerNode(TYPE_NTS::VAR_DEF, true);
    node->addChild(typeIdenP());
    TYPE = node->getLastChild()->getIDENtype();
    int i = 0;
    while (true) {
        i++;
        preReadSym(1);
        if (cacheContainsSym(TYPE_SYM::SEMICN) || 
            cacheContainsSym(TYPE_SYM::COMMA)) {
            flushPreRead();
            node->addChild(varDerWithoutInitP(LAYER, TYPE));
            break;
        }
        if (cacheContainsSym(TYPE_SYM::ASSIGN)) {
            flushPreRead();
            node->addChild(varDerWithInitP(LAYER, TYPE));
            break;
        }
        if (i >= 9) {
            flushPreRead();
            printPos(998613);
            break;
        }
    }
    return node;
}

inline SynNode* Parser::typeIdenP() { // no highlight
    NonTerNode* node = new NonTerNode(TYPE_NTS::TYPEIDEN, false);
    if (symbol.type == TYPE_SYM::INTTK) {
        node->IDEN_TYPE = TYPE_INT;
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::CHARTK) {
        node->IDEN_TYPE = TYPE_CHAR;
        node->addChild(new TerNode(symbol));
        nextSym();
    } {
        printPos(51749);
    }
    return node;
}

inline SynNode* Parser::termP(int layer) {
    int LAYER = layer;
    NonTerNode* node = new NonTerNode(TYPE_NTS::TERM, true);
    node->setCHARexp(false);
    node->addChild(factorP(LAYER));
    if (node->getLastChild()->isCHARexp()) {
        node->setCHARexp(true);
    }
    while (symbol.type == TYPE_SYM::MULT || symbol.type == TYPE_SYM::DIV) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(factorP(LAYER));
        node->setCHARexp(false); // more than one factor, not a char type term
    }
    return node;
}

inline SynNode* Parser::factorP(int layer) {
    int LAYER = layer;
    NonTerNode* node = new NonTerNode(TYPE_NTS::FACTOR, true);
    node->setCHARexp(false);
    if (symbol.type == TYPE_SYM::LPARENT) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(expressionP(LAYER));
        if (!(symbol.type == TYPE_SYM::RPARENT)) { 
            printPos(914151);
            addErrorMessage(symbol.line, 'l', "因子中缺少右括号");
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::CHARCON) {
        node->addChild(new TerNode(symbol));
        node->setCHARexp(true);
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::PLUS ||
        symbol.type == TYPE_SYM::MINU || symbol.type == TYPE_SYM::INTCON) {
        node->addChild(intP());
    }
    else if (symbol.type == TYPE_SYM::IDENFR) {
        int TYPE;
        string NAME = symbol.token;
        if (getIDENtype(NAME, &TYPE, LAYER)) {
            addErrorMessage(symbol.line, 'c', "因子中引用了未定义的名字");
        }
        if (TYPE == TYPE_CHAR) {
            node->setCHARexp(true);
        }
        preReadSym(1);
        if (!cacheContainsSym(TYPE_SYM::LPARENT)) {
            flushPreRead();
            node->addChild(idenP());
            if (symbol.type == TYPE_SYM::LBRACK) {
                node->addChild(new TerNode(symbol));
                nextSym();
                node->addChild(expressionP(LAYER)); // TODO
                if (node->getLastChild()->isCHARexp()) {
                    addErrorMessage(symbol.line, 'i', "表达式数组下标为字符型");
                }
                if (!(symbol.type == TYPE_SYM::RBRACK)) {
                    printPos(6516151);
                    addErrorMessage(symbol.line, 'm', "因子中数组元素缺少右中括号");
                }
                node->addChild(new TerNode(symbol));
                nextSym();
                if (symbol.type == TYPE_SYM::LBRACK) {
                    node->addChild(new TerNode(symbol));
                    nextSym();
                    node->addChild(expressionP(LAYER)); //TODO
                    if (node->getLastChild()->isCHARexp()) {
                        addErrorMessage(symbol.line, 'i', "表达式数组下标为字符型");
                    }
                    if (!(symbol.type == TYPE_SYM::RBRACK))  {
                        printPos(881651);
                        addErrorMessage(symbol.line, 'm', "因子中二维数组元素缺少右中括号");
                    }
                    node->addChild(new TerNode(symbol));
                    nextSym();
                }
            } else {
                // just var scaler
            }
        }
        else if (cacheContainsSym(TYPE_SYM::LPARENT)) {
            flushPreRead();
            node->addChild(callFuncSenP(LAYER));
        }
    }
    else printPos(99517);
    return node;
}

inline SynNode* Parser::expressionP(int layer) {
    int LAYER = layer;
    NonTerNode* node = new NonTerNode(TYPE_NTS::EXPERSSION, true);
    node->setCHARexp(false);
    bool hasOP = false;
    if (symbol.type == TYPE_SYM::PLUS || symbol.type == TYPE_SYM::MINU) {
        hasOP = true;
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    node->addChild(termP(LAYER));
    if (hasOP && node->getLastChild()->isCHARexp()) {
        node->setCHARexp(true);
    }
    while (symbol.type == TYPE_SYM::PLUS || symbol.type == TYPE_SYM::MINU) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(termP(LAYER));
        node->setCHARexp(false);
    }
    return node;
}

inline SynNode* Parser::sentenceP(int layer) { // TODO
    int LAYER = layer;
    NonTerNode* node = new NonTerNode(TYPE_NTS::SENTENCE, true);
    if (symbol.type == TYPE_SYM::WHILETK || symbol.type == TYPE_SYM::FORTK) {
        node->addChild(loopSenP(LAYER + 1));
    } 
    else if (symbol.type == TYPE_SYM::IFTK) {
        node->addChild(ifelseSenP(LAYER + 1));
    }
    else if (symbol.type == TYPE_SYM::SCANFTK) {
        node->addChild(readSenP());
        if (!(symbol.type == TYPE_SYM::SEMICN)) {
            printPos(883833);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::PRINTFTK) {
        node->addChild(writeSenP(LAYER));
        if (!(symbol.type == TYPE_SYM::SEMICN)) {
            printPos(426621);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::SWITCHTK) {
        node->addChild(switchSenP(LAYER + 1));
    }
    else if (symbol.type == TYPE_SYM::RETURNTK) {
        node->addChild(returnSenP(LAYER));
        if (!(symbol.type == TYPE_SYM::SEMICN)) {
            printPos(99578);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::SEMICN) {
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::IDENFR) {
        preReadSym(1);
        if (cacheContainsSym(TYPE_SYM::LPARENT)) {
            flushPreRead();
            node->addChild(callFuncSenP(LAYER));
        }
        else {
            flushPreRead();
            node->addChild(assignSenP(LAYER));
        }
        if (!(symbol.type == TYPE_SYM::SEMICN)) printPos(88174);
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::LBRACE) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceListP(LAYER + 1));
        if (!(symbol.type == TYPE_SYM::RBRACE)) printPos(737892);
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else printPos(6262111);
    return node;
}

inline SynNode* Parser::arguListP(int layer, int* num, FuncSymEntry* func) {
    int LAYER = layer;
    int CATE = CAT_VAR, TYPE, line = 0, n = 0;
    string NAME;
    NonTerNode* node = new NonTerNode(TYPE_NTS::ARGLIST, true);
    if (symbol.type == TYPE_SYM::INTTK || symbol.type == TYPE_SYM::CHARTK) {
        node->addChild(typeIdenP());
        TYPE = node->getLastChild()->getIDENtype();
        node->addChild(idenP(&NAME));
        symbolTable.insertSymbolEntry(new FormalVarSymEntry(NAME, CATE, TYPE, LAYER, func->getINDEX()));
        func->addParaType(TYPE);
        n++;
        while (symbol.type == TYPE_SYM::COMMA) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(typeIdenP());
            TYPE = node->getLastChild()->getIDENtype();
            node->addChild(idenP(&NAME, &line));
            if (checkDuplicate(NAME, LAYER)) {
                addErrorMessage(line, 'b', "参数列表中重复定义名字");
            }
            symbolTable.insertSymbolEntry(new FormalVarSymEntry(NAME, CATE, TYPE, LAYER, func->getINDEX()));
            func->addParaType(TYPE);
            n++;
        }
    }
    *num = n;
    return node;
}

SynNode* Parser::refuncDefineP(int layer) {
    int LAYER = layer, TYPE, CATE = CAT_FUNC, LINE = -1, ARGNUM=0;
    string NAME;
    NonTerNode* node = new NonTerNode(TYPE_NTS::REFUNC_DEF, true);
    node->addChild(decHeadP(&NAME, &LINE));
    if (checkDuplicate(NAME, LAYER)) {
        addErrorMessage(LINE, 'b', "有返回值函数名字重复定义");
    }
    TYPE = node->getLastChild()->getIDENtype();
    FuncSymEntry* symFUNC = new FuncSymEntry(NAME, CATE, TYPE, LAYER, -1);
    symbolTable.insertSymbolEntry(symFUNC);
    if (symbol.type == TYPE_SYM::LPARENT) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(arguListP(LAYER, &ARGNUM, symFUNC));
        if (!(symbol.type == TYPE_SYM::RPARENT)) {
            printPos(123456);
            addErrorMessage(symbol.line, 'l', "nonvoid函数定义缺少右括号");
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else printPos(22009);
    symFUNC->setARGNUM(ARGNUM);
    //this->returnFuncList.insert(name);
    if (symbol.type == TYPE_SYM::LBRACE) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(compoundSenP(LAYER + 1));
        if (!(symbol.type == TYPE_SYM::RBRACE)) printPos(891947);
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else printPos(91415);
    return node;
}

SynNode* Parser::nonrefuncDefineP(int layer) {
    int LAYER = layer, TYPE = TYPE_VOID, CATE = CAT_FUNC, LINE = -1, ARGNUM=0;
    string NAME;
    NonTerNode* node = new NonTerNode(TYPE_NTS::NONREFUNC_DEF, true);
    if (symbol.type == TYPE_SYM::VOIDTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(idenP(&NAME));
        if (checkDuplicate(NAME, LAYER)) {
            addErrorMessage(LINE, 'b', "有返回值函数名字重复定义");
        }
        FuncSymEntry* symFUNC = new FuncSymEntry(NAME, CATE, TYPE, LAYER, -1);
        symbolTable.insertSymbolEntry(symFUNC);
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(arguListP(LAYER, &ARGNUM, symFUNC));
            if (!(symbol.type == TYPE_SYM::RPARENT)) {
                printPos(8085);
                addErrorMessage(symbol.line, 'l', "void函数定义缺少右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else printPos(8633);
        symFUNC->setARGNUM(ARGNUM);
        //this->nonreturnFuncList.insert(name);
        if (symbol.type == TYPE_SYM::LBRACE) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(compoundSenP(LAYER + 1));
            if (!(symbol.type == TYPE_SYM::RBRACE)) printPos(9976);
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else printPos(9842);
    }
    else printPos(7522);
    return node;
}

inline SynNode* Parser::callFuncSenP(int layer) {
    int LAYER = layer, LINE = -1;
    string NAME;
    SynNode* iden = idenP(&NAME, &LINE);
    NonTerNode* node = nullptr;
    FuncSymEntry* func = getFUNC_CALL(NAME);
    if (func == nullptr) {
        addErrorMessage(LINE, 'c', "函数调用了未定义的名字");
    }
    else if (func->getCATE() != CAT_FUNC) {
        printPos(71313);
    }
    else {
        if (func->getTYPE() == TYPE_VOID) {
            node = new NonTerNode(TYPE_NTS::CALL_NONREFUNC_SEN, true);
        }
        else {
            node = new NonTerNode(TYPE_NTS::CALL_REFUNC_SEN, true);
        }
    }
    node->addChild(iden);
    if (symbol.type == TYPE_SYM::LPARENT) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(valueArgueListP(LAYER, func));
        if (!(symbol.type == TYPE_SYM::RPARENT)) {
            printPos(99875);
            addErrorMessage(symbol.line, 'l', "函数调用缺少右括号");
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else printPos(88278);
    return node;
}

inline SynNode* Parser::valueArgueListP(int layer, FuncSymEntry* func) {
    int LAYER = layer, ARGUNUM = func->getARGNUM(), n = 0, TEM_TYPE, LINE = -1;
    NonTerNode* node = new NonTerNode(TYPE_NTS::VALUE_ARGLIST, true);
    int i_para = 0;
    if (symbol.type == TYPE_SYM::PLUS ||
        symbol.type == TYPE_SYM::MINU ||
        symbol.type == TYPE_SYM::IDENFR ||
        symbol.type == TYPE_SYM::LPARENT ||
        symbol.type == TYPE_SYM::LBRACK||
        symbol.type == TYPE_SYM::INTCON||
        symbol.type == TYPE_SYM::CHARCON) {
        node->addChild(expressionP(LAYER));
        TEM_TYPE = node->getLastChild()->getIDENtype();
        if (TEM_TYPE != func->getParaTypeList()[i_para]) {
            addErrorMessage(symbol.line, 'e', "函数调用参数个数不匹配");
        }
        n++;
        i_para++;
        while (symbol.type == TYPE_SYM::COMMA) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP(LAYER));
            TEM_TYPE = node->getLastChild()->getIDENtype();
            if (TEM_TYPE != func->getParaTypeList()[i_para]) {
                addErrorMessage(symbol.line, 'e', "函数调用参数个数不匹配");
            }
            n++;
        }
    }
    if (func->getARGNUM() != n) {
        addErrorMessage(symbol.line, 'd', "函数调用个数不匹配");
    }
    return node;
}

inline SynNode* Parser::assignSenP(int layer) {
    int LAYER = layer, LEFT_TYPE, RIGHT_TYPE, SUB_TYPE, LEFT_CATE, LINE = -1;
    SymTableEntry* LEFT_VALUE;
    string NAME;
    NonTerNode* node = new NonTerNode(TYPE_NTS::ASSIGN_SEN, true);
    node->addChild(idenP(&NAME, &LINE));
    LEFT_TYPE = node->getLastChild()->getIDENtype();
    LEFT_VALUE = getEntryByName(NAME);
    if (LEFT_VALUE == nullptr) {
        addErrorMessage(LINE, 'c', "赋值语句引用了未定义的名字");
    }
    else if (LEFT_VALUE->getCATE() == CAT_CONST) {
        addErrorMessage(LINE, 'j', "给常量赋值");
    }
    else if (dynamic_cast<ArraySymEntry*>(LEFT_VALUE) != nullptr) {
        printPos(713339);
    }
    else {
        if (symbol.type == TYPE_SYM::LBRACK) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP(LAYER));
            SUB_TYPE = node->getLastChild()->getIDENtype();
            if (SUB_TYPE == TYPE_CHAR) {
                addErrorMessage(symbol.line, 'i', "赋值语句数组下标为字符型");
            }
            if (!(symbol.type == TYPE_SYM::RBRACK)) {
                printPos(7733);
                addErrorMessage(symbol.line, 'm', "赋值语句中数组缺少右中括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            if (symbol.type == TYPE_SYM::LBRACK) {
                node->addChild(new TerNode(symbol));
                nextSym();
                node->addChild(expressionP(LAYER));
                SUB_TYPE = node->getLastChild()->getIDENtype();
                if (SUB_TYPE == TYPE_CHAR) {
                    addErrorMessage(symbol.line, 'i', "赋值语句数组下标为字符型");
                }
                if (!(symbol.type == TYPE_SYM::RBRACK)) {
                    printPos(776133);
                    addErrorMessage(symbol.line, 'm', "赋值语句中数组缺少右中括号");
                }
                node->addChild(new TerNode(symbol));
                nextSym();
            }
        }
        if (symbol.type == TYPE_SYM::ASSIGN) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP(LAYER));
            RIGHT_TYPE = node->getLastChild()->getIDENtype();
        } else printPos(62562);
    }
    
    return node;
}

inline SynNode* Parser::ifelseSenP(int layer) {
    int LAYER = layer, LINE = -1;
    NonTerNode* node = new NonTerNode(TYPE_NTS::IFELSE_SEN, true);
    if (symbol.type == TYPE_SYM::IFTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(conditionP(LAYER));
            if (!(symbol.type == TYPE_SYM::RPARENT)) {
                printPos(5662325);
                addErrorMessage(symbol.line, 'l', "ifelse中缺失右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym(); 
        } else printPos(66725);
    } else printPos(9947252);
    node->addChild(sentenceP(LAYER));
    if (symbol.type == TYPE_SYM::ELSETK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceP(LAYER));
    }
    return node;
}

inline SynNode* Parser::compareOpP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::COMPARE_OP, false);
    if (!(symbol.type == TYPE_SYM::LSS ||
        symbol.type == TYPE_SYM::LEQ ||
        symbol.type == TYPE_SYM::GRE ||
        symbol.type == TYPE_SYM::GEQ ||
        symbol.type == TYPE_SYM::EQL ||
        symbol.type == TYPE_SYM::NEQ)) {
            printPos(4946561);
        }
    node->addChild(new TerNode(symbol));
    nextSym();
    return node;
}

inline SynNode* Parser::conditionP(int layer) {
    int LAYER = layer;
    NonTerNode* node = new NonTerNode(TYPE_NTS::CONDITION, true);
    node->addChild(expressionP(LAYER));
    if (node->getLastChild()->isCHARexp()) {
        addErrorMessage(symbol.line, 'f', "条件判断出现char");
    }
    node->addChild(compareOpP());
    node->addChild(expressionP(LAYER));
    if (node->getLastChild()->isCHARexp()) {
        addErrorMessage(symbol.line, 'f', "条件判断出现char");
    }
    return node;
}

inline SynNode* Parser::stepLengthP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::STEP_LEN, true);
    node->addChild(unsignedIntP());
    return node;
}

inline SynNode* Parser::loopSenP(int layer) {
    int LAYER = layer;
    NonTerNode* node = new NonTerNode(TYPE_NTS::LOOP_SEN, true);
    if (symbol.type == TYPE_SYM::WHILETK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(conditionP(LAYER));
            if (!(symbol.type == TYPE_SYM::RPARENT)) {
                printPos(11451);
                addErrorMessage(symbol.line, 'l', "while语句缺少右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        } else printPos(414151);
        node->addChild(sentenceP(LAYER));
    }   
    else if (symbol.type == TYPE_SYM::FORTK) {
        string NAME;
        SymTableEntry* sym = nullptr;
        int LINE, TYPE, CATE = CAT_VAR;
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP(&NAME, &LINE));
            sym = getEntryByName(NAME);
            if (sym == nullptr) {
                addErrorMessage(LINE, 'c', "for语句中引用未定义的名字");
            }
            else if (sym->getCATE() == CAT_CONST) {
                addErrorMessage(LINE, 'j', "for语句中给常量赋值");
            }
            if (!(symbol.type == TYPE_SYM::ASSIGN)) printPos(841656);
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP(LAYER));
            if (!(symbol.type == TYPE_SYM::SEMICN)) {
                printPos(99713);
                addErrorMessage(symbol.line, 'k', "for语句中缺少分号1");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(conditionP(LAYER));
            if (!(symbol.type == TYPE_SYM::SEMICN)) {
                printPos(99713);
                addErrorMessage(symbol.line, 'k', "for语句中缺少分号2");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP(&NAME, &LINE));
            sym = getEntryByName(NAME);
            if (sym == nullptr) {
                addErrorMessage(LINE, 'c', "for语句中引用未定义的名字");
            }
            else if (sym->getCATE() == CAT_CONST) {
                addErrorMessage(LINE, 'j', "for语句中给常量赋值");
            }
            if (!(symbol.type == TYPE_SYM::ASSIGN)) printPos(61319);
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP(&NAME, &LINE));
            sym = getEntryByName(NAME);
            if (sym == nullptr) {
                addErrorMessage(LINE, 'c', "for语句中引用未定义的名字");
            }
            else if (sym->getCATE() == CAT_CONST) {
                addErrorMessage(LINE, 'j', "for语句中给常量赋值");
            }
            if (!(symbol.type == TYPE_SYM::PLUS || symbol.type == TYPE_SYM::MINU)) printPos(26262);
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(stepLengthP());
            if (!(symbol.type == TYPE_SYM::RPARENT)) {
                printPos(911451);
                addErrorMessage(symbol.line, 'l', "for语句缺少右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        } else printPos(907691);
        node->addChild(sentenceP(LAYER));
    }
    return node;
}

inline SynNode* Parser::switchSenP(int layer) {
    int LAYER = layer, TYPE_EXP;
    NonTerNode* node = new NonTerNode(TYPE_NTS::SWITCH_SEN, true);
    if (symbol.type == TYPE_SYM::SWITCHTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP(LAYER));
            TYPE_EXP = node->getLastChild()->isCHARexp() ? TYPE_CHAR : TYPE_INT;
            if (!(symbol.type == TYPE_SYM::RPARENT)) {
                printPos(77251);
                addErrorMessage(symbol.line, 'l', "switch缺少右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else printPos(14149);
        if (symbol.type == TYPE_SYM::LBRACE) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(caseListP(LAYER, TYPE_EXP));
            node->addChild(defaultP(LAYER));
            if (!(symbol.type == TYPE_SYM::RBRACE)) printPos(214156);
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else printPos(991737);
    }
    else printPos(626666);
    return node;
}

inline SynNode* Parser::caseSenP(int layer, int type_exp) {
    int LAYER = layer, TYPE_EXP = type_exp, TYPE_CON;
    NonTerNode* node = new NonTerNode(TYPE_NTS::CASE_SEN, true);
    if (symbol.type == TYPE_SYM::CASETK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constP());
        TYPE_CON = node->getLastChild()->isCHARconst() ? TYPE_CHAR : TYPE_INT;
        if (TYPE_CON != TYPE_EXP) {
            addErrorMessage(symbol.line, 'o', "case语句常量类型不一致");
        }
        if (!(symbol.type == TYPE_SYM::COLON)) printPos(366134);
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceP(LAYER));
    } else printPos(96911);
    return node;
}

inline SynNode* Parser::caseListP(int layer, int type_exp) {
    int LAYER = layer, TYPE_EXP = type_exp;
    NonTerNode* node = new NonTerNode(TYPE_NTS::CASE_LIST, true);
    node->addChild(caseSenP(LAYER, TYPE_EXP));
    while (symbol.type == TYPE_SYM::CASETK) {
        node->addChild(caseSenP(LAYER, TYPE_EXP));
    }
    return node;
}

inline SynNode* Parser::defaultP(int layer) {
    int LAYER = layer;
    NonTerNode* node = new NonTerNode(TYPE_NTS::DEFAULT, true);
    if (symbol.type == TYPE_SYM::DEFAULTTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (!(symbol.type == TYPE_SYM::COLON)) printPos(888888);
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceP(LAYER));
    }
    else {
        printPos(2227222);
        addErrorMessage(symbol.line, 'p', "default语句缺失");
    }
    return node;
}

inline SynNode* Parser::readSenP() {
    int LINE;
    string NAME;
    SymTableEntry* sym=nullptr;
    NonTerNode* node = new NonTerNode(TYPE_NTS::READ_SEN, true);
    if (symbol.type == TYPE_SYM::SCANFTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP(&NAME, &LINE));
            sym = getEntryByName(NAME);
            if (sym == nullptr) {
                addErrorMessage(LINE, 'c', "读语句引用了未定义的名字");
            }
            else if (sym->getCATE() == CAT_CONST) {
                addErrorMessage(LINE, 'j', "读语句给常量赋值");
            }
            if (!(symbol.type == TYPE_SYM::RPARENT)) {
                printPos(214746);
                addErrorMessage(symbol.line, 'l', "读语句缺少右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym(); 
        } else printPos(77242);
    } else printPos(998754);
    return node;
}

inline SynNode* Parser::writeSenP(int layer) {
    int LAYER = layer, LINE;
    string NAME;
    SymTableEntry* sym = nullptr;
    NonTerNode* node = new NonTerNode(TYPE_NTS::WRITE_SEN, true);
    if (symbol.type == TYPE_SYM::PRINTFTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            if (symbol.type == TYPE_SYM::STRCON) {
                node->addChild(stringP());
                if (symbol.type == TYPE_SYM::COMMA) {
                    node->addChild(new TerNode(symbol));
                    nextSym();
                    node->addChild(expressionP(LAYER));
                }
            } else {
                node->addChild(expressionP(LAYER));
            }
            if (!(symbol.type == TYPE_SYM::RPARENT)) {
                printPos(424249);
                addErrorMessage(symbol.line, 'l', "写语句没有右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym(); 
        } else printPos(892648);
    }
    return node;
}

SynNode* Parser::returnSenP(int layer) {
    int LAYER = layer;
    NonTerNode* node = new NonTerNode(TYPE_NTS::RETURN_SEN, true);
    if (symbol.type == TYPE_SYM::RETURNTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP(LAYER));
            if (!(symbol.type == TYPE_SYM::RPARENT)) {
                printPos(1984);
                addErrorMessage(symbol.line, 'l', "返回语句缺少右括号");
            }
            node->addChild(new TerNode(symbol));
            nextSym();
        }
    }

    return node;
}

inline SynNode* Parser::sentenceListP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::SEN_LIST, true);
    while (true) {
        if (symbol.type == TYPE_SYM::WHILETK ||
            symbol.type == TYPE_SYM::FORTK ||
            symbol.type == TYPE_SYM::IFTK ||
            symbol.type == TYPE_SYM::SCANFTK ||
            symbol.type == TYPE_SYM::PRINTFTK ||
            symbol.type == TYPE_SYM::SWITCHTK ||
            symbol.type == TYPE_SYM::RETURNTK ||
            symbol.type == TYPE_SYM::SEMICN ||
            symbol.type == TYPE_SYM::IDENFR ||
            symbol.type == TYPE_SYM::LBRACE) {
            node->addChild(sentenceP());
        }
        else break;
    }
    return node;
}

inline SynNode* Parser::compoundSenP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::COMPOUND_SEN, true);
    if (symbol.type == TYPE_SYM::CONSTTK) {
        node->addChild(constDecP());
    } 
    if (symbol.type == TYPE_SYM::INTTK || symbol.type == TYPE_SYM::CHARTK) {
        node->addChild(varDecP());
    }
    node->addChild(sentenceListP());
    return node;
}

inline SynNode* Parser::mainP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::MAIN, true);
    if (!(symbol.type == TYPE_SYM::VOIDTK)) printPos(626);
    node->addChild(new TerNode(symbol));
    nextSym();
    if (!(symbol.type == TYPE_SYM::MAINTK)) printPos(525);
    node->addChild(new TerNode(symbol));
    nextSym();
    if (!(symbol.type == TYPE_SYM::LPARENT)) printPos(556);
    node->addChild(new TerNode(symbol));
    nextSym();
    if (!(symbol.type == TYPE_SYM::RPARENT)) printPos(715);
    node->addChild(new TerNode(symbol));
    nextSym();
    if (!(symbol.type == TYPE_SYM::LBRACE)) printPos(987);
    node->addChild(new TerNode(symbol));
    nextSym(); 
    node->addChild(compoundSenP());
    if (!(symbol.type == TYPE_SYM::RBRACE)) printPos(158);
    node->addChild(new TerNode(symbol));
    nextSym(); 
    return node;
}

SynNode* Parser::parse() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::PROGRAM, true);
    if (symbol.type == TYPE_SYM::CONSTTK) {
        node->addChild(constDecP());
    }
    preReadSym(2);
    if (!(cacheContainsSym(TYPE_SYM::LPARENT))) {
        flushPreRead();
        node->addChild(varDecP());
    }
    flushPreRead();
    while (true) {
        preReadSym(2);
        if (cacheContainsSym(TYPE_SYM::LPARENT) && 
            !cacheContainsSym(TYPE_SYM::MAINTK)) {
            if (symbol.type == TYPE_SYM::VOIDTK) {
                flushPreRead();
                node->addChild(nonrefuncDefineP());
            }
            else {
                flushPreRead();
                node->addChild(refuncDefineP());
            }
        }
        else {
            flushPreRead();
            break;
        }
    }
    node->addChild(mainP());
    return node;
}