#include "lexical.h"
#include "parser.h"
#include "synTree.h"
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
    nextSym();
    return node;
}

inline SynNode* Parser::intP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::INT, true);
    if (this->symbol.type == TYPE_SYM::PLUS || 
        this->symbol.type == TYPE_SYM::MINU) {
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    node->addChild(unsignedIntP());
    return node;
}

inline SynNode* Parser::charP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::CHAR, false);
    if (symbol.type == TYPE_SYM::CHARCON) {
        node->addChild(new TerNode(symbol));
    }
    else printPos(313);
    nextSym();
    return node;
}

inline SynNode* Parser::constDefP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::CONST_DEF, true);
    if (symbol.type == TYPE_SYM::INTTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(idenP());
        if (!(symbol.type == TYPE_SYM::ASSIGN)) {
            printPos(414123);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(intP());
        while (symbol.type == TYPE_SYM::COMMA) {
            node->addChild(new TerNode(symbol));
            nextSym();
            if (!(symbol.type == TYPE_SYM::IDENFR)) printPos(18964);
            node->addChild(new TerNode(symbol));
            nextSym();
            if (!(symbol.type == TYPE_SYM::ASSIGN)) printPos(97135);
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(intP());
        }
    }
    else if (this->symbol.type == TYPE_SYM::CHARTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(idenP());
        if (!(this->symbol.type == TYPE_SYM::ASSIGN)) {
            printPos(3);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(charP());
        while (this->symbol.type == TYPE_SYM::COMMA) {
            node->addChild(new TerNode(symbol));
            nextSym();
            if (!(symbol.type == TYPE_SYM::IDENFR)) printPos(18964);
            node->addChild(new TerNode(symbol));
            nextSym();
            if (!(symbol.type == TYPE_SYM::ASSIGN)) printPos(97135);
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(charP());
        }
    }
    else {
        printPos(4123123);
    }  
    return node;
}

inline SynNode* Parser::constDecP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::CONST_DEC, true);
    if (this->symbol.type == TYPE_SYM::CONSTTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constDefP());
        if (!(this->symbol.type == TYPE_SYM::SEMICN)) printPos(76262);
        node->addChild(new TerNode(symbol));
        nextSym();
    } else {
        printPos(6381);
    }
    while (this->symbol.type == TYPE_SYM::CONSTTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constDefP());
        if (!(this->symbol.type == TYPE_SYM::SEMICN)) printPos(51444);
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    return node;
}

inline SynNode* Parser::idenP(string* s /*= nullptr*/) { // no highlight
    NonTerNode* node = new NonTerNode(TYPE_NTS::IDEN, false);
    if (!(symbol.type == TYPE_SYM::IDENFR)) printPos(22);
    node->addChild(new TerNode(symbol));
    if (s) *s = symbol.token;
    nextSym();
    return node;
}

inline SynNode* Parser::decHeadP(string* s = nullptr) {
    NonTerNode* node = new NonTerNode(TYPE_NTS::DEC_HEAD, true);
    if (this->symbol.type == TYPE_SYM::INTTK || 
        this->symbol.type == TYPE_SYM::CHARTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(idenP(s));
    } else {
        printPos(3313);
    }
    return node;
}

inline SynNode* Parser::constP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::CONSTANT, true);
    if (symbol.type == TYPE_SYM::CHARCON) {
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else {
        node->addChild(intP());
    }
    return node;
}

inline SynNode* Parser::arrayConstP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::ARRAY_CONST, false);
    if (symbol.type == TYPE_SYM::LBRACE) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constP());
        while (symbol.type == TYPE_SYM::COMMA) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(constP());
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
    return node;
}

inline SynNode* Parser::doubleArrayConstP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::DBARRAY_CONST, false);
    if (symbol.type == TYPE_SYM::LBRACE) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(arrayConstP());
        while (symbol.type == TYPE_SYM::COMMA) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(arrayConstP());
        }
        if (!(symbol.type == TYPE_SYM::RBRACE)) printPos(84762);
        node->addChild(new TerNode(symbol));
        nextSym();
    } else {
        printPos(151515);
    }
    return node;
}

inline SynNode* Parser::oneDdeclareP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::ONED_DEC, false);
    if (symbol.type == TYPE_SYM::LBRACK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(unsignedIntP());
        if (!(symbol.type == TYPE_SYM::RBRACK)) {
            printPos(36536);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    } else {
        printPos(42424);
    }
    return node;
}

inline SynNode* Parser::varDerWithInitP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::VARDEF_WITH_INIT, true);
    node->addChild(idenP());
    if (symbol.type == TYPE_SYM::ASSIGN) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constP());
    } 
    else if (symbol.type == TYPE_SYM::LBRACK) {
        node->addChild(oneDdeclareP());
        if (symbol.type == TYPE_SYM::ASSIGN) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(arrayConstP());
        }
        else if (symbol.type == TYPE_SYM::LBRACK) {
            node->addChild(oneDdeclareP());
            if (symbol.type == TYPE_SYM::ASSIGN) {
                node->addChild(new TerNode(symbol));
                nextSym();
                node->addChild(doubleArrayConstP());
            } else {
                printPos(55255);
            }
        } else {
            printPos(62673);
        }
    }
    else printPos(16161);
    return node;
}

inline SynNode* Parser::varDerWithoutInitP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::VAR_DEFWIOU_INIT, true);
    node->addChild(idenP());
    if (symbol.type == TYPE_SYM::LBRACK) {
        node->addChild(oneDdeclareP());
        if (symbol.type == TYPE_SYM::LBRACK) {
            node->addChild(oneDdeclareP());
        }
    }
    while (symbol.type == TYPE_SYM::COMMA) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (!(symbol.type == TYPE_SYM::IDENFR)) printPos(134145);
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LBRACK) {
            node->addChild(oneDdeclareP());
            if (symbol.type == TYPE_SYM::LBRACK) {
                node->addChild(oneDdeclareP());
            }
        }
    }
    return node;
}

inline SynNode* Parser::varDecP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::VAR_DEC, true);
    node->addChild(varDefP());
    if (!(symbol.type == TYPE_SYM::SEMICN)) printPos(9146);
    node->addChild(new TerNode(symbol));
    nextSym();

    while (true) {
        preReadSym(2);
        if (!cacheContainsSym(TYPE_SYM::LPARENT) &&
            (symbol.type == TYPE_SYM::CHARTK ||
                symbol.type == TYPE_SYM::INTTK)) {
            flushPreRead();
            node->addChild(varDefP());
            if (!(symbol.type == TYPE_SYM::SEMICN)) printPos(22431);
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else {
            flushPreRead();
            break;
        }
    }
    return node;
}

inline SynNode* Parser::varDefP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::VAR_DEF, true);
    node->addChild(typeIdenP());
    int i = 0;
    while (true) {
        i++;
        preReadSym(1);
        if (cacheContainsSym(TYPE_SYM::SEMICN) || 
            cacheContainsSym(TYPE_SYM::COMMA)) {
            flushPreRead();
            node->addChild(varDerWithoutInitP());
            break;
        }
        if (cacheContainsSym(TYPE_SYM::ASSIGN)) {
            flushPreRead();
            node->addChild(varDerWithInitP());
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
    if (symbol.type == TYPE_SYM::INTTK || symbol.type == TYPE_SYM::CHARTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
    } else {
        printPos(51749);
    }
    return node;
}

inline SynNode* Parser::arguListP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::ARGLIST, true);
    if (symbol.type == TYPE_SYM::INTTK || symbol.type == TYPE_SYM::CHARTK) {
        node->addChild(typeIdenP());
        node->addChild(idenP());
        while (symbol.type == TYPE_SYM::COMMA) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(typeIdenP());
            node->addChild(idenP());
        }
    }
    return node;
}

inline SynNode* Parser::termP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::TERM, true);
    node->addChild(factorP());
    while (symbol.type == TYPE_SYM::MULT || symbol.type == TYPE_SYM::DIV) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(factorP());
    }
    return node;
}

inline SynNode* Parser::factorP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::FACTOR, true);
    if (symbol.type == TYPE_SYM::LPARENT) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(expressionP());
        if (!(symbol.type == TYPE_SYM::RPARENT)) printPos(914151);
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::LBRACK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(expressionP());
        if (!(symbol.type == TYPE_SYM::RBRACK)) printPos(88174);
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::CHARCON) {
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::PLUS ||
        symbol.type == TYPE_SYM::MINU || symbol.type == TYPE_SYM::INTCON) {
        node->addChild(intP());
    }
    else if (symbol.type == TYPE_SYM::IDENFR) {
        preReadSym(1);
        if (!cacheContainsSym(TYPE_SYM::LPARENT)) {
            flushPreRead();
            node->addChild(idenP());
            if (symbol.type == TYPE_SYM::LBRACK) {
                node->addChild(new TerNode(symbol));
                nextSym();
                node->addChild(expressionP());
                if (!(symbol.type == TYPE_SYM::RBRACK)) printPos(6516151);
                node->addChild(new TerNode(symbol));
                nextSym();
                if (symbol.type == TYPE_SYM::LBRACK) {
                    node->addChild(new TerNode(symbol));
                    nextSym();
                    node->addChild(expressionP());
                    if (!(symbol.type == TYPE_SYM::RBRACK)) printPos(144444);
                    node->addChild(new TerNode(symbol));
                    nextSym();
                }
            }
        }
        else {
            flushPreRead();
            node->addChild(callFuncSenP());
        }
    }
    else printPos(99517);
    return node;
}

inline SynNode* Parser::expressionP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::EXPERSSION, true);
    if (symbol.type == TYPE_SYM::PLUS || symbol.type == TYPE_SYM::MINU) {
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    node->addChild(termP());
    while (symbol.type == TYPE_SYM::PLUS || symbol.type == TYPE_SYM::MINU) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(termP());
    }
    return node;
}

inline SynNode* Parser::sentenceP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::SENTENCE, true);
    if (symbol.type == TYPE_SYM::WHILETK || symbol.type == TYPE_SYM::FORTK) {
        node->addChild(loopSenP());
    } 
    else if (symbol.type == TYPE_SYM::IFTK) {
        node->addChild(ifelseSenP());
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
        node->addChild(writeSenP());
        if (!(symbol.type == TYPE_SYM::SEMICN)) {
            printPos(426621);
        }
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::SWITCHTK) {
        node->addChild(switchSenP());
    }
    else if (symbol.type == TYPE_SYM::RETURNTK) {
        node->addChild(returnSenP());
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
            node->addChild(callFuncSenP());
        }
        else {
            flushPreRead();
            node->addChild(assignSenP());
        }
        if (!(symbol.type == TYPE_SYM::SEMICN)) printPos(88174);
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else if (symbol.type == TYPE_SYM::LBRACE) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceListP());
        if (!(symbol.type == TYPE_SYM::RBRACE)) printPos(737892);
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else printPos(6262111);
    return node;
}

inline SynNode* Parser::callFuncSenP() {
    string name;
    SynNode* iden = idenP(&name);
    NonTerNode* node = nullptr;
    if (returnFuncList.find(name) != returnFuncList.end()) {
        node = new NonTerNode(TYPE_NTS::CALL_REFUNC_SEN, true);
    } 
    else if (nonreturnFuncList.find(name) != nonreturnFuncList.end()) {
        node = new NonTerNode(TYPE_NTS::CALL_NONREFUNC_SEN, true);
    }
    else printPos(442421);
    node->addChild(iden);
    if (symbol.type == TYPE_SYM::LPARENT) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(valueArgueListP());
        if (!(symbol.type == TYPE_SYM::RPARENT)) printPos(99875);
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else printPos(88278);
    return node;
}

SynNode* Parser::refuncDefineP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::REFUNC_DEF, true);
    string name;
    node->addChild(decHeadP(&name));
    if (symbol.type == TYPE_SYM::LPARENT) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(arguListP());
        if (!(symbol.type == TYPE_SYM::RPARENT)) printPos(123456);
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else printPos(22009);
    this->returnFuncList.insert(name);
    if (symbol.type == TYPE_SYM::LBRACE) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(compoundSenP());
        if (!(symbol.type == TYPE_SYM::RBRACE)) printPos(891947);
        node->addChild(new TerNode(symbol));
        nextSym();
    }
    else printPos(91415);
    return node;
}

SynNode* Parser::nonrefuncDefineP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::NONREFUNC_DEF, true);
    string name;
    if (symbol.type == TYPE_SYM::VOIDTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(idenP(&name));
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(arguListP());
            if (!(symbol.type == TYPE_SYM::RPARENT)) printPos(8085);
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else printPos(8633);
        this->nonreturnFuncList.insert(name);
        if (symbol.type == TYPE_SYM::LBRACE) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(compoundSenP());
            if (!(symbol.type == TYPE_SYM::RBRACE)) printPos(9976);
            node->addChild(new TerNode(symbol));
            nextSym();
        }
        else printPos(9842);
    }
    else printPos(7522);
    return node;
}

inline SynNode* Parser::valueArgueListP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::VALUE_ARGLIST, true);
    if (symbol.type == TYPE_SYM::PLUS ||
        symbol.type == TYPE_SYM::MINU ||
        symbol.type == TYPE_SYM::IDENFR ||
        symbol.type == TYPE_SYM::LPARENT ||
        symbol.type == TYPE_SYM::LBRACK||
        symbol.type == TYPE_SYM::INTCON||
        symbol.type == TYPE_SYM::CHARCON) {
        node->addChild(expressionP());
        while (symbol.type == TYPE_SYM::COMMA) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP());
        }
    }
    return node;
}

inline SynNode* Parser::assignSenP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::ASSIGN_SEN, true);
    node->addChild(idenP());
    if (symbol.type == TYPE_SYM::LBRACK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(expressionP());
        if (!(symbol.type == TYPE_SYM::RBRACK)) printPos(7733);
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LBRACK) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP());
            if (!(symbol.type == TYPE_SYM::RBRACK)) printPos(22333);
            node->addChild(new TerNode(symbol));
            nextSym();
        }
    }
    if (symbol.type == TYPE_SYM::ASSIGN) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(expressionP());
    } else printPos(62562);
    return node;
}

inline SynNode* Parser::ifelseSenP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::IFELSE_SEN, true);
    if (symbol.type == TYPE_SYM::IFTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(conditionP());
            if (!(symbol.type == TYPE_SYM::RPARENT)) printPos(5662325);
            node->addChild(new TerNode(symbol));
            nextSym(); 
        } else printPos(66725);
    } else printPos(9947252);
    node->addChild(sentenceP());
    if (symbol.type == TYPE_SYM::ELSETK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceP());
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

inline SynNode* Parser::conditionP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::CONDITION, true);
    node->addChild(expressionP());
    node->addChild(compareOpP());
    node->addChild(expressionP());
    return node;
}

inline SynNode* Parser::stepLengthP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::STEP_LEN, true);
    node->addChild(unsignedIntP());
    return node;
}

inline SynNode* Parser::loopSenP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::LOOP_SEN, true);
    if (symbol.type == TYPE_SYM::WHILETK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(conditionP());
            if (!(symbol.type == TYPE_SYM::RPARENT)) printPos(11451);
            node->addChild(new TerNode(symbol));
            nextSym();
        } else printPos(414151);
        node->addChild(sentenceP());
    }   
    else if (symbol.type == TYPE_SYM::FORTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP());
            if (!(symbol.type == TYPE_SYM::ASSIGN)) printPos(841656);
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP());
            if (!(symbol.type == TYPE_SYM::SEMICN)) printPos(99713);
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(conditionP());
            if (!(symbol.type == TYPE_SYM::SEMICN)) printPos(99713);
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP());
            if (!(symbol.type == TYPE_SYM::ASSIGN)) printPos(61319);
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP());
            if (!(symbol.type == TYPE_SYM::PLUS || symbol.type == TYPE_SYM::MINU)) printPos(26262);
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(stepLengthP());
            if (!(symbol.type == TYPE_SYM::RPARENT)) printPos(441451);
            node->addChild(new TerNode(symbol));
            nextSym();
        } else printPos(907691);
        node->addChild(sentenceP());
    }
    return node;
}

inline SynNode* Parser::caseSenP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::CASE_SEN, true);
    if (symbol.type == TYPE_SYM::CASETK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(constP());
        if (!(symbol.type == TYPE_SYM::COLON)) printPos(366134);
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceP());
    } else printPos(96911);
    return node;
}

inline SynNode* Parser::caseListP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::CASE_LIST, true);
    node->addChild(caseSenP());
    while (symbol.type == TYPE_SYM::CASETK) {
        node->addChild(caseSenP());
    }
    return node;
}

inline SynNode* Parser::switchSenP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::SWITCH_SEN, true);
    if (symbol.type == TYPE_SYM::SWITCHTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP());
            if (!(symbol.type == TYPE_SYM::RPARENT)) printPos(77251);
            node->addChild(new TerNode(symbol));
            nextSym(); 
        } else printPos(14149);
        if (symbol.type == TYPE_SYM::LBRACE) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(caseListP());
            node->addChild(defaultP());
            if (!(symbol.type == TYPE_SYM::RBRACE)) printPos(214156);
            node->addChild(new TerNode(symbol));
            nextSym(); 
        } else printPos(991737);
    } else printPos(626666);
    return node;
}

inline SynNode* Parser::defaultP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::DEFAULT, true);
    if (symbol.type == TYPE_SYM::DEFAULTTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (!(symbol.type == TYPE_SYM::COLON)) printPos(888888);
        node->addChild(new TerNode(symbol));
        nextSym();
        node->addChild(sentenceP());
    }
    else {
        printPos(2227222);
        addErrorMessage(symbol.line, 'p', "Ã»ÓÐdefaultÓï¾ä");
    }
    return node;
}

inline SynNode* Parser::readSenP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::READ_SEN, true);
    if (symbol.type == TYPE_SYM::SCANFTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(idenP());
            if (!(symbol.type == TYPE_SYM::RPARENT)) printPos(214746);
            node->addChild(new TerNode(symbol));
            nextSym(); 
        } else printPos(77242);
    } else printPos(998754);
    return node;
}

inline SynNode* Parser::writeSenP() {
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
                    node->addChild(expressionP());
                }
            } else {
                node->addChild(expressionP());
            }
            if (!(symbol.type == TYPE_SYM::RPARENT)) printPos(424249);
            node->addChild(new TerNode(symbol));
            nextSym(); 
        } else printPos(892648);
    }
    return node;
}

SynNode* Parser::returnSenP() {
    NonTerNode* node = new NonTerNode(TYPE_NTS::RETURN_SEN, true);
    if (symbol.type == TYPE_SYM::RETURNTK) {
        node->addChild(new TerNode(symbol));
        nextSym();
        if (symbol.type == TYPE_SYM::LPARENT) {
            node->addChild(new TerNode(symbol));
            nextSym();
            node->addChild(expressionP());
            if (!(symbol.type == TYPE_SYM::RPARENT)) printPos(1984);
            node->addChild(new TerNode(symbol));
            nextSym();
        }
    } else printPos(777333);
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